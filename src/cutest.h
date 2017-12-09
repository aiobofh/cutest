/*********************************************************************
    CUTest header file - To be included in your test suite
    Copyright (C) 2017 Joakim Ekblad - AiO Secure Teletronics

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define CUTEST_VERSION "1.0.3"

/*********************************************************************
 *::
 *
 *  The____ ____ _____ ____ ____ _____
 *    / __// / //_  _// __// __//_  _/
 *   / /_ / / /  / / / __//_  /  / /
 *  /___//___/  /_/ /___//___/  /_/Framework
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
 * thought that counts. Please join me and port it to other environments.
 *
 * The CUTest framework is tightly bound to a very specific build
 * system layout too. So let's admit that GNU Make is also needed.
 *
 * A huge tip is to check out the examples folder, it contains both
 * naive, simple and realistic examples of various CUTest usages.
 *
 * Version history
 * ---------------
 *
 * * v1.0.3 2017-11-18 Portability improvements and ease-of-use
 *
 *   - Add warnings for files that are missing test cases
 *   - Add warnings for left-over test cases for removed files
 *   - Segfault-recovery (error-status)
 *   - Support for automatic stubbing/mocking on PowerPC
 *   - Clang support
 *   - Bug-fix for error/fail output in the JUnit format
 *   - Support for -fsanitize (works best for clang, currently)
 *   - GCC 4 portability
 *   - Aproximate calls to real functions with variadic arguments, like
 *     printf, sprintf, fprintf in module tests
 *
 * * v1.0.2 2017-08-30 Release work flow fix, skipping and output fix
 *
 *   - Fixed the documentation generator to be run before release
 *   - Made the release build more determenistic and reduced text output
 *   - Reduced coverage text report to console to only show non-covered
 *   - Enabled coverage reports also when running test through valgrind
 *
 * * v1.0.1 2017-08-15 Fix-up release
 *
 *   - Fixed release date and documentation
 *   - Improved Makefile for release handling
 *   - Lenient (type-agnostic) asserts enabled by default
 *   - Refactored the assert_eq mechanism
 *   - Allow for higher warning level by default when lenient asserts
 *     are used
 *
 * * v1.0.0 2017-06-16 Initial release
 *
 *   - Implementation of pure C programs to generate suite specific
 *     frameworks in the src-directory
 *   - Implementation of the testing framework and easy-to-read
 *     examples in the examples-directory
 *   - Development of "includable" Makefiles for test-running and
 *     code coverage calculations
 *   - Authoring of a full documentation
 *
 * Features
 * --------
 *
 * * Automated generation of controllable mocks for all C-functions
 *   called by the design under test. (x86 and ARM tested)
 * * Automatic mocking of all function calls in a function under test
 * * C-Function stubbing
 * * Generic asserts in 1, 2 and 3 argument flavors.
 * * JUnit XML reports for Jenkins integration
 * * Very few dependencies to other tools (``echo``, ``gcc``, ``as``,
 *   ``make``, ``which``, ``grep``, ``sed``, ``rst2html``, ``less``,
 *   ``nm`` ``gcovr`` and ``cproto``)
 * * Code-coverage JUnit reports (requires ``gcovr``, ``egrep``)
 * * In-line documentation to ReSTructured Text or HTML
 *   (requires additional tools: ``grep``, ``sed`` and ``rst2html``)
 * * Memory leakage detection using Valgrind (requires ``valgrind``)
 * * Automatic renaming of ``main()`` to ``MAIN()`` in your design under
 *   test to make it reachable for testing.
 * * Type-aware generic ``assert_eq()``-macros (CUTEST_LENIENT_ASSERTS)
 * * This quite comprehensive documentation
 * * Jenkins/CI-friendly output formats
 *
 * Organize your directories
 * -------------------------
 *
 * The CUTest framework make some assumptions, but should be fairly
 * flexible. By default the paths are set to support a flat structure
 * with test-case source files and design under test source files in
 * the same folder.
 *
 * However you MUST name your test-case source file as the
 * corresponding design under test source file.
 *
 * So... If you have a file ``dut.c`` you need a ``dut_test.c`` file
 * to test the functions in the ``dut.c`` file.
 *
 * Here is a flat example::
 *
 *   src/dut.c       <- your program (design under test)
 *   src/dut_test.c  <- test suite for dut.c (must #include cutest.h)
 *   src/Makefile    <- build your system and run tests (include cutest.mk)
 *
 * You should apply a ``clean``-target in your ``Makefile`` with double
 * colon so that the ``clean``-target in the ``cutest.mk`` is also
 * evaluated when you do ``make clean`` to cleanup artifacts.
 *
 * ... So keep your clean:-target clean ;).
 *
 * Some more complex examples
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^
 *
 * Most programmers have their own ideas on what a neat directory
 * structure should look like for their projects and with their
 * perspective
 *
 * I will try to show some scenarios that CUTest support.
 *
 * Separate folders for source-code and test-code used from top-level
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * If you have your top-make-file in the my_project-folder, and you
 * usually build your system and run tests from there you should set-up
 * ``CUTEST_TEST_DIR=test`` and ``CUTEST_SRC_DIR=src``::
 *
 *   my_project/Makefile         <- build your system and run tests
 *   my_project/src/dut.c        <- your program (design under test)
 *   my_project/test/dut_test.c  <- test suite for ../src/dut.c
 *
 * Usage::
 *
 *   $ cd my_project
 *   $ make check
 *
 * Separate folders for source-code and test-code used from test-folder
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * If you have your test-make-file in the test-folder and only build tests
 * from there, but build your system from the source-folder you should do
 * a set-up in your test/Makefile like this: ``CUTEST_TEST_DIR=./`` which
 * is default, and ``CUTEST_SRC_DIR=../src``. This approach keep your
 * product code completely free from test-related stuff - even the build
 * system remain unchanged::
 *
 *   my_project/src/Makefile    <- build your system
 *   my_project/src/dut.c       <- your program (design under test)
 *   my_project/test/Makefile   <- run your tests (make check)
 *   my_project/test/dut_test.c <- test suite for ../src/dut.c
 *
 * Usage::
 *
 *   $ cd my_project/test
 *   $ make check
 *
 * As you can see, it should be possible to arrange your project folder in
 * the way you want.
 *
 * Include paths
 * -------------
 *
 * If you have many ``-I../path/to/somewhere`` passed to the build of
 * your project collect all ``-I``-flags into the ``CUTEST_IFLAGS``
 * variable before inclusion of ``cutest.mk`` and the include paths
 * will be passed on to ``cproto`` and the test-runner build
 * automatically. Hopefully this is simplifying your integration a bit.
 *
 * Example
 * -------
 *
 * I prefer example-based documentation so this is also what I provide
 * here. You will see many code-snippets (hopefully all of them are
 * in good shape). This example is a generic example showing how to
 * arrange your tests in a test-suite that corresponds to a file with the
 * design under test.
 *
 * foo_test.c::
 *
 *   // Test-suite for foo.c
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
 *   // My awesome mathimatical helpers
 *
 *   int adder(int a, int b) { return a + b; }
 *   int foo(int i, int j) { return adder(a, b); }
 *
 * Makefile for a simple directory structure::
 *
 *   #
 *   # Makefile
 *   #
 *
 *   include /path/to/cutest/src/cutest.mk
 *
 *   # The following lines has nothing to do with CUTest
 *
 *   %.o: %.c # Default target to build objects from sources
 *           @$(CC) -o $@ -c $< $(CFLAGS)
 *
 *   clean:: # Notice the double colon here (also clean in cutest.mk)
 *           @$(RM) *.o
 *
 *
 * Makefile for automatically downloading cutest into your project::
 *
 *   #
 *   # Makefile
 *   #
 *
 *   -include cutest/src/cutest.mk
 *
 *   all:
 *      @make -s cutest && \  # Always make sure we have CUTest
 *      make -s check && \    # Always run all unit-tests
 *      make -s my_program    # Then compile our my_program
 *
 *   cutest: # Download cutest v1.0.0 by cloning the GitHub repo
 *      @git clone -b v1.0.0 https://github.com/aiobofh/cutest.git
 *
 *   %.o: %.c # Default target to build objects from sources
 *           @$(CC) -o $@ -c $< $(CFLAGS)
 *
 *   my_program: foo.o main.o
 *           @$(CC) -o $@ $^ $(CFLAGS)
 *
 *   clean::
 *      @$(RM) *.o cutest my_program
 *
 * Or you can point to a specific branch or tag in the cutest.git
 * repository using the ``-b <name>`` flag to ``git clone``.
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
 * Command line to run all tests with Valgrind memory leakage checks::
 *
 *   $ make valgrind
 *   ...
 *
 * There are more examples available in the examples folder.
 *
 * Command line to remove your current cutest installation (clean-up)::
 *
 *   $ rm -rf cutest
 *   $ make cutest
 *   ...
 *
 * This will remove your currently cloned version of cutest and download
 * a new one. Don't add the ``cutest`` folder to your own project-
 * repository, unless you have very specific needs. E.g.: No internet
 * connection on the development machines or you truly want an older
 * version at all times! A good practice is to put cutest it in your
 * ``.gitignore`` file if you are using Git.
 *
 * Extend linking dependencies to your original code in other files
 * ----------------------------------------------------------------
 *
 * In many situations your test-suite just call the function under test,
 * and the function itself calls other functions. These functions can be
 * defined in the same file as the function under test, or somewhere
 * else. The first case is simple for CUTest to find, however if you call
 * functions in an API with code defined in some other file or library
 * you need to help CUTest out. This is done in your ``Makefile`` that
 * includes the ``cutest.mk`` file.
 *
 * CUTest must know the implementation to be able to make calls to it if
 * you currently want the mock-up function to call the actual function.
 * For example when writing a module-test or integration-test.
 *
 * Let's say that you have two files ``other.c`` and ``this.c`` and you
 * are developing the ``this.c`` file (using test-driven design, obviously)
 * the function you're writing is calling the ``other_func()`` from the
 * ``other.c`` file, declared in ``other.h`` which ``this.c`` includes::
 *
 *   #include "other.h"
 *
 *   int this_func() {
 *     :
 *     other_funct();
 *     :
 *   }
 *
 * When the build-system links the ``this_test`` executable there is no
 * good way (currently) to link the ``other.c`` file to the ``this_test``
 * executable. But you can add the dependency yourself by adding it in the
 * Makefile like so::
 *
 *   -include "cutest.mk"
 *
 *   this_test: other.c and_another.c
 *
 * And the dependency is handled in the ``cutest.mk`` file when it sets
 * up the ``this_test`` build target.
 *
 * .. note:: This will build the ``other.c`` with the CUTEST_CFLAGS that
 *           might be a little bit harsher than you're used to, so you can
 *           get a shit-load of warnings you've never seen before.
 *
 * In-line documentation to ReSTructured Text and/or HTML
 * ------------------------------------------------------
 *
 * You can always read the ``cutest.h`` file, since it's the only one
 * around.
 *
 * When you have included the ``cutest.mk`` makefile in your own
 * ``Makefile`` you can build the documentation using::
 *
 *   $ make cutest_help       # Will print out the manual to console
 *   $ make cutest_help.html  # Generate a HTML document
 *   $ make cutest_help.rst   # Generate a RST document
 *
 */
