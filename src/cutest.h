/*********************************************************************
   ---------------    ____ ____ _____ ____ ____ _____ ---------------
   ---------------   / __// / //_  _// __// __//_  _/ ---------------
   ---------------  / /_ / / /  / / / __//_  /  / /   ---------------
   --------------- /___//___/  /_/ /___//___/  /_/    ---------------
 *
 * CUTest - The C Unit Test framework
 * ==================================
 *
 * Author: AiO (AiO Secure Teletronics - https://www.aio.nu)
 *
 * Project site: https://github.com/aiobofh/cutest
 *
 * Thank you for downloading the CUTest framework! I hope it will make
 * your software development, using test-driven design an easier task.
 *
 * CUTest is a C testing framework written in pure C. The idea behind
 * CUTest is to provide a platform independent C Unit Test framework,
 * but I guess it will only work in Linux for GCC anyway :). It's the
 * thought that counts. Please join me and port it to other enviroments.
 *
 * The CUTest framework is tightly bound to a very specific build
 * system layout too. So let's admit that GNU Make is also needed.
 *
 * A huge tip is to check out the examples folder, it contains both
 * naive, simple and realistic examples of various CUTest usages.
 *
 * Features
 * --------
 *
 * * Automated generation of controllable mocks for all C-functions
 *   called by the design under test.
 * * C-Function stubbing
 * * Generic asserts in 1, 2 and 3 argument flavors.
 * * JUnit XML reports for Jenkins integration
 * * Very few dependencies to other tools (`echo`, `gcc`, `as`, `make`,
 *   `which`, `grep`, `sed`, `rst2html`, `less`, 'nm` and `cproto`)
 * * In-line documentation to ReSTructured Text or HTML
 *   (requires additional tools: `grep`, `sed` and `rst2html`)
 * * Memory leakage detection using Valgrind (requires `valgrind`)
 *
 * Organize your directories
 * -------------------------
 *
 * The CUTest framework make some expecations but should be fairly
 * flexible by default the paths are set to support a flat structure
 * with test-case source files and design under test source files in
 * the same folder.
 *
 * However you MUST name your test-case source file as the
 * corresponding design under test source file.
 *
 * So... If you have a file dut.c you need a dut_test.c file to test
 * the functions in the dut.c file.
 *
 * Here is a flat example::
 *
 *   src/dut.c       <- your program (design under test)
 *   src/dut_test.c  <- test suite for dut.c (must #include cutest.h)
 *   src/Makefile
 *
 * ... So keep your clean:-target clean ;).
 *
 * Here is a more complex example::
 *
 *   my_project/src/dut.c
 *   my_project/src/Makefile
 *   my_project/test/dut_test.c
 *   my_project/test/Makefile
 *
 * In this case you need to set the CUTEST_SRC_DIR=../src in the test
 * Makefile in my_project/test/Makefile.
 *
 * Include paths
 * -------------
 *
 * If you have many -I../path/to/somewhere passed to the build of your
 * project collect all -I-flags into the CUTEST_IFLAGS variable before
 * inclusion of cutest.mk and the include paths will be passed on to
 * cproto and the test-runner build automatically. Hopefully easing
 * your integration a bit.
 *
 * Example
 * -------
 *
 * foo_test.c::
 *
 *   #include "cutest.h"
 *
 *   test(adder_shall_add_two_arguments_and_return_their_sum) {
 *     assert_eq(3, adder(1, 2), "adder shall return 3 = 1 + 2");
 *     assert_eq(6, adder(3, 3), "adder shall return 6 = 3 + 3");
 *   }
 *
 *   test(foo_shall_call_the_adder_function_once_with_correct_args) {
 *     // When calling foo() the adder(i, j) funciton call will call a
 *     // mock.
 *     foo(1, 2);
 *     assert_eq(1, cutest_mock.adder.call_count,
 *               "adder shall be called once");
 *     assert_eq(1, cutest_mock.adder.args.arg0,
 *               "first argument shall be 1");
 *     assert_eq(2, cutest_mock.adder.args.arg1,
 *               "second argument shall be 2");
 *   }
 *
 *   test(foo_shall_return_the_adder_functions_result_unmodified) {
 *     cutest_mock.adder.retval = 123456;
 *     assert_eq(123456, foo(1, 2),
 *               "foo shall return adder's return value");
 *   }
 *
 *   module_test(foo_shall_return_the_adder_functions_result) {
 *     assert_eq(3, foo(1, 2),
 *               "foo shall return adder's return value");
 *   }
 *
 * foo.c::
 *
 *   int adder(int a, int b) { return a + b; }
 *   int foo(int i, int j) { return adder(a, b); }
 *
 * Makefile for a simple directory structure::
 *
 *   CUTEST_SRC_DIR=./ # If you have a flat directory structure
 *   include /path/to/cutest/src/cutest.mk
 *
 *
 * Makefile for automatically downloading cutest into your project::
 *
 *   CUTEST_SRC_DIR=./ # If you have a flat directory structure
 *   include cutest/src/cutest.mk
 *   cutest:
 *      git clone https://github.com/aiobofh/cutest.git
 *   clean::
 *      rm -rf cutest
 *
 * Command line to build a test runner and execute it::
 *
 *   $ make foo_test
 *   $ ./foo_test
 *   ...
 *
 * Command line to run all test suites::
 *
 *   $ make check
 *   ...
 *
 * Command line to run all tests with valgrind memory leakage checks::
 *
 *   $ make valgrind
 *   ...
 *
 * There are more examples available in the examples folder.
 *
 * In-line documentation to ReSTructured Text and/or HTML
 * ------------------------------------------------------
 *
 * You can always read the cutest.h file, since it's the only one
 * around.
 *
 * When you have inclued the cutest.mk makefile in your own Makefile
 * you can build the documentation using::
 *
 *   $ make cutest_help       # Will print out the manual to console
 *   $ make cutest_help.html  # Generate a HTML document
 *   $ make cutest_help.rst   # Generate a RST document
 *
 */
#ifndef _CUTEST_H_
#define _CUTEST_H_

#ifdef CUTEST_LENIENT_ASSERTS
#pragma GCC diagnostic ignored "-Wnonnull"
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#pragma GCC diagnostic ignored "-Wformat="
#endif

/*
 * To compile the test runner you should never ever have
 * `CUTEST_RUN_MAIN` nor `CUTEST_MOCK_MAIN` defined to the compiler.
 * They are used to compile the *CUTest test runner generator* and
 * the *CUTest mock generator* respectively.
 *
 */
#ifndef CUTEST_RUN_MAIN
#ifndef CUTEST_MOCK_MAIN
#ifndef CUTEST_PROX_MAIN

#define _XOPEN_SOURCE
#include <time.h>

extern struct tm *localtime_r(const time_t *timep, struct tm *result);

/*
 * The test() macro
 * ----------------
 *
 * Every unit test is defined with this macro. All function calls within the
 * called functions from a test will be automatically mocked. You can
 * override by setting the func-member of the mock-control struct to the
 * original function if needed, or to any other API compatible function -
 * To stub the funcitonality.
 *
 * Example::
 *
 *   test(main_should_return_0_on_successful_execution)
 *   {
 *     ... Test body ...
 *   }
 *
 */
#define test(NAME) void cutest_##NAME()

/*
 * The module_test() macro
 * -----------------------
 *
 * A module test differs from a unit test, since nothing will be
 * stubbed/mocked in the design under test. You can still stub things by
 * setting the func-member of the mock-control struct to any API compatible
 * function.
 *
 */
#define module_test(NAME) void cutest_module_##NAME()

#ifdef CUTEST_LENIENT_ASSERTS
enum cutest_typename {
  CUTEST_BOOL = 1,
  CUTEST_UNSIGNED_CHAR,
  CUTEST_CHAR,
  CUTEST_SIGNED_CHAR,
  CUTEST_SHORT_INT,
  CUTEST_UNSIGNED_SHORT_INT,
  CUTEST_INT,
  CUTEST_UNSIGNED_INT,
  CUTEST_LONG_INT,
  CUTEST_UNSIGNED_LONG_INT,
  CUTEST_LONG_LONG_INT,
  CUTEST_UNSIGNED_LONG_LONG_INT,
  CUTEST_FLOAT,
  CUTEST_DOUBLE,
  CUTEST_LONG_DOUBLE,

  CUTEST_C_BOOL = 100,
  CUTEST_C_UNSIGNED_CHAR,
  CUTEST_C_CHAR,
  CUTEST_C_SIGNED_CHAR,
  CUTEST_C_SHORT_INT,
  CUTEST_C_UNSIGNED_SHORT_INT,
  CUTEST_C_INT,
  CUTEST_C_UNSIGNED_INT,
  CUTEST_C_LONG_INT,
  CUTEST_C_UNSIGNED_LONG_INT,
  CUTEST_C_LONG_LONG_INT,
  CUTEST_C_UNSIGNED_LONG_LONG_INT,
  CUTEST_C_FLOAT,
  CUTEST_C_DOUBLE,
  CUTEST_C_LONG_DOUBLE,

  CUTEST_OTHER,

  CUTEST_P_BOOL = 1000,
  CUTEST_P_CHAR,
  CUTEST_P_UNSIGNED_CHAR,
  CUTEST_P_SIGNED_CHAR,
  CUTEST_P_SHORT_INT,
  CUTEST_P_UNSIGNED_SHORT_INT,
  CUTEST_P_INT,
  CUTEST_P_UNSIGNED_INT,
  CUTEST_P_LONG_INT,
  CUTEST_P_UNSIGNED_LONG_INT,
  CUTEST_P_LONG_LONG_INT,
  CUTEST_P_UNSIGNED_LONG_LONG_INT,
  CUTEST_P_FLOAT,
  CUTEST_P_DOUBLE,
  CUTEST_P_LONG_DOUBLE,
  CUTEST_P_VOID,

