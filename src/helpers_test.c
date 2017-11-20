#include "cutest.h"

/* For convenience 'm' is shorter to write than 'cutest_mock' */
#define m cutest_mock

/*****************************************************************************
 * file_exists()
 */
test(file_exists_shall_call_fopen_correctly)
{
  file_exists("some_file");
  assert_eq(1, m.fopen.call_count);
  assert_eq("some_file", m.fopen.args.arg0);
  assert_eq("r", m.fopen.args.arg1);
}

test(file_exists_shall_return_0_if_file_does_not_exist)
{
  assert_eq(0, file_exists("some_file"));
}

test(file_sxists_shall_print_an_error_if_file_does_not_exist)
{
  file_exists("some_file");
  assert_eq(1, m.fprintf.call_count);
  assert_eq(stderr, m.fprintf.args.arg0);
}

test(file_exists_shall_return_1_if_file_exists)
{
  m.fopen.retval = 0x1234;
  assert_eq(1, file_exists("some_file"));
}

test(file_exists_shall_call_fclose_correctly_if_file_exists)
{
  m.fopen.retval = 0x1234;
  file_exists("some_file");
  assert_eq(1, m.fclose.call_count);
  assert_eq(0x1234, m.fclose.args.arg0);
}

test(file_exists_shall_not_call_fclose_if_file_could_not_be_opened)
{
  file_exists("some_file");
  assert_eq(0, m.fclose.call_count);
}

module_test(file_exists_shall_return_1_if_a_file_exists_physically)
{
  assert_eq(1, file_exists("cutest_run_test.c"));
}

module_test(file_exists_shall_return_0_if_a_file_does_not_exists_physically)
{
  m.fprintf.func = NULL;
  assert_eq(0, file_exists("oogabooga"));
}
