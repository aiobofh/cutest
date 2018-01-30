#include <string.h>

#include "cutest.h"

#include "cutest_conf.h"

/*#define main MAIN */
#define m cutest_mock

test(cc_have_option_shall_construct_a_command_using_sprintf)
{
  cc_have_option("some option", "exe", "exe.c");
  assert_eq(1, m.sprintf.call_count);
}

test(cc_have_option_shall_call_system_to_evaluate_option)
{
  cc_have_option("some option", "exe", "exe.c");
  assert_eq(1, m.system.call_count);
}

test(cc_have_option_shall_return_0_if_option_does_notexists)
{
  m.system.retval = EXIT_FAILURE;
  assert_eq(0, cc_have_option("some option", "exe", "exe.c"));
}

test(cc_have_option_shall_return_1_if_option_exists)
{
  m.system.retval = EXIT_SUCCESS;
  assert_eq(1, cc_have_option("some option", "exe", "exe.c"));
}

/*****************************************************************************
 * generate_tmp_executable_name_filename()
 */
test(generate_tmp_executable_filename_shall_allocate_memory_for_filename)
{
  generate_tmp_executable_filename();
  assert_eq(1, m.malloc.call_count);
  assert_eq(strlen("tmpXXXXXX") + 1, m.malloc.args.arg0);
}

test(generate_tmp_executable_filename_shall_return_null_if_out_of_memory)
{
  assert_eq(NULL, generate_tmp_executable_filename());
}

test(generate_tmp_executable_filename_shall_copy_template_for_filename)
{
  char buf[10];
  m.malloc.retval = buf;
  generate_tmp_executable_filename();
  assert_eq(1, m.memcpy.call_count);
  assert_eq(buf, m.memcpy.args.arg0);
  assert_eq("tmpXXXXXX", m.memcpy.args.arg1);
}

test(generate_tmp_executable_filename_shall_call_tmpnam_correctly)
{
  char buf[10];
  m.malloc.retval = buf;
  generate_tmp_executable_filename();
  assert_eq(1, m.mkstemp.call_count);
  assert_eq(buf, m.mkstemp.args.arg0);
}

test(generate_tmp_executable_filename_shall_return_the_pointer_to_the_new_filename)
{
  char buf[10];
  m.malloc.retval = buf;
  assert_eq(buf, generate_tmp_executable_filename());
}

/*****************************************************************************
 * generate_tmp_executable_source_filename()
 */
test(generate_tmp_source_filename_shall_allocate_memory_for_filename)
{
  m.strlen.func = strlen;
  generate_tmp_source_filename("basename");
  assert_eq(1, m.malloc.call_count);
  assert_eq(strlen("basename.c") + 1, m.malloc.args.arg0);
}

test(generate_tmp_source_filename_shall_return_null_if_out_of_memory)
{
  assert_eq(NULL, generate_tmp_source_filename("basename"));
}

test(generate_tmp_source_filename_shall_copy_the_basename_to_the_new_allocation)
{
  char cname[11];
  char* name = "basename";
  m.malloc.retval = cname;
  generate_tmp_source_filename(name);
  assert_eq(1, m.memcpy.call_count);
  assert_eq(cname, m.memcpy.args.arg0);
  assert_eq(name, m.memcpy.args.arg1);
}

module_test(generate_tmp_source_filename_shall_append_dot_c_to_basename)
{
  char* ptr = NULL;
  assert_eq("basename.c", ptr = generate_tmp_source_filename("basename"));
  free(ptr);
}

/****************************************************************************
 * generate_tmp_program_for_cc_evaluation()
 */
test(generate_tmp_program_for_cc_evaluation_shall_open_the_correct_file_for_writing)
{
  generate_tmp_program_for_cc_evaluation("my_file");
  assert_eq(1, m.fopen.call_count);
  assert_eq("my_file", m.fopen.args.arg0);
  assert_eq("w", m.fopen.args.arg1);
}

test(generate_tmp_program_for_cc_evaluation_shall_return_0_if_unable_to_write)
{
  assert_eq(0, generate_tmp_program_for_cc_evaluation("my_file"));
}

test(generate_tmp_program_for_cc_evaluation_shall_write_a_naive_program)
{
  m.fopen.retval = 0x1234;
  generate_tmp_program_for_cc_evaluation("my_file");
#ifdef CUTEST_GCC
  assert_eq(1, m.fwrite.call_count);
  assert_eq(0x1234, m.fwrite.args.arg3);
#else
  assert_eq(1, m.fputs.call_count);
  assert_eq(0x1234, m.fputs.args.arg1);
#endif
}

test(generate_tmp_program_for_cc_evaluation_shall_close_the_file)
{
  m.fopen.retval = 0x1234;
  generate_tmp_program_for_cc_evaluation("my_file");
  assert_eq(1, m.fclose.call_count);
  assert_eq(0x1234, m.fclose.args.arg0);
}

