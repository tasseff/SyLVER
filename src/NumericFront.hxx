/// \file
/// \copyright 2016- The Science and Technology Facilities Council (STFC)
/// \author    Jonathan Hogg
/// \author    Florent Lopez

#pragma once

// SpLDLT
#include "SymbolicFront.hxx"
#include "kernels/ldlt_app.hxx"

// SSIDS
#include "ssids/cpu/cpu_iface.hxx"

namespace spldlt {

   // static const int INNER_BLOCK_SIZE = 32;
   
   template<typename T, typename PoolAllocator>
   class Tile {
      typedef std::allocator_traits<PoolAllocator> PATraits;
   public:

      /// \brief Constuctor.
      // Tile()
      //    : m_(0), n_(0), lda_(0), a_(nullptr)
      // {}

      /// \brief Constuctor.
      /// \param i Tile's row index.
      /// \param j Tile's column index.
      /// \param m Number of rows in matrix.
      /// \param n Number of columns in matrix.
      /// \param a Pointer to matrix data.
      /// \param lda leading dimension associated with data.
      // Tile(int i, int j, int m, int n, T* a, int lda)
      //    : i_(i), j_(j), m_(m), n_(n), lda_(lda), a_(a)
      // {}

      /// \brief Constuctor.
      /// \param i Tile's row index.
      /// \param j Tile's column index.
      /// \param m Number of rows in matrix.
      /// \param n Number of columns in matrix.
      Tile(int i, int j, int m, int n, int lda, 
            PoolAllocator const& pool_alloc)
         : i(i), j(j), m(m), n(n), lda(lda), a(nullptr),
           pool_alloc_(pool_alloc)
      {}
      
      /// \brief Descructor.
      ~Tile() {
         free();
      }

      void alloc() {
         size_t block_sz = lda*n;
         a = (block_sz>0) ? PATraits::allocate(pool_alloc_, block_sz)
            : nullptr;
      }

      void free() {
         if(!a) return;
         size_t block_sz = lda*n;
         PATraits::deallocate(pool_alloc_, a, block_sz);
         a = nullptr;
      }

      /// @brief Zero block
      void zero() {
         if(!a) return;
         for (int j = 0; j < n; j++) {
            for (int i = 0; i < m; i++) {
               a[j*lda+i] = 0.0;
            }
         }
      }

#if defined(SPLDLT_USE_STARPU)
      // Register handle on block
      void register_handle() {
         // Make sure pointer in allocated
         if (!a) return;
         // Register block in StarPU
         starpu_matrix_data_register(
               &hdl, STARPU_MAIN_RAM, reinterpret_cast<uintptr_t>(a),
               lda, m, n, sizeof(T));
      }

      // Unregister handle on block asynchronously
      void unregister_handle_submit() {
         starpu_data_unregister_submit(hdl);
      }
#endif

      int i; ///< block's row
      int j; ///< block's column
      int m; ///< number of rows in matrix
      int n; ///< number of columns in matrix
      int lda; ///< leading dimension of underlying storage
      T* a; ///< pointer to underlying matrix storage  
#if defined(SPLDLT_USE_STARPU)
      starpu_data_handle_t hdl;
#endif
   private:
      PoolAllocator pool_alloc_; // Our own version of pool allocator for freeing
   };

   // class spral::ssids::cpu::SymbolicNode;

   template <typename T, typename PoolAllocator>
   class NumericFront {
      typedef std::allocator_traits<PoolAllocator> PATraits;
      typedef typename std::allocator_traits<PoolAllocator>::template rebind_alloc<int> IntAlloc;
      typedef spldlt::ldlt_app_internal::Block<T, INNER_BLOCK_SIZE, IntAlloc> BlockSpec;
   public:
      /**
       * \brief Constructor
       * \param symb Associated symbolic node.
       * \param pool_alloc Pool Allocator to use for contrib allocation.
       */
      NumericFront(
            SymbolicFront& symb,
            PoolAllocator const& pool_alloc, int blksz)
         : symb(symb), contrib(nullptr), pool_alloc_(pool_alloc), blksz(blksz),
           backup(nullptr), cdata(nullptr), ndelay_in(0), ndelay_out(0),
           lcol(nullptr), nelim1(0), nelim(0)
      {}
      
