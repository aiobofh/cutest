#define _XOPEN_SOURCE
#include <time.h>
#include <math.h>
#include <signal.h>
#include <setjmp.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "cutest.h"

jmp_buf buf;

static struct {
  int cnt;
  char test_name[1024][1024];
} cutest_tests_to_run;

typedef struct cutest_opts_s {
  int verbose;
  int log_errors;
  int junit;
  int no_linefeed;
  int segfault_recovery;
  int print_tests;
} cutest_opts_t;
static cutest_opts_t cutest_opts;

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

static int cutest_exit_code = EXIT_SUCCESS;
static cutest_stats_t cutest_stats;
static int cutest_assert_fail_cnt = 0;
static int cutest_error_cnt = 0;

extern struct tm *localtime_r(const time_t *timep, struct tm *result);

#ifdef CUTEST_LENIENT_ASSERTS

int cutest_assert_eq_char(const char a, const char b, char* output)
{
  if (a == b) {
    return 0;
  }
  sprintf(output, "'%c' (%d), '%c' (%d)", a, a, b, b);
  return 1;
}

int cutest_assert_eq_str(const char* a, const char *b, char* output)
{
  if (0 == strcmp(a, b)) {
    return 0;
  }
  sprintf(output, "\"%s\", \"%s\"", a, b);
  return 1;
}

int cutest_compare_float(float f1, float f2)
{
  float epsilon = 0.00001;
  if (fabsf(f1 - f2) <= epsilon) {
    return 1;
  }
  return 0;
}

int cutest_compare_double(double d1, double d2)
{
  double epsilon = 0.00001;
  if (fabs(d1 - d2) <= epsilon) {
    return 1;
  }
  return 0;
}

int cutest_compare_long_double(long double ld1, long double ld2)
{
  long double epsilon = 0.00001;
  if (fabsl(ld1 - ld2) <= epsilon) {
    return 1;
  }
  return 0;
}


int cutest_assert_eq_float(const float a, const float b, char* output)
{
  if (cutest_compare_float(a, b)) {
    return 0;
  }
  sprintf(output, "%f, %f", a, b);
  return 1;
}

int cutest_assert_eq_double(const double a, const double b, char* output)
{
  if (cutest_compare_double(a, b)) {
    return 0;
  }
  sprintf(output, "%f, %f", a, b);
  return 1;
}

int cutest_assert_eq_long_double(const long double a, const long double b,
                                 char* output)
{
  if (cutest_compare_long_double(a, b)) {
    return 0;
  }
  sprintf(output, "%Lf, %Lf", a, b);
  return 1;
}

int cutest_assert_eq_default(const unsigned long long a,
                             const unsigned long long b,
                             char* output)
{
  if (a == b) {
    return 0;
  }
  sprintf(output, "%lld, %lld", a, b);
  return 1;
}

#endif

void cutest_increment_skips(char* reason)
{
  cutest_stats.skip_reason = reason;
  cutest_stats.skip_cnt++;
}

void cutest_increment_fails(const char* error_output)
{
  strcat(cutest_stats.current_error_output, error_output);
  cutest_assert_fail_cnt++;
}

static void cutest_segfault_handler(int s)
{
  switch(s) {
  case SIGSEGV:
    signal(SIGSEGV, cutest_segfault_handler);
    longjmp(buf, 1);
    break;
  default:
    fprintf(stderr, "ERROR: Misconfigured CUTest version\n");
  }
}

int cutest_test_name_argument_given(const char* test_name)
{
  int i = 0;
  if (0 == cutest_tests_to_run.cnt) {
    return 1;
  }
  while (i < cutest_tests_to_run.cnt) {
    if (0 == strcmp(test_name, cutest_tests_to_run.test_name[i])) {
      return 1;
    }
    i++;
  }
  return 0;
}

