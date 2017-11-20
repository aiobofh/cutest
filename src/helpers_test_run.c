/*
 * This file is generated by './cutest_run helpers_test.c helpers_mocks.h'.
 *
 * Compile this program to get a test-case executor.
 *
 */

#define _XOPEN_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define main MAIN /* To make sure we can test main() */
#include "helpers_mocks.h" /* Mock-up functions and design under test */
#include "helpers_test.c" /* Test-cases */
#undef main

int main(int argc, char* argv[])
{
  cutest_startup(argc, argv, "helpers_test.c");

  if (1 == cutest_test_name_argument_given("file_exists_shall_call_fopen_correctly")) {
    cutest_execute_test(cutest_file_exists_shall_call_fopen_correctly, "file_exists_shall_call_fopen_correctly", 0, argv[0]);
  }
  if (1 == cutest_test_name_argument_given("file_exists_shall_return_0_if_file_does_not_exist")) {
    cutest_execute_test(cutest_file_exists_shall_return_0_if_file_does_not_exist, "file_exists_shall_return_0_if_file_does_not_exist", 0, argv[0]);
  }
  if (1 == cutest_test_name_argument_given("file_sxists_shall_print_an_error_if_file_does_not_exist")) {
    cutest_execute_test(cutest_file_sxists_shall_print_an_error_if_file_does_not_exist, "file_sxists_shall_print_an_error_if_file_does_not_exist", 0, argv[0]);
  }
  if (1 == cutest_test_name_argument_given("file_exists_shall_return_1_if_file_exists")) {
    cutest_execute_test(cutest_file_exists_shall_return_1_if_file_exists, "file_exists_shall_return_1_if_file_exists", 0, argv[0]);
  }
  if (1 == cutest_test_name_argument_given("file_exists_shall_call_fclose_correctly_if_file_exists")) {
    cutest_execute_test(cutest_file_exists_shall_call_fclose_correctly_if_file_exists, "file_exists_shall_call_fclose_correctly_if_file_exists", 0, argv[0]);
  }
  if (1 == cutest_test_name_argument_given("file_exists_shall_not_call_fclose_if_file_could_not_be_opened")) {
    cutest_execute_test(cutest_file_exists_shall_not_call_fclose_if_file_could_not_be_opened, "file_exists_shall_not_call_fclose_if_file_could_not_be_opened", 0, argv[0]);
  }
  if (1 == cutest_test_name_argument_given("file_exists_shall_return_1_if_a_file_exists_physically")) {
    cutest_execute_test(cutest_file_exists_shall_return_1_if_a_file_exists_physically, "file_exists_shall_return_1_if_a_file_exists_physically", 1, argv[0]);
  }
  if (1 == cutest_test_name_argument_given("file_exists_shall_return_0_if_a_file_does_not_exists_physically")) {
    cutest_execute_test(cutest_file_exists_shall_return_0_if_a_file_does_not_exists_physically, "file_exists_shall_return_0_if_a_file_does_not_exists_physically", 1, argv[0]);
  }
  if (1 == cutest_test_name_argument_given("count_white_spaces_shall_return_the_number_of_white_spaces_in_begining")) {
    cutest_execute_test(cutest_count_white_spaces_shall_return_the_number_of_white_spaces_in_begining, "count_white_spaces_shall_return_the_number_of_white_spaces_in_begining", 0, argv[0]);
  }
  if (1 == cutest_test_name_argument_given("count_white_spaces_shall_return_max_length_if_overrun")) {
    cutest_execute_test(cutest_count_white_spaces_shall_return_max_length_if_overrun, "count_white_spaces_shall_return_max_length_if_overrun", 0, argv[0]);
  }
  cutest_shutdown("helpers_test.c");

  return cutest_exit_code;
}