      /**
       * \brief Destructor
       */
      ~NumericFront() {
         free_contrib();
         free_contrib_blocks();
         free_backup();
         free_cdata();
      }

      /// \brief Allocate block structures and memory space for
      /// contrib (below diagonal)
      void alloc_contrib_blocks() {

         int m = get_nrow();
         int n = get_ncol();
         size_t contrib_dimn = m-n; // Dimension of contribution block
         if (contrib_dimn>0) {
            int nr = (m-1) / blksz + 1; // number of block rows in front amtrix
            int rsa = n / blksz; // index of first block in contribution blocks  
            int ncontrib = nr-rsa;

            contrib_blocks.reserve(ncontrib*ncontrib);
            for(int j = rsa; j < nr; j++) {
               // First col in contrib block
               int first_col = std::max(j*blksz, n);
               // Tile width
               int blkn = std::min((j+1)*blksz, m) - first_col;
               for(int i = rsa; i < nr; i++) {
                  // First col in contrib block
                  int first_row = std::max(i*blksz, n);
                  // Tile height
                  int blkm = std::min((i+1)*blksz, m) - first_row;
                  contrib_blocks.emplace_back(i-rsa, j-rsa, blkm, blkn, blkm, pool_alloc_);
               }
            }
   
            for(int j = 0; j < ncontrib; j++) {
               for(int i = j; i < ncontrib; i++) {
                  // printf("[alloc_contrib_blocks] i: %d, j: %d\n", i, j);
                  contrib_blocks[j*ncontrib+i].alloc();
               }
            }
         }
      }

      void free_contrib_blocks() {
         int m = get_nrow();
         int n = get_ncol();            
         size_t contrib_dimn = m-n; // Dimension of contribution block
         if (contrib_dimn>0 && contrib_blocks.size()>0) {
            int nr = get_nr(); // number of block rows in front amtrix
            int rsa = n / blksz; // index of first block in contribution blocks  
            int ncontrib = nr-rsa;
            for(int j = 0; j < ncontrib; j++) {
               for(int i = j; i < ncontrib; i++) {
                  contrib_blocks[j*ncontrib+i].free();
               }
            }
         }
      }

      /// @breif zero contribution blocks
      /// 
      /// Note: Mainly used for testing purpose as we avoid explictily
      /// zeroing
      void zero_contrib_blocks() {

         int m = get_nrow();
         int n = get_ncol();            
         size_t contrib_dimn = m-n; // Dimension of contribution block
         if (contrib_dimn>0 && contrib_blocks.size()>0) {
            int nr = get_nr(); // number of block rows in front amtrix
            int rsa = n / blksz; // index of first block in contribution blocks  
            int ncontrib = nr-rsa;
            for(int j = 0; j < ncontrib; j++) {
               for(int i = j; i < ncontrib; i++) {
                  contrib_blocks[j*ncontrib+i].zero();
               }
            }            
         }
      }

      /**
       * \brief Allocate space for contribution block.
       *
       * Note done at construction time, as a major memory commitment that is
       * transitory.
       */
      void alloc_contrib() {
         size_t contrib_dimn = symb.nrow - symb.ncol;
         contrib_dimn = contrib_dimn*contrib_dimn;
         contrib = (contrib_dimn>0) ? PATraits::allocate(pool_alloc_, contrib_dimn)
            : nullptr;
      }

      /** \brief Free space for contribution block (if allocated) */
      void free_contrib() {
         if(!contrib) return;
         size_t contrib_dimn = symb.nrow - symb.ncol;
         contrib_dimn = contrib_dimn*contrib_dimn;
         PATraits::deallocate(pool_alloc_, contrib, contrib_dimn);
         contrib = nullptr;
      }

      /// @brief Allocate backups to prepare for the LDL^T
      /// facorization using APTP strategy
      void alloc_backup() {
         backup = 
            new spldlt::ldlt_app_internal::CopyBackup<T, PoolAllocator>(
                  get_nrow(), get_ncol(), blksz, pool_alloc_);
      }
      
