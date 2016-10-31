/***************************************************************************
   ------------------    ____ ____ _____ ____ ____ _____ ------------------
   ------------------   / __// / //_  _// __// __//_  _/ ------------------
   ------------------  / /_ / / /  / / / __//_  /  / /   ------------------
   ------------------ /___//___/  /_/ /___//___/  /_/    ------------------
 *
 * CUTest - The C Unit Test framework
 * ==================================
 *
 * Author: AiO (AiO Secure Teletronics)
 * Project site: https://github.com/aiobofh/cutest
 *
 * Thank you for downloading the CUTest framework! I hope it will make your
 * software development, using test-driven design an easier task.
 *
 * CUTest is a C testing framework written in pure C. The idea behind
 * CUTest is to provide a platform independent C Unit Test framework,
 * but I guess it will only work in Linux for GCC anyway :). It's the
 * thought that counts. Please join me and port it to other enviroments.
 *
 * The CUTest framework is tightly bound to a very specific build system
 * layout too. So let's admit that GNU Make is also needed.
 *
 * Features
 * --------
 *
 * * Automated generation of controllable mocks for all C-functions, with
 *   code footprint in the form of the inclusion of call.h and usage of the
 *   call() macro)
 * * C-Function stubbing
 * * Generic asserts in 1, 2 and 3 argument flavors.
 * * JUnit XML reports for Jenkins integration
 * * Very few dependencies to other tools (`echo`, `gcc`, `make` and `cproto`)
 * * In-line documentation to ReSTructured Text or HTML
 *   (requires additional tools: `grep`, `sed` and `rst2html`)
 *
 * In-line documentation to ReSTructured Text and/or HTML
 * ------------------------------------------------------
 *
 * You can always read the cutest.h file, since it's the only one around :D.
 *
 * ... or you can generate a ReSTructured Text-style output using by adding
 * another target to your Makefile::
 *
 *   # Print the CUTest manual
 *   cutest_help.rst: cutest.h
 *       $(Q)grep -e '^ * ' $< |                 \
 *       grep -v '/' |                                 \
 *       grep -v -e '^  ' | \
 *       sed -e 's/^ \* //g;s/^ \*$//g' > $@
 *
 * ... or if you prefer HTML you can add this too::
 *
 *   # Print the CUTest manuial as HTML
 *   cutest_help.html: cutest_help.rst
 *       rst2html $< > $@
 *
 * Flat directory ''structure''
 * ----------------------------
 *
 * The CUTest framework expects you to work with your design and tests in
 * the same folder, however this is not mandatory. You can tweak the example
 * Makefile snippets here to change that behaviour.
 *
 * However, this is the tested way to structure your design under test and
 * your test suites::
 *
 *   src/cutest.h    <- May be symlink to your local installation of cutest.h
 *   src/call.h      <- May be symlink to your local installation of call.h
 *   :
 *   src/dut.c       <- your program (design under test) (can #include call.h)
 *   src/dut_test.c  <- test suite for dut.c (should #include cutest.h)
 *   src/Makefile
 *
 * ... So keep your clean:-target clean ;).
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
 *   test(foo_shall_call_the_adder_function_once_with_correct_arguments) {
 *     // When calling foo() the call(adder(i, j))-macro will call a mock.
 *     foo(1, 2);
 *     assert_eq(1, cutest_mock.adder.call_count, "adder shall be called once");
 *     assert_eq(1, cutest_mock.adder.args.arg0, "first argument shall be 1");
 *     assert_eq(2, cutest_mock.adder.args.arg1, "second argument shall be 2");
 *   }
 *
 *   test(foo_shall_return_the_adder_functions_result_unmodified) {
 *     cutest_mock.adder.retval = 123456;
 *     assert_eq(123456, foo(1, 2), "foo shall return adder's return value");
 *   }
 *
 * foo.c::
 *
 *   #include "call.h" // Contains a book-mark macro to find mockables.
 *
 *   int adder(int a, int b) { return a + b; }
 *   int foo(int i, int j) { return call(adder(a, b)); }
 *
 * Makefile::
 *
 *   .PRECIOUS: %_mocks.h
 *   # Generate mocks from the call()-macro in a source-file.
 *   %_mocks.h: %.c cutest.h cutest_mock
 *       $(Q)./cutest_mock $< > $@
 *
 *   .PRECIOUS: %_test_run.c
 *   # Generate a test-runner program code from a test-source-file
 *   %_test_run.c: %_test.c %_mocks.h cutest.h cutest_run
 *       $(Q)./cutest_run $(filter-out cutest.h,$^) > $@
 *
 *   # Compile a test-runner from the generate test-runner program code
 *   %_test: %_test_run.c
 *       $(Q)$(CC) $^ $(CUTEST_CFLAGS) -DNDEBUG -o $@
 *
 *   check: $(subst .c,,$(wildcard *_test.c))
 *       $(Q)R=true; for i in $^; do           \
 *         ./$$i $V -j || (rm $$i || R=false);  \
 *       done; echo ""; `$$R`
 *
 * Command line::
 *
 *   $ make foo_test
 *   $ ./foo_test
 *   ...
 *
 *   $ make check
 *   ...
 *
 * It's a lot of rules and targets for one simple test case, but it scales
 * very well for rerunning only needed tests.
 *
 */