  CUTEST_C_P_BOOL = 10000,
  CUTEST_C_P_CHAR,
  CUTEST_C_P_UNSIGNED_CHAR,
  CUTEST_C_P_SIGNED_CHAR,
  CUTEST_C_P_SHORT_INT,
  CUTEST_C_P_UNSIGNED_SHORT_INT,
  CUTEST_C_P_INT,
  CUTEST_C_P_UNSIGNED_INT,
  CUTEST_C_P_LONG_INT,
  CUTEST_C_P_UNSIGNED_LONG_INT,
  CUTEST_C_P_LONG_LONG_INT,
  CUTEST_C_P_UNSIGNED_LONG_LONG_INT,
  CUTEST_C_P_FLOAT,
  CUTEST_C_P_DOUBLE,
  CUTEST_C_P_LONG_DOUBLE,
  CUTEST_C_P_VOID,

  CUTEST_LAST
};

#define cutest_typename(x)                                              \
  _Generic((x),                                                         \
           _Bool:                          CUTEST_BOOL,                 \
           char:                           CUTEST_CHAR,                 \
           unsigned char:                  CUTEST_UNSIGNED_CHAR,        \
           signed char:                    CUTEST_SIGNED_CHAR,          \
           short int:                      CUTEST_SHORT_INT,            \
           unsigned short int:             CUTEST_UNSIGNED_SHORT_INT,   \
           int:                            CUTEST_INT,                  \
           unsigned int:                   CUTEST_UNSIGNED_INT,         \
           long int:                       CUTEST_LONG_INT,             \
           unsigned long int:              CUTEST_UNSIGNED_LONG_INT,    \
           long long int:                  CUTEST_LONG_LONG_INT,        \
           unsigned long long int:         CUTEST_UNSIGNED_LONG_LONG_INT, \
           float:                          CUTEST_FLOAT,                \
           double:                         CUTEST_DOUBLE,               \
           long double:                    CUTEST_LONG_DOUBLE,          \
                                                                        \
           const _Bool:                    CUTEST_C_BOOL,               \
           const char:                     CUTEST_C_CHAR,               \
           const unsigned char:            CUTEST_C_UNSIGNED_CHAR,      \
           const signed char:              CUTEST_C_SIGNED_CHAR,        \
           const short int:                CUTEST_C_SHORT_INT,          \
           const unsigned short int:       CUTEST_C_UNSIGNED_SHORT_INT, \
           const int:                      CUTEST_C_INT,                \
           const unsigned int:             CUTEST_C_UNSIGNED_INT,       \
           const long int:                 CUTEST_C_LONG_INT,           \
           const unsigned long int:        CUTEST_C_UNSIGNED_LONG_INT,  \
           const long long int:            CUTEST_C_LONG_LONG_INT,      \
           const unsigned long long int:   CUTEST_C_UNSIGNED_LONG_LONG_INT, \
           const float:                    CUTEST_C_FLOAT,              \
           const double:                   CUTEST_C_DOUBLE,             \
           const long double:              CUTEST_C_LONG_DOUBLE,        \
                                                                        \
           _Bool *:                        CUTEST_P_BOOL,               \
           char *:                         CUTEST_P_CHAR,               \
           unsigned char *:                CUTEST_P_UNSIGNED_CHAR,      \
           signed char *:                  CUTEST_P_SIGNED_CHAR,        \
           short int *:                    CUTEST_P_SHORT_INT,          \
           unsigned short int *:           CUTEST_P_UNSIGNED_SHORT_INT, \
           int *:                          CUTEST_P_INT,                \
           unsigned int *:                 CUTEST_P_UNSIGNED_INT,       \
           long int *:                     CUTEST_P_LONG_INT,           \
           unsigned long int *:            CUTEST_P_UNSIGNED_LONG_INT,  \
           long long int *:                CUTEST_P_LONG_LONG_INT,      \
           unsigned long long int *:       CUTEST_P_UNSIGNED_LONG_LONG_INT, \
           float *:                        CUTEST_P_FLOAT,              \
           double *:                       CUTEST_P_DOUBLE,             \
           long double *:                  CUTEST_P_LONG_DOUBLE,        \
                                                                        \
           const _Bool *:                  CUTEST_C_P_BOOL,             \
           const char *:                   CUTEST_C_P_CHAR,             \
           const unsigned char *:          CUTEST_C_P_UNSIGNED_CHAR,    \
           const signed char *:            CUTEST_C_P_SIGNED_CHAR,      \
           const short int *:              CUTEST_C_P_SHORT_INT,        \
           const unsigned short int *:     CUTEST_C_P_UNSIGNED_SHORT_INT, \
           const int *:                    CUTEST_C_P_INT,              \
           const unsigned int *:           CUTEST_C_P_UNSIGNED_INT,     \
           const long int *:               CUTEST_C_P_LONG_INT,         \
           const unsigned long int *:      CUTEST_C_P_UNSIGNED_LONG_INT, \
           const long long int *:          CUTEST_C_P_LONG_LONG_INT,    \
           const unsigned long long int *: CUTEST_C_P_UNSIGNED_LONG_LONG_INT, \
           const float *:                  CUTEST_C_P_FLOAT,            \
           const double *:                 CUTEST_C_P_DOUBLE,           \
           const long double *:            CUTEST_C_P_LONG_DOUBLE,      \
                                                                        \
           default: CUTEST_OTHER)

#define cutest_typename_is_string(VARIABLE)              \
  ((cutest_typename((VARIABLE)) == CUTEST_P_CHAR) ||     \
   (cutest_typename((VARIABLE)) == CUTEST_C_P_CHAR))

#define cutest_typename_is_pointer(VARIABLE)    \
  (cutest_typename(VARIABLE) > CUTEST_OTHER)
#endif

/*
 * The assert_eq() macro
 * ---------------------
 *
 * This macro makes it easy to understand the test-case flow, it is a
 * variadic macro that takes two or three arguments. Use the form you
 * feel most comfortable with.
 *
 * Example::
 *
 *   ...
 *   assert_eq(1, 1, "1 should be eqial to 1");
 *   ...
 *   assert_eq(1, 1);
 *   ...
 *   assert_eq(0, strcmp("expected", some_variable));
 *   ...
 *   assert_eq(some_true_expression);
 *
 * If you have defined CUTEST_LENIENT_ASSERTS  (and use C11 or above)
 * CUTest is able to make more readable asserts and error messages by
 * analyzing the datatypes of the arguments. As you can notice in the
 * example above; comparing two strings are a but cumbersome. However
 * This feature makes things very much easier.
 *
 * Example::
 *
 *   ...
 *   assert_eq("expected", some_variable);
 *   ...
 *
 */
#ifdef CUTEST_LENIENT_ASSERTS

#define assert_eq_3(EXP, REF, STR)                                 \
  if (cutest_typename_is_string((EXP)) &&                          \
      cutest_typename_is_string((REF))) {                          \
    if (0 == strcmp((char*)(EXP), (char*)(REF))) {                 \
      sprintf(cutest_stats.error_output,                           \
              "%s %s:%d assert_eq(\"%s\", \"%s\", " STR ") "       \
              "failed\n", cutest_stats.error_output,               \
              __FILE__, __LINE__, (char*)EXP, (char*)REF);         \
      cutest_assert_fail_cnt++;                                    \
    }                                                              \
  }                                                                \
  else {                                                           \
    if ((EXP) != (REF)) {                                          \
      sprintf(cutest_stats.error_output,                           \
              "%s %s:%d assert_eq(" #EXP ", " #REF ", " STR ") "   \
              "failed\n", cutest_stats.error_output,               \
              __FILE__, __LINE__);                                 \
      cutest_assert_fail_cnt++;                                    \
    }                                                              \
  }

#define assert_eq_2(EXP, REF)                                      \
  if (cutest_typename_is_string((EXP)) &&                          \
      cutest_typename_is_string((REF))) {                          \
    if (0 != strcmp((char*)(EXP), (char*)(REF))) {                 \
      sprintf(cutest_stats.error_output,                           \
              "%s %s:%d assert_eq(\"%s\", \"%s\") "                \
              "failed\n", cutest_stats.error_output,               \
              __FILE__, __LINE__, (char*)EXP, (char*)REF);         \
      cutest_assert_fail_cnt++;                                    \
    }                                                              \
  }                                                                \
  else {                                                           \
    if ((EXP) != (REF)) {                                          \
      sprintf(cutest_stats.error_output,                           \
              "%s %s:%d assert_eq(" #EXP ", " #REF ") "            \
              "failed\n", cutest_stats.error_output,               \
              __FILE__, __LINE__);                                 \
      cutest_assert_fail_cnt++;                                    \
    }                                                              \
  }

#else

#define assert_eq_3(EXP, REF, STR)                                 \
  if ((EXP) != (REF)) {                                            \
    sprintf(cutest_stats.error_output,                             \
            "%s %s:%d assert_eq(" #EXP ", " #REF ", " STR ") "     \
            "failed\n", cutest_stats.error_output,                 \
            __FILE__, __LINE__);                                   \
    cutest_assert_fail_cnt++;                                      \
  }

#define assert_eq_2(EXP, REF)                                      \
  if ((EXP) != (REF)) {                                            \
    sprintf(cutest_stats.error_output,                             \
            "%s %s:%d assert_eq(" #EXP ", " #REF ") "              \
            "failed\n", cutest_stats.error_output,                 \
            __FILE__, __LINE__);                                   \
    cutest_assert_fail_cnt++;                                      \
  }

#endif

#define assert_eq_1(EXP)                                           \
  if (!(EXP)) {                                                    \
    sprintf(cutest_stats.error_output,                             \
            "%s %s:%d assert_eq(" #EXP ") "                        \
            "failed\n", cutest_stats.error_output,                 \
            __FILE__, __LINE__);                                   \
    cutest_assert_fail_cnt++;                                      \
  }

#define assert_eq_select(X, A1, A2, A3, MACRO, ...) MACRO

