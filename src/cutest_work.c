/*********************************************************************
   ---    ____ ____ _____ ____ ____ _____   __  __ ____ ____ ____ ---
   ---   / __// / //_  _// __// __//_  _/  / /_/ //   // _ // / / ---
   ---  / /_ / / /  / / / __//_  /  / /   / / / // / //   \/  <'  ---
   --- /___//___/  /_/ /___//___/  /_/    \____//___//_/_//_/_/   ---
   ---                                                            ---
    .-----.
  .:  _ _  :.
  ::   _ _ ::
 ..---._.---..
  '__.' '.__'
  :   ( )   :
  ':.'''''.:'
   ':::::::'
     |'''| [AiO]
   -'     '-

 *
 * CUTest worker
 * =============
 *
 * The ``cutst_work`` tool probes the host hardware and tries to run
 * as many test suites in parallel as possible to provide as fast
 * feedback as possible.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "helpers.h"

static void usage(const char* program_name)
{
  printf("USAGE: %s <-V|-v|-n> suite1 suite2 .. suiteN\n\n"
         "  -v  Be verbose naming all test names and pass/fail\n"
         "  -n  No line-feed after non-verbos to get '.' from all suites on one line\n"
         "  -V  Invoke the test suites through valgrind\n\n", program_name);
}

static int get_number_of_cores()
{
#ifdef _SC_NPROCESSORS_ONLN
  return sysconf(_SC_NPROCESSORS_ONLN);
#else
  return 1;
#endif
}

static int all_input_files_exist(int argc, char* argv[])
{
  int idx;
  int retval = 1;

  for (idx = 2; idx < argc; idx++) {
    if (!file_exists(argv[idx])) {
      fprintf(stderr, "ERROR: '%s' does not exist\n", argv[idx]);
      retval = 0;
    }
  }
  return retval;
}

static int run_test_suite(const char* executable_file_name, int verbose, int stderr_log)
{
  int valgrindlen = 0;
  int retval = 0;
  int optlen = 0;
  char* command = NULL;

  if (NULL == executable_file_name) {
    fprintf(stderr, "ERROR: Internal error, suite executable is NULL pointer\n");
    return -1;
  }
  if (0 == verbose) {
    optlen = strlen(" -j -s");
  }
  else if (2 == verbose) {
    optlen = strlen(" -v -j -s");
    valgrindlen = strlen("valgrind --track-origins=yes -q ");
  }
  else {
    optlen = strlen(" -? -j -s");
  }
  if (1 == stderr_log) {
    optlen += strlen(" -l");
  }
  command = malloc(valgrindlen + strlen(executable_file_name) + optlen + 1);
  if (NULL == command) {
    fprintf(stderr, "ERROR: Out of memory while allocating suite command.\n");
    return -1;
  }
  command[0] = 0;
  if (2 == verbose) {
    strcat(command, "valgrind --track-origins=yes -q ");
  }
  strcat(command, executable_file_name);
  if (1 == verbose) {
    strcat(command, " -v");
  }
  else if (2 == verbose) {
    strcat(command, " -v");
  }
  else if (-1 == verbose) {
    strcat(command, " -n");
  }
  strcat(command, " -j -s");
  if (1 == stderr_log) {
    strcat(command, " -l");
  }

  retval = system(command);

  free(command);
  return retval;
}

static int run_test_suites(int core_idx, int cores, int argc, char* argv[],
                           int verbose, int stderr_log) {
  int suite_idx = core_idx + 1;
  int retval = 0;

  while (suite_idx < argc - 1) {
    const int r = run_test_suite(argv[suite_idx + 1], verbose, stderr_log);
    if (-1 == r) {
      return -1;
    }
    retval |= r;
    suite_idx += cores;
  }

  return retval;
}

#define min(a,b) ((a < b) ? a : b)

static int handle_args(int argc, char* argv[]) {
  int verbose = 0;
  const char* program_name = argv[0];

  if (argc < 3) {
    usage(program_name);
    exit(EXIT_FAILURE);
  }

  if (0 == strcmp("-v", argv[1])) {
    verbose = 1;
  }
  else if (0 == strcmp("-n", argv[1])) {
    verbose = -1;
  }
  else if (0 == strcmp("-V", argv[1])) {
    verbose = 2;
  }
  else {
    usage(program_name);
    exit(EXIT_FAILURE);
  }

  if (0 == all_input_files_exist(argc, argv)) {
    exit(EXIT_FAILURE);
  }
  return verbose;
}

static void launch_child_processes(int allocated_cores, int argc,
                                   char* argv[], int verbose)
{
  int idx = 0;
  pid_t pid[128];

  for (idx = 0; idx < allocated_cores; idx++) {
    if (0 > (pid[idx] = fork())) {
      fprintf(stderr, "ERROR: Failed to fork\n");
      exit(EXIT_FAILURE);
    }
    else if (pid[idx] == 0) {
      int r = run_test_suites(idx, allocated_cores, argc, argv, verbose, 1);
      if (0 != r) {
        r = EXIT_FAILURE;
      }
      else {
        r = EXIT_SUCCESS;
      }
      exit(r);
    }
  }
}

static int slight_delay()
{
  int a = 0;
  int i = 0;

  for (i = 0; i < 10; i++) {
    a += i * 2;
  }
  return a;
}

static int wait_for_child_processes(int allocated_cores, int verbose)
{
  int retval = 0;

  while (allocated_cores > 0) {
    int status = 0;
    (void)waitpid(0, &status, 0);
    --allocated_cores;
    retval |= WEXITSTATUS(status);
  }
  slight_delay();
  if (-1 == verbose) {
    puts("");
  }

  return retval;
}

static void print_log(const char* suite_name)
{
  const size_t log_name_len = strlen(suite_name) + strlen(".log") + 1;
  char* log_name = malloc(log_name_len);
  FILE *fd = NULL;
  char buf[1024];

  if (NULL == log_name) {
    fprintf(stderr, "ERROR: Out of memory while allocating log file name\n");
    return;
  }
  memset(log_name, 0, log_name_len);
  strcpy(log_name, suite_name);
  strcat(log_name, ".log");

  fd = fopen(log_name, "r");
  if (NULL == fd) {
    free(log_name);
    return;
  }
  while (NULL != fgets(buf, sizeof(buf), fd)) {
    buf[strlen(buf)] = 0;
    fputs(buf, stderr);
  }
  fclose(fd);
  unlink(log_name);
  free(log_name);
}

static void print_logs(int argc, char* argv[])
{
  int suite_idx = 2;

  while (suite_idx < argc) {
    print_log(argv[suite_idx]);
    suite_idx++;
  }
}

static int launch_process(int argc, char* argv[], int verbose)
{
  int r = run_test_suites(0, 1, argc, argv, verbose, 0);

  if (-1 == verbose) {
    puts("");
  }
  return r;
}

int main(int argc, char* argv[]) {
  int verbose = 0;
  const int cores = get_number_of_cores();
  const int suites = argc - 1;
  int retval = 0;
  int allocated_cores = min(suites, cores);

  verbose = handle_args(argc, argv);

  if (allocated_cores > 1) {
    launch_child_processes(allocated_cores, argc, argv, verbose);
    retval = wait_for_child_processes(allocated_cores, verbose);
    print_logs(argc, argv);
  }
  else {
    retval = launch_process(argc, argv, verbose);
    if (0 != retval) {
      retval = EXIT_FAILURE;
    }
    else {
      retval = EXIT_SUCCESS;
    }
  }

  return retval;
}