#ifndef _CUTEST_H_
#define _CUTEST_H_

/*
 * To compile the test runner you should never ever have `CUTEST_RUN_MAIN`
 * nor `CUTEST_MOCK_MAIN` defined to the compiler. They are used to compile
 * the *CUTest test runner generator* and the *CUTest mock generator*
 * respectively.
 *
 */
#ifndef CUTEST_RUN_MAIN
#ifndef CUTEST_MOCK_MAIN

#define _XOPEN_SOURCE
#include <time.h>

extern struct tm *localtime_r(const time_t *timep, struct tm *result);

/*
 * The call() macro
 * ----------------
 *
 * By default this is defined in the call.h header. This must be used to
 * call _any_ function that is to be possible to replace with a mock using
 * the CUTest-framework.
 *
 * However when writing tests, all call() macros will actually call the
 * corresponding mock-up function instead of the real deal. Hence the call()
 * macro is overreidden by cutest.h inclusion.
 *
 */
#define _CALL_H_ /* Make sure that the call-macro is never included. */
#define call(FUNCTION) cutest_mock_##FUNCTION /* ... Instad use this */

/*
 * The test() macro
 * ----------------
 *
 * Every test is defined with this macro.
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
 *
 */
#define assert_eq_3(EXP, REF, STR)                                 \
  if ((EXP) != (REF)) {                                            \
    sprintf(cutest_stats.error_output,                             \
            "%s %s:%d assert_eq(" #EXP ", " #REF ", " STR ") "     \
            "failed\n", cutest_stats.error_output,                 \
            __FILE__, __LINE__);                                   \
    cutest_assert_fail_cnt++;                                      \
  }

#define assert_eq_2(EXP, REF) \
  if ((EXP) != (REF)) {                                            \
    sprintf(cutest_stats.error_output,                             \
            "%s %s:%d assert_eq(" #EXP ", " #REF ") "              \
            "failed\n", cutest_stats.error_output,                 \
            __FILE__, __LINE__);                                   \
    cutest_assert_fail_cnt++;                                      \
  }

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

/*
 * Test initialization
 * -------------------
 *
 * In between every test() macro the CUTest framework will clear all the
 * mock controls and test framwork state so that every test is run in
 * isolation.
 *
 */
static void cutest_startup(int argc, char* argv[], const char* suite_name)
{
  int i;
  cutest_opts.verbose = 0;
  cutest_opts.junit = 0;
  for (i = 1; i < argc; i++) {
    if ((0 == strcmp(argv[i], "-v")) || (0 == strcmp(argv[i], "--verbose"))) {
      cutest_opts.verbose = 1;
    }
    if ((0 == strcmp(argv[i], "-j")) || (0 == strcmp(argv[i], "--junit"))) {
      cutest_opts.junit = 1;
    }
    if ((0 == strcmp(argv[i], "-n")) || (0 == strcmp(argv[i], "--no-linefeed"))) {
      cutest_opts.no_linefeed = 1;
    }
  }
  memset(cutest_junit_report, 0, sizeof(cutest_junit_report));
  memset(&cutest_stats, 0, sizeof(cutest_stats));
  strcpy(cutest_stats.suite_name, suite_name);
  strcpy(cutest_stats.design_under_test, suite_name);
}