#define assert_eq_chooser(...)                  \
  assert_eq_select(,##__VA_ARGS__,              \
                   assert_eq_3,                 \
                   assert_eq_2,                 \
                   assert_eq_1,)

#define assert_eq(...) assert_eq_chooser(__VA_ARGS__)(__VA_ARGS__)

static int cutest_assert_fail_cnt = 0;
static struct {
  int verbose;
  int junit;
  int no_linefeed;
} cutest_opts;
static int cutest_exit_code = EXIT_SUCCESS;

#define CUTEST_MAX_JUNIT_BUFFER_SIZE 1024*1024

struct {
  char suite_name[128];
  char design_under_test[128];
  char error_output[1024*1024];
  int test_cnt;
  int fail_cnt;
  float elapsed_time;
} cutest_stats;

static char cutest_junit_report[CUTEST_MAX_JUNIT_BUFFER_SIZE + 1];
static char cutest_junit_report_tmp[CUTEST_MAX_JUNIT_BUFFER_SIZE + 1];

/*
 * Test initialization
 * -------------------
 *
 * In between every test() macro the CUTest framework will clear all
 * the mock controls and test framwork state so that every test is
 * run in isolation.
 *
 */
static void cutest_startup(int argc, char* argv[],
                           const char* suite_name)
{
  int i;
  cutest_opts.verbose = 0;
  cutest_opts.junit = 0;
  for (i = 1; i < argc; i++) {
    if ((0 == strcmp(argv[i], "-v")) ||
        (0 == strcmp(argv[i], "--verbose"))) {
      cutest_opts.verbose = 1;
    }
    if ((0 == strcmp(argv[i], "-j")) ||
        (0 == strcmp(argv[i], "--junit"))) {
      cutest_opts.junit = 1;
    }
    if ((0 == strcmp(argv[i], "-n")) ||
        (0 == strcmp(argv[i], "--no-linefeed"))) {
      cutest_opts.no_linefeed = 1;
    }
  }
  memset(cutest_junit_report, 0, sizeof(cutest_junit_report));
  memset(cutest_junit_report_tmp, 0, sizeof(cutest_junit_report_tmp));
  memset(&cutest_stats, 0, sizeof(cutest_stats));
  strcpy(cutest_stats.suite_name, suite_name);
  strcpy(cutest_stats.design_under_test, suite_name);
}

/*
 * Test execution
 * --------------
 *
 * When executing tests the elapsed time for execution is sampled and
 * used in the JUnit report. Depending on command line options an
 * output is printed to the console, either as a short version with
 * '.' for successful test run and 'F' for failed test run, but if set
 * to verbose '-v' '[PASS]' and '[FAIL]' output is produced. What
 * triggers a failure is if an assert_eq() is not fulfilled.
 *
 * If the test runner is started with verbose mode '-v' the offending
 * assert will be printed to the console directly after the fail. If
 * in normal mode all assert-failures will be collected and printed
 * in the shutdown process.
 *
 */
static void cutest_execute_test(void (*func)(), const char *name, int do_mock) {
  time_t start_time = time(NULL);
  time_t end_time;
  double elapsed_time;
  memset(&cutest_mock, 0, sizeof(cutest_mock));
  if (1 == do_mock) {
    cutest_set_mocks_to_original_functions();
  }
  func();
  if (cutest_opts.verbose) {
    if (cutest_assert_fail_cnt == 0) {
      printf("[PASS]: %s\n", name);
    }
    else {
      printf("[FAIL]: %s\n", name);
      printf("%s", cutest_stats.error_output);
      memset(cutest_stats.error_output, 0,
             sizeof(cutest_stats.error_output));
    }
  }
  else {
    if (cutest_assert_fail_cnt == 0) {
      printf(".");
    }
    else {
      printf("F");
    }
    fflush(stdout);
  }

  if (cutest_assert_fail_cnt != 0) {
    cutest_exit_code = EXIT_FAILURE;
  }

  cutest_stats.test_cnt++;
  cutest_stats.fail_cnt += (cutest_assert_fail_cnt != 0);

  end_time = time(NULL);

  elapsed_time = (double)(end_time - start_time) / (double)1000;

  cutest_stats.elapsed_time += elapsed_time;

  strcpy(cutest_junit_report_tmp, cutest_junit_report);
  /* Ugly-output some JUnit XML for each test-case */
  sprintf(cutest_junit_report,
          "%s    <testcase classname=\"%s\" name=\"%s\" time=\"%f\">\n",
          cutest_junit_report_tmp, cutest_stats.design_under_test,
          name, elapsed_time);
  if (cutest_assert_fail_cnt != 0) {
    strcpy(cutest_junit_report_tmp, cutest_junit_report);
    sprintf(cutest_junit_report,
            "%s      <failure message=\"test failure\">%s</failure>\n",
            cutest_junit_report_tmp, cutest_stats.error_output);
  }
  strcpy(cutest_junit_report_tmp, cutest_junit_report);
  sprintf(cutest_junit_report, "%s    </testcase>\n",
          cutest_junit_report_tmp);

  cutest_assert_fail_cnt = 0;
}

/*
 * Shutdown process
 * ----------------
 *
 * At the end of the execution the CUTest test-runner program will
 * output a JUnit XML report if specified with the -j command line
 * option.
 *
 */
static void cutest_shutdown(const char* filename)
{
  char junit_report_name[1024];
  char timestamp[40];
  time_t current_time;
  int i;
  struct tm tm;

  if (0 == cutest_opts.verbose) {
    /*
     * Add an enter if not running in verbose, to line break after
     * the ...
     */
    if ((0 == cutest_opts.no_linefeed) ||
        (0 != strlen(cutest_stats.error_output))) {
      printf("\n");
    }
    printf("%s", cutest_stats.error_output);
  }
  else {
    /* Print a simple summary. */
    printf("%d passed, %d failed.\n",
           cutest_stats.test_cnt - cutest_stats.fail_cnt,
           cutest_stats.fail_cnt);
  }

  memset(junit_report_name, 0, sizeof(junit_report_name));

  for (i = strlen(filename); i > 0; i--) {
    if ('.' == filename[i]) {
      strncpy(junit_report_name, filename, i);
      strcat(junit_report_name, ".junit_report.xml");
      break;
    }
  }

  /* Create a textual timestamp */
  current_time = time(NULL);
  localtime_r(&current_time, &tm);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", &tm);

  if (cutest_opts.junit) {
    FILE *stream = fopen(junit_report_name, "w+");
    /* Ugly-output a JUnit XML-report. */
    fprintf(stream,
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<testsuites>\n"
            "  <testsuite name=\"%s\"\n"
            "             errors=\"%d\"\n"
            "             tests=\"%d\"\n"
            "             failures=\"%d\"\n"
            "             time=\"%f\"\n"
            "             timestamp=\"%s\">\n"
            "%s"
            "  </testsuite>\n"
            "</testsuites>\n",
            filename, 0, cutest_stats.test_cnt, cutest_stats.fail_cnt,
            (double)cutest_stats.elapsed_time / (double)1000, timestamp,
            cutest_junit_report);
    fclose(stream);
  }
}