test(generate_tmp_program_for_cc_evaluation_shall_return_1_on_success)
{
  m.fopen.retval = 0x1234;
  assert_eq(1, generate_tmp_program_for_cc_evaluation("my_file"));
}

/****************************************************************************
 * get_cc_capabilities()
 */
test(get_cc_capabilities_shall_check_for_all_capabilites)
{
  cc_capabilities_t c;
  m.generate_tmp_program_for_cc_evaluation.retval = 1;
  get_cc_capabilities(&c, "tmp", "tmp.c");
  assert_eq(sizeof(cc_capabilities_t) / sizeof(int), m.cc_have_option.call_count);
}

/*****************************************************************************
 * main()
 */

test(main_shall_print_error_if_too_few_arguments)
{
  char* argv[4] = {"progname", "src", "tst", "cutest"};
  main(4, argv);
#ifdef CUTEST_GCC
  assert_eq(1, m.fwrite.call_count);
#else
  assert_eq(1, m.fprintf.call_count);
#endif
}

test(main_shall_allocate_temprary_name_for_tmp_executable)
{
  char* argv[5] = {"progname", "src", "tst", "cutest", "cutest"};
  main(5, argv);
  assert_eq(1, m.generate_tmp_executable_filename.call_count);
}

test(main_shall_allocate_temprary_name_for_tmp_source)
{
  char* argv[5] = {"progname", "src", "tst", "cutest", "cutest"};
  main(5, argv);
  assert_eq(1, m.generate_tmp_source_filename.call_count);
}

test(main_shall_get_compiler_capabilities)
{
  char* argv[5] = {"progname", "src", "tst", "cutest", "cutest"};
  m.generate_tmp_executable_filename.retval = 0x1234;
  m.generate_tmp_source_filename.retval = 0x5678;
  m.generate_tmp_program_for_cc_evaluation.retval = 1;
  main(5, argv);
  assert_eq(1, m.get_cc_capabilities.call_count);
  assert_eq(0x1234, m.get_cc_capabilities.args.arg1);
  assert_eq(0x5678, m.get_cc_capabilities.args.arg2);
}

test(main_shall_print_header)
{
  char* argv[5] = {"progname", "src", "tst", "cutest", "cutest"};
  m.generate_tmp_program_for_cc_evaluation.retval = 1;
  main(5, argv);
  assert_eq(1, m.print_head.call_count);
}

test(main_shall_print_capabilities)
{
  char* argv[5] = {"progname", "src", "tst", "cutest", "cutest"};
  m.generate_tmp_program_for_cc_evaluation.retval = 1;
  main(5, argv);
  assert_eq(1, m.print_cc_capabilities.call_count);
}

test(main_shall_print_paths)
{
  char* argv[5] = {"progname", "src", "tst", "cutest", "cutest"};
  m.generate_tmp_program_for_cc_evaluation.retval = 1;
  main(5, argv);
  assert_eq(1, m.print_paths.call_count);
}

test(main_shall_print_footer)
{
  char* argv[5] = {"progname", "src", "tst", "cutest", "cutest"};
  m.generate_tmp_program_for_cc_evaluation.retval = 1;
  main(5, argv);
  assert_eq(1, m.print_footer.call_count);
}

const char* remove_stub_filename[3];
int remove_stub(const char* pathname)
{
  remove_stub_filename[m.remove.call_count - 1] = pathname;
  return 0;
}

test(main_shall_remove_the_temporary_files)
{
  char* argv[5] = {"progname", "src", "tst", "cutest", "cutest"};
  m.generate_tmp_executable_filename.retval = 0x1234;
  m.generate_tmp_source_filename.retval = 0x5678;
  m.generate_tmp_program_for_cc_evaluation.retval = 1;
  m.remove.func = remove_stub;
  main(5, argv);
  assert_eq(2, m.remove.call_count);
  assert_eq(0x1234, remove_stub_filename[0]);
  assert_eq(0x5678, remove_stub_filename[1]);
}

const char* free_stub_ptr[3];
void free_stub(void* ptr)
{
  free_stub_ptr[m.free.call_count - 1] = ptr;
}

test(main_shall_free_temproary_filenames)
{
  char* argv[5] = {"progname", "src", "tst", "cutest", "cutest"};
  m.generate_tmp_executable_filename.retval = 0x1234;
  m.generate_tmp_source_filename.retval = 0x5678;
  m.generate_tmp_program_for_cc_evaluation.retval = 1;
  m.free.func = free_stub;
  main(5, argv);
  assert_eq(2, m.free.call_count);
  assert_eq(0x1234, free_stub_ptr[0]);
  assert_eq(0x5678, free_stub_ptr[1]);
}

test(main_shall_return_EXIT_SUCCESS_if_all_is_ok)
{
  char* argv[5] = {"progname", "src", "tst", "cutest", "cutest"};
  m.generate_tmp_program_for_cc_evaluation.retval = 1;
  assert_eq(EXIT_SUCCESS, main(5, argv));
}

#undef main
