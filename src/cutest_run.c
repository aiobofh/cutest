/*********************************************************************
   ------    ____ ____ _____ ____ ____ _____   ____ ____ _ __ --------
   ------   / __// / //_  _// __// __//_  _/  / _ // / //|/ / -------
   ------  / /_ / / /  / / / __//_  /  / /   /   |/ / //   /  -------
   ------ /___//___/  /_/ /___//___/  /_/   /_/_//___//_|_/   -------
 *
 * CUTest test runner generator
 * ============================
 *
 * The ``cutest_run`` tool will parse your test suite and produce an
 * executable program with some command line options to enable you to
 * control it a little bit.
 *
 * How to build the tool
 * ---------------------
 *
 * Just include the ``cutest.mk`` makefile in your own ``Makefile`` in
 * your folder containing the source code for the ``*_test.c`` files.
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
 * And it will scan the test suite source-code for uses of the ``test()``
 * and ``module_test()`` macros and output a C program containing
 * everything needed to test your code alongside with the ``cutest.h``
 * file.
 *
 * However, if you use the ``Makefile`` targets specified in the
 * beginning of this document you will probably not need to run it
 * manually.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK_SIZE 1024

#include "cutest_run.h"
#include "helpers.h"

static void usage(const char* program_name)
{
  printf("USAGE: %s <test-c-source-file> <c-mock-header-file>\n",
         program_name);
}

static void replace_last_parenthesis_with_0(char *buf, int start)
{
  size_t i;
  const size_t len = strlen(buf);
  for (i = start; i < len; i++) {
    if (')' == buf[i]) {
      buf[i] = 0;
      return;
    }
  }
  fprintf(stderr, "ERROR: Malformed test-case '%s'", buf);
}

static int skip_comments(const char* buf)
{
  const size_t len = strlen(buf);
  size_t i;
  int slash_star_comment = 0;
  for (i = 0; i < len - 1; i++) {
    if (('/' == buf[i]) && ('*' == buf[i+1])) {
      slash_star_comment++;
    }
    else if (('*' == buf[i]) && ('/' == buf[i+1])) {
      slash_star_comment--;
    }
  }
  return slash_star_comment;
}

static struct test_s next_test(char* buf)
{
  struct test_s retval = {NULL, 0};
  size_t len;
  if (0 == strncmp(buf, "test(", len=strlen("test("))) {
    replace_last_parenthesis_with_0(buf, len);
    retval.name = &buf[len];
    retval.reset_mocks = 0;
  }
  else if (0 == strncmp(buf, "module_test(", len = strlen("module_test("))) {
    replace_last_parenthesis_with_0(buf, len);
    retval.name = &buf[len];
    retval.reset_mocks = 1;
  }
  return retval;
}

static void print_header(const char* program_name,
                         const char* test_source_file_name,
                         const char* mock_header_file_name)
{
  printf("/*\n"
         " * This file is generated by '%s %s %s'.\n"
         " *\n"
         " * Compile this program to get a test-case executor.\n"
         " *\n"
         " */\n"
         "\n"
         "#define _XOPEN_SOURCE\n"
         "#include <stdlib.h>\n"
         "#include <stdio.h>\n"
         "#include <string.h>\n"
         "\n"
         "#define main MAIN /* To make sure we can test main() */\n"
         "#define inline\n"
         "#include \"%s\" /* Mock-up functions and design under test */\n"
         "#include \"%s\" /* Test-cases */\n"
         "#undef inline\n"
         "#undef main\n"
         "\n",
         program_name,
         test_source_file_name,
         mock_header_file_name,
         mock_header_file_name,
         test_source_file_name);
}

static void print_main_function_prologue(const char* test_source_file_name)
{
  printf("int main(int argc, char* argv[])\n"
         "{\n"
         "  cutest_startup(argc, argv, \"%s\");\n\n",
         test_source_file_name);
}

static void print_test_case_executor(const char* name, int reset_mocks)
{
  printf("  if (1 == cutest_test_name_argument_given(\"%s\")) {\n"
         "    cutest_execute_test(cutest_%s, \"%s\", %d, argv[0]);\n"
         "  }\n",
         name, name, name, reset_mocks);
}

static void print_main_function_epilogue(const char* test_source_file_name)
{
  printf("  cutest_shutdown(\"%s\");\n\n"
         "  return cutest_exit_code;\n"
         "}\n",
         test_source_file_name);
}

static void print_test_case_executions(const char* test_source_file_name)
{
  FILE *fd = fopen(test_source_file_name, "r");

  int still_inside_a_comment = 0;

  while (!feof(fd)) {

    char buf[CHUNK_SIZE];
    if (NULL == fgets(buf, CHUNK_SIZE, fd)) {
      break;
    }

    still_inside_a_comment += skip_comments(buf);
    if (still_inside_a_comment > 0) {
      continue;
    }

    struct test_s t = next_test(buf);
    if (NULL == t.name) {
      continue;
    }

    print_test_case_executor(t.name, t.reset_mocks);
  }

  fclose(fd);
}

static void print_test_names_printer(const char* test_source_file_name)
{
  FILE *fd = fopen(test_source_file_name, "r");

  int still_inside_a_comment = 0;

  printf("  if (cutest_opts.print_tests) {\n");

  while (!feof(fd)) {

    char buf[CHUNK_SIZE];
    if (NULL == fgets(buf, CHUNK_SIZE, fd)) {
      break;
    }

    still_inside_a_comment += skip_comments(buf);
    if (still_inside_a_comment > 0) {
      continue;
    }

    struct test_s t = next_test(buf);
    if (NULL == t.name) {
      continue;
    }

    printf("    puts(\"%s\");\n", t.name);
  }

  printf("    exit(EXIT_SUCCESS);\n"
         "  }\n");

  fclose(fd);
}
/*
 * The test runner program
 * -----------------------
 *
 * The generated test runner program will inventory all the tests in
 * the specified suite and run them in the order that they appear in
 * the suite.
 *
 * The first thing that happens is the start-up process, then all
 * tests are run in isolation, followed by the Shutdown process.
 */
int main(int argc, char* argv[]) {
  const char* program_name = argv[0];

  if (argc < 3) {
    fprintf(stderr, "ERROR: Missing arguments\n");
    usage(program_name);
    return EXIT_FAILURE;
  }

  const char* test_source_file_name = argv[1];
  const char* mock_header_file_name = argv[2];

  if (!file_exists(test_source_file_name) ||
      !file_exists(mock_header_file_name)) {
    return EXIT_FAILURE;
  }

  print_header(program_name, test_source_file_name, mock_header_file_name);
  print_main_function_prologue(test_source_file_name);
  print_test_names_printer(test_source_file_name);
  print_test_case_executions(test_source_file_name);
  print_main_function_epilogue(test_source_file_name);

  return EXIT_SUCCESS;
}