/*
 * Work-flow using test-driven design
 * ----------------------------------
 *
 * As you might have noticed this documentation often use the phrase
 * "Test-Driven Design" instead of "Test-Driven Development". This is
 * a conscious choice, since the whole idea about CUTest is to drive
 * the *design* of your software, rather than just make tests for your
 * code. It's a nuance of difference in the meaning of these.
 *
 * So... Let's walk-through one way of using CUTest to do just this...
 *
 * Let's say you want to write a piece of code that write ten lines of
 * text to a file on disc. Obviously you don't want to actually *write*
 * the file for just testing your ideas. This is where the automatic
 * mocking of ALL called functions in your design come in handy. This
 * work-flow example will also show you how to write module-tests that
 * make some kind of "kick-the-tires" sanity check that the integration
 * to the OS actually works with file access and all.
 *
 * Let's do this step-by-step...
 *
 * 1. You have to write a function called ``write_file``. And it shall
 *    take one single argument (a pointer to the file name stored in a
 *    string) where to store the file.
 *
 *    a. Write a simple test that assumes everything will go well. This
 *       implies that you can determine the success of the operation
 *       somehow. Let's use the old "return zero on success" paradigm.
 *       So... Let's call the design under test called ``write_file``
 *       with some kind of file-name as argument and expect it to
 *       return 0 (zero). Create a file called ``file_operations_test.c``
 *       and include ``cutest.h`` in the top of it.
 *
 *       Code::
 *
 *        test(write_file_shall_return_0_if_all_went_well)
 *        {
 *          assert_eq(0, write_file("my_filename.txt"));
 *        }
 *
 *    b. Now... When you try to compile this code using ``make check``
 *       everything will fail!
 *
 *       You will get build and compilation errors, since there is no
 *       corresponding file that contain the design under test yet.
 *
 *    c. Create a file called ``file_operations.c`` and implement a
 *       function called ``write_file`` that takes one ``const char*``
 *       argument as file name. And start by just fulfilling the test
 *       by returning a 0 (zero) from it.
 *
 *       Code::
 *
 *        int write_file(const char* filename)
 *        {
 *          return 0;
 *        }
 *
 *    d. No you should be able to compile and run your test using
 *       ``make check``. And the test should probably pass, if you
 *       did it correctly. And since the assumption of your test that
 *       ``write_file`` should return 0 (zero) on success probably will
 *       be true for all eternity you will probably have to revisit and
 *       re-factor it as the function becomes more complete.
 *
 * 2. Using the standard library to write code that opens a file
 *
 *    a. You probably already know that you will need to open a file to
 *       write you file contents to inside your ``write_file`` function.
 *       Let's make sure that we call ``fopen()`` in a good way, using
 *       the given file name and the correct file opening mode.
 *       Since this test probably will look nicer using
 *       CUTEST_LENIENT_ASSERTS, define it using ``#define`` before
 *       your ``#include "cutest.h"`` line. Now you can use strings as
 *       arguments to the ``assert_eq()`` macro instead of having to use
 *       ``strcmp()`` return value to compare two strings.
 *
 *       Code::
 *
 *        test(write_file_shall_open_the_correct_file_for_writing)
 *        {
 *          (void)write_file("my_filename.txt");
 *
 *          assert_eq(1, cutest_mock.fopen.call_count);
 *          assert_eq("my_filename.txt", cutest_mock.fopen.args.arg0);
 *          assert_eq("w", cutest_mock.fopen.args.arg1);
 *        }
 *
 *       As you can see this test will call the design under test with
 *       a file name as argument, then assert that the ``fopen()``
 *       function in the standard library is called *once*. Then it
 *       verifies that the two arguments passed to ``fopen()``is
 *       correct, by asserting that the first argument should be the
 *       file name passed to ``write_file`` and that the file is opened
 *       in *write* mode.
 *
 *    b. Once again, if you compile this the build will break. So, lets
 *       just implement the code to open the file. Revisit your code in
 *       ``file_operations.c`` and add the call to the ``fopen()``
 *       function.
 *
 *       Code::
 *
 *        int write_file(const char* filename)
 *        {
 *           fopen(filename, "w");
 *           return 0;
 *        }
 *
 *       Now you should be able to build the test again and run it using
 *       ``make check``. Let's take a break here... When running the
 *       test you will call your design under test by calling it as a
 *       function. The way CUTest works is that it detects ANY function
 *       call inside a callable function (e.g. ``fopen(...)`` and it
 *       will be replaced to call a generated mock-up of the same
 *       function. The mock-up mimics the API, with the same arguments
 *       as the original function. But the *actual* ``fopen()`` is never
 *       called by default when writing a unit-test.
 *
 *       Hence you can check various aspects of the function call in your
 *       test, using ``assert_eq`` on values expected, like in the test-
 *       case we just wrote. We're checking the arguments of the call to
 *       ``fopen()`` and how many times the ``write_file`` design calls
 *       the ``fopen()`` function.
 *
 *       Pretty neat, right?
 *
 * 3. OK - Common sense tell us, that if a file is opened, it should
 *    probably be closed also. Otherwise the OS would end up with a bunch
 *    of opened files.
 *
 *    a. So let's define a test for checking that the provided file name
 *       actually close the _correct_ file too, before the design under
 *       test exits and return it's 0 (zero).
 *
 *       This time you will have to manipulate the return value of
 *       the ``fopen()`` function, to something that makes it easy to
 *       recognize as argument to the ``fclose()`` value. Making sure
 *       that the design close the correct file. This is done by setting
 *       the retval of the ``fopen()`` mock-up control structure by
 *       assigning a value to ``cutest_mock.fopen.retval``.
 *
 *       Code::
 *
 *        #define FOPEN_OK_RETVAL (FILE*)1234
 *
 *        test(write_file_shall_close_the_opened_file)
 *        {
 *          cutest_mock.fopen.retval = FOPEN_OK_RETVAL;
 *
 *          (void)write_file("my_filename.txt");
 *
 *          assert_eq(1, cutest_mock.fclose.call_count);
 *          assert_eq(FOPEN_OK_RETVAL, cutest_mock.fclose.args.arg0);
 *        }
 *
 *    b. And once again: If you try to compile and run this, the test
 *       will fail, due to the fact that you have not implemented the
 *       code to call the ``fclose()`` function yet. So let's re-factor
 *       the design under test again.
 *
 *       Code::
 *
 *        int write_file(const char* filename)
 *        {
 *           FILE* fp = fopen(filename, "w");
 *           fclose(fp);
 *           return 0;
 *        }
 *
 *       Done! This should no pass the test you wrote earlier.
 *
 * 3. Now when you're getting the hang of things, lets touch a bit
 *    trickier subject. Error handling.
 *
 *    When interacting with the surrounding world via OS functionality
 *    or users it's extremely important to take care of potential
 *    errors to produce robust design. In this case it's easy to see
 *    that the OS might be unable to open the file to write to for
 *    various reasons. For example the path in which to write the
 *    file could be non-existent or the user might not have access to
 *    write files. In any case the ``write_file`` design should
 *    allow the OS to fail and gracefully report its inability to
 *    operate on the file to the programmer using it.
 *
 *    a. Let's start by manipulating (pretending) that ``fopen()`` is not
 *       able to open the file for writing, and expect some kind of return
 *       value, indicating what went wrong. In this example you also can
 *       practice self-documenting code by writing a function to do the
 *       pretending part.
 *
 *       Code::
 *
 *        static void pretend_that_fopen_will_fail()
 *        {
 *          cutest_mock.fopen.retval = NULL;
 *        }
 *
 *        test(write_file_shall_return_1_if_file_could_not_be_opened)
 *        {
 *          pretend_that_fopen_will_fail();
 *
 *          assert_eq(1, write_file("my_filename.txt"));
 *        }
 *
 *       So. We expect the ``write_file`` function to return 1 (one) if
 *       the OS was not able to open the file for writing. And we pretend
 *       that ``fopen()`` will fail by returning ``NULL`` as ``FILE*``
 *       return value.
 *
 *    b. Note that you can't always assume that the function returns 0,
 *       nor that it does need/can close the file anymore when this is
 *       done. Hence you will have to re-factor the two earlier written
 *       naive tests to take this error handling into account.
 *
 *       First of all. If ``fopen()`` succeeds, the design should close
 *       the file using ``fclose()`` and only then return 0 (zero).
 *
 *       Code::
 *
 *        static void pretend_that_fopen_will_go_well()
 *        {
 *          cutest_mock.fopen.retval = FOPEN_OK_RETVAL;
 *        }
 *
 *        test(write_file_shall_open_the_correct_file_for_writing)
 *        {
 *          pretend_that_fopen_will_go_well();
 *
 *          (void)write_file("my_filename.txt");
 *
 *          assert_eq(1, cutest_mock.fopen.call_count);
 *          assert_eq("my_filename.txt", cutest_mock.fopen.args.arg0);
 *          assert_eq("w", cutest_mock.fopen.args.arg1);
 *        }
 *
 *        test(write_file_shall_close_the_opened_file)
 *        {
 *          pretend_that_fopen_will_go_well();
 *
 *          (void)write_file("my_filename.txt");
 *
 *          assert_eq(1, cutest_mock.fclose.call_count);
 *          assert_eq(FOPEN_OK_RETVAL, cutest_mock.fclose.args.arg0);
 *        }
 *
 *    c. One could argue that there should be a test for making sure that
 *       the file is not closed if it was never opened. Depending on the
 *       level of white-box testing you want you could probably skip this
 *       test, since you know what the expressions inside the design under
 *       test will look like. If you still want it, a test like that
 *       would look like this:
 *
 *       Code::
 *
 *        test(write_file_shall_not_try_to_close_an_unopened_file)
 *        {
 *          pretend_that_fopen_will_fail();
 *
 *          (void)write_file("my_filename.txt");
 *
 *          assert_eq(0, cutest_mock.fclose.call_count);
 *        }
 *
 *    d. Now we are perfectly set to implement the code. The old tests are
 *       re-factored and the new one is written. So move over to the
 *       design under test, and make it return 1 (one) if the ``fopen()``
 *       function fails.
 *
 *       Code::
 *
 *        int write_file(const char* filename)
 *        {
 *           FILE* fp = fopen(filename, "w");
 *           if (NULL == fp) {
 *             return 1;
 *           }
 *           fclose(fp);
 *           return 0;
 *        }
 *
 *       And true enough. This tests will pass since the ``write_file``
 *       function mirrors the tests expectations of it.
 *
 * 4. Even more error handling is needed to build a robust piece of code.
 *
 *    Even closing a file could theoretically go wrong. Lets look in-to
 *    making a specific return value from ``write_file`` if ``fclose()``
 *    did not work as intended.
 *
 *    a. Make sure that you write a test to assert that ``write_file``
 *       returns a 3 (three) if the file could not be closed.
 *
 *       Code::
 *
 *        #define FCLOSE_NOT_OK_RETVAL 1
 *
 *        void pretend_that_fclose_will_fail()
 *        {
 *          cutest_mock.fclose.retval = FCLOSE_NOT_OK_RETVAL;
 *        }
 *
 *        void pretend_that_fopen_will_go_well_but_fclose_will_fail()
 *        {
 *          pretend_that_fopen_will_go_well();
 *          pretend_that_fclose_will_fail();
 *        }
 *
 *        test(write_file_shall_return_3_if_file_could_not_be_closed)
 *        {
 *          pretend_that_fopen_will_go_well_but_fclose_will_fail();
 *
 *          assert_eq(3, write_file("my_filename.txt"));
 *        }
 *
 *       Setting the test up to pretend that a file is opened successfully
 *       but closing it fails for some reason and the ``write_file``
 *       design will return 3.
 *
 *    b. Implement the design of ``write_file`` accordingly.
 *
 *       Code::
 *
 *        int write_file(const char* filename)
 *        {
 *           FILE* fp = fopen(filename, "w");
 *           if (NULL == fp) {
 *             return 1;
 *           }
 *           if (0 != fclose(fp)) {
 *             return 3;
 *           }
 *           return 0;
 *        }
 *
 *       As you can see, exit-early mind-set makes things quite easy to
 *       test. Just calling the same design under test over and over and
 *       just assert various aspects of the algorithm, only manipulating
 *       the mocks so that the program flow reaches the part you want.
 *
 * 4. Testing a loop that writes rows to the opened file.
 *
 *    a. Let's say you want your code to write ten lines of text into the
 *       specified file, lets do a simple test that verifies that the
 *       ``fputs()`` function is called exactly 10 times.
 *
 *       Code::
 *
 *        test(write_file_shall_write_10_lines_to_the_opened_file)
 *        {
 *          pretend_that_fopen_will_go_well();
 *
 *          (void)write_file("my_filename.txt");
 *
 *          assert_eq(10, cutest_mock.fputs.call_count);
 *        }
 *
 *    b. And implement the design accordingly
 *
 *       Code::
 *
 *        int write_file(const char* filename)
 *        {
 *           int i = 10;
 *           FILE* fp = fopen(filename, "w");
 *           if (NULL == fp) {
 *             return 1;
 *           }
 *           while (i-- > 0) {
 *             fputs("A text row\n", fp);
 *           }
 *           if (0 != fclose(fp)) {
 *             return 3;
 *           }
 *           return 0;
 *        }
 *
 *       There we go. Ten rows written to the file using ``fputs``.
 *
 * 6. Even more robust code by verifying that ``fputs`` is able to write
 *    to disc.
 *
 *    a. Since ``fputs`` can fail let's expect our code to return another
 *       value if this happens. Implement a test that pretend ``fputs``
 *       is unable to operate properly ``write_file`` return 2 (two).
 *
 *       Code::
 *
 *        test(write_file_shall_return_2_if_file_could_not_be_written)
 *        {
 *          pretend_that_fopen_will_go_well_but_fputs_will_fail();
 *
 *          assert_eq(2, write_file("my_filename.txt"));
 *        }
 *
 *    b. You would probably still want ``fclose`` to be called even tho
 *       the writing went wrong, once again helping the OS to reduce the
 *       number of open files. So let's re-factor the previously written
 *       test for this ``fclose``. The previous test was called
 *       ``write_file_shall_close_the_opened_file``. It is still a valid
 *       name, but if a file could be opened the ``write_file`` design
 *       implies that ``fputs`` will be called some 10 times.... For
 *       example it could look something like this:
 *
 *       Code::
 *
 *        test(write_file_shall_close_the_opened_file_if_able_to_write_to_file)
 *        {
 *          pretend_that_fopen_and_fputs_will_go_well();
 *
 *          (void)write_file("my_filename.txt");
 *
 *          assert_eq(1, cutest_mock.fclose.call_count);
 *          assert_eq(FOPEN_OK_RETVAL, cutest_mock.fclose.args.arg0);
 *        }
 *
 *       Also, ``fclose`` fclose should be called correctly if fputs will
 *       fail, and such test could imply that ``fputs`` is probably only
 *       called once. Or the OS could run out of disc space... This test
 *       example implies that something went wrong on the first write and
 *       ``fputs`` should probably not be called more than once.
 *
 *       Code:::
 *
 *        test(write_file_shall_close_the_opened_file_if_unable_to_write_to_file)
 *        {
 *          pretend_that_fopen_will_go_well_but_fputs_will_fail();
 *
 *          (void)write_file("my_filename.txt");
 *
 *          assert_eq(1, cutest_mock.fputs.call_count);
 *          assert_eq(1, cutest_mock.fclose.call_count);
 *          assert_eq(FOPEN_OK_RETVAL, cutest_mock.fclose.args.arg0);
 *        }
 *
 *    c. Now we have most cases covered I would say. Lets implement the
 *       writing of lines as something that match our test assertions.
 *
 *       Code::
 *
 *        int write_file(const char* filename)
 *        {
 *           int i = 10;
 *           int retval = 0;
 *           FILE* fp = fopen(filename, "w");
 *           if (NULL == fp) {
 *             return 1;
 *           }
 *           while (i-- > 0) {
 *             if (0 == fputs("A text row\n", fp)) {
 *               retval = 2;
 *               break;
 *             }
 *           }
 *           if (0 != fclose(fp)) {
 *             retval = 3;
 *           }
 *           return retval;
 *        }
 *
 *      There we have it. A fully functional design driven by small tests
 *      implemented in pure C.
 *
 * 7. Sometimes it can be a good idea to make some hand-waving integration
 *    tests. These can be done in advance or after a design has been done.
 *
 *    If you practice *acceptance-test-driven development* it should be
 *    done in advance. But if you just want to verify that your code and
 *    design actually works in the real world it is often easier to do
 *    when the design is completed. And IF you do it in advance you need
 *    to accept that the test will not work until the complete design is
 *    implemented.
 *
 *    Here are a few simple tests that make sure that the ``write_file``
 *    design actually write stuff to disc and that it looks somewhat
 *    correct.
 *
 *    Code:::
 *
 *     int count_lines_in_file(const char* tmp_filename)
 *     {
 *       int cnt = 0;
 *       char buf[1024];
 *       FILE *fp = fopen(tmp_filename, "r");
 *       while (!feof(fp)) { if (0 != fgets(buf, 1024, fp)) { cnt++; } };
 *       fclose(fp);
 *       return cnt;
 *     }
 *
 *     module_test(write_file_shall_write_a_10_lines_long_file_to_disc_if_possible)
 *     {
 *       pid_t p = getpid();
 *       char tmp_filename[1024];
 *
 *       sprintf(tmp_filename, "/tmp/%ld_real_file", p);
 *
 *       assert_eq(0, write_file(tmp_filename));
 *       assert_eq(10, count_lines_in_file(tmp_filename));
 *
 *       unlink(tmp_filename);
 *     }
 *
 *     module_test(write_file_shall_fail_if_writing_to_disc_is_not_possible)
 *     {
 *       const char* tmp_filename = "/tmp/this_path_sould_not_exist/oogabooga";
 *
 *       assert_eq(1, write_file(tmp_filename));
 *     }
 *
 *    Worth noticing is that these kind of tests use the ``module_test``
 *    macro in the CUTest framework. Since it implies that the original
 *    functions used in the design under test should be used rather than
 *    just mock-ups. To speak the truth, the CUTest framework actually
 *    mock-up everything, but in the ``module_test`` implementation the
 *    custs_mock.<function>.func function pointer is set to the original
 *    function. Hence you can still verify call counts, arguments passed
 *    but the over-all functionality of you design will be run for real.
 *    Note that this can definitely impact execution time.
 *
 *    Another thing worth noticing is that many developers beleive that
 *    these kind of integration tests or module tests are unit-tests.
 *    One could argue that they're not, since they do not drive the
 *    design, nor do they test only _your_ code, but they test already
 *    tested code, like ``fopen``, ``close`` and ``fputs`` in this case.
 *    Which might seem like waste of clock cycles.
 *
 * That's it folks! I hope you enjoyed this example of a work-flow and
 * please come back to the author with feedback!
 *
 */