static void run_usage(const char* program_name)
{
  printf("USAGE: %s [-h] [-v|-l|-j|-n|-s|-p] <test-case-names-list>\n\n"
         "  -h, --help              Show this help text\n"
         "  -v, --verbose           Run the tests in verbose mode\n"
         "  -l, --log-errors        Log errors (stderr) to %s.log\n"
         "  -j, --junit             Produce a JUnit output to %s.junit.xml\n"
         "  -n, --no-line-feed      Don't add linefeed to the last output row.\n"
         "  -s, --segfault-recovery Kepp running tests after an Error (crash).\n"
         "  -p, --print-tests       Just print the test names in the suite.\n",
         program_name,
         program_name,
         program_name);
}

static void handle_args(cutest_opts_t* opts, const char* suite_name,
                        int argc, char* argv[]) {
  int i;
  for (i = 1; i < argc; i++) {
    if ((0 == strcmp(argv[i], "-h")) ||
        (0 == strcmp(argv[i], "--help"))) {
      run_usage(suite_name);
      exit(EXIT_SUCCESS);
    }
    if ((0 == strcmp(argv[i], "-v")) ||
        (0 == strcmp(argv[i], "--verbose"))) {
      opts->verbose = 1;
      continue;
    }
    if ((0 == strcmp(argv[i], "-l")) ||
        (0 == strcmp(argv[i], "--log-errors"))) {
      opts->log_errors = 1;
      continue;
    }
    if ((0 == strcmp(argv[i], "-j")) ||
        (0 == strcmp(argv[i], "--junit"))) {
      opts->junit = 1;
      continue;
    }
    if ((0 == strcmp(argv[i], "-n")) ||
        (0 == strcmp(argv[i], "--no-linefeed"))) {
      opts->no_linefeed = 1;
      continue;
    }
    if ((0 == strcmp(argv[i], "-s")) ||
        (0 == strcmp(argv[i], "--segfault-recovery"))) {
      opts->segfault_recovery = 1;
      continue;
    }
    if ((0 == strcmp(argv[i], "-p")) ||
        (0 == strcmp(argv[i], "--print-tests"))) {
      opts->print_tests = 1;
      continue;
    }

    strcpy(cutest_tests_to_run.test_name[cutest_tests_to_run.cnt++], argv[i]);
  }
}

int cutest_startup(int argc, char* argv[], const char* suite_name,
                   cutest_junit_report_t* junit_report, size_t test_cnt)
{
  memset(&cutest_tests_to_run, 0, sizeof(cutest_tests_to_run));
  memset(&cutest_opts, 0, sizeof(cutest_opts));

  handle_args(&cutest_opts, suite_name, argc, argv);

  memset(&cutest_stats, 0, sizeof(cutest_stats));
  strcpy(cutest_stats.suite_name, suite_name);
  strcpy(cutest_stats.design_under_test, suite_name);

  if (1 == cutest_opts.segfault_recovery) {
    signal(SIGSEGV, cutest_segfault_handler);
  }

  if (1 == cutest_opts.junit) {
    memset(junit_report, 0, sizeof(junit_report) * test_cnt);
  }

  return cutest_opts.print_tests;
}

void catch_segfault_through_gdb(const char* name, const char* prog_name)
{
  char buf[1024];
  FILE* fp = NULL;

  sprintf(buf, " Segmentation fault! Caught in: %s\n", name);

  strcat(cutest_stats.current_error_output, buf);

  sprintf(buf, "gdb --batch -ex \"r -v %s\" -ex \"bt\" %s", name, prog_name);

  fp = popen(buf, "r");

  if (NULL == fp) {
    fprintf(stderr,
            "ERROR: Unable to execute '%s' to investigate segfault\n", buf);
    return;
  }

  while (!feof(fp)) {
    char b[1024];
    if (NULL != fgets(b, sizeof(b), fp)) {
      strcat(cutest_stats.current_error_output, "  ");
      strcat(cutest_stats.current_error_output, b);
    }
  }

  pclose(fp);

  strcat(cutest_stats.current_error_output, "\n");
}

