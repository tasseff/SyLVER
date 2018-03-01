/// \file
/// \copyright 2016- The Science and Technology Facilities Council (STFC)
/// \author    Florent Lopez

#pragma once

#include <vector>

#include "ssids/cpu/cpu_iface.hxx"
// #include "ssids/cpu/factor.hxx"
// #include "ssids/cpu/BuddyAllocator.hxx"
// #include "ssids/cpu/NumericNode.hxx"
// #include "ssids/cpu/ThreadStats.hxx"
#include "ssids/cpu/kernels/cholesky.hxx"
#include "ssids/cpu/kernels/ldlt_app.hxx"
// #include "ssids/cpu/kernels/assemble.hxx"
#include "ssids/cpu/Workspace.hxx"

#include "kernels/ldlt_app.hxx"
#include "BuddyAllocator.hxx"
// #include "Workspace.hxx"
// #include "SymbolicSNode.hxx"
#include "SymbolicTree.hxx"
#include "NumericFront.hxx"
// #include "kernels/assemble.hxx"
// #include "kernels/common.hxx"
#include "tasks.hxx"
#include "factor_indef.hxx"
#include "tasks_factor_indef.hxx"


#if defined(SPLDLT_USE_STARPU)
#include <starpu.h>
#include "StarPU/kernels.hxx"
#include "StarPU/factor_indef.hxx"
#endif

// profiling
// #include <chrono>

// using namespace spral::ssids::cpu;
// using namespace spldlt;

#if defined(SPLDLT_USE_STARPU)
// using namespace spldlt::starpu;
#endif

namespace spldlt {

   extern "C" void spldlt_print_debuginfo_c(void *akeep, void *fkeep, int p);
   // extern "C" void spldlt_get_contrib_c(void *akeep, void *fkeep, int p, void **child_contrib);
   // extern "C" void spldlt_get_contrib_c(void *akeep, void *fkeep, int p);

   template<typename T,
            size_t PAGE_SIZE,
            typename FactorAllocator,
            bool posdef>
   class NumericTree {
      typedef spldlt::BuddyAllocator<T,std::allocator<T>> PoolAllocator;
   public:

   public:
      // Delete copy constructors for safety re allocated memory
      NumericTree(const NumericTree&) =delete;
      NumericTree& operator=(const NumericTree&) =delete;
      
      NumericTree(
            void* fkeep, 
            SymbolicTree& symbolic_tree, 
            T *aval,
            void** child_contrib,
            struct spral::ssids::cpu::cpu_factor_options& options)
         : fkeep_(fkeep), symb_(symbolic_tree),
           factor_alloc_(symbolic_tree.get_factor_mem_est(1.0)),
           pool_alloc_(symbolic_tree.get_pool_size<T>())
      {
         // Blocking size
         int blksz = options.cpu_block_size;

         // Associate symbolic fronts to numeric ones; copy tree structure
         fronts_.reserve(symbolic_tree.nnodes_+1);
         for(int ni=0; ni<symb_.nnodes_+1; ++ni) {
            fronts_.emplace_back(symbolic_tree[ni], pool_alloc_, blksz);
            auto* fc = symbolic_tree[ni].first_child;
            fronts_[ni].first_child = fc ? &fronts_[fc->idx] : nullptr;
            auto* nc = symbolic_tree[ni].next_child;
            fronts_[ni].next_child = nc ? &fronts_[nc->idx] :  nullptr;
         }
         
         std::vector<spral::ssids::cpu::Workspace> workspaces;
         // std::vector<spldlt::Workspace> workspaces;
         int nworkers = 0;
#if defined(SPLDLT_USE_STARPU)
         nworkers = starpu_cpu_worker_get_count();
#endif
         printf("[NumericTree] blksz = %d, nworkers = %d\n", blksz, nworkers);
         
         spldlt::starpu::codelet_init<T, FactorAllocator, PoolAllocator>();
         if (!posdef) { 

            static const int INNER_BLOCK_SIZE = 32;

            // Init StarPU codelets
            spldlt::starpu::codelet_init_indef<T, INNER_BLOCK_SIZE, CopyBackup<T, PoolAllocator>, PoolAllocator>();
            spldlt::starpu::codelet_init_factor_indef<T, PoolAllocator>();
            
            // Prepare workspaces
            workspaces.reserve(nworkers);
            for(int i = 0; i < nworkers; ++i)
               workspaces.emplace_back(PAGE_SIZE);
         }

         auto start = std::chrono::high_resolution_clock::now();
         if (posdef) factor_mf_posdef(aval, child_contrib, options);
         else        factor_mf_indef(aval, child_contrib, workspaces, options);
         // factor_mf_posdef(aval, child_contrib, options);
         // factor_mf_indef(aval, child_contrib, options);
         auto end = std::chrono::high_resolution_clock::now();
         long ttotal = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
         printf("[NumericTree] Task submission: %e\n", 1e-9*ttotal);

#if defined(SPLDLT_USE_STARPU)
         starpu_task_wait_for_all();
#endif         
      }