#endif /* CUTEST_PROX_MAIN */
#endif /* CUTEST_MOCK_MAIN */
#endif /* CUTEST_RUN_MAIN */

/*********************************************************************
   --    ____ ____ _____ ____ ____ _____   ____   ____ ____ ____ ----
   --   / __// / //_  _// __// __//_  _/  /    \ /   // __// / / ----
   --  / /_ / / /  / / / __//_  /  / /   / / / // / // /_ /  <'  ----
   -- /___//___/  /_/ /___//___/  /_/   /_/_/_//___//___//_/_/   ----
 *
 * CUTest mock generator
 * =====================
 *
 * This is a tool that can be used to generate mock-up functions. It
 * inspects a specified source-code file (written i C language) and
 * looks for uses of the funcitons listed in a file which list all
 * function that is replaceable with a mock when developing code using
 * test-driven design.
 *
 * Requirements
 * ------------
 *
 * To be able to generate well formatted function declarations to
 * mutate into mock-ups this tool make use of the ``cproto`` tool.
 *
 * How to compile the tool
 * -----------------------
 *
 * Just include the cutest.mk makefile in your own Makefile in your
 * folder containing the source code for the ``*_test.c`` files.
 *
 * The tool is automatically compiled when making the check target
 * But if you want to make the tool explicitly just call::
 *
 *  $ make cutest_mock
 *
 * Usage
 * -----
 *
 * If you *need* to run the tool manually this is how::
 *
 *  $ ./cutest_mock design_under_test.c mockables.lst /path/to/cutest
 *
 * And it will scan the source-code for mockable functions and
 * output a header file-style text, containing everything needed to
 * test your code alongside with the `cutest.h` file.
 *
 * The mockables.lst is produced by `nm dut.o | sed 's/.* //g'`.
 *
 * However, if you use the Makefile targets specified in the beginning
 * of this document you will probably not need to run it manually.
 *
 */

#ifdef CUTEST_MOCK_MAIN

#define _XOPEN_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#define MAX_CUTEST_MOCKS 1024
#define MAX_CUTEST_MOCK_ARGS 128
#define MAX_CUTEST_MOCK_NAME_LENGTH 128

typedef struct cutest_mock_arg_type_s {
  int is_static;
  int is_const;
  int is_struct;
  char name[MAX_CUTEST_MOCK_NAME_LENGTH + 1];
} cutest_mock_arg_type_t;

typedef struct cutest_mock_arg_s {
  char name[MAX_CUTEST_MOCK_NAME_LENGTH + 1];
  char function_pointer_name[MAX_CUTEST_MOCK_NAME_LENGTH + 1];
  cutest_mock_arg_type_t type;
  int is_function_pointer;
  int is_array;
} cutest_mock_arg_t;

typedef struct cutest_mock_return_type_s {
  int is_static;
  int is_const;
  int is_inline;
  int is_struct;
  char name[MAX_CUTEST_MOCK_NAME_LENGTH + 1];
} cutest_mock_return_type_t;

typedef struct cutest_mock_s {
  char name[MAX_CUTEST_MOCK_NAME_LENGTH + 1];
  cutest_mock_return_type_t return_type;
  int arg_cnt;
  cutest_mock_arg_t arg[MAX_CUTEST_MOCK_ARGS];
} cutest_mock_t;

typedef struct cutest_mocks_s {
  int mock_cnt;
  cutest_mock_t mock[MAX_CUTEST_MOCKS];
} cutest_mocks_t;

cutest_mocks_t mocks;

static inline long get_file_size(FILE *fd)
{
  long file_size;
  (void)fseek(fd, 0L, SEEK_END);
  file_size = ftell(fd);
  fseek(fd, 0L, SEEK_SET);
  return file_size;
}

