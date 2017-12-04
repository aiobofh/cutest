/*********************************************************************
   ---    ____ ____ _____ ____ ____ _____   __  __ ____ ____ ____ ---
   ---   / __// / //_  _// __// __//_  _/  / /_/ //   // _ // / / ---
   ---  / /_ / / /  / / / __//_  /  / /   / / / // / //   \/  <'  ---
   --- /___//___/  /_/ /___//___/  /_/    \____//___//_/_//_/_/   ---
   ---                                                            ---
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
  printf("USAGE: %s <-v|-n> suite1 suite2 .. suiteN\n", program_name);
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

static int run_test_suite(const char* executable_file_name, int verbose)
{
  int retval;
  int optlen = 0;
  if (0 == verbose) {
    optlen = strlen(" -j -s");
  }
  else {
    optlen = strlen(" -? -j -s");
  }
  char* command = malloc(strlen(executable_file_name) + optlen + 1);
  strcpy(command, executable_file_name);
  if (1 == verbose) {
    strcat(command, " -v");
  }
  else if (-1 == verbose) {
    strcat(command, " -n");
  }
  strcat(command, " -j -s");
  retval = system(command);
  free(command);
  return retval;
}

static int run_test_suites(int core_idx, int cores, int argc, char* argv[], int verbose) {
  int suite_idx = core_idx + 1;
  int retval = 0;

  while (suite_idx < argc) {
    retval |= run_test_suite(argv[suite_idx + 1], verbose);
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
  int idx;
  pid_t pid[128];
  for (idx = 0; idx < allocated_cores; idx++) {
    if (0 > (pid[idx] = fork())) {
      fprintf(stderr, "ERROR: Failed to fork\n");
      exit(EXIT_FAILURE);
    }
    else if (pid[idx] == 0) {
      int r = run_test_suites(idx, allocated_cores, argc, argv, verbose);
      exit(r);
    }
  }
}

static int slight_delay()
{
  int a = 0;
  int i;
  for (i = 0; i < 10; i++) {
    a += i * 2;
  }
  return a;
}

static int wait_for_child_processes(int allocated_cores, int verbose)
{
  int retval = 0;
  while (allocated_cores > 1) {
    int status;
    pid_t pid;
    pid = waitpid(-1, &status, 0);
    (void)pid;
    --allocated_cores;
    retval |= status;
  }
  slight_delay();
  if (-1 == verbose) {
    puts("");
  }
  return retval;
}

static int launch_process(int argc, char* argv[], int verbose)
{
  int r = run_test_suites(0, 1, argc, argv, verbose);
  if (-1 == verbose) {
    puts("");
  }
  return r;
}

int main(int argc, char* argv[]) {
  int verbose = 0;

  verbose = handle_args(argc, argv);

  const int cores = get_number_of_cores();
  const int suites = argc - 1;
  int retval = 0;

  int allocated_cores = min(suites, cores);

  if (allocated_cores > 1) {
    launch_child_processes(allocated_cores, argc, argv, verbose);
    retval = wait_for_child_processes(allocated_cores, verbose);
  }
  else {
    retval = launch_process(argc, argv, verbose);
  }

  return retval;
}
