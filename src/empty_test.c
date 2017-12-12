#ifdef CUTEST_LENIENT_ASSERTS
#undef CUTEST_LENIENT_ASSERTS
#endif

#include "cutest.h"

#define main MAIN

test(main_shall_return_0)
{
  assert_eq(0, main(0, 0));
}