/*
 * Test execution
 * --------------
 *
 * When executing tests the elapsed time for execution is sampled and used
 * in the JUnit report. Depending on command line options an output is printed
 * to the console, either as a short version with '.' for successful test run
 * and 'F' for failed test run, but if set to verbose '-v' '[PASS]' and
 * '[FAIL]' output is produced. What triggers a failure is if an assert_eq()
 * is not fulfilled.
 *
 * If the test runner is started with verbose mode '-v' the offending assert
 * will be printed to the console directly after the fail. If in normal mode
 * all assert-failures will be collected and printed in the shutdown process.
 *
 */
static void cutest_execute_test(void (*func)(), const char *name) {
  time_t start_time = time(NULL);
  time_t end_time;
  double elapsed_time;
  memset(&cutest_mock, 0, sizeof(cutest_mock));
  func();
  if (cutest_opts.verbose) {
    if (cutest_assert_fail_cnt == 0) {
      printf("[PASS]: %s\n", name);
    }
    else {
      printf("[FAIL]: %s\n", name);
      printf("%s", cutest_stats.error_output);
      memset(cutest_stats.error_output, 0, sizeof(cutest_stats.error_output));
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

  /* Ugly-output some JUnit XML for each test-case */
  sprintf(cutest_junit_report,
          "%s    <testcase classname=\"%s\" name=\"%s\" time=\"%f\">\n",
          cutest_junit_report, cutest_stats.design_under_test,
          name, elapsed_time);
  if (cutest_assert_fail_cnt != 0) {
    sprintf(cutest_junit_report,
            "%s      <failure message=\"test failure\">%s</failure>\n",
            cutest_junit_report, "Some assert text");
  }
  sprintf(cutest_junit_report, "%s    </testcase>\n", cutest_junit_report);

  cutest_assert_fail_cnt = 0;
}

/*
 * Shutdown process
 * ----------------
 *
 * At the end of the execution the CUTest test-runner program will output
 * a JUnit XML report if specified with the -j command line option.
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
    /* Add an enter if not running in verbose, to line break after the ... */
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

#endif /* CUTEST_MOCK_MAIN */
#endif /* CUTEST_RUN_MAIN */

/***************************************************************************
   -----    ____ ____ _____ ____ ____ _____   ____   ____ ____ ____ -------
   -----   / __// / //_  _// __// __//_  _/  /    \ /   // __// / / -------
   -----  / /_ / / /  / / / __//_  /  / /   / / / // / // /_ /  <'  -------
   ----- /___//___/  /_/ /___//___/  /_/   /_/_/_//___//___//_/_/   -------
 *
 * CUTest mock generator
 * =====================
 *
 * This is a tool that can be used to generate mock-up functions. It
 * inspects a specified source-code file (written i C language) and looks
 * for uses of the cutest-specific call() macro which should encapsulate
 * every function that is replaceable with a mock when developing code using
 * test-driven design.
 *
 * Requirements
 * ------------
 *
 * To be able to generate well formatted function declarations to mutate
 * into mock-ups this tool make use of the ``cproto`` tool.
 *
 * How to compile the tool
 * -----------------------
 *
 * Makefile::
 *
 *   # Generate a very strange C-program including cutest.h for int main().
 *   cutest_mock.c: cutest.h
 *       echo "#include \"cutest.h\"" > $@
 *
 *   # Build a tool to generate a test runner program.
 *   cutest_mock: cutest_mock.c
 *       $(Q)which cproto >/dev/null || \
 *       (echo "ERROR: cproto is not installed in your path"; false) && \
 *       $(CC) $< $(CUTEST_CFLAGS) -DCUTEST_MOCK_MAIN -o $@
 *
 * Usage
 * -----
 *
 * If you *need* to run the tool manually this is how::
 *
 *   $ ./cutest_mock design_under_test.c
 *
 * And it will scan the source-code for uses of the `call()` macro and
 * output a header file-style text, containing everything needed to test
 * your code alongside with the `cutest.h` file.
 *
 * However, if you use the Makefile targets specified in the beginning of
 * this document you will probably not need to run it manually.
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
  cutest_mock_arg_type_t type;
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

static inline int find_next_call_macro(char* dst, char* whole_file)
{
  static size_t pos = 0;
  char c;
  int is_multi_line_comment = 0;
  int is_end_of_line_comment = 0;
  while (0 != (c = whole_file[pos])) {
    if (('/' == whole_file[pos]) &&
        ('*' == whole_file[pos + 1])) {
      is_multi_line_comment = 1;
    }
    if (is_multi_line_comment) {
      if (('*' == whole_file[pos]) &&
          ('/' == whole_file[pos + 1])) {
        is_multi_line_comment = 0;
      }
      pos++;
      continue;
    }
    if (('/' == whole_file[pos]) &&
        ('/' == whole_file[pos + 1])) {
      is_end_of_line_comment = 1;
    }
    if (is_end_of_line_comment) {
      if (('\n' == whole_file[pos]) ||
          ('\r' == whole_file[pos])) {
        is_end_of_line_comment = 0;
      }
      pos++;
      continue;
    }

    if ((',' == c) ||
        ('(' == c) ||
        (' ' == c) ||
        ('=' == c)) {
      if (('c' == whole_file[pos + 1]) &&
          ('a' == whole_file[pos + 2]) &&
          ('l' == whole_file[pos + 3]) &&
          ('l' == whole_file[pos + 4]) &&
          ('(' == whole_file[pos + 5])) {
        char* start = &whole_file[(pos += 6)];
        char* end;
        while ('(' != whole_file[pos++])
          ;
        end = &whole_file[pos - 1];
        assert('(' == whole_file[pos - 1]);
        strncpy(dst, start, end - start);
        return 1;
      }
    }
    pos++;
  }
  return 0;
}

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

static int get_return_type(cutest_mock_return_type_t* return_type,
                           const char* buf)
{
  int pos = 0;
  int dst_pos = 0;
  pos += skip_white_spaces(&buf[pos]);
  while (' ' != buf[pos]) {
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
      return_type->is_struct = 1;
      pos += 7;
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
    while ((' ' != buf[pos]) && (',' != buf[pos]) && (')' != buf[pos])) {
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
    while (',' != buf[pos] && ')' != buf[pos]) {
      mock->arg[mock->arg_cnt].name[dst_pos] = buf[pos];
      dst_pos++;
      pos++;
    }
    if (0 != strcmp(mock->arg[mock->arg_cnt].type.name, "void")) {
      if ('\0' == mock->arg[mock->arg_cnt].name[0]) {
        sprintf(mock->arg[mock->arg_cnt].name, "arg%d", mock->arg_cnt);
      }
      mock->arg_cnt++;
    }
    pos++;
  }
  pos++;
  return pos;
}

static void cproto(const char* filename)
{
  char buf[1024];
  char command[1024];
  FILE* pd;

  sprintf(command, "cproto -i -s -x \"%s\"\n", filename);

  pd = popen(command, "r");

  if (NULL == pd) {
    fprintf(stderr, "ERROR: Unable to execute command '%s'\n", command);
    return;
  }

  while (fgets(buf, sizeof(buf), pd)) {
    int pos = 0;
    int retval = 0;
    if (('/' == buf[0]) && ('*' == buf[1])) {
      continue;
    }

    memset(&mocks.mock[mocks.mock_cnt], 0,
           sizeof(mocks.mock[mocks.mock_cnt]));
    pos += get_return_type(&mocks.mock[mocks.mock_cnt].return_type,
                           &buf[pos]);
    retval = get_function_name(mocks.mock[mocks.mock_cnt].name, &buf[pos]);
    if (-1 == retval) {
      /* Not a function */
      continue;
    }
    pos += retval;
    pos += get_function_args(&mocks.mock[mocks.mock_cnt], &buf[pos]);
    mocks.mock_cnt++;
  }

  pclose(pd);
}

