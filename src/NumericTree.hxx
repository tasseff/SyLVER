#pragma once

#include "ssids/cpu/cpu_iface.hxx"
#include "ssids/cpu/Workspace.hxx"
#include "ssids/cpu/factor.hxx"
#include "ssids/cpu/BuddyAllocator.hxx"
#include "ssids/cpu/NumericNode.hxx"
#include "ssids/cpu/ThreadStats.hxx"
#include "ssids/cpu/kernels/cholesky.hxx"

#include "SymbolicSNode.hxx"
#include "SymbolicTree.hxx"
#include "kernels/assemble.hxx"
#include "kernels/common.hxx"
#include "factorize.hxx"

using namespace spral::ssids::cpu;

namespace spldlt {

   template<typename T,
            size_t PAGE_SIZE,
            typename FactorAllocator,
            bool posdef> //< true for Cholesky factoriztion, false for indefinte
   class NumericTree {
      typedef BuddyAllocator<T,std::allocator<T>> PoolAllocator;
   public:
      /* Delete copy constructors for safety re allocated memory */
      NumericTree(const NumericTree&) =delete;
      NumericTree& operator=(const NumericTree&) =delete;

      NumericTree(SymbolicTree const& symbolic_tree, T const* aval, 
                  struct cpu_factor_options const& options)
         : symb_(symbolic_tree), 
           factor_alloc_(symbolic_tree.get_factor_mem_est(1.0)),
           pool_alloc_(symbolic_tree.get_pool_size<T>())
      {
         // printf("[NumericTree] block size: %d\n",  options.cpu_task_block_size);
         /* Associate symbolic nodes to numeric ones; copy tree structure */
         nodes_.reserve(symbolic_tree.nnodes_+1);
         for(int ni=0; ni<symb_.nnodes_+1; ++ni) {
            nodes_.emplace_back(symbolic_tree[ni], pool_alloc_);
            auto* fc = symbolic_tree[ni].first_child;
            nodes_[ni].first_child = fc ? &nodes_[fc->idx] : nullptr;
            auto* nc = symbolic_tree[ni].next_child;
            nodes_[ni].next_child = nc ? &nodes_[nc->idx] :  nullptr;
         }

         /* blocking size */
         int nb = options.cpu_task_block_size;

         /* Allocate workspace */
         // Workspace *work = new Workspace(PAGE_SIZE);
         Workspace work(PAGE_SIZE);
         Workspace colmap(PAGE_SIZE);
         Workspace rowmap(PAGE_SIZE);
         
         // printf("[NumericTree] nnodes: %d\n", symb_.nnodes_);

         // Initialize nodes because right-looking update
         for(int ni = 0; ni < symb_.nnodes_; ++ni)
            init_node(symb_[ni], nodes_[ni], factor_alloc_, pool_alloc_, 
                      // work,
                      aval);

         /* Loop over singleton nodes in order */
         for(int ni = 0; ni < symb_.nnodes_; ++ni) {

            SymbolicSNode const& snode = symb_[ni];
            
            /* Extract useful information about node */
            int m = snode.nrow;
            int n = snode.ncol;
            int ldl = align_lda<T>(m);
            T *lcol = nodes_[ni].lcol;
            int nc = (n-1)/nb +1; // number of block column
            
            // Factorize node
            factorize_node_posdef(snode, nodes_[ni], options);

            // Apply factorization operation to ancestors
            apply_node(snode, nodes_[ni],
                       symb_.nnodes_, symb_, nodes_,
                       nb, work, rowmap, colmap);
            
         }
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
            int m = symb_[ni].nrow;
            int n = symb_[ni].ncol;
            int nelim = (posdef) ? n
               : nodes_[ni].nelim;
            int ndin = (posdef) ? 0
               : nodes_[ni].ndelay_in;
            int ldl = align_lda<T>(m+ndin);
            // printf("[NumericTree] solve fwd, node: %d, nelim: %d, ldl: %d\n", ni, nelim, ldl);
            /* Build map (indef only) */
            int const *map;
            if(!posdef) {
               // indef need to allow for permutation and/or delays
               for(int i=0; i<n+ndin; ++i)
                  map_alloc[i] = nodes_[ni].perm[i];
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
               cholesky_solve_fwd(m, n, nodes_[ni].lcol, ldl, nrhs, xlocal, symb_.n);
            } else { /* indef */
               ldlt_app_solve_fwd(m+ndin, nelim, nodes_[ni].lcol, ldl, nrhs,
                                  xlocal, symb_.n);
            }

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

         /* Allocate memory - map only needed for indef bwd/diag_bwd solve */
         double* xlocal = new double[nrhs*symb_.n];
         int* map_alloc = (!posdef && do_bwd) ? new int[symb_.n]
            : nullptr;

         /* Perform solve */
         for(int ni=symb_.nnodes_-1; ni>=0; --ni) {
            int m = symb_[ni].nrow;
            int n = symb_[ni].ncol;
            int nelim = (posdef) ? n
               : nodes_[ni].nelim;
            int ndin = (posdef) ? 0
               : nodes_[ni].ndelay_in;

            /* Build map (indef only) */
            int const *map;
            if(!posdef) {
               // indef need to allow for permutation and/or delays
               if(do_bwd) {
                  for(int i=0; i<n+ndin; ++i)
                     map_alloc[i] = nodes_[ni].perm[i];
                  for(int i=n; i<m; ++i)
                     map_alloc[i+ndin] = symb_[ni].rlist[i];
                  map = map_alloc;
               } else { // if only doing diagonal, only need first nelim<=n+ndin
                  map = nodes_[ni].perm;
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
               cholesky_solve_bwd(m, n, nodes_[ni].lcol, ldl, nrhs, xlocal, symb_.n);
            } else {
               if(do_diag) ldlt_app_solve_diag(
                     nelim, &nodes_[ni].lcol[(n+ndin)*ldl], nrhs, xlocal, symb_.n
                     );
               if(do_bwd) ldlt_app_solve_bwd(
                     m+ndin, nelim, nodes_[ni].lcol, ldl, nrhs, xlocal, symb_.n
                     );
            }

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
      SymbolicTree const& symb_;
      FactorAllocator factor_alloc_;
      PoolAllocator pool_alloc_;
      std::vector<NumericNode<T,PoolAllocator>> nodes_;
   };

} /* end of namespace spldlt */
