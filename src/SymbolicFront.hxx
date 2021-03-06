#pragma once

#include "ssids/cpu/SymbolicNode.hxx"

#if defined(SPLDLT_USE_STARPU)
#include <starpu.h>
#endif

// using namespace spral::ssids::cpu;

namespace sylver {

   struct SymbolicFront : spral::ssids::cpu::SymbolicNode {
      // int sa; // index of first column in node (pivotal order)
      // int en; // index of last column in node (pivotal order)
      int least_desc; // least descendants, to allow easy walk of subtrees
#if defined(SPLDLT_USE_STARPU)
      starpu_data_handle_t hdl; // symbolic handle representing the node
      std::vector<starpu_data_handle_t> handles; // array containing the block handles
      // std::vector<starpu_data_handle_t> contrib_handles; // array containing the block handles in the contribution block
#endif 
      int *map; // Array mapping rows and columns index with the parent front  
      int part; // Partition info
      int exec_loc; // Execution location info
      int contrib_idx; // Index of contribution
      bool is_in_subtree() const {
         return (exec_loc != -1);
      }
   };
}