/*
 * Mock-ups
 * --------
 *
 * The cutest_mock tool scans the design under test for call() macros, and
 * create a mock-up control stucture, unique for every callable mockable
 * function, so that tests can be fully controlled.
 *
 * The control structures are encapsulated in the global struct instance
 * called 'mocks'.
 *
 * In a test they can be accessed like this::
 *
 *   mocks.<name_of_called_function>.<property>...
 *
 * If you have::
 *
 *   FILE* fp = call(fopen("filename.c", "r"));
 *
 * in your code, a mock called cutest_mock_fopen() will be generated. It
 * will affect the cutest_mock.fopen mock-up control structure.
 *
 * For accurate information please build your <dut>_mocks.h file and
 * inspect the structs yourself.
 *
 * Stubbing
 * --------
 *
 * To stub a function encapsulated in a call() macro in your design under
 * test you can easily write your own stub in your test-file, just pointing
 * the cutest_mock.<dut>.func function pointer to your stub.
 *
 */
static void print_mock_ctl(cutest_mock_t* mock)
{
  int i;
  printf("  struct {\n");
  printf("    int call_count;\n");
  if (0 != strcmp(mock->return_type.name, "void")) {
    printf("    %s retval;\n", mock->return_type.name);
  }
  printf("    %s (*func)(", mock->return_type.name);
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

    printf("%s %s", mock->arg[i].type.name, mock->arg[i].name);
    if (i < mock->arg_cnt - 1) {
      printf(", ");
    }
  }
  printf(");\n");


  if (mock->arg_cnt > 0) {
    printf("    struct {\n");
  }
  for (i = 0; i < mock->arg_cnt; i++) {
    printf("      %s%s %s;\n",
           (mock->arg[i].type.is_struct ? "struct " : ""),
           mock->arg[i].type.name, mock->arg[i].name);
  }
  if (mock->arg_cnt > 0) {
    printf("    } args;\n");
  }
  printf("  } %s;\n\n", mock->name);
}

