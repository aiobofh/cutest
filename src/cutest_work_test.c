#include "cutest.h"

#define m cutest_mock
#define main MAIN

/*****************************************************************************
 * usage();
 */
test(usage_shall_print_something)
{
  usage(0x1234);
  assert_eq(1, m.printf.call_count);
}

/*****************************************************************************
 * get_number_of_cores()
 */
test(get_number_of_cores_shall_get_number_of_cores)
{
  get_number_of_cores();
#ifdef _SC_NPROCESSORS_ONLN
  assert_eq(1, m.sysconf.call_count);
#endif
}

test(get_number_of_cores_shall_return_the_number_of_cores_detected)
{
#ifdef _SC_NPROCESSORS_ONLN
  m.sysconf.retval = 123;
  assert_eq(123, get_number_of_cores());
#else
  assert_eq(1, get_number_of_cores());
#endif
}

/*****************************************************************************
 * all_input_files_exist()
 */
test(all_input_files_exist_shall_call_file_exists_correct_number_of_times)
{
  char* argv[5] = {"program_name", "-v", "suite1", "suite2", "suite3"};
  all_input_files_exist(3 + 2, argv);
  assert_eq(3, m.file_exists.call_count);
}

test(all_input_files_exist_shall_output_an_error_if_file_does_not_exist)
{
  char* argv[5] = {"program_name", "-v", "suite1", "suite2", "suite3"};
  m.file_exists.retval = 0;
  all_input_files_exist(3 + 2, argv);
  assert_eq(3, m.fprintf.call_count);
}

test(all_input_files_exist_shall_return_1_if_all_files_exist)
{
  char* argv[5] = {"program_name", "-v", "suite1", "suite2", "suite3"};
  m.file_exists.retval = 1;
  assert_eq(1, all_input_files_exist(3 + 2, argv));
}

test(all_input_files_exist_shall_return_0_some_files_are_missing)
{
  char* argv[5] = {"program_name", "-v", "suite1", "suite2", "suite3"};
  m.file_exists.retval = 0;
  assert_eq(0, all_input_files_exist(3 + 2, argv));
}

/*****************************************************************************
 * run_test_suite()
 */
test(run_test_suite_shall_output_an_error_if_executable_name_is_null)
{
  run_test_suite(NULL, 0, 0);
#ifdef CUTEST_GCC
  assert_eq(1, m.fwrite.call_count);
  assert_eq(stderr, m.fwrite.args.arg3);
#else
  assert_eq(1, m.fprintf.call_count);
  assert_eq(stderr, m.fprintf.args.arg0);
#endif
}

test(run_test_suite_shall_allocate_correct_amount_of_memory_for_cmd)
{
  char buf[1024];
  memset(buf, 0, sizeof(buf));
  m.strlen.func = strlen;
  m.strstr.func = strstr;
  m.malloc.retval = buf;
  run_test_suite("bogus_suite_runner", 0, 0);
  assert_eq(1, m.malloc.call_count);
  assert_eq(strlen("./bogus_suite_runner -j -s") + 1, m.malloc.args.arg0);
}

test(run_test_suite_shall_allocate_correct_amount_of_memory_for_cmd_with_pfx)
{
  char buf[1024];
  memset(buf, 0, sizeof(buf));
  m.strlen.func = strlen;
  m.strstr.func = strstr;
  m.malloc.retval = buf;
  run_test_suite("./bogus_suite_runner", 0, 0);
  assert_eq(1, m.malloc.call_count);
  assert_eq(strlen("./bogus_suite_runner -j -s") + 1, m.malloc.args.arg0);
}

test(run_test_suite_shall_output_an_error_if_out_of_memory)
{
  char buf[1024];
  memset(buf, 0, sizeof(buf));
  m.strlen.func = strlen;
  m.strstr.func = strstr;
  m.malloc.retval = NULL;
  run_test_suite("./bogus_suite_runner", 0, 0);
#ifdef CUTEST_GCC
  assert_eq(1, m.fwrite.call_count);
  assert_eq(stderr, m.fwrite.args.arg3);
#else
  assert_eq(1, m.fprintf.call_count);
  assert_eq(stderr, m.fprintf.args.arg0);
#endif
}


