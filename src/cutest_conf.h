#ifndef _CUTEST_CONF_H_
#define _CUTEST_CONF_H_

typedef enum cc_std_e {
  STD_GCC_C90 = 0,
  STD_GCC_C99 = 1,
  STD_GCC_C11 = 2,
  STD_NONE
} cc_std_t;

typedef enum cc_capability_e {
  CC_GCC_PEDANTIC = 0,
  CC_GCC_VISIBILITY_HIDDEN = 1,
  CC_GCC_TEST_COVERAGE = 2,
  CC_GCC_EXTRA = 3,
  CC_GCC_NO_PRAGMAS = 4,
  CC_GCC_LTO = 5,
  CC_GCC_VARIADIC_MACROS = 6,
  CC_NONE
} cc_capability_t;

typedef struct cc_capabilities_s {
  int pedantic;
  int visibility_hidden;
  int test_coverage;
  int extra;
  int no_pragmas;
  int lto;
  int variadic_macros;
} cc_capabilities_t;

#endif