#include <assert.h>

static int skip_white_spaces(const char* buf) {
  int pos = 0;
  while (' ' == buf[pos]) {
    pos++;
  }
  return pos;
}

static int find_name_pos(const char* buf) {
  int i;
  const int len = strlen(buf);
  int start = 0;
  for (i = 0; i < len; i++) {
    if (buf[i] == ' ') {
      start = i + 1;
    }
    if (buf[i] == '(') {
      return start;
    }
  }
  return -1;
}

static int get_return_type(cutest_mock_return_type_t* return_type,
                           const char* buf)
{
  int pos = 0;
  int dst_pos = 0;
  pos += skip_white_spaces(&buf[pos]);

  int namepos = find_name_pos(buf);

  if (-1 == namepos) {
    return 0;
  }

  /*while (' ' != buf[pos]) { */
  while(pos < namepos - 1) {
    if (0 == strncmp(&buf[pos], "static ", 7)) {
      return_type->is_static = 1;
      pos += 7;
      continue;
    }
    if (0 == strncmp(&buf[pos], "struct ", 7)) {
      return_type->is_struct = 1;
      pos += 7;
      continue;
    }
    if (0 == strncmp(&buf[pos], "inline ", 7)) {
      return_type->is_inline = 1;
      pos += 7;
      continue;
    }
    if (0 == strncmp(&buf[pos], "__extension__ ", 14)) {
      pos += 14;
      continue;
    }
    return_type->name[dst_pos] = buf[pos];
    dst_pos++;
    pos++;
  }
  pos += skip_white_spaces(&buf[pos]);
  if ('*' == buf[pos]) {
    return_type->name[dst_pos] = '*';
    pos++;
    if ('*' == buf[pos]) {
      dst_pos++;
      return_type->name[dst_pos] = '*';
      pos++;
    }
  }
  pos += skip_white_spaces(&buf[pos]);
  return pos;
}

static int get_function_name(char* name, const char* buf)
{
  int pos = 0;
  while ('(' != buf[pos]) {
    if (';' == buf[pos]) {
      return -1;
    }
    name[pos] = buf[pos];
    pos++;
  }
  if ('(' != buf[pos]) {
    return -1;
  }
  return pos + 1;
}

static int get_function_args(cutest_mock_t* mock, const char* buf) {
  int pos = 0;
  while (';' != buf[pos]) {
    int dst_pos = 0;
    /* Arg type */
    pos += skip_white_spaces(&buf[pos]);
    int paren = 0;
    while ((' ' != buf[pos]) &&
           (',' != buf[pos]) &&
           ((')' != buf[pos]) && (0 == paren))) {
      if (0 == strncmp("const ", &buf[pos], 6)) {
        mock->arg[mock->arg_cnt].type.is_const = 1;
        pos += 6;
        continue;
      }
      if (0 == strncmp("static ", &buf[pos], 7)) {
        mock->arg[mock->arg_cnt].type.is_static = 1;
        pos += 7;
        continue;
      }
      if (0 == strncmp("struct ", &buf[pos], 7)) {
        mock->arg[mock->arg_cnt].type.is_struct = 1;
        pos += 7;
        continue;
      }
      mock->arg[mock->arg_cnt].type.name[dst_pos] = buf[pos];
      dst_pos++;
      pos++;
    }
    pos += skip_white_spaces(&buf[pos]);
    if ('*' == buf[pos]) {
      mock->arg[mock->arg_cnt].type.name[dst_pos] = '*';
      pos++;
      if ('*' == buf[pos]) {
        dst_pos++;
        mock->arg[mock->arg_cnt].type.name[dst_pos] = '*';
        pos++;
      }
    }

    while ((paren != 0) || ((',' != buf[pos]) && (')' != buf[pos]))) {
      /* Handle function pointers as arguments */
      if (buf[pos] == '(') {
        if (0 == paren) {
          dst_pos = 0;
        }
        paren++;
      }
      if ((buf[pos] == ')') && (2 == paren)) {
        paren = 0;
        mock->arg[mock->arg_cnt].is_function_pointer = 1;
      }

      if (buf[pos] == '[') {
        mock->arg[mock->arg_cnt].is_array = 1;
      }

      mock->arg[mock->arg_cnt].name[dst_pos] = buf[pos];
      dst_pos++;
      pos++;
    }

    if ((0 != strcmp(mock->arg[mock->arg_cnt].type.name, "void")) &&
        (0 == mock->arg[mock->arg_cnt].is_function_pointer)) {
      if ('\0' == mock->arg[mock->arg_cnt].name[0]) {
        if (0 == mock->arg[mock->arg_cnt].is_function_pointer) {
          if (1 == mock->arg[mock->arg_cnt].is_array) {
            strcat(mock->arg[mock->arg_cnt].type.name, "*");
          }
          sprintf(mock->arg[mock->arg_cnt].name, "arg%d", mock->arg_cnt);
        }
      }
      mock->arg_cnt++;
    }
    else if (1 == mock->arg[mock->arg_cnt].is_function_pointer) {
      char tmp[1024];
      strcpy(tmp, mock->arg[mock->arg_cnt].name);
      int dp = 2;
      while (tmp[dp] != ')') {
        dp++;
      }
      mock->arg[mock->arg_cnt].name[2] = 0;
      sprintf(mock->arg[mock->arg_cnt].function_pointer_name, "arg%d",
              mock->arg_cnt);
      strcat(mock->arg[mock->arg_cnt].name,
             mock->arg[mock->arg_cnt].function_pointer_name);
      strcat(mock->arg[mock->arg_cnt].name, &tmp[dp]);
      mock->arg_cnt++;
    }
    pos++;
  }
  pos++;
  return pos;
}

static int get_mockables(char mockable[1024][256], const char* filename) {
  char buf[1024];

  /*
   * Run nm on the object file in order to get a list of functions that
   * are actually used in the file.
   */
  FILE* fd = fopen(filename, "r");
  if (NULL == fd) {
    fprintf(stderr, "ERROR: Unable to open '%s'\n", filename);
    return 0;
  }

  int mockable_cnt = 0;

  while (fgets(buf, sizeof(buf), fd)) {
    buf[strlen(buf) - 1] = 0;
    strcpy(mockable[mockable_cnt++], buf);
  }

  fclose(fd);

  return mockable_cnt;
}

static void cproto(const int argc, const char* argv[])
{
  char buf[1024];
  char cproto[256];
  char command[1024];
  char iflags[1024];

  FILE* pd;

  const char* filename = argv[1];
  const char* nmfilename = argv[2];
  const char* cutest_path = argv[3];

  char mockable[1024][256];
  const int mockable_cnt = get_mockables(mockable, nmfilename);

  /*
   * Run cproto to generate prototypes for all possible functions available.
   */
  sprintf(cproto, "cproto -i -s -x -I\"%s\"", cutest_path);
  strcpy(iflags, "");
  for (int i = 3; i < argc; i++) {
    if (('-'  == argv[i][0]) && ('I' != argv[i][1])) {
      fprintf(stderr,
              "ERROR: You can only pass -I-flags with arguments "
              "as optional argumetns to cutest_mock. Not '%s'!",
              argv[i]);
      exit(EXIT_FAILURE);
    }
    strcat(iflags, argv[i]);
  }
  sprintf(command, "%s %s \"%s\" 2>/dev/null | sort -u\n", cproto, iflags,
          filename);

  pd = popen(command, "r");

  if (NULL == pd) {
    fprintf(stderr, "ERROR: Unable to execute command '%s'\n", command);
    return;
  }

  printf("/*\n");
  printf(" * %s", command);
  printf(" *\n");
  while (fgets(buf, sizeof(buf), pd)) {
    char extbuf[256];
    int i;
    int pos = 0;
    int retval = 0;
    int found_mockable = 0;
    if (('/' == buf[0]) && ('*' == buf[1])) {
      continue;
    }

    memset(&mocks.mock[mocks.mock_cnt], 0,
           sizeof(mocks.mock[mocks.mock_cnt]));
    pos += get_return_type(&mocks.mock[mocks.mock_cnt].return_type,
                           &buf[pos]);

    memset(extbuf, 0, sizeof(extbuf));

    retval = get_function_name(extbuf,
                               &buf[pos]);

    int already_defined = 0;
    for (i = 0; i < mocks.mock_cnt; i++) {
      if (0 == strcmp(extbuf, mocks.mock[i].name)) {
        already_defined = 1;
        break;
      }
    }
    if (1 == already_defined) {
      continue;
    }

    strcpy(mocks.mock[mocks.mock_cnt].name, extbuf);

    if (-1 == retval) {
      /* Not a function */
      continue;
    }

    char* name = NULL;
    /* Check if the function is actually used, if not just skip it */
    for (i = 0; i < mockable_cnt; i++) {
      name = mocks.mock[mocks.mock_cnt].name;
      if (0 == strncmp(name, mockable[i], strlen(mockable[i]))) {
        found_mockable = 1;
        break;
      }
    }

    if (0 == found_mockable) {
      continue;
    }

    printf(" * %s\n", name);

    pos += retval;
    pos += get_function_args(&mocks.mock[mocks.mock_cnt], &buf[pos]);
    mocks.mock_cnt++;
  }

  printf(" *\n");
  printf(" */\n\n");

  pclose(pd);
}

/*
 * Mock-ups
 * --------
 *
 * The cutest_mock tool scans the design under test for call() macros,
 * and create a mock-up control stucture, unique for every callable
 * mockable function, so that tests can be fully controlled.
 *
 * The control structures are encapsulated in the global struct
 * instance called 'mocks'.
 *
 * In a test they can be accessed like this::
 *
 *   mocks.<name_of_called_function>.<property>...
 *
 * If you have::
 *
 *   FILE* fp = fopen("filename.c", "r");
 *
 * in your code, a mock called cutest_mock_fopen() will be generated.
 * It will affect the cutest_mock.fopen mock-up control structure.
 *
 * For accurate information please build your <dut>_mocks.h file and
 * inspect the structs yourself.
 *
 * Stubbing
 * --------
 *
 * To stub a function in your design under test you can easily write
 * your own stub in your test-file, just pointing the
 * cutest_mock.<dut>.func function pointer to your stub.
 *
 */