#ifndef _CUTEST_H_
#define _CUTEST_H_

#include "stdlib.h"

#ifdef CUTEST_LENIENT_ASSERTS
/* Since the lenient asserts do some int-magic casting */
#pragma GCC diagnostic ignored "-Wint-conversion"
#endif

#ifdef CUTEST_CLANG
/* Since __VA_ARGS__ are used in a GNU:ish way */
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

/*
 * Test-runner
 * -----------
 *
 * As you can see in the example above, the CUTest-part of your build
 * system will produce a ``*_test`` executable. This what is referred
 * to as the test-runner. This binary contains all your test-cases in
 * sequence as you have them in your test-suite, and it also contains
 * your design under test, along with mock-up versions of all functions
 * and other things that are used internally.
 *
 * To compile the test runner successfully you should never ever have
 * ``CUTEST_MOCK_MAIN`` defined to the compiler. They are used to
 * compile the *CUTest test runner* generator* and the *CUTest mock
 * generator* respectively.
 *
 */
typedef enum cutest_verdict_e {
  CUTEST_TEST_SKIPPED,
  CUTEST_TEST_ERROR,
  CUTEST_TEST_FAILED,
  CUTEST_TEST_OK
} cutest_verdict_t;

typedef struct cutest_junit_report_s {
  cutest_verdict_t verdict;
  const char* name;
  char* message;
  float time;
} cutest_junit_report_t;