test(run_test_suite_shall_allocate_correct_amount_of_memory_for_verbose_cmd)
{
  char buf[1024];
  memset(buf, 0, sizeof(buf));
  m.strlen.func = strlen;
  m.malloc.retval = buf;
  run_test_suite("bogus_suite_runner", 1, 0);
  assert_eq(1, m.malloc.call_count);
  assert_eq(strlen("./bogus_suite_runner -v -j -s") + 1, m.malloc.args.arg0);
}

test(run_test_suite_shall_allocate_correct_amount_of_memory_for_logging_cmd)
{
  char buf[1024];
  memset(buf, 0, sizeof(buf));
  m.strlen.func = strlen;
  m.malloc.retval = buf;
  run_test_suite("bogus_suite_runner", 1, 1);
  assert_eq(1, m.malloc.call_count);
  assert_eq(strlen("./bogus_suite_runner -v -j -s -l") + 1, m.malloc.args.arg0);
}

test(run_test_suite_shall_allocate_correct_amount_of_memory_for_valgrind_cmd)
{
  char buf[1024];
  memset(buf, 0, sizeof(buf));
  m.strlen.func = strlen;
  m.malloc.retval = buf;
  run_test_suite("bogus_suite_runner", 2, 0);
  assert_eq(1, m.malloc.call_count);
  assert_eq(strlen("valgrind --track-origins=yes -q ./bogus_suite_runner -v -j -s") + 1, m.malloc.args.arg0);
}

test(run_test_suite_shall_free_cmd)
{
  char buf[1024];
  memset(buf, 0, sizeof(buf));
  m.strlen.func = strlen;
  m.malloc.retval = buf;
  run_test_suite("bogus_suite_runner", 0, 0);
  assert_eq(1, m.free.call_count);
  assert_eq(buf, m.free.args.arg0);
}

char system_stub_arg[1024];
int system_stub(const char* command) {
  strcpy(system_stub_arg, command);
  return 1234;
}

module_test(run_test_suite_shall_execute_correct_command)
{
  m.system.func = system_stub;
  assert_eq(1234, run_test_suite("suite_runner", 0, 0))
  assert_eq("./suite_runner -j -s", system_stub_arg);
}

module_test(run_test_suite_shall_execute_correct_command_verbose)
{
  m.system.func = system_stub;
  assert_eq(1234, run_test_suite("suite_runner", 1, 0))
  assert_eq("./suite_runner -v -j -s", system_stub_arg);
}

module_test(run_test_suite_shall_execute_correct_command_no_line_feed)
{
  m.system.func = system_stub;
  assert_eq(1234, run_test_suite("suite_runner", -1, 0))
  assert_eq("./suite_runner -n -j -s", system_stub_arg);
}

module_test(run_test_suite_shall_execute_correct_command_logging)
{
  m.system.func = system_stub;
  assert_eq(1234, run_test_suite("suite_runner", -1, 1))
  assert_eq("./suite_runner -n -j -s -l", system_stub_arg);
}


/*****************************************************************************
 * run_test_suites()
 */
test(run_test_suites_shall_run_test_suite_for_every_suite)
{
  char* argv[] = {"program", "-v", "suite1", "suite2"};
  run_test_suites(0, 1, 4, argv, 123, 0);
  assert_eq(2, m.run_test_suite.call_count);
  assert_eq("suite2", m.run_test_suite.args.arg0);
  assert_eq(123, m.run_test_suite.args.arg1);
}

test(run_test_suites_shall_run_test_suite_for_every_one_suite)
{
  char* argv[] = {"program", "-v", "suite1"};
  run_test_suites(0, 1, 3, argv, 123, 0);
  assert_eq(1, m.run_test_suite.call_count);
  assert_eq("suite1", m.run_test_suite.args.arg0);
  assert_eq(123, m.run_test_suite.args.arg1);
}

test(run_test_suites_shall_return_negative_1_if_run_test_suite_fails)
{
  char* argv[] = {"program", "-v", "suite1"};
  m.run_test_suite.retval = -1;
  assert_eq(-1, run_test_suites(0, 1, 3, argv, 123, 0));
}

/*****************************************************************************
 * handle_args()
 */
test(handle_args_shall_call_usage_if_arg_count_is_off)
{
  char* argv[] = {"program_name"};
  handle_args(1, argv);
  assert_eq(1, m.usage.call_count);
  assert_eq("program_name", m.usage.args.arg0);
}

