#include "cutest.h"

#define FOPEN_OK_RETVAL (FILE*)1234
#define FPUTS_OK_RETVAL 1
#define FCLOSE_NOT_OK_RETVAL 1

/*
 * Some helper functions to make the test cases more readable, you can
 * consider these as set-up functions/fixtures for the test-cases in this
 * file.
 */
void pretend_that_fopen_will_go_well()
{
  cutest_mock.fopen.retval = FOPEN_OK_RETVAL;
}

void pretend_that_fopen_will_fail()
{
  /* CUTest will automaticall set fopen's retval to 0 */
}

void pretend_that_fputs_will_fail()
{
  /* CUTest will automaticall set fputs's retval to 0 */
}

void pretend_that_fputs_will_go_well()
{
  cutest_mock.fputs.retval = FPUTS_OK_RETVAL;
}

void pretend_that_fclose_will_go_well()
{
  /* CUTest will automaticall set fclose's retval to 0 */
}

void pretend_that_fclose_will_fail()
{
  cutest_mock.fclose.retval = FCLOSE_NOT_OK_RETVAL;
}

void pretend_that_fopen_will_go_well_but_fputs_will_fail()
{
  pretend_that_fopen_will_go_well();
  pretend_that_fputs_will_fail();
}

void pretend_that_fopen_and_fputs_will_go_well()
{
  pretend_that_fopen_will_go_well();
  pretend_that_fputs_will_go_well();
}

void pretend_that_fopen_and_fputs_will_go_well_but_fclose_will_fail()
{
  pretend_that_fopen_and_fputs_will_go_well();
  pretend_that_fclose_will_fail();
}

void pretend_that_fopen_fputs_and_fclose_will_go_well()
{
  pretend_that_fopen_and_fputs_will_go_well();
  pretend_that_fclose_will_go_well();
}

/************************************************************************
 *
 * Unit-tests for write_file()
 *
 */
test(write_file_shall_open_the_correct_file_for_writing)
{
  const char* filename = "my_filename.txt";

  (void)write_file(filename);

  assert_eq(filename, cutest_mock.fopen.args.arg0);
  assert_eq(0, strcmp("w", cutest_mock.fopen.args.arg1));
}

test(write_file_shall_return_1_if_file_could_not_be_opened)
{
  pretend_that_fopen_will_fail();

  assert_eq(1, write_file("my_filename.txt"));
}

test(write_file_shall_return_2_if_file_could_not_be_written)
{
  pretend_that_fopen_will_go_well_but_fputs_will_fail();

  assert_eq(2, write_file("my_filename.txt"));
}

test(write_file_shall_return_3_if_file_could_not_be_closed)
{
  pretend_that_fopen_and_fputs_will_go_well_but_fclose_will_fail();

  assert_eq(3, write_file("my_filename.txt"));
}

test(write_file_shall_write_10_lines_to_the_opened_file)
{
  pretend_that_fopen_and_fputs_will_go_well();

  (void)write_file("my_filename.txt");

  assert_eq(10, cutest_mock.fputs.call_count);
}

test(write_file_shall_close_the_opened_file_if_fopen_managed_to_open_it)
{
  pretend_that_fopen_will_go_well();

  (void)write_file("my_filename.txt");

  assert_eq(1, cutest_mock.fclose.call_count);
  assert_eq(FOPEN_OK_RETVAL, cutest_mock.fclose.args.arg0);
}

test(write_file_shall_close_the_opened_file_if_unable_to_write_to_file)
{
  pretend_that_fopen_will_go_well_but_fputs_will_fail();

  (void)write_file("my_filename.txt");

  assert_eq(1, cutest_mock.fputs.call_count);
  assert_eq(1, cutest_mock.fclose.call_count);
  assert_eq(FOPEN_OK_RETVAL, cutest_mock.fclose.args.arg0);
}

test(write_file_shall_close_the_opened_file_if_able_to_write_to_file)
{
  pretend_that_fopen_and_fputs_will_go_well();

  (void)write_file("my_filename.txt");

  assert_eq(1, cutest_mock.fclose.call_count);
  assert_eq(FOPEN_OK_RETVAL, cutest_mock.fclose.args.arg0);
}

test(write_file_shall_return_0_if_all_went_well) {
  pretend_that_fopen_fputs_and_fclose_will_go_well();

  assert_eq(0, write_file("my_filename.txt"));
}