      ////////////////////////////////////////////////////////////////////////////////   
      // factor_mf_indef
      void factor_mf_indef(
            T *aval, void** child_contrib, 
            std::vector<spral::ssids::cpu::Workspace> &workspaces,
            struct spral::ssids::cpu::cpu_factor_options& options) {

         printf("[factor_mf_indef] posdef = %d\n", posdef);
         // printf("[factor_mf_indef] nparts = %d\n", symb_.nparts_);
         int INIT_PRIO = 4;
         int ASSEMBLE_PRIO = 4;

         // Blocking size
         int blksz = options.cpu_block_size;

         // Register symbolic handles.
         for(int ni = 0; ni < symb_.nnodes_; ++ni) {
            // TODO move hdl registration to activate task
            starpu_void_data_register(&(symb_[ni].hdl)); // Node's symbolic handle
            starpu_void_data_register(&(fronts_[ni].contrib_hdl)); // Node's symbolic handle
         }

         // for(int p = 0; p < symb_.nparts_; ++p) {

         //    int root = symb_.part_[p+1]-2; // symb_.part_ is 1-indexed
         //    // printf("[factor_mf] part = %d, root = %d\n", p, root);

         //    // Check if current partition is a subtree
         //    if (symb_[root].exec_loc != -1) {

         //       factor_subtree_task(
         //             symb_.akeep_, fkeep_, symb_[root], aval, p, child_contrib, 
         //             &options);
         //    }
         // }
         for(int p = 0; p < symb_.nsubtrees_; ++p) {
            int root = symb_.subtrees_[p]-1; // subtrees is 1-indexed
            factor_subtree_task(
                  symb_.akeep_, fkeep_, symb_[root], aval, p, child_contrib, 
                  &options);
// #if defined(SPLDLT_USE_STARPU)
//             starpu_task_wait_for_all();
// #endif
         }
         // #if defined(SPLDLT_USE_STARPU)
         //          starpu_task_wait_for_all();
         // #endif         
         // Allocate mapping array
         // int *map = new int[symb_.n+1];

         for(int ni = 0; ni < symb_.nnodes_; ++ni) {
            
            SymbolicFront &sfront = symb_[ni];
            // Skip iteration if node is in a subtree
            if (sfront.exec_loc != -1) continue;

            // printf("[factor_mf_indef] ni = %d\n", ni);
            
            // Activate and init frontal matrix
            activate_init_front_task(
                  false, sfront, fronts_[ni], child_contrib, blksz, 
                  factor_alloc_, pool_alloc_, aval);
// #if defined(SPLDLT_USE_STARPU)
//             starpu_task_wait_for_all();
// #endif

            // assemble contributions from children fronts and subtreess
            // assemble(symb_.n, fronts_[ni], child_contrib, pool_alloc_, blksz);
            assemble_task(symb_.n, sfront, fronts_[ni], child_contrib, pool_alloc_, blksz);
// #if defined(SPLDLT_USE_STARPU)
//             starpu_task_wait_for_all();
// #endif

            // Compute factors and Schur complement
            // factor_front_posdef(sfront, fronts_[ni], options);

            // Compute factors
            // factor_front_indef_nocontrib(
            //       sfront, fronts_[ni], workspaces,  pool_alloc_, options);
            // factor_front_indef_nocontrib_task(
            //       fronts_[ni], workspaces,  pool_alloc_, options);
// #if defined(SPLDLT_USE_STARPU)
//             starpu_task_wait_for_all();
// #endif
            // printf("[factor_mf_indef] nelim = %d\n", fronts_[ni].nelim);
            // printf("[factor_mf_indef] ndelay_in = %d\n", fronts_[ni].ndelay_in);
            // printf("[factor_mf_indef] ndelay_out = %d\n", fronts_[ni].ndelay_out);

            // form contrib
            // form_contrib_front(sfront, fronts_[ni], blksz);
            // form_contrib_front_task(fronts_[ni], blksz);
// #if defined(SPLDLT_USE_STARPU)
//             starpu_task_wait_for_all();
// #endif

            factor_front_indef_task(
                  fronts_[ni], workspaces,  pool_alloc_, options);

// #if defined(SPLDLT_USE_STARPU)
//             starpu_task_wait_for_all();
// #endif

            // Assemble contributions from children nodes into non
            // fully-summed columns
            // assemble_contrib(fronts_[ni], child_contrib, blksz);
            assemble_contrib_task(fronts_[ni], child_contrib, blksz);
// #if defined(SPLDLT_USE_STARPU)
//             starpu_task_wait_for_all();
// #endif
            
            // Deactivate children fronts
            for (auto* child=fronts_[ni].first_child; child!=NULL; child=child->next_child) {

               SymbolicFront &csnode = child->symb;

               if (csnode.exec_loc == -1) {
                  // fini_node(*child);
                  fini_node_task(*child, blksz, INIT_PRIO);
                  //#if defined(SPLDLT_USE_STARPU)
                  // TODO put in fini_node kernel
                  // unregister_node_submit(csnode, *child, blksz);
                  //#endif
               }
#if defined(SPLDLT_USE_STARPU)
               // Unregister symbolic handle on child node
               starpu_data_unregister_submit(csnode.hdl);
#endif
            } // Loop over child nodes

// #if defined(SPLDLT_USE_STARPU)
//             starpu_task_wait_for_all();
// #endif

         }
      }