void cutest_startup(int argc, char* argv[], const char* suite_name,
                    cutest_junit_report_t* junit_report, size_t test_cnt);
void cutest_execute_test(cutest_junit_report_t* junit_report,
                         void (*func)(), const char *name,
                         int do_mock, const char *prog_name);
void cutest_shutdown(const char* filename,
                     cutest_junit_report_t* junit_report, size_t test_cnt);

/*
 * These functions are generated
 */
void cutest_set_mocks_to_original_functions();
int cutest_test_name_argument_given(const char* test_name);
/*
 * The test() macro
 * ----------------
 *
 * Every unit test is defined with this macro. All function calls
 * within the called functions from a test will be automatically
 * mocked. You can override by setting the ``func``-member of the
 * mock-control struct to the original function if needed, or to any
 * other API compatible function - To stub the functionality.
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
 * stubbed/mocked in the design under test. You can still stub things
 * by setting the ``func``-member of the mock-control struct to any
 * API compatible function.
 *
 */
#define module_test(NAME) void cutest_##NAME()

/*
 * The assert_eq() macro
 * ---------------------
 *
 * This macro makes it easy to understand the test-case flow, it is a
 * "variadic" macro that takes two or three arguments. Use the form you
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
 * If you have defined ``CUTEST_LENIENT_ASSERTS`` (and use C11 or
 * above) CUTest is able to make more readable asserts and error
 * messages by analyzing the data-types of the arguments. As you can
 * notice in the example above; comparing two strings are a but
 * cumbersome. However This feature makes things very much easier.
 *
 * Example::
 *
 *   ...
 *   assert_eq("expected", some_variable);
 *   ...
 *
 * The ``CUTEST_LENIENT_ASSERTS`` is now enabled by default in CUTest but require
 * C11. If you want to disable it just set your envinment variable ``LENIENT=0``
 * when invoking the make-system and it will be disabled.
 *
 * By default the lenient assert macro is trying to convert the expected value
 * and reference value by casting to an unsigned long long, just to cover as
 * many cases as possible. Strings and floats are treated differently for better
 * and more understandable print-outs on what differs.
 *
 */