test(handle_args_shall_exit_with_EXIT_FAILURE_if_arg_count_is_off)
{
  char* argv[] = {"program_name", "-v"};
  handle_args(2, argv);
  m.all_input_files_exist.retval = 1;
  assert_eq(2, m.exit.call_count);
  assert_eq(EXIT_FAILURE, m.exit.args.arg0);
}

test(handle_args_shall_exit_with_EXIT_FAILURE_if_not_verbose_nor_nor_valgrind_no_line_feed)
{
  char* argv[] = {"program_name", "test_suite1", "test_suite2"};
  handle_args(3, argv);
  m.all_input_files_exist.retval = 1;
  assert_eq(1, m.exit.call_count);
  assert_eq(EXIT_FAILURE, m.exit.args.arg0);
}

test(handle_args_shall_call_usage_if_not_verbose_nor_no_line_feed)
{
  char* argv[] = {"program_name", "test_suite1", "test_suite2"};
  m.strcmp.retval = 1;
  handle_args(3, argv);
  assert_eq(1, m.usage.call_count);
}

test(handle_args_shall_exit_with_EXIT_FAILURE_if_test_suites_could_not_be_found)
{
  char* argv[] = {"program_name", "-v", "test_suite"};
  handle_args(3, argv);
  m.all_input_files_exist.retval = 0;
  assert_eq(1, m.exit.call_count);
  assert_eq(EXIT_FAILURE, m.exit.args.arg0);
}

test(handle_args_shall_return_1_if_verbose)
{
  char* argv[] = {"program_name", "-v", "test_suite"};
  m.strcmp.func = strcmp;
  m.all_input_files_exist.retval = 1;
  assert_eq(1, handle_args(3, argv));
}

test(handle_args_shall_return_2_if_valgrind)
{
  char* argv[] = {"program_name", "-V", "test_suite"};
  m.strcmp.func = strcmp;
  m.all_input_files_exist.retval = 1;
  assert_eq(2, handle_args(3, argv));
}

test(handle_args_shall_return_negative_1_if_no_line_feed)
{
  char* argv[] = {"program_name", "-n", "test_suite"};
  m.strcmp.func = strcmp;
  m.all_input_files_exist.retval = 1;
  assert_eq(-1, handle_args(3, argv));
}

test(handle_args_shall_print_usage_if_none_of_the_nVv_flags_are_provided)
{
  char* argv[] = {"program_name", "-?", "test_suite"};
  m.strcmp.retval = 1;
  handle_args(3, argv);
  assert_eq(1, m.usage.call_count);
}

test(handle_args_shall_exit_with_EXIT_FAILURE_if_none_of_the_nVv_flags)
{
  char* argv[] = {"program_name", "-?", "test_suite"};
  m.strcmp.retval = 1;
  m.all_input_files_exist.retval = 1;
  handle_args(3, argv);
  assert_eq(1, m.exit.call_count);
  assert_eq(EXIT_FAILURE, m.exit.call_count);
}

/*****************************************************************************
 * launch_child_processes()
 */
test(launch_child_processes_shall_fork_enough_times)
{
  launch_child_processes(8, 10, 0x1234, 0);
  assert_eq(8, m.fork.call_count);
}

test(launch_child_processes_shall_output_errors_if_fork_fails)
{
  m.fork.retval = -1;
  launch_child_processes(8, 10, 0x1234, 0);
#ifdef CUTEST_GCC
  assert_eq(8, m.fwrite.call_count);
#else
  assert_eq(8, m.fprintf.call_count);
#endif
}

test(launch_child_processes_shall_exit_with_EXIT_FAILURE_if_fork_fails)
{
  m.fork.retval = -1;
  launch_child_processes(8, 10, 0x1234, 0);
  assert_eq(8, m.exit.call_count);
  assert_eq(EXIT_FAILURE, m.exit.args.arg0);
}

test(launch_child_processes_shall_call_run_test_suites_for_each_fork)
{
  launch_child_processes(8, 10, 0x1234, 0);
  assert_eq(8, m.run_test_suites.call_count);
}