      ////////////////////////////////////////////////////////////////////////////////   
      // factor_mf_posdef
      void factor_mf_posdef(
            T *aval, void** child_contrib, 
            struct spral::ssids::cpu::cpu_factor_options const& options) {

         // printf("[factor_mf_posdef] nparts = %d\n", symb_.nparts_);
         int INIT_PRIO = 4;
         int ASSEMBLE_PRIO = 4;

         // Blocking size
         int blksz = options.cpu_block_size;

         // Register symbolic handles on nodes
         for(int ni = 0; ni < symb_.nnodes_; ++ni) {
            // TODO move hdl registration to activate task
            starpu_void_data_register(&(symb_[ni].hdl));
         }
         
         // for(int p = 0; p < symb_.nparts_; ++p) {

         //    int root = symb_.part_[p+1]-2; // Part is 1-indexed
         //    // printf("[factor_mf] part = %d, root = %d\n", p, root);

         //    // Check if current partition is a subtree
         //    if (symb_[root].exec_loc != -1) {

         //       factor_subtree_task(
         //             symb_.akeep_, fkeep_, symb_[root], aval, p, child_contrib, 
         //             &options);
         //    }
         // }
         for(int p = 0; p < symb_.nsubtrees_; ++p) {
            int root = symb_.subtrees_[p]-1; // subtrees is 1-indexed
            printf("[factor_mf] nsubtrees = %d, p = %d, root = %d\n", symb_.nsubtrees_, p, root);
            factor_subtree_task(
                  symb_.akeep_, fkeep_, symb_[root], aval, p, child_contrib, 
                  &options);
         }

         
// #if defined(SPLDLT_USE_STARPU)
//          starpu_task_wait_for_all();
// #endif         

         // for(int p = 0; p < symb_.nparts_; ++p) {
         //    spldlt_print_debuginfo_c(symb_.akeep_, fkeep_, p);
         // }

         // Allocate mapping array
         int *map = new int[symb_.n+1];