void verbose_verdict(cutest_stats_t* stats, const char* name,
                     int error_cnt, int fail_cnt)
{
  if (NULL != stats->skip_reason) {
    printf("[SKIP]: %s\n", name);
  }
  else if (error_cnt != 0) {
    printf("[ERROR]: %s\n", name);
    printf("%s", stats->current_error_output);
  }
  else if (fail_cnt == 0) {
    printf("[PASS]: %s\n", name);
  }
  else {
    printf("[FAIL]: %s\n", name);
    printf("%s", stats->current_error_output);
  }
}

void simple_verdict(cutest_stats_t* stats, int error_cnt, int fail_cnt)
{
  if (NULL != stats->skip_reason) {
    printf("S");
  }
  else if (error_cnt != 0) {
    printf("E");
  }
  else if (fail_cnt == 0) {
    printf(".");
  }
  else {
    printf("F");
  }
  fflush(stdout);
}

void output_test_verdict_to_screen(int verbose, cutest_stats_t* stats,
                                   const char* name, int error_cnt,
                                   int fail_cnt)
{
  if (1 == verbose) {
    verbose_verdict(stats, name, error_cnt, fail_cnt);
  }
  else {
    simple_verdict(stats, error_cnt, fail_cnt);
  }
}

void append_output_to_junit_report(cutest_junit_report_t* junit_report,
                                   cutest_stats_t* stats, const char* name)
{
  junit_report->name = name;

  if (NULL != stats->skip_reason) {
    junit_report->verdict = CUTEST_TEST_SKIPPED;
    junit_report->message = NULL;
  }
  else if (0 != cutest_error_cnt) {
    junit_report->verdict = CUTEST_TEST_ERROR;
    junit_report->message = malloc(strlen(stats->current_error_output));
    strcpy(junit_report->message, stats->current_error_output);
  }
  else if (0 != cutest_assert_fail_cnt) {
    junit_report->verdict = CUTEST_TEST_FAILED;
    junit_report->message = malloc(strlen(stats->current_error_output));
    strcpy(junit_report->message, stats->current_error_output);
  }
  else {
    junit_report->verdict = CUTEST_TEST_OK;
    junit_report->message = NULL;
  }

}

void cutest_execute_test(cutest_junit_report_t* junit_report,
                         void (*func)(), const char *name,
                         int do_mock, const char *prog_name)
{
  cutest_stats.current_error_output[0] = 0;
  cutest_stats.skip_reason = NULL;

  if (1 == do_mock) {
    cutest_set_mocks_to_original_functions();
  }

  if ((0 == cutest_opts.segfault_recovery) || (!setjmp(buf))) {

    func(); /* Call the test case function this is probably good step-into */

  }
  else {
    catch_segfault_through_gdb(name, prog_name);
    cutest_error_cnt++;
  }

  output_test_verdict_to_screen(cutest_opts.verbose, &cutest_stats,
                                name, cutest_error_cnt,
                                cutest_assert_fail_cnt);

  if (cutest_assert_fail_cnt != 0) {
    cutest_exit_code = EXIT_FAILURE;
  }

  cutest_stats.test_cnt++;
  cutest_stats.fail_cnt += (cutest_assert_fail_cnt != 0);
  cutest_stats.error_cnt += (cutest_error_cnt != 0 );

  if (1 == cutest_opts.junit) {
    append_output_to_junit_report(junit_report, &cutest_stats, name);
  }

  cutest_assert_fail_cnt = 0;
  cutest_error_cnt = 0;

  strcat(cutest_stats.error_output, cutest_stats.current_error_output);
  memset(cutest_stats.current_error_output, 0,
         sizeof(cutest_stats.current_error_output));
}