static void print_mock_ctl(cutest_mock_t* mock)
{
  int i;
  printf("  struct {\n");
  printf("    int call_count;\n");
  if (0 != strcmp(mock->return_type.name, "void")) {
    if (mock->return_type.is_struct) {
      printf("    struct %s retval;\n", mock->return_type.name);
    }
    else {
      printf("    %s retval;\n", mock->return_type.name);
    }
  }
  if (mock->return_type.is_struct) {
    printf("    struct %s (*func)(", mock->return_type.name);
  }
  else {
    printf("    %s (*func)(", mock->return_type.name);
  }
  for (i = 0; i < mock->arg_cnt; i++) {
    if (mock->arg[i].type.is_const) {
      printf("const ");
    }
    if (mock->arg[i].type.is_static) {
      printf("static ");
    }
    if (mock->arg[i].type.is_struct) {
      printf("struct ");
    }

    if ((mock->arg[i].type.name[0] == '.') &&
        (mock->arg[i].type.name[1] == '.') &&
        (mock->arg[i].type.name[2] == '.')) {
      printf("%s", mock->arg[i].type.name);
    }
    else {
      printf("%s %s", mock->arg[i].type.name, mock->arg[i].name);
    }
    if (i < mock->arg_cnt - 1) {
      printf(", ");
    }
  }
  printf(");\n");


  if (mock->arg_cnt > 0) {
    printf("    struct {\n");
  }
  for (i = 0; i < mock->arg_cnt; i++) {
    if ((mock->arg[i].type.name[0] == '.') &&
        (mock->arg[i].type.name[1] == '.') &&
        (mock->arg[i].type.name[2] == '.')) {
      continue;
    }
    printf("      %s%s %s;\n",
           (mock->arg[i].type.is_struct ? "struct " : ""),
           mock->arg[i].type.name, mock->arg[i].name);
  }
  if (mock->arg_cnt > 0) {
    printf("    } args;\n");
  }
  printf("  } %s;\n\n", mock->name);
}

static void print_ctls()
{
  int i;
  for (i = 0; i < mocks.mock_cnt; i++) {
    print_mock_ctl(&mocks.mock[i]);
  }
}

static void print_pre_processor_directives(const char* filename) {
  char buf[1024];

  FILE* f = fopen(filename, "r");

  if (NULL == f) {
    fprintf(stderr, "ERROR: Unable to open '%s'\n", filename);
    return;
  }

  while (fgets(buf, sizeof(buf), f)) {
    if ('#' != buf[0]) {
      continue;
    }
    printf("%s", buf);
  }
  fclose(f);
  printf("\n");
}

static void print_forward_declaration(cutest_mock_t* mock)
{
  int i;
  for (i = 0; i < mock->arg_cnt; i++) {
    char buf[128];
    memset(buf, 0, sizeof(buf));
    if (mock->arg[i].type.is_struct) {
      int j;
      printf("struct ");
      for (j = 0; j < (int)strlen(mock->arg[i].type.name); j++) {
        if (mock->arg[i].type.name[j] == '*') {
          break;
        }
        buf[j] = mock->arg[i].type.name[j];
      }
      printf("%s;\n", buf);
    }
  }
}

static void print_forward_declarations()
{
  int i;
  for (i = 0; i < mocks.mock_cnt; i++) {
    print_forward_declaration(&mocks.mock[i]);
  }
}

static void print_dut_declaration(cutest_mock_t* mock)
{
  int i;
  printf("extern ");
  if (mock->return_type.is_const) {
    printf("const ");
  }
  if (mock->return_type.is_struct) {
    printf("struct ");
  }
  printf("%s %s(", mock->return_type.name, mock->name);

  for (i = 0; i < mock->arg_cnt; i++) {
    if (mock->arg[i].type.is_static) {
      printf("static ");
    }
    if (mock->arg[i].type.is_const) {
      printf("const ");
    }
    if (mock->arg[i].type.is_struct) {
      printf("struct ");
    }
    if ((mock->arg[i].type.name[0] == '.') &&
        (mock->arg[i].type.name[1] == '.') &&
        (mock->arg[i].type.name[2] == '.')) {
      printf("...");
    }
    else {
      printf("%s %s", mock->arg[i].type.name, mock->arg[i].name);
    }
    if (i < mock->arg_cnt - 1) {
      printf(", ");
    }
  }
  printf(")");
}

static void print_mock_declaration(cutest_mock_t* mock)
{
  int i;
  if (mock->return_type.is_inline) {
    printf("inline ");
  }
  if (mock->return_type.is_const) {
    printf("const ");
  }
  if (mock->return_type.is_struct) {
    printf("struct ");
  }
  printf("%s cutest_%s(", mock->return_type.name, mock->name);

  for (i = 0; i < mock->arg_cnt; i++) {
    if (mock->arg[i].type.is_static) {
      printf("static ");
    }
    if (mock->arg[i].type.is_const) {
      printf("const ");
    }
    if (mock->arg[i].type.is_struct) {
      printf("struct ");
    }
    if ((mock->arg[i].type.name[0] == '.') &&
        (mock->arg[i].type.name[1] == '.') &&
        (mock->arg[i].type.name[2] == '.')) {
      printf("...");
    }
    else {
      printf("%s %s", mock->arg[i].type.name, mock->arg[i].name);
    }
    if (i < mock->arg_cnt - 1) {
      printf(", ");
    }
  }
  printf(")");
}

static void print_dut_declarations()
{
  int i;
  for (i = 0; i < mocks.mock_cnt; i++) {
    print_dut_declaration(&mocks.mock[i]);
    printf(";\n");
  }
}

static void print_mock_declarations()
{
  int i;
  for (i = 0; i < mocks.mock_cnt; i++) {
    print_mock_declaration(&mocks.mock[i]);
    printf(";\n");
  }
}

static int has_variadic_arg(cutest_mock_t* mock) {
  int i;
  for (i = 0; i < mock->arg_cnt; i++) {
    if ((mock->arg[i].type.name[0] == '.') &&
        (mock->arg[i].type.name[1] == '.') &&
        (mock->arg[i].type.name[2] == '.')) {
      return 1;
    }
  }
  return 0;
}

static void print_mock(cutest_mock_t* mock)
{
  int i;
  print_mock_declaration(mock);
  printf("\n");
  printf("{\n");
  printf("  cutest_mock.%s.call_count++;\n", mock->name);
  for (i = 0; i < mock->arg_cnt; i++) {
    if (mock->arg[i].type.is_const) {
      printf("  cutest_mock.%s.args.%s = (%s%s)%s;\n", mock->name,
             mock->arg[i].name,
             (mock->arg[i].type.is_struct ? "struct " : ""),
             mock->arg[i].type.name, mock->arg[i].name);
    }
    else {
      if ((mock->arg[i].type.name[0] == '.') &&
          (mock->arg[i].type.name[1] == '.') &&
          (mock->arg[i].type.name[2] == '.')) {
        printf("  /* Can't handle va_list arguments yet. Skipping */\n");
      }
      else {
        if (0 == mock->arg[i].is_function_pointer) {
          printf("  cutest_mock.%s.args.%s = %s;\n", mock->name,
                 mock->arg[i].name,
                 mock->arg[i].name);
        }
        else {
          printf("  cutest_mock.%s.args.%s = %s;\n", mock->name,
                 mock->arg[i].function_pointer_name,
                 mock->arg[i].function_pointer_name);
        }
      }
    }
  }
  if (0 != strcmp(mock->return_type.name, "void")) {
    printf("  if (NULL != cutest_mock.%s.func) {\n", mock->name);
    if (1 == has_variadic_arg(mock)) {
      printf("    fprintf(stderr, \"Can't redirect variadic args\\n\");\n");
    }
    else {
      printf("    return cutest_mock.%s.func(", mock->name);
      for (i = 0; i < mock->arg_cnt; i++) {
        if (0 == mock->arg[i].is_function_pointer) {
          printf("%s", mock->arg[i].name);
        }
        else {
          printf("%s", mock->arg[i].function_pointer_name);
        }
        if (i < mock->arg_cnt - 1) {
          printf(", ");
        }
      }
      printf(");\n");
    }
    printf("  }\n");
    printf("  return cutest_mock.%s.retval;\n", mock->name);
  }
  else {
    printf("  if (NULL != cutest_mock.%s.func) {\n", mock->name);
    printf("    cutest_mock.%s.func(", mock->name);
    for (i = 0; i < mock->arg_cnt; i++) {
      printf("%s", mock->arg[i].name);
      if (i < mock->arg_cnt - 1) {
        printf(", ");
      }
    }
    printf(");\n");
    printf("  }\n");
  }
  printf("}\n");
  printf("\n");
}

static void print_mocks()
{
  int i;
  for (i = 0; i < mocks.mock_cnt; i++) {
    print_mock(&mocks.mock[i]);
  }
}

static void print_func_assignments()
{
  int i;
  printf("void cutest_set_mocks_to_original_functions() {\n");
  for (i = 0; i < mocks.mock_cnt; i++) {
    printf("  cutest_mock.%s.func = %s;\n", mocks.mock[i].name, mocks.mock[i].name);
  }
  printf("}\n\n");
}