         // Loop over node in the assemnly tree
         for(int ni = 0; ni < symb_.nnodes_; ++ni) {
            
            SymbolicFront &sfront = symb_[ni];
            
            // Skip iteration if node is in a subtree
            if (sfront.exec_loc != -1) continue;
            
            // Activate frontal matrix
            activate_front(
                  posdef, sfront, fronts_[ni], child_contrib, blksz, 
                  factor_alloc_, pool_alloc_);

            // Initialize frontal matrix 
            // init_node(sfront, fronts_[ni], aval); // debug
            init_node_task(sfront, fronts_[ni], aval, INIT_PRIO);
// #if defined(SPLDLT_USE_STARPU)
//             starpu_task_wait_for_all();
// #endif

            // build lookup vector, allowing for insertion of delayed vars
            // Note that while rlist[] is 1-indexed this is fine so long as lookup
            // is also 1-indexed (which it is as it is another node's rlist[]
            for(int i=0; i<sfront.ncol; i++)
               map[ sfront.rlist[i] ] = i;
            for(int i=sfront.ncol; i<sfront.nrow; i++)
               map[ sfront.rlist[i] ] = i + fronts_[ni].ndelay_in;

            // Assemble front: fully-summed columns 
            for (auto* child=fronts_[ni].first_child; child!=NULL; child=child->next_child) {
           
               SymbolicFront &child_sfront = symb_[child->symb.idx]; // Children symbolic node

               int ldcontrib = child_sfront.nrow - child_sfront.ncol;
               // Handle expected contributions (only if something there)
               if (ldcontrib>0) {

                  int cm = child_sfront.nrow - child_sfront.ncol;
                  // int* cache = work.get_ptr<int>(cm); // TODO move cache array
                  // Compute column mapping from child front into parent 
                  child_sfront.map = new int[cm];
                  for (int i=0; i<cm; i++)
                     child_sfront.map[i] = map[ child_sfront.rlist[child_sfront.ncol+i] ];

                  // Skip iteration if child node is in a subtree
                  if (child_sfront.exec_loc != -1) {

                     // Assemble contribution block from subtrees into
                     // fully-summed coefficients
                     assemble_subtree_task(
                           sfront, fronts_[ni], child_sfront, child_contrib, 
                           child_sfront.contrib_idx, child_sfront.map, blksz, 
                           ASSEMBLE_PRIO);
// #if defined(SPLDLT_USE_STARPU)
//                      starpu_task_wait_for_all();
// #endif

                  }
                  else {

                     int csa = child_sfront.ncol / blksz;
                     int cnr = (child_sfront.nrow-1) / blksz + 1; // number of block rows
                     // Loop over blocks in contribution blocks
                     for (int jj = csa; jj < cnr; ++jj) {
                        for (int ii = jj; ii < cnr; ++ii) {
                           // assemble_block(nodes_[ni], *child, ii, jj, csnode.map, blksz);
                           assemble_block_task(
                                 sfront, fronts_[ni], child_sfront, *child, ii, jj, 
                                 child_sfront.map, blksz, ASSEMBLE_PRIO);
// #if defined(SPLDLT_USE_STARPU)
//                            starpu_task_wait_for_all();
// #endif
                        }
                     }
                  }
               }
            } // Loop over child nodes
// #if defined(SPLDLT_USE_STARPU)
//                            starpu_task_wait_for_all();
// #endif

            // Compute factors and Schur complement 
            factor_front_posdef(sfront, fronts_[ni], options);
// #if defined(SPLDLT_USE_STARPU)
//             starpu_task_wait_for_all();
// #endif

            // Assemble front: non fully-summed columns i.e. contribution block 
            for (auto* child=fronts_[ni].first_child; child!=NULL; child=child->next_child) {
               
               // SymbolicNode const& csnode = child->symb;
               SymbolicFront &child_sfront = symb_[child->symb.idx];
               
               int ldcontrib = child_sfront.nrow - child_sfront.ncol;
               // Handle expected contributions (only if something there)
               // if (child->contrib) {
               if (ldcontrib>0) {
                  // Skip iteration if child node is in a subtree
                  if (child_sfront.exec_loc != -1) {
                     
                     // Assemble contribution block from subtrees into non
                     // fully-summed coefficients
                     assemble_contrib_subtree_task(
                           sfront, fronts_[ni], child_sfront, child_contrib, 
                           child_sfront.contrib_idx, child_sfront.map, blksz, 
                           ASSEMBLE_PRIO);
// #if defined(SPLDLT_USE_STARPU)
//                      starpu_task_wait_for_all();
// #endif

                  }
                  else {
                     
                     // int cm = csnode.nrow - csnode.ncol;
                     // int* cache = work.get_ptr<int>(cm); // TODO move cache array
                     // for (int i=0; i<cm; i++)
                     // csnode.map[i] = map[ csnode.rlist[csnode.ncol+i] ];

                     int csa = child_sfront.ncol / blksz;
                     // Number of block rows in child node
                     int cnr = (child_sfront.nrow-1) / blksz + 1; 
                     // int cnc = (csnode.ncol-1) / blksz + 1; // number of block columns in child node
                     // Lopp over blocks in contribution blocks
                     for (int jj = csa; jj < cnr; ++jj) {                     
                        // int c_sa = (csnode.ncol > jj*blksz) ? 0 : (jj*blksz-csnode.ncol); // first col in block
                        // int c_en = std::min((jj+1)*blksz-csnode.ncol, cm); // last col in block
                        // assemble_expected_contrib(c_sa, c_en, nodes_[ni], *child, map, cache);
                        for (int ii = jj; ii < cnr; ++ii) {
                           // assemble_contrib_block(nodes_[ni], *child, ii, jj, csnode.map, blksz)
                           assemble_contrib_block_task(
                                 sfront, fronts_[ni], child_sfront, *child, ii, jj, 
                                 child_sfront.map, blksz, ASSEMBLE_PRIO);
// #if defined(SPLDLT_USE_STARPU)
//                            starpu_task_wait_for_all();
// #endif

                        }
                     }
                  }
               }
// #if defined(SPLDLT_USE_STARPU)
//                      starpu_task_wait_for_all();
// #endif

               if (child_sfront.exec_loc == -1) {
                  // fini_node(*child);
                  fini_node_task(*child, blksz, INIT_PRIO);
// #if defined(SPLDLT_USE_STARPU)
//                   starpu_task_wait_for_all();
// #endif

// #if defined(SPLDLT_USE_STARPU)
//                   unregister_node_submit(child_sfront, *child, blksz);
// #endif
               }
#if defined(SPLDLT_USE_STARPU)
               // Unregister symbolic handle on child node
               starpu_data_unregister_submit(child_sfront.hdl);
#endif


            } // Loop over child nodes

// #if defined(SPLDLT_USE_STARPU)
//             starpu_task_wait_for_all();
// #endif
         } // Loop over nodes in the assemnly tree  
      }