/*
 * When using ``CUTEST_LENIENT_ASSERTS`` you can also write your own compare
 * for the assert_eq() macro. This is very useful when you write your own data
 * types and want to be sure that they are compared in a relevant way. You can
 * force the assert_eq() macro to use your function by defining the macro
 * ``CUTEST_MY_OWN_EQ_COMPARATORS``, and match the datatype to the function you
 * want to use as compare function.
 *
 * Example::
 *
 *  typedef struct my_own_type_s {
 *    int a;
 *    char b[3];
 *    long c;
 *  } my_own_type_t;
 *
 *  static int compare_my_own_type_t(my_own_type_t a, my_own_type_t b, char* output)
 *  {
 *    if ((a.a == b.a) && // The actual compare operation
 *        (a.b[0] == b.b[0]) &&
 *        (a.b[1] == b.b[1]) &&
 *        (a.b[2] == b.b[2]) &&
 *        (a.c == b.c)) {
 *      return 0; // Return 0 if all is OK
 *    }
 *    // Otherwise generate a text to be put inside the assert_eq() failure output
 *    // between the parenthesis 'assert_eq(<MY TEXT>) failed'
 *    sprintf(output, "{%d, \"%c%c%c\", %ld}, {%d, \"%c%c%c\", %ld}",
 *            a.a, a.b[0], a.b[1], a.b[2], a.c,
 *            b.a, b.b[0], b.b[1], b.b[2], b.c);
 *    return 1; // Return something other than 0 if the assert failed.
 *  }
 *
 *  #define CUTEST_MY_OWN_EQ_COMPARATORS(EXP, REF)        \
 *    my_own_type_t: compare_my_own_type_t,
 *
 * Then you should be able to write a test that looks something like this::
 *
 *  test(cutest_shall_compare_own_data_types_correctly)
 *  {
 *    my_own_type_t values = {1, "234", 5};
 *    my_own_type_t gold = {1, "234", 5};
 *    assert_eq(gold, values); // Will invoke your own compare function
 *  }
 *
 */