test(launch_child_processes_shall_exit_with_EXIT_SUCCESS_from_run_test_suites)
{
  m.run_test_suites.retval = 0;
  launch_child_processes(8, 10, 0x1234, 0);
  assert_eq(8, m.exit.call_count);
  assert_eq(EXIT_SUCCESS, m.exit.args.arg0);
}

test(launch_child_processes_shall_exit_with_EXIT_FAILURE_from_run_test_suites)
{
  m.run_test_suites.retval = 1234;
  launch_child_processes(8, 10, 0x1234, 0);
  assert_eq(8, m.exit.call_count);
  assert_eq(EXIT_FAILURE, m.exit.args.arg0);
}

/*****************************************************************************
 * slight_delay()
 */
test(slight_delay_shall_perform_a_simple_mathematical_calculation_to_delay)
{
  assert_eq(90, slight_delay());
}

/*****************************************************************************
 * wait_for_child_processes()
 */
test(wait_for_child_processes_shall_wait_for_all_child_processes_forked)
{
  wait_for_child_processes(8, 0);
  assert_eq(8, m.waitpid.call_count);
}

test(wait_for_child_processes_shall_output_a_line_feed_if_not_verbose)
{
  wait_for_child_processes(8, -1);
  assert_eq(1, m.puts.call_count);
}

test(wait_for_child_processes_shall_not_output_a_line_feed_if_verbose)
{
  wait_for_child_processes(8, 1);
  assert_eq(0, m.puts.call_count);
}

pid_t waitpid_stub(pid_t pid, int* status, int options)
{
  (void)pid;
  (void)options;
  *status = 1 << (m.waitpid.call_count - 1);
  return 0;
}

test(wait_for_child_processes_shall_return_a_ored_status)
{
  m.waitpid.func = waitpid_stub;
  assert_eq(EXIT_SUCCESS, wait_for_child_processes(3, 1));
}

/*****************************************************************************
 * print_log()
 */
test(print_log_shall_allocate_correct_number_of_bytes_for_log_file_name)
{
  m.strlen.func = strlen;
  print_log("command");
  assert_eq(1, m.malloc.call_count);
  assert_eq(strlen("command.log") + 1, m.malloc.args.arg0);
}

test(print_log_shall_output_an_error_message_if_allocation_failed)
{
  print_log("command");
#ifdef CUTEST_GCC
  assert_eq(1, m.fwrite.call_count);
  assert_eq(stderr, m.fwrite.args.arg3);
#else
  assert_eq(1, m.fprintf.call_count);
  assert_eq(stderr, m.fprintf.args.arg0);
#endif
}

test(print_log_shall_append_log_suffix_to_suite_name)
{
  char buf[128];
  m.memset.func = memset;
  m.strcpy.func = strcpy;
  m.strcat.func = strcat;
  m.malloc.retval = buf;
  print_log("suite_name");
  assert_eq("suite_name.log", buf);
}

test(print_log_shall_open_the_correct_file_for_reading)
{
  char buf[128];
  m.memset.func = memset;
  m.strcpy.func = strcpy;
  m.strcat.func = strcat;
  m.malloc.retval = buf;
  print_log("suite_name");
  assert_eq(1, m.fopen.call_count);
  assert_eq(buf, m.fopen.args.arg0);
  assert_eq("r", m.fopen.args.arg1);
}

test(print_log_shall_free_the_memory_if_a_file_could_not_be_opened)
{
  char buf[128];
  m.memset.func = memset;
  m.strcpy.func = strcpy;
  m.strcat.func = strcat;
  m.malloc.retval = buf;
  print_log("suite_name");
  assert_eq(1, m.free.call_count);
  assert_eq(buf, m.free.args.arg0);
}

static char* fgets_stub(char* s, int size, FILE *stream)
{
  (void)stream;
  (void)size;
  if (m.fgets.call_count >= 2) {
    return NULL;
  }
  return s;
}

test(print_log_shall_fgets_all_lines_in_a_file_to_fputs)
{
  char buf[128];
  m.memset.func = memset;
  m.strcpy.func = strcpy;
  m.strcat.func = strcat;
  m.malloc.retval = buf;
  m.fopen.retval = 0x1234;
  m.fgets.func = fgets_stub;
  print_log("suite_name");
  assert_eq(2, m.fgets.call_count);
  assert_eq(0x1234, m.fgets.args.arg2);
}