      void solve_fwd(int nrhs, double* x, int ldx) const {

         // printf("[NumericTree] solve fwd, nrhs: %d\n", nrhs);
         // for (int i = 0; i < ldx; ++i) printf(" %10.4f", x[i]);
         // printf("[NumericTree] solve fwd, posdef: %d\n", posdef);

         /* Allocate memory */
         double* xlocal = new double[nrhs*symb_.n];
         int* map_alloc = (!posdef) ? new int[symb_.n] : nullptr; // only indef
        
         /* Main loop */
         for(int ni=0; ni<symb_.nnodes_; ++ni) {

            // Skip iteration if node is in a subtree
            if (symb_[ni].exec_loc != -1) continue;

            int m = symb_[ni].nrow;
            int n = symb_[ni].ncol;
            int nelim = (posdef) ? n
               : fronts_[ni].nelim;
            int ndin = (posdef) ? 0
               : fronts_[ni].ndelay_in;
            int ldl = align_lda<T>(m+ndin);
            // printf("[NumericTree] solve fwd, node: %d, nelim: %d, ldl: %d\n", ni, nelim, ldl);
            /* Build map (indef only) */
            int const *map;
            if(!posdef) {
               // indef need to allow for permutation and/or delays
               for(int i=0; i<n+ndin; ++i)
                  map_alloc[i] = fronts_[ni].perm[i];
               for(int i=n; i<m; ++i)
                  map_alloc[i+ndin] = symb_[ni].rlist[i];
               map = map_alloc;
            } else {
               // posdef there is no permutation
               map = symb_[ni].rlist;
            }

            /* Gather into dense vector xlocal */
            // FIXME: don't bother copying elements of x > m, just use beta=0
            //        in dgemm call and then add as we scatter
            for(int r=0; r<nrhs; ++r)
               for(int i=0; i<m+ndin; ++i)
                  xlocal[r*symb_.n+i] = x[r*ldx + map[i]-1]; // Fortran indexed

            /* Perform dense solve */
            if(posdef) {
               spral::ssids::cpu::cholesky_solve_fwd(
                     m, n, fronts_[ni].lcol, ldl, nrhs, xlocal, symb_.n);
            } else { /* indef */
               spral::ssids::cpu::ldlt_app_solve_fwd(
                     m+ndin, nelim, fronts_[ni].lcol, ldl, nrhs, xlocal, symb_.n);
            }

            // tests/debug
            // spral::ssids::cpu::cholesky_solve_fwd(
            //       m+ndin, nelim, fronts_[ni].lcol, ldl, nrhs, xlocal, symb_.n);


            /* Scatter result */
            for(int r=0; r<nrhs; ++r)
               for(int i=0; i<m+ndin; ++i)
                  x[r*ldx + map[i]-1] = xlocal[r*symb_.n+i];
         }

         /* Cleanup memory */
         if(!posdef) delete[] map_alloc; // only used in indef case
         delete[] xlocal;
      }