#ifdef CUTEST_LENIENT_ASSERTS

#ifndef CUTEST_MY_OWN_EQ_COMPARATORS
#define CUTEST_MY_OWN_EQ_COMPARATORS(EXP, REF)
#endif

int cutest_assert_eq_char(const char a, const char b, char* output);
int cutest_assert_eq_str(const char* a, const char *b, char* output);
int cutest_compare_float(float f1, float f2);
int cutest_compare_double(double d1, double d2);
int cutest_compare_long_double(long double ld1, long double ld2);
int cutest_assert_eq_float(const float a, const float b, char* output);
int cutest_assert_eq_double(const double a, const double b, char* output);
int cutest_assert_eq_long_double(const long double a, const long double b,
                                 char* output);
int cutest_assert_eq_default(const unsigned long long a,
                             const unsigned long long b,
                             char* output);

#define assert_eq_comp(EXP, REF)                             \
  _Generic((EXP),                                            \
           float: cutest_assert_eq_float,                    \
           const float: cutest_assert_eq_float,              \
           double: cutest_assert_eq_double,                  \
           const double: cutest_assert_eq_double,            \
           long double: cutest_assert_eq_long_double,        \
           const long double: cutest_assert_eq_long_double,  \
           char: cutest_assert_eq_char,                      \
           const char: cutest_assert_eq_char,                \
           char*: cutest_assert_eq_str,                      \
           const char*: cutest_assert_eq_str,                \
           CUTEST_MY_OWN_EQ_COMPARATORS(EXP, REF)            \
  default: cutest_assert_eq_default)

#define assert_eq_3(EXP, REF, STR)                                      \
  {                                                                     \
    char cutest_o[1024];                                                \
    cutest_o[0] = 0;                                                    \
    const int cutest_assert_retval =                                    \
      assert_eq_comp((EXP), (REF))((EXP), (REF), cutest_o);             \
    if (0 != cutest_assert_retval) {                                    \
      char error_output_buf[1024];                                      \
      sprintf(error_output_buf,                                         \
              " %s:%d assert_eq(%s, " STR ") failed\n",                 \
              __FILE__,                                                 \
              __LINE__,                                                 \
              cutest_o);                                                \
      strcat(cutest_stats.current_error_output, error_output_buf);      \
      cutest_assert_fail_cnt++;                                         \
    }                                                                   \
  }

#define assert_eq_2(EXP, REF)                                           \
  {                                                                     \
    char cutest_o[1024];                                                \
    cutest_o[0] = 0;                                                    \
    const int cutest_assert_retval =                                    \
      assert_eq_comp((EXP), (REF))((EXP), (REF), cutest_o);             \
    if (0 != cutest_assert_retval) {                                    \
      char error_output_buf[1024];                                      \
      sprintf(error_output_buf,                                         \
              " %s:%d assert_eq(%s) failed\n",                          \
              __FILE__,                                                 \
              __LINE__,                                                 \
              cutest_o);                                                \
      strcat(cutest_stats.current_error_output, error_output_buf);      \
      cutest_assert_fail_cnt++;                                         \
    }                                                                   \
  }

