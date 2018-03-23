#include "testing_form_contrib.hxx"

namespace spldlt { namespace tests {

   int run_form_contrib_tests() {
      
      int nerr = 0;
      
      // TEST(( form_contrib_test<double>(0.01, 1e-20, true, 64, 32, 0, 31, 32) ));
      TEST(( form_contrib_test<double>(0.01, 1e-20, true, 64, 32, 10, 20, 32) ));
      
      return nerr;
   }

}} // end of namespace spldlt::tests