      template <bool do_diag, bool do_bwd>
      void solve_diag_bwd_inner(int nrhs, double* x, int ldx) const {
         if(posdef && !do_bwd) return; // diagonal solve is a no-op for posdef

         // printf("[solve_diag_bwd_inner] do_diag = %d, do_bwd = %d\n", do_diag, do_bwd);

         /* Allocate memory - map only needed for indef bwd/diag_bwd solve */
         double* xlocal = new double[nrhs*symb_.n];
         int* map_alloc = (!posdef && do_bwd) ? new int[symb_.n]
            : nullptr;

         /* Perform solve */
         for(int ni=symb_.nnodes_-1; ni>=0; --ni) {
            
            // Skip iteration if node is in a subtree
            if (symb_[ni].exec_loc != -1) continue;

            int m = symb_[ni].nrow;
            int n = symb_[ni].ncol;
            int nelim = (posdef) ? n
               : fronts_[ni].nelim;
            int ndin = (posdef) ? 0
               : fronts_[ni].ndelay_in;

            /* Build map (indef only) */
            int const *map;
            if(!posdef) {
               // indef need to allow for permutation and/or delays
               if(do_bwd) {
                  for(int i=0; i<n+ndin; ++i)
                     map_alloc[i] = fronts_[ni].perm[i];
                  for(int i=n; i<m; ++i)
                     map_alloc[i+ndin] = symb_[ni].rlist[i];
                  map = map_alloc;
               } else { // if only doing diagonal, only need first nelim<=n+ndin
                  map = fronts_[ni].perm;
               }
            } else {
               // posdef there is no permutation
               map = symb_[ni].rlist;
            }

            /* Gather into dense vector xlocal */
            int blkm = (do_bwd) ? m+ndin
               : nelim;
            int ldl = align_lda<T>(m+ndin);
            for(int r=0; r<nrhs; ++r)
               for(int i=0; i<blkm; ++i)
                  xlocal[r*symb_.n+i] = x[r*ldx + map[i]-1];

            /* Perform dense solve */
            if(posdef) {
               spral::ssids::cpu::cholesky_solve_bwd(
                     m, n, fronts_[ni].lcol, ldl, nrhs, xlocal, symb_.n);
            } else {
               if(do_diag) spral::ssids::cpu::ldlt_app_solve_diag(
                     nelim, &fronts_[ni].lcol[(n+ndin)*ldl], nrhs, xlocal, symb_.n
                     );
               if(do_bwd) spral::ssids::cpu::ldlt_app_solve_bwd(
                     m+ndin, nelim, fronts_[ni].lcol, ldl, nrhs, xlocal, symb_.n
                     );
            }

            // tests/debug
            // spral::ssids::cpu::cholesky_solve_bwd(
            //       m+ndin, nelim, fronts_[ni].lcol, ldl, nrhs, xlocal, symb_.n);

            /* Scatter result (only first nelim entries have changed) */
            for(int r=0; r<nrhs; ++r)
               for(int i=0; i<nelim; ++i)
                  x[r*ldx + map[i]-1] = xlocal[r*symb_.n+i];
         }

         /* Cleanup memory */
         if(!posdef && do_bwd) delete[] map_alloc; // only used in indef case
         delete[] xlocal;
      }

      void solve_diag(int nrhs, double* x, int ldx) const {
         solve_diag_bwd_inner<true, false>(nrhs, x, ldx);
      }

      void solve_diag_bwd(int nrhs, double* x, int ldx) const {
         solve_diag_bwd_inner<true, true>(nrhs, x, ldx);
      }

      void solve_bwd(int nrhs, double* x, int ldx) const {
         solve_diag_bwd_inner<false, true>(nrhs, x, ldx);
      }

      private:
         void* fkeep_;
         SymbolicTree& symb_;
         std::vector<spldlt::NumericFront<T,PoolAllocator>> fronts_;
         FactorAllocator factor_alloc_;
         PoolAllocator pool_alloc_;
      };
         
} /* end of namespace spldlt */