void cutest_append_junit_node(FILE* stream, const char* design_under_test,
                              cutest_junit_report_t* junit_report)
{
  fprintf(stream,
          "    <testcase classname=\"%s\" name=\"%s\">\n",
          design_under_test, junit_report->name);

  switch (junit_report->verdict) {
  case CUTEST_TEST_SKIPPED:
    fprintf(stream,
            "       <skipped/>\n");
    break;
  case CUTEST_TEST_ERROR:
    fprintf(stream,
            "       <error message=\"segfault\">\n"
            "%s\n"
            "       </error>\n",
            junit_report->message);
    break;
  case CUTEST_TEST_FAILED:
    fprintf(stream,
            "       <failure message=\"test failure\">\n"
            "%s\n"
            "       </failure>\n",
            junit_report->message);
    break;
  default:
    break;
  }

  fprintf(stream,
          "    </testcase>\n");
}

void write_junit_report(const char* report_file_name,
                        const char* test_file_name,
                        cutest_stats_t* stats,
                        cutest_junit_report_t* junit_report,
                        size_t test_cnt)
{
  const time_t current_time = time(NULL);
  char timestamp[40];
  struct tm tm;
  FILE *stream = NULL;
  size_t i = 0;

  /* Create a textual timestamp */
  localtime_r(&current_time, &tm);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", &tm);

  stream = fopen(report_file_name, "w+");

  /* Ugly-output a JUnit XML-report. */
  fprintf(stream,
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
          "<testsuites>\n"
          "  <testsuite name=\"%s\"\n"
          "             errors=\"%d\"\n"
          "             tests=\"%d\"\n"
          "             failures=\"%d\"\n"
          "             skipped=\"%d\"\n"
          "             timestamp=\"%s\">\n",
          test_file_name, 0, stats->test_cnt, stats->fail_cnt,
          stats->skip_cnt,
          timestamp);

  for (i = 0; i < test_cnt; i++) {
    cutest_append_junit_node(stream, test_file_name, &junit_report[i]);
    if (NULL != junit_report[i].message) {
      free(junit_report[i].message);
    }
  }

  fprintf(stream,
          "  </testsuite>\n"
          "</testsuites>\n");

  fclose(stream);
}

void write_log_file(const char* test_suite_file_name, const char* error_buf)
{
  const size_t dotpos =
    strstr(test_suite_file_name, ".") - test_suite_file_name;
  char* log_file_name = malloc(dotpos + 4);
  FILE* fd = NULL;

  strncpy(log_file_name, test_suite_file_name, dotpos);
  log_file_name[dotpos] = 0;
  strcat(log_file_name, ".log");

  fd = fopen(log_file_name, "w");
  if (NULL == fd) {
    fprintf(stderr, "ERROR: Could not open log-file '%s' for writing.\n",
            log_file_name);
    goto cleanup;
  }

  fwrite(error_buf, strlen(error_buf), 1, fd);
  fclose(fd);

 cleanup:
  free(log_file_name);
}

int cutest_shutdown(const char* filename,
                     cutest_junit_report_t* junit_report, size_t test_cnt)
{
  char junit_report_name[1024];
  int i;

  if (0 == cutest_opts.verbose) {
    /*
     * Add an enter if not running in verbose, to line break after
     * the ...
     */
    if ((0 == cutest_opts.no_linefeed) ||
        (0 != strlen(cutest_stats.error_output))) {
      if (0 == cutest_opts.log_errors) {
        printf("\n");
      }
    }
    if (0 == cutest_opts.log_errors) {
      printf("%s", cutest_stats.error_output);
    }
    else if (0 != strlen(cutest_stats.error_output)) {
      write_log_file(filename, cutest_stats.error_output);
    }
  }
  else {
    /* Print a simple summary. */
    printf("%d passed, %d failed.\n",
           cutest_stats.test_cnt - cutest_stats.fail_cnt,
           cutest_stats.fail_cnt);
  }


  if (1 == cutest_opts.junit) {
    memset(junit_report_name, 0, sizeof(junit_report_name));

    for (i = strlen(filename); i > 0; i--) {
      if ('.' == filename[i]) {
        strncpy(junit_report_name, filename, i);
        strcat(junit_report_name, ".junit_report.xml");
        break;
      }
    }

    write_junit_report(junit_report_name, filename, &cutest_stats,
                       junit_report, test_cnt);
  }

  return cutest_exit_code;
}
