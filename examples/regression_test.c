#include "cutest.h"

#define m cutest_mock

test(return_int_shall_return_1234) {
  assert_eq(1234, return_int());
}

test(use_return_int_shall_forward_return_int) {
  m.return_int.retval = 4321;
  assert_eq(4321, use_return_int());
}

module_test(use_return_int_shall_forward_return_int_1234) {
  assert_eq(1234, use_return_int());
}

test(return_int_ptr_shall_return_1234) {
  assert_eq(1234, return_int_ptr());
}

test(use_return_int_ptr_shall_forward_return_int_ptr) {
  m.return_int_ptr.retval = 4321;
  assert_eq(4321, use_return_int_ptr());
}

module_test(use_return_int_ptr_shall_forward_return_int_ptr_1234)
{
  assert_eq(1234, use_return_int());
}

test(return_my_struct_ptr_shall_return_1234) {
  assert_eq(1234, return_my_struct_ptr());
}

test(use_return_my_struct_ptr_shall_forward_return_my_struct_ptr) {
  m.return_my_struct_ptr.retval = 4321;
  assert_eq(4321, use_return_my_struct_ptr());
}

module_test(use_return_my_struct_ptr_shall_return_my_struct_ptr_1234) {
  assert_eq(1234, use_return_my_struct_ptr());
}

test(return_const_char_ptr_shall_return_1234) {
  assert_eq(1234, return_const_char_ptr());
}

test(use_return_const_char_ptr_shall_forward_return_const_char_ptr) {
  m.return_const_char_ptr.retval = 4321;
  assert_eq(4321, use_return_const_char_ptr());
}

module_test(use_return_const_char_ptr_shall_forward_return_const_char_ptr_1234) {
  assert_eq(1234, use_return_const_char_ptr());
}

test(return_cumbersome_my_struct_ptr_shall_return_1234) {
  assert_eq(1234, return_cumbersome_my_struct_ptr());
}

test(use_return_cumbersome_my_struct_ptr_shall_forward_return_cumbersome_my_struct_ptr) {
  m.return_cumbersome_my_struct_ptr.retval = 4321;
  assert_eq(4321, use_return_cumbersome_my_struct_ptr());
}

module_test(use_return_cumbersome_my_struct_ptr_shall_return_cumbersome_my_struct_ptr_1234) {
  assert_eq(1234, use_return_cumbersome_my_struct_ptr());
}

test(arg_int_shall_return_argument) {
  assert_eq(1234, arg_int(1234, 5678));
}

test(use_arg_int_shall_call_arg_int_and_forward_return) {
  m.arg_int.retval = 4321;
  assert_eq(4321, use_arg_int(1234, 5678));
  assert_eq(1234, m.arg_int.args.arg0);
  assert_eq(5678, m.arg_int.args.arg1);
}

module_test(use_arg_int_shall_call_arg_int_and_forward_return_given_as_input)
{
  assert_eq(1234, use_arg_int(1234, 5678));
}

test(arg_const_int_shall_return_argument) {
  assert_eq(1234, arg_const_int(1234, 5678));
}

test(use_arg_const_int_shall_call_arg_const_int_and_forward_return) {
  m.arg_const_int.retval = 4321;
  assert_eq(4321, use_arg_const_int(1234, 5678));
  assert_eq(1234, m.arg_const_int.args.arg0);
  assert_eq(5678, m.arg_const_int.args.arg1);
}

module_test(use_arg_const_int_shall_call_arg_const_int_and_forward_return_given_as_input)
{
  assert_eq(1234, use_arg_const_int(1234, 5678));
}


test(arg_char_ptr_shall_return_argument) {
  assert_eq(1234, arg_char_ptr(1234, 5678));
}

test(use_arg_char_ptr_shall_call_arg_char_ptr_and_forward_return) {
  m.arg_char_ptr.retval = 4321;
  assert_eq(4321, use_arg_char_ptr(1234, 5678));
  assert_eq(1234, m.arg_char_ptr.args.arg0);
  assert_eq(5678, m.arg_char_ptr.args.arg1);
}

module_test(use_arg_char_ptr_shall_call_arg_char_ptr_and_forward_return_given_as_input)
{
  assert_eq(1234, use_arg_char_ptr(1234, 5678));
}

test(arg_array_shall_return_argument) {
  assert_eq(1234, arg_array(1234, 5678));
}

test(use_arg_array_shall_call_arg_array_and_forward_return) {
  m.arg_array.retval = 4321;
  assert_eq(4321, use_arg_array(1234, 5678));
  assert_eq(1234, m.arg_array.args.arg0);
  assert_eq(5678, m.arg_array.args.arg1);
}

module_test(use_arg_array_shall_call_arg_array_and_forward_return_given_as_input)
{
  assert_eq(1234, use_arg_array(1234, 5678));
}

test(arg_function_pointer_shall_return_argument)
{
  assert_eq(1234, arg_function_pointer(1234, arg_int));
}

test(use_arg_function_pointer_shall_call_use_function_pointer_and_return_argument)
{
  m.arg_function_pointer.retval = 4321;
  assert_eq(4321, use_arg_function_pointer(1234, arg_int));
  assert_eq(1234, m.arg_function_pointer.args.arg0);
  assert_eq(arg_int, m.arg_function_pointer.args.arg1);
}

module_test(use_arg_function_pointer_shall_call_the_given_pointer_and_return) {
  assert_eq(1234, use_arg_function_pointer(1234, arg_int));
}

test(use_arg_function_pointer_with_mockable_shall_call_arg_function_pointer_with_a_mocked_function)
{
  use_arg_function_pointer_with_mockable(1234);
  assert_eq(1, m.arg_function_pointer.call_count);
  assert_eq(cutest_arg_int, m.arg_function_pointer.args.arg1);
}

module_test(use_arg_function_pointer_with_mockable_shall_call_arg_function_pointer_with_a_mocked_function_and_use_it)
{
  use_arg_function_pointer_with_mockable(1234);
  assert_eq(1, m.arg_int.call_count);
}

test(arg_array_ptr_shall_return_argument)
{
  assert_eq(1234, arg_array_ptr(1234, 5678));
}

test(use_arg_array_ptr_shall_call_arg_array_ptr_and_forward_return)
{
  m.arg_array_ptr.retval = 1234;
  assert_eq(1234, use_arg_array_ptr(1234, 5678));
  assert_eq(1, m.arg_array_ptr.call_count);
  assert_eq(1234, m.arg_array_ptr.args.arg0);
  assert_eq(5678, m.arg_array_ptr.args.arg1);
}

test(arg_variadic_shall_return_argument)
{
  assert_eq(1234, arg_variadic(1234));
}