static void print_ctl(const char* function_name)
{
  int i;
  for (i = 0; i < mocks.mock_cnt; i++) {
    if (0 == strcmp(mocks.mock[i].name, function_name)) {
      print_mock_ctl(&mocks.mock[i]);
    }
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

static void print_forward_declarations(const char* function_name)
{
  int i;
  for (i = 0; i < mocks.mock_cnt; i++) {
    if (0 == strcmp(mocks.mock[i].name, function_name)) {
      print_forward_declaration(&mocks.mock[i]);
    }
  }
}

static void print_mock_declaration(cutest_mock_t* mock)
{
  int i;
  if (mock->return_type.is_static) {
    printf("static ");
  }
  if (mock->return_type.is_inline) {
    printf("inline ");
  }
  if (mock->return_type.is_const) {
    printf("const ");
  }
  printf("%s cutest_mock_%s(", mock->return_type.name, mock->name);

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
    printf("%s %s", mock->arg[i].type.name, mock->arg[i].name);
    if (i < mock->arg_cnt - 1) {
      printf(", ");
    }
  }
  printf(")");
}

static void print_mock_declarations(const char* function_name)
{
  int i;
  for (i = 0; i < mocks.mock_cnt; i++) {
    if (0 == strcmp(mocks.mock[i].name, function_name)) {
      print_mock_declaration(&mocks.mock[i]);
      printf(";\n");
    }
  }
}

static void print_mock(cutest_mock_t* mock)
{
  int i;
  print_mock_declaration(mock);
  printf("\n");
  printf("{\n");
  printf("  cutest_mock.%s.call_count++;\n", mock->name);
  for (i = 0; i < mock->arg_cnt; i++) {
    printf("  cutest_mock.%s.args.%s = (%s%s)%s;\n", mock->name,
           mock->arg[i].name,
           (mock->arg[i].type.is_struct ? "struct " : ""),
           mock->arg[i].type.name, mock->arg[i].name);
  }
  if (0 != strcmp(mock->return_type.name, "void")) {
    printf("  if (NULL != cutest_mock.%s.func) {\n", mock->name);
    printf("    return cutest_mock.%s.func(", mock->name);
    for (i = 0; i < mock->arg_cnt; i++) {
      printf("%s", mock->arg[i].name);
      if (i < mock->arg_cnt - 1) {
        printf(", ");
      }
    }
    printf(");\n");
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

static void print_mocks(const char* function_name)
{
  int i;
  for (i = 0; i < mocks.mock_cnt; i++) {
    if (0 == strcmp(mocks.mock[i].name, function_name)) {
      print_mock(&mocks.mock[i]);
    }
  }
}

int main(int argc, char* argv[])
{
  char* file;
  char called_functions[1024][128];
  int called_functions_cnt = 0;
  char function_name[1024];
  FILE *fd;
  int i;

  if (argc != 2) {
    fprintf(stderr, "ERROR: Missing argument\n");
    printf("USAGE: cutest_mock <c-source-file>\n");
    exit(EXIT_FAILURE);
  }

  memset(&mocks, 0, sizeof(mocks));

  cproto(argv[1]);

  fd = fopen(argv[1], "r");
  const long file_size = get_file_size(fd);

  file = malloc(file_size);
  if (NULL == file) {
    fprintf(stderr, "ERROR: Source file too big, out of memory.");
    exit(EXIT_FAILURE);
  }

  memset(called_functions, 0, sizeof(called_functions));

  (void)(fread(file, file_size, 1, fd)+1); /* Hehe, ignore return warning */

  fclose(fd);

  memset(function_name, 0, sizeof(function_name));

  while (find_next_call_macro(function_name, file)) {
    int already_found_function = 0;
    for (i = 0; i < called_functions_cnt; i++) {
      if (0 == strcmp(called_functions[i], function_name)) {
        already_found_function = 1;
        break;
      }
    }
    if (0 == already_found_function) {
      strcpy(called_functions[called_functions_cnt++], function_name);
    }
    memset(function_name, 0, sizeof(function_name));
  }

  free(file);

  printf("/*\n"
         " * This file is generated by '%s %s'\n"
         " */\n\n", argv[0], argv[1]);
  printf("#define _CALL_H_ /* Make sure that the call macro stays modified */\n");
  printf("#define call(FUNCTION) cutest_mock_##FUNCTION\n\n");

  for (i = 0; i < called_functions_cnt; i++) {
    print_forward_declarations(called_functions[i]);
  }
  printf("\n");

  print_pre_processor_directives(argv[1]);

  /* Mock function declarations */

  for (i = 0; i < called_functions_cnt; i++) {
    print_mock_declarations(called_functions[i]);
  }
  printf("\n");

  /* Include the design under test */
  printf("#include \"%s\" /* Include design under test. */\n\n", argv[1]);

  /* Produce the cutest_mock struct instance */
  printf("struct {\n\n");
  printf("  int keep_the_struct_with_contents_if_no_mocks_generated;\n\n");

  for (i = 0; i < called_functions_cnt; i++) {
    print_ctl(called_functions[i]);
  }

  printf("} cutest_mock;\n\n");

  for (i = 0; i < called_functions_cnt; i++) {
    print_mocks(called_functions[i]);
  }

  printf("\n");

  return 0;
}

#endif /* CUTEST_MOCK_MAIN */

/***************************************************************************
   ---------    ____ ____ _____ ____ ____ _____   ____ ____ _ __ ----------
   ---------   / __// / //_  _// __// __//_  _/  / _ // / //|/ / ----------
   ---------  / /_ / / /  / / / __//_  /  / /   /   |/ / //   /  ----------
   --------- /___//___/  /_/ /___//___/  /_/   /_/_//___//_|_/   ----------
 *
 * CUTest test runner generator
 * ============================
 *
 * The cutest_run tool will parse your test suite and produce an executable
 * program with some command line options to enable you to control it a
 * little bit.
 *
 * How to build the tool
 * ---------------------
 *
 * Makefile::
 *
 *   # Generate a very strange C-program including cutest.h for int main().
 *   cutest_run.c: cutest.h Makefile
 *     $(Q)echo "#include \"cutest.h\"" > $@
 *
 *   # Build a tool to generate a test suite runner.
 *   cutest_run: cutest_run.c
 *     $(Q)$(CC) $< $(CUTEST_CFLAGS) -DCUTEST_RUN_MAIN -o $@
 *
 * Usage
 * -----
 *
 * If you *need* to run the tool manually this is how::
 *
 *   $ ./cutest_run dut_test.c dut_mocks.h
 *
 * And it will scan the test suite source-code for uses of the `test()`
 * macro and output a C program containing everything needed to test
 * your code alongside with the `cutest.h` file.
 *
 * However, if you use the Makefile targets specified in the beginning of
 * this document you will probably not need to run it manually.
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
 * The generated test runner program will inventory all the tests in the
 * specified suite and run them in the order that they appear in the suite.
 *
 * The first thing that happens is the Startup process, then all tests are
 * run in isolation, followed by the Shutdown process.
 */
int main(int argc, char* argv[]) {
  if (argc < 3) {
    fprintf(stderr, "ERROR: Missing argument\n");
    printf("USAGE: cutest_run_tests <test-c-source-file> <c-mock-header-file>\n");
    exit(EXIT_FAILURE);
  }

  printf("/*\n"
         " * This file is generated by '%s %s %s'.\n"
         " *\n"
         " * Compile this program to get a test-case executor binary.\n"
         " *\n"
         " */\n\n", argv[0], argv[1], argv[2]);
  printf("#include <stdlib.h>\n");
  printf("#include <stdio.h>\n");
  printf("#include <string.h>\n");
  printf("\n");
  printf("#include \"%s\" /* Mock-up functions and design under test */\n", argv[2]);
  printf("#include \"%s\" /* Test-cases */\n", argv[1]);
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
          printf("  cutest_execute_test(cutest_%s, \"%s\");\n", start, start);
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