      void free_backup() {
         if(!backup) return;
         delete backup;
         backup = nullptr;
      }
      
      void alloc_cdata() {
         cdata = 
            new spldlt::ldlt_app_internal::ColumnData<T, IntAlloc> (
                  get_ncol(), blksz, IntAlloc(pool_alloc_));
      }
      
      void free_cdata() {
         if (!cdata) return;
         delete cdata;
         cdata = nullptr;
      }

      /// @brief Allocate blocks (structre only)
      /// Note: factor data must be allocated
      /// Note: cdata must be allocated and intialized
      void alloc_blocks() {

         int const m = get_nrow();
         int const n = get_ncol();
         int const ldl = get_ldl();
         int const mblk = get_nr();
         int const nblk = get_nc();
         int const num_blocks = nblk*mblk;
         blocks.reserve(num_blocks);

         for(int jblk=0; jblk<nblk; jblk++) {
            for(int iblk=0; iblk<mblk; iblk++) {
               // Create and insert block at the end (column-wise storage)
               blocks.emplace_back(iblk, jblk, m, n, *cdata, &lcol[jblk*blksz*ldl+iblk*blksz], ldl, blksz);
               // alternativel store pointer
               // blocks[jblk*mblk + iblk] = new BlockSpec(iblk, jblk, m, n, cdata, &a[jblk*block_size*lda+iblk*block_size], lda, block_size);
#if defined(SPLDLT_USE_STARPU)
               // register handle for block (iblk, jblk)
               if (iblk >= jblk) blocks[jblk*mblk+iblk].register_handle(); 
#endif
            }
         }
      }
      
      /// @brief Return the number of rows in the node
      inline int get_nrow() const {
         return symb.nrow + ndelay_in;
      }

      /// @brief Return the number of columns in the node
      inline int get_ncol() const {
         return symb.ncol + ndelay_in;
      }

      /// @brief Return the number of block rows in the node
      inline int get_nr() const {
         return (get_nrow()-1) / blksz + 1;
      }

      /// @brief Return the number of block rows in the node
      inline int get_nc() const {
         return (get_ncol()-1) / blksz + 1;
      }

      /** \brief Return leading dimension of node's lcol member. */
      inline size_t get_ldl() const {
         return spral::ssids::cpu::align_lda<T>(symb.nrow + ndelay_in);
      }

#if defined(SPLDLT_USE_STARPU)
      starpu_data_handle_t get_hdl() const {
         return symb.hdl;
      }
#endif
      
   public:
      /* Symbolic node associate with this one */
      SymbolicFront& symb;

      /* Fixed data from analyse */
      NumericFront<T, PoolAllocator>* first_child; // Pointer to our first child
      NumericFront<T, PoolAllocator>* next_child; // Pointer to parent's next child

      /* Data that changes during factorize */
      int ndelay_in; // Number of delays arising from children
      int ndelay_out; // Number of delays arising to push into parent
      int nelim1; // Number of columns succesfully eliminated during first pass
      int nelim; // Number of columns succesfully eliminated
      T *lcol; // Pointer to start of factor data
      int *perm; // Pointer to permutation
      T *contrib; // Pointer to contribution block
      int blksz; // Tileing size
      std::vector<spldlt::Tile<T, PoolAllocator>> contrib_blocks; // Tile structures containing contrib
      spldlt::ldlt_app_internal::CopyBackup<T, PoolAllocator> *backup; // Stores backups of matrix blocks
      // Structures for indef factor
      spldlt::ldlt_app_internal::ColumnData<T, IntAlloc> *cdata;
      std::vector<BlockSpec> blocks;
#if defined(SPLDLT_USE_STARPU)
      starpu_data_handle_t contrib_hdl; // Symbolic handle for contribution blocks
#endif
   private:
      PoolAllocator pool_alloc_; // Our own version of pool allocator for freeing
      // contrib
   };

} /* namespaces spldlt */
