#include "cutest.h"

test(bar_shall_multiply_input_argument_by_four)
{
  assert_eq(0, bar(0));
  assert_eq(4, bar(1));
  assert_eq(8, bar(2));
}

test(foo_shall_not_call_bar_if_argument_is_null)
{
  foo(NULL);
  assert_eq(0, cutest_mock.bar.call_count);
}

test(foo_shall_call_bar_with_dereferenced_argument_if_argument_is_not_null)
{
  int my_integer = 3;
  foo(&my_integer);
  assert_eq(1, cutest_mock.bar.call_count);
  assert_eq(3, cutest_mock.bar.args.arg0);
}

test(foo_shall_propagate_bar_return_value_to_integer_refered_by_argument)
{
  int my_integer = 3;
  cutest_mock.bar.retval = 123456;
  foo(&my_integer);
  assert_eq(123456, my_integer);
}

module_test(foo_shall_multiply_input_argument_by_four)
{
  int my_integer = 3;
  foo(&my_integer);
  assert_eq(12, my_integer);
}

test(moo_shall_not_call_bar_if_argument_is_null)
{
  moo(NULL);
  assert_eq(0, cutest_mock.mii.call_count);
}

module_test(moo_shall_multiply_input_argument_by_four)
{
  int my_integer = 3;
  moo(&my_integer);
  assert_eq(12, my_integer);
  assert_eq(1, cutest_mock.mii.call_count);
}

module_test(muu_shall_multiply_input_array_by_four)
{
  int val1 = 1;
  int val2 = 3;
  int* my_integers[2] = {&val1, &val2};
  muu(my_integers);
  assert_eq(4, val1);
  assert_eq(12, val2);
}