#else

#define assert_eq_3(EXP, REF, STR)                                 \
  if ((EXP) != (REF)) {                                            \
    char error_output_buf[1024];                                   \
    sprintf(error_output_buf,                                      \
            " %s:%d assert_eq(" #EXP ", " #REF ", " STR ") "       \
            "failed\n",                                            \
            __FILE__, __LINE__);                                   \
    strcat(cutest_stats.current_error_output, error_output_buf);   \
    cutest_assert_fail_cnt++;                                      \
  }

#define assert_eq_2(EXP, REF)                                      \
  if ((EXP) != (REF)) {                                            \
    char error_output_buf[1024];                                   \
    sprintf(error_output_buf,                                      \
            " %s:%d assert_eq(" #EXP ", " #REF ") "                \
            "failed\n",                                            \
            __FILE__, __LINE__);                                   \
    strcat(cutest_stats.current_error_output, error_output_buf);   \
    cutest_assert_fail_cnt++;                                      \
  }

#endif

#define assert_eq_1(EXP)                                           \
  if (!(EXP)) {                                                    \
    char error_output_buf[1024];                                   \
    sprintf(error_output_buf,                                      \
            " %s:%d assert_eq(" #EXP ") "                          \
            "failed\n",                                            \
            __FILE__, __LINE__);                                   \
    strcat(cutest_stats.current_error_output, error_output_buf);   \
    cutest_assert_fail_cnt++;                                      \
  }

#define assert_eq_select(X, A1, A2, A3, MACRO, ...) MACRO

#define assert_eq_chooser(...)                  \
  assert_eq_select(,##__VA_ARGS__,              \
                   assert_eq_3,                 \
                   assert_eq_2,                 \
                   assert_eq_1,)

#define assert_eq(...) assert_eq_chooser(__VA_ARGS__)(__VA_ARGS__)

/*
 * The skip() macro
 * ----------------
 *
 * This is a feature that come in handy when you are unable to run a test
 * for some reason, but intend to fix it sooner or later. Just put this
 * macro in the first line of your test-case and the test will be skipped
 * and logged as skipped. It requires a string as argument, which should
 * contain the reason for the test being skipped currently. And remember
 * to refactor/re-implement code so that all tests pass :) .
 *
 * Example::
 *
 *  test(this_test_will_be_skipped)
 *  {
 *    skip("This test is being skipped, since the code just will crash");
 *    assert_eq(0, product_code_that_will_crash());
 *  }
 *
 */
#define skip(REASON)                            \
  cutest_stats.skip_reason = REASON;            \
  cutest_stats.skip_cnt++;                      \
  return

static int cutest_assert_fail_cnt = 0;
typedef struct cutest_opts_s {
  int verbose;
  int log_errors;
  int junit;
  int no_linefeed;
  int segfault_recovery;
  int print_tests;
} cutest_opts_t;
static cutest_opts_t cutest_opts;
static int cutest_exit_code = EXIT_SUCCESS;

#define CUTEST_MAX_JUNIT_BUFFER_SIZE 1024*1024

typedef struct cutest_stats_s {
  char suite_name[128];
  char design_under_test[128];
  char error_output[1024*1024*10];
  char current_error_output[1024];
  int test_cnt;
  int fail_cnt;
  int error_cnt;
  int skip_cnt;
  char* skip_reason;
  float elapsed_time;
} cutest_stats_t;

cutest_stats_t cutest_stats;

extern int cutest_error_cnt;

