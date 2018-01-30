#ifndef _CUTEST_MAKE_H_
#define _CUTEST_MAKE_H_

#define OS_CURRENT_DIRECTORY "."
#define OS_PATH_SEPARATOR "/"

#ifndef CUTEST_SRC_PATH
#warning "You did not provide CUTEST_SRC_PATH as define to your compiler, cutest_make will assume you have your source files for your product in the current directory"
#define CUTEST_SRC_PATH OS_CURRENT_DIRECTORY
#endif

#ifndef CUTEST_TST_PATH
#warning "You did not provide CUTEST_TST_PATH as define to your compiler, cutest_make will assume you have your test files for your product in the current directory"
#define CUTEST_TST_PATH OS_CURRENT_DIRECTORY
#endif

#ifndef CUTEST_PATH
#warning "You did not provide CUTEST_PATH as define to your compiler, cutest_make will assume you have your downloaded version of cutest in a folder called 'cutest' in the current directory"
#define CUTEST_PATH "cutest"
#endif

#ifndef CUTEST_CC_VISIBILITY_HIDDEN
#define CUTEST_CC_VISIBILITY_HIDDEN "-fvisibility=hidden"
#endif

#ifndef CUTEST_CC_LTO
#define CUTEST_CC_LTO "-flto"
#endif

#ifndef CUTEST_PROX_C
#define CUTEST_PROX_C CUTEST_PATH OS_PATH_SEPARATOR "cutest_prox.c"
#endif

#ifndef CUTEST_MOCK_C
#define CUTEST_MOCK_C CUTEST_PATH OS_PATH_SEPARATOR "cutest_mock.c"
#endif

#ifndef CUTEST_RUN_C
#define CUTEST_RUN_C CUTEST_PATH OS_PATH_SEPARATOR "cutest_run.c"
#endif

#ifndef HELPERS_C
#define HELPERS_C CUTEST_PATH OS_PATH_SEPARATOR "helpers.c"
#endif

#ifndef MOCKABLE_C
#define MOCKABLE_C CUTEST_PATH OS_PATH_SEPARATOR "mockable.c"
#endif

#ifndef ARG_C
#define ARG_C CUTEST_PATH OS_PATH_SEPARATOR "arg.c"
#endif

#ifndef TESTCASE_C
#define TESTCASE_C CUTEST_PATH OS_PATH_SEPARATOR "testcase.c"
#endif

#ifndef CUTEST_C
#define CUTEST_C CUTEST_PATH OS_PATH_SEPARATOR "cutest.c"
#endif

#ifndef CUTEST_H
#define CUTEST_H CUTEST_PATH OS_PATH_SEPARATOR "cutest.h"
#endif

#ifndef CPROTO
#define CPROTO CUTEST_PATH OS_PATH_SEPARATOR "cproto-4.7m" OS_PATH_SEPARATOR "cproto"
#endif

typedef struct file_s {
  char* file_name;
} file_t;

typedef struct file_node_s {
  file_t file;
  struct file_node_s* next;
  int skip;
} file_node_t;

typedef struct file_list_s {
  file_node_t* first;
  file_node_t* last;
} file_list_t;

#define ARTIFACT_FIELDS                         \
  char* str;                                    \
  unsigned long long int timestamp;             \
  int fake_main

/*
 * Base-class for some kind of artifact
 */
typedef struct artifact_s {
  ARTIFACT_FIELDS;
} artifact_t;

#define ARTIFACT(NAME)                          \
  typedef struct NAME##_s {                     \
    ARTIFACT_FIELDS;                            \
  } NAME##_t

/*
 * Provide 100% type safety when constructing the build-dependencies.
 */
ARTIFACT(mockables_lst);
ARTIFACT(mockables_s);
ARTIFACT(mockables_o);
ARTIFACT(proxified_s);
ARTIFACT(mocks_h);
ARTIFACT(test_run_c);
ARTIFACT(test_run);
ARTIFACT(test_c);
ARTIFACT(test);
ARTIFACT(c);
ARTIFACT(cproto);

ARTIFACT(helpers_c);
ARTIFACT(helpers_o);
ARTIFACT(mockable_c);
ARTIFACT(mockable_o);
ARTIFACT(arg_c);
ARTIFACT(arg_o);
ARTIFACT(testcase_c);
ARTIFACT(testcase_o);
ARTIFACT(cutest_h);
ARTIFACT(cutest_c);
ARTIFACT(cutest_o);
ARTIFACT(cutest_prox_c);
ARTIFACT(cutest_prox_o);
ARTIFACT(cutest_prox);
ARTIFACT(cutest_mock_c);
ARTIFACT(cutest_mock_o);
ARTIFACT(cutest_mock);
ARTIFACT(cutest_run_c);
ARTIFACT(cutest_run_o);
ARTIFACT(cutest_run);

#endif
