#include "cutest.h"

test(write_file_shall_open_the_correct_file_for_writing)
{
  const char* filename = "my_filename.txt";

  (void)write_file(filename);

  assert_eq(filename, cutest_mock.fopen.args.arg0);
  assert_eq(0, strcmp("w", cutest_mock.fopen.args.arg1));
}

test(write_file_shall_return_1_if_file_could_not_be_opened)
{
  /* CUTest will automaticall set fopen's retval to 0 */
  assert_eq(1, write_file("my_filename.txt"));
}

test(write_file_shall_return_2_if_file_could_not_be_written)
{
  /* CUTest will automaticall set fputs's retval to 0 */
  cutest_mock.fopen.retval = (FILE*)1; /* Pretend that fopen went well */

  assert_eq(2, write_file("my_filename.txt"));
}

test(write_file_shall_return_3_if_file_could_not_be_closed)
{
  /* CUTest will automaticall set fclose's retval to 0 */
  cutest_mock.fopen.retval = (FILE*)1; /* Pretend that fopen went well */
  cutest_mock.fputs.retval = 1; /* Pretend that fputs went well */
  cutest_mock.fclose.retval = 1; /* Pretend that fclose failed */

  assert_eq(3, write_file("my_filename.txt"));
}

test(write_file_shall_write_10_lines_to_the_opened_file)
{
  cutest_mock.fopen.retval = (FILE*)1; /* Pretend that fopen went well */
  cutest_mock.fputs.retval = 1; /* Pretend that fputs went well */

  (void)write_file("my_filename.txt");

  assert_eq(10, cutest_mock.fputs.call_count);
}

test(write_file_shall_close_the_opened_file_if_fopen_managed_to_open_it)
{
  cutest_mock.fopen.retval = (FILE*)123; /* Pretend that fopen went well */

  (void)write_file("my_filename.txt");

  assert_eq(1, cutest_mock.fclose.call_count);
  assert_eq((FILE*)123, cutest_mock.fclose.args.arg0);
}

test(write_file_shall_close_the_opened_file_if_unable_to_write_to_file)
{
  /* CUTest will automaticall set fputs's retval to 0 */
  cutest_mock.fopen.retval = (FILE*)123; /* Pretend that fopen went well */

  (void)write_file("my_filename.txt");

  assert_eq(1, cutest_mock.fputs.call_count);
  assert_eq(1, cutest_mock.fclose.call_count);
  assert_eq((FILE*)123, cutest_mock.fclose.args.arg0);
}

test(write_file_shall_close_the_opened_file_if_able_to_write_to_file)
{
  cutest_mock.fopen.retval = (FILE*)123; /* Pretend that fopen went well */
  cutest_mock.fputs.retval = 1; /* Pretend that fputs went well */

  (void)write_file("my_filename.txt");

  assert_eq(1, cutest_mock.fclose.call_count);
  assert_eq((FILE*)123, cutest_mock.fclose.args.arg0);
}

test(write_file_shall_return_0_if_all_went_well) {
  cutest_mock.fopen.retval = (FILE*)1; /* Pretend that fopen went well */
  cutest_mock.fputs.retval = 1; /* Pretend that fputs went well */

  assert_eq(0, write_file("my_filename.txt"));
}