/*
 * Phases in the test-build and -execution
 * ---------------------------------------
 *
 * First off - There are a lot of magical things happening, hidden from
 * your eyes when you build a test-runner. For example - The build-system
 * defined in ``cutest.mk`` will make some assumptions about your code, and
 * generate many intermediate files.
 *
 * Building
 * ^^^^^^^^
 *
 * Before the build-system starts building your source code it extracts
 * some parts of ``cutest.h`` into small executable binaries. These are
 * helper tools to parse and extract information from your test-suite and
 * your design under test. For example it builds the execution sequence
 * for your specific unit-tests in your test-suite, and set-up a ``main()``
 * function. (That's why your own ``main()`` is automatically renamed to
 * ``MAIN()`` if you want to program it using unit-tests and TDD).
 *
 * Then all function calls to other functions are replaced with function
 * calls to mock-up functions instead, by modifying the Assembler output
 * from compiling your design under test. This is done by changing the
 * jump-destinations for ``call``, ``jmp`` and such instructions (So far
 * tested on x86 and ARM). This allows your production code to stay
 * intact in C-code format. We don't want to clutter it with test-code.
 *
 * Once your test-runner is built it should be able to run.
 *
 * Test initialization
 * ^^^^^^^^^^^^^^^^^^^
 *
 * In between every ``test()`` or ``module_test()`` definition in your
 * test-suite, the CUTest framework will clear all the mock controls
 * and test framework state so that every test is run in isolation.
 *
 * You still need to keep track of your own global data or internal
 * state, if your code require such things.
 *
 */

/*
 * Test execution
 * ^^^^^^^^^^^^^^
 *
 * When executing tests the elapsed time for execution is sampled and
 * used in the JUnit report. Depending on command line options an
 * output is printed to the console, either as a short version with
 * '.' for successful test run, 'F' for failed test run, 'E' for an
 * error (crash), or 'S' for skipped tests. But if the test-runner is set
 * to verbose ``-v``: ``[PASS]``, ``[FAIL]``, ``[ERROR]`` and ``[SKIP]``
 * output is produced.
 *
 * * PASS - All went good and all asserts were fulfilled.
 * * FAIL - One or more asserts were not fulfilled.
 * * ERROR - The design under test or the test case crashed.
 * * SKIP - The test is skipped using the ``skip()`` macro
 *
 * If the test runner is started with verbose mode ``-v`` the offending
 * assert will be printed to the console directly after the fail. If
 * in normal mode all assert-failures will be collected and printed
 * in the shutdown process.
 *
 * By default the ``check`` build target provided by ``cutest.mk`` will
 * try to output as little as possible. However you can override this
 * by setting the ``Q`` environment variable to empty
 * (``make check Q=``). This will make the console output more verbose.
 *
 */