test(print_log_shall_fputs_all_lines_in_a_file_to_stderr)
{
  char buf[128];
  m.memset.func = memset;
  m.strcpy.func = strcpy;
  m.strcat.func = strcat;
  m.malloc.retval = buf;
  m.fopen.retval = 0x1234;
  m.fgets.func = fgets_stub;
  print_log("suite_name");
  assert_eq(1, m.fputs.call_count);
  assert_eq(stderr, m.fputs.args.arg1);
}

/*****************************************************************************
 * print_logs()
 */
test(print_logs_shall_call_print_log_for_every_suite)
{
  char* argv[5] = {"ignore", "ingore", "take0", "take1", "take2"};
  print_logs(5, argv);
  assert_eq(3, m.print_log.call_count);
}

/*****************************************************************************
 * launch_process()
 */
test(launch_process_shall_call_run_test_suites_to_launch_process)
{
  m.fprintf.func = fprintf;
  char* argv[] = {"3", "4"};
  launch_process(2, argv, 5);
  assert_eq(1, m.run_test_suites.call_count);
  assert_eq(0, m.run_test_suites.args.arg0);
  assert_eq(1, m.run_test_suites.args.arg1);
  assert_eq(2, m.run_test_suites.args.arg2);
  assert_eq(argv, m.run_test_suites.args.arg3);
  assert_eq(5, m.run_test_suites.args.arg4);
}

test(launch_process_shall_add_an_extra_line_feed_if_not_no_linefeed)
{
  char* argv[] = {"3", "4"};
  launch_process(2, argv, -1);
  assert_eq(1, m.puts.call_count);
}

test(launch_process_shall_not_add_an_extra_line_feed_if_no_linefeed)
{
  char* argv[] = {"3", "4"};
  launch_process(2, argv, 1);
  assert_eq(0, m.puts.call_count);
}

test(launch_process_shall_return_the_return_value_from_run_test_suites)
{
  char* argv[] = {"3", "4"};
  m.run_test_suites.retval = 1234;
  assert_eq(1234, launch_process(2, argv, 4));
}

/*****************************************************************************
 * main()
 */
test(main_shall_call_handle_args_correctly)
{
  main(1, 0x2);
  assert_eq(1, m.handle_args.call_count);
  assert_eq(1, m.handle_args.args.arg0);
  assert_eq(0x2, m.handle_args.args.arg1);
}

test(main_shall_get_the_number_of_cores_using_get_number_of_cores)
{
  main(1, 0x2);
  assert_eq(1, m.get_number_of_cores.call_count);
}

test(main_shall_call_launch_child_processes_if_allocated_cores_are_more_than_1)
{
  m.get_number_of_cores.retval = 4;
  m.handle_args.retval = 6;
  main(4, 0x5);
  assert_eq(1, m.launch_child_processes.call_count);
  assert_eq(3, m.launch_child_processes.args.arg0);
  assert_eq(4, m.launch_child_processes.args.arg1);
  assert_eq(0x5, m.launch_child_processes.args.arg2);
  assert_eq(6, m.launch_child_processes.args.arg3);

  assert_eq(0, m.launch_process.call_count);
}

test(main_shall_call_launch_processes_if_one_suite)
{
  m.get_number_of_cores.retval = 4;
  m.handle_args.retval = 3;
  main(1, 0x2);
  assert_eq(0, m.launch_child_processes.call_count);

  assert_eq(1, m.launch_process.call_count);
  assert_eq(1, m.launch_process.args.arg0);
  assert_eq(0x2, m.launch_process.args.arg1);
  assert_eq(3, m.launch_process.args.arg2);
}

test(main_shall_return_EXIT_FAILURE_if_launch_process_fails)
{
  m.get_number_of_cores.retval = 4;
  m.handle_args.retval = 3;
  m.launch_process.retval = -1;
  assert_eq(EXIT_FAILURE, main(1, 0x2));
}

test(main_shall_return_EXIT_SUCCESS_if_launch_process_fails)
{
  m.get_number_of_cores.retval = 4;
  m.handle_args.retval = 3;
  m.launch_process.retval = 0;
  assert_eq(EXIT_SUCCESS, main(1, 0x2));
}

#undef main