int main(const int argc, const char* argv[])
{
  char* file;
  FILE *fd;

  if (argc < 4) {
    fprintf(stderr, "ERROR: Missing argument\n");
    printf("USAGE: cutest_mock "
           "<c-source-file> <o-object-file> <cutest-path> [-I flags]\n");
    exit(EXIT_FAILURE);
  }

  memset(&mocks, 0, sizeof(mocks));

  cproto(argc, argv);

  fd = fopen(argv[1], "r");
  const long file_size = get_file_size(fd);

  file = malloc(file_size);
  if (NULL == file) {
    fprintf(stderr, "ERROR: Source file too big, out of memory.");
    exit(EXIT_FAILURE);
  }

  (void)(fread(file, file_size, 1, fd)+1); /* Ignore return warning */

  fclose(fd);

  free(file);

  printf("/*\n"
         " * This file is generated by '%s %s'\n"
         " */\n\n", argv[0], argv[1]);
  /*
  print_includes(argv[1]);
  */
  print_forward_declarations();
  printf("\n");

  print_pre_processor_directives(argv[1]);

  /* Extern dut-function declarations */
  print_dut_declarations();
  printf("\n");

  /* Mock function declarations */
  print_mock_declarations();
  printf("\n");

  /* Produce the cutest_mock struct instance */
  printf("struct {\n\n");
  printf("  int keep_the_struct_with_contents_if_no_mocks_generated;"
         "\n\n");

  print_ctls();

  printf("} cutest_mock;\n\n");

  print_mocks();
  printf("\n");

  print_func_assignments();
  printf("\n");

  return 0;
}

#endif /* CUTEST_MOCK_MAIN */

/*********************************************************************
   ----    ____ ____ _____ ____ ____ _____   ____ ____ ____ __   -----
   ----   / __// / //_  _// __// __//_  _/  / _ // _ //   /| /.' -----
   ----  / /_ / / /  / / / __//_  /  / /   / __//   \/ / /_' \   -----
   ---- /___//___/  /_/ /___//___/  /_/   /_/  /_/_//___//_/_/   -----
 *
 * CUTest proxification tool
 * =========================
 *
 * The cutest_prox tool reads an elaborated assembler source file and
 * a file containing a list of mockable functions to produce a new
 * assembler output with all calls to local (or other) functions
 * replaced by CUTest mocks.
 *
 * How to build the tool
 * ---------------------
 *
 * Just include the ``cutest.mk`` makefile in your own Makefile in your
 * folder containing the source code for the ``*_test.c`` files.
 *
 * The tool is automatically compiled when making the check target.
 * But if you want to make the tool explicitly just call::
 *
 *  $ make cutest_prox
 *
 * Usage
 * -----
 *
 * If you *need* to run the tool manually this is how::
 *
 *  $ ./cutest_prox dut_mockables.s dut_mockables.lst
 *
 * And an assembler file will be outputed to stdout.
 *
 * */

#ifdef CUTEST_PROX_MAIN

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int get_mockables(char mockable[1024][256], const char* filename) {
  char buf[1024];

  /*
   * Run nm on the object file in order to get a list of functions that
   * are actually used in the file.
   */
  FILE* fd = fopen(filename, "r");
  if (NULL == fd) {
    fprintf(stderr, "ERROR: Unable to open '%s'\n", filename);
    return 0;
  }

  int mockable_cnt = 0;

  while (fgets(buf, sizeof(buf), fd)) {
    buf[strlen(buf) - 1] = 0;
    strcpy(mockable[mockable_cnt++], buf);
  }

  fclose(fd);

  return mockable_cnt;
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    fprintf(stderr, "ERROR: Missing argument\n");
    printf("USAGE: cutest_run_tests "
           "<dut-asm-source-file> <mockables-list-file>\n");
    exit(EXIT_FAILURE);
  }

  printf("/*\n"
         " * This file is generated by '%s %s %s'.\n"
         " *\n"
         " * Compile this program to get a tool to proxify an "
         "assembly file.\n"
         " *\n"
         " */\n\n", argv[0], argv[1], argv[2]);
  printf("#include <stdlib.h>\n");
  printf("#include <stdio.h>\n");
  printf("#include <string.h>\n");

  FILE* fd = fopen(argv[1], "r");
  if (NULL == fd) {
    fprintf(stderr, "ERROR: Unable to open file '%s'\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  char mockable[1024][256];
  const int mockable_cnt = get_mockables(mockable, argv[2]);

  while(!feof(fd)) {
    int i;
    char buf[1024];
    if (NULL == fgets(buf, sizeof(buf), fd)) {
      if (feof(fd)) {
        break;
      }
      fprintf(stderr, "ERROR: Unable to read file '%s'\n", argv[1]);
      exit(EXIT_FAILURE);
    }
    buf[strlen(buf) - 1] = 0;
    for (i = 0; i < mockable_cnt; i++) {
      int found = 0;
      const char *s = strstr(buf, mockable[i]);
      const int mocklen = strlen(mockable[i]);
      if (NULL != s) {
        const int pos = s - buf;
        if (pos < 3) {
          continue;
        }
        const int end = pos + mocklen - 1;
        if ((buf[pos - 1] == '\t') && (buf[end + 1] == '\t')) {
          char newbuf[1024];
          /* This might be a keeper... If run on somehting else than x64...
          fprintf(stderr, "DEBUG: found '%s' on '%s'\n", mockable[i], buf);
          */
          buf[pos] = 0;
          sprintf(newbuf, "%scutest_%s%s", buf, mockable[i], &buf[end + 1]);
          strcpy(buf, newbuf);
          found = 1;
          break;
        }
      }
      if (1 == found) {
        break;
      }
    }
    puts(buf);
  }

  fclose(fd);

  return 0;
}

#endif /* CUTST_PROX_MAIN */

/*********************************************************************
   ------    ____ ____ _____ ____ ____ _____   ____ ____ _ __ --------
   ------   / __// / //_  _// __// __//_  _/  / _ // / //|/ / -------
   ------  / /_ / / /  / / / __//_  /  / /   /   |/ / //   /  -------
   ------ /___//___/  /_/ /___//___/  /_/   /_/_//___//_|_/   -------
 *
 * CUTest test runner generator
 * ============================
 *
 * The cutest_run tool will parse your test suite and produce an
 * executable program with some command line options to enable you to
 * control it a little bit.
 *
 * How to build the tool
 * ---------------------
 *
 * Just include the ``cutest.mk`` makefile in your own Makefile in your
 * folder containing the source code for the ``*_test.c`` files.
 *
 * The tool is automatically compiled when making the check target.
 * But if you want to make the tool explicitly just call::
 *
 *  $ make cutest_run
 *
 * Usage
 * -----
 *
 * If you *need* to run the tool manually this is how::
 *
 *  $ ./cutest_run dut_test.c dut_mocks.h
 *
 * And it will scan the test suite source-code for uses of the `test()`
 * macro and output a C program containing everything needed to test
 * your code alongside with the `cutest.h` file.
 *
 * However, if you use the Makefile targets specified in the
 * beginning of this document you will probably not need to run it
 * manually.
 *
 */

#ifdef CUTEST_RUN_MAIN

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * The test runner program
 * -----------------------
 *
 * The generated test runner program will inventory all the tests in
 * the specified suite and run them in the order that they appear in
 * the suite.
 *
 * The first thing that happens is the Startup process, then all
 * tests are run in isolation, followed by the Shutdown process.
 */
int main(int argc, char* argv[]) {
  if (argc < 3) {
    fprintf(stderr, "ERROR: Missing argument\n");
    printf("USAGE: cutest_run_tests "
           "<test-c-source-file> <c-mock-header-file>\n");
    exit(EXIT_FAILURE);
  }

  printf("/*\n"
         " * This file is generated by '%s %s %s'.\n"
         " *\n"
         " * Compile this program to get a test-case executor.\n"
         " *\n"
         " */\n\n", argv[0], argv[1], argv[2]);
  printf("#include <stdlib.h>\n");
  printf("#include <stdio.h>\n");
  printf("#include <string.h>\n");
  printf("\n");
  printf("#define main MAIN\n");
  printf("#include \"%s\" "
         "/* Mock-up functions and design under test */\n", argv[2]);
  printf("#include \"%s\" /* Test-cases */\n", argv[1]);
  printf("#undef main\n");
  printf("\n");
  printf("int main(int argc, char* argv[])\n"
         "{\n"
         "  cutest_startup(argc, argv, \"%s\");\n\n", argv[1]);

  FILE *fd = fopen(argv[1], "r");
  while (!feof(fd)) {
    char buf[1024];
    if (NULL == fgets(buf, 1024, fd)) {
      break; /* End of file */
    }
    if (('t' == buf[0]) &&
        ('e' == buf[1]) &&
        ('s' == buf[2]) &&
        ('t' == buf[3]) &&
        ('(' == buf[4])) {
      int name_pos = 5;
      int i;
      for (i = 5; i < (int)strlen(buf); i++) {
        if (')' == buf[i]) {
          char *start = &buf[5];
          int name_len = i - name_pos;
          buf[name_pos + name_len] = '\0';
          printf("  cutest_execute_test(cutest_%s, \"%s\", 0);\n",
                 start, start);
          break;
        }
      }
    }
    else if (('m' == buf[0]) &&
             ('o' == buf[1]) &&
             ('d' == buf[2]) &&
             ('u' == buf[3]) &&
             ('l' == buf[4]) &&
             ('e' == buf[5]) &&
             ('_' == buf[6]) &&
             ('t' == buf[7]) &&
             ('e' == buf[8]) &&
             ('s' == buf[9]) &&
             ('t' == buf[10]) &&
             ('(' == buf[11])) {
      int name_pos = 12;
      int i;
      for (i = 12; i < (int)strlen(buf); i++) {
        if (')' == buf[i]) {
          char *start = &buf[12];
          int name_len = i - name_pos;
          buf[name_pos + name_len] = '\0';
          printf("  cutest_execute_test(cutest_module_%s, \"%s\", 1);\n",
                 start, start);
          break;
        }
      }
    }
  }
  fclose(fd);
  printf("  cutest_shutdown(\"%s\");\n\n", argv[1]);
  printf("  return cutest_exit_code;\n"
         "}\n");
  return 0;
}

#endif /* CUTEST_RUN_MAIN */

#endif