/*
 * Shutdown process
 * ^^^^^^^^^^^^^^^^
 *
 * At the end of the execution the CUTest test-runner program will
 * output a JUnit XML report if specified with the ``-j`` command line
 * option.
 *
 */

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
 * work-flow example will also show you how to write *module-tests* that
 * make some kind of "kick-the-tires-sanity-check" that the integration
 * to the OS actually works with file access and all.
 *
 * Let's do this step-by-step...
 *
 * 1. You have to write a function called ``write_file``. And it shall
 *    take one single argument (a pointer to the file-name stored in a
 *    string) where to store the file in your file-system.
 *
 *    a. Write a simple test that assumes everything will go well. This
 *       implies that you can determine the success of the operation
 *       somehow. Let's use the old "return zero on success" paradigm.
 *       So... Let's call the design under test function called
 *       ``write_file`` with some kind of file-name as argument and
 *       expect it to return 0 (zero).
 *
 *       Create a file called ``file_operations_test.c`` and include
 *       ``cutest.h`` in the top of it.
 *
 *       Code::
 *
 *        #include "cutest.h"
 *
 *        test(write_file_shall_return_0_if_all_went_well)
 *        {
 *          assert_eq(0, write_file("my_filename.txt"));
 *        }
 *
 *    b. Now... When you try to compile this code using ``make check``
 *       everything will fail!
 *
 *       You will get build and compilation errors, simply because there
 *       is no corresponding file that contain the design under test yet.
 *
 *    c. Create a file called ``file_operations.c`` and implement a
 *       function called ``write_file`` that takes one ``const char*``
 *       argument as file name. And start, by just fulfilling the test;
 *       returning a 0 (zero) from it.
 *
 *       Code::
 *
 *        int write_file(const char* filename)
 *        {
 *          return 0;
 *        }
 *
 *    d. Now you should be able to compile and run your test using
 *       ``make check``. And the test should probably pass, if you
 *       did it correctly. And since the assumption of your test that
 *       ``write_file`` should return 0 (zero) on success, probably will
 *       not be true for all eternity you will probably have to revisit and
 *       re-factor it as the function becomes more complete.
 *
 * 2. Using the standard library to write code that opens a file
 *
 *    a. You probably already know that you will need to open a file to
 *       write your file contents to inside your ``write_file`` function.
 *       Let's make sure that we call ``fopen()`` in a good way, using
 *       the given file name and the correct file opening mode.
 *       Since this test probably will look nicer using
 *       CUTEST_LENIENT_ASSERTS; define it using ``#define`` before
 *       your ``#include "cutest.h"``-line. Now you can use strings as
 *       arguments to the ``assert_eq()`` macro instead of having to use
 *       the ``strcmp()`` return value equals 0 (zero) to compare two
 *       strings.
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
 *       a file-name as argument, then assert that the ``fopen()``
 *       function, in the standard library is, called *once*. Then it
 *       verifies that the two arguments passed to ``fopen()`` are
 *       correct, by asserting that the first argument should be the
 *       file-name passed to ``write_file`` and that the file is opened
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
 *       ``make check``. Let's take a break here... And think a bit.
 *
 *       When running the test it will call your design under test by
 *       calling it as an ordinary function...
 *
 *       The way CUTest works is that it detects ANY function
 *       call inside a callable function (e.g. ``fopen(...)`` and it
 *       will be replaced to call a generated mock-up of the same
 *       function. The mock-up mimics the API, with the same arguments
 *       as the original function. But the *actual* ``fopen()`` is never
 *       called by default when writing a unit-test.
 *
 *       Hence you can check various aspects of the function call in your
 *       test, using ``assert_eq`` on values expected - Like in the test-
 *       case we just wrote. We're checking the arguments of the call to
 *       ``fopen()`` and how many times the ``write_file`` design calls
 *       the ``fopen()`` function.
 *
 *       Pretty neat, right?
 *
 * 3. OK - Common sense tell us, that if a file is opened, it should
 *    probably be closed too. Otherwise the OS would end up with a bunch
 *    of opened files.
 *
 *    a. So let's define a test for checking that the provided file name
 *       actually close the _correct_ file, before the design under
 *       test exits and return it's 0 (zero).
 *
 *       This time you will have to manipulate the return value of
 *       the ``fopen()`` function to something that makes it easy to
 *       recognize as argument to the ``fclose()`` value. Hance making sure
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
 *       specified file. Create a simple test that verifies that the
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
 *    a. Since ``fputs`` can fail, let's expect our code to return another
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
 *    ``custs_mock.<func>.func`` function pointer is set to the original
 *    function. Hence you can still verify call counts, arguments passed
 *    but the over-all functionality of you design will be run for real.
 *    Note that this can definitely impact execution time.
 *
 *    Another thing worth noticing is that many developers believe that
 *    these kind of integration tests or module tests *are* unit-tests.
 *    One could argue that they're not, since they do not drive the
 *    design, nor do they test only _your_ code, but they test already
 *    tested code, like ``fopen``, ``close`` and ``fputs`` in this case.
 *    Which might seem like waste of clock cycles.
 *
 * That's it folks! I hope you enjoyed this example of a work-flow and
 * please come back to the author with feedback!
 *
 * Contribute
 * ----------
 *
 * Wow! You've come this far in all this mumbo-jumbo text! Anyhow: If you
 * lack functionality or have invented something awesome that would
 * contribute to the feature-set of CUTest, please contribute! The code
 * is on GitHub, and no-one would be happier than me to have more
 * developers collaborating and making the product more awesome.
 *
 * Send me an e-mail or contact me via GitHub.
 *
 * Thanks for reading!
 *
 * //AiO
 */
#endif
