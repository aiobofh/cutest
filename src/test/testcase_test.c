#include "cutest.h"

/* For convenience 'm' is shorter to write than 'cutest_mock' */
#define m cutest_mock

#include "testcase.h"

/*****************************************************************************
 * allocate_testcase_node()
 */

test(allocate_testcase_node_shall_allocate_memory_for_a_new_node)
{
  (void)allocate_testcase_node();
  assert_eq(1, m.malloc.call_count);
  assert_eq(sizeof(testcase_node_t), m.malloc.args.arg0);
}

test(allocate_testcase_node_shall_output_an_error_if_allocation_failed)
{
  allocate_testcase_node();
#ifdef __GNUC__
  assert_eq(1, m.fwrite.call_count);
  assert_eq(stderr, m.fwrite.args.arg3);
#else
  assert_eq(1, m.fprintf.call_count);
  assert_eq(stderr, m.fprintf.args.arg0);
#endif
}

test(allocate_testcase_node_shall_return_pointer_to_new_node_if_all_is_ok)
{
  testcase_node_t node;
  m.malloc.retval = &node;
  assert_eq(&node, allocate_testcase_node());
}

test(allocate_testcase_node_shall_set_the_next_pointer_to_null)
{
  testcase_node_t node;
  node.next = (testcase_node_t*)0x1234;
  m.malloc.retval = &node;
  m.memset.func = memset;
  allocate_testcase_node();
  assert_eq(NULL, node.next);
}

test(allocate_testcase_node_shall_return_null_if_something_wrong)
{
  assert_eq(NULL, allocate_testcase_node());
}

/*****************************************************************************
 * free_testcase_node()
 */

test(free_testcase_node_shall_free_node)
{
  free_testcase_node(0x1234);
  assert_eq(1, m.free.call_count);
  assert_eq(0x1234, m.free.args.arg0);
}

/*****************************************************************************
 * new_testcase()
 */

test(new_symbol_shall_grab_the_string_length_of_the_input_testcase)
{
  new_testcase(0x1234);
  assert_eq(1, m.strlen.call_count);
  assert_eq(0x1234, m.strlen.args.arg0);
}

test(new_testcase_shall_allocate_correct_amount_of_memory_for_name)
{
  m.strlen.retval = 10;
  new_testcase(0x1234);
  assert_eq(1, m.malloc.call_count);
  assert_eq(10 + 1, m.malloc.args.arg0); // plus \0
}

test(new_testcase_shall_output_an_error_if_allocation_failed)
{
  new_testcase(0x1234);
#ifdef __GNUC__
  assert_eq(1, m.fwrite.call_count);
  assert_eq(stderr, m.fwrite.args.arg3);
#else
  assert_eq(1, m.fprintf.call_count);
  assert_eq(stderr, m.fprintf.args.arg0);
#endif
}

test(new_symbol_shall_new_the_full_string)
{
  m.strlen.retval = 10;
  m.malloc.retval = 0x1234;
  new_testcase(0x5678);
  assert_eq(1, m.strncpy.call_count);
  assert_eq(0x1234, m.strncpy.args.arg0);
  assert_eq(0x5678, m.strncpy.args.arg1);
  assert_eq(10 + 1, m.strncpy.args.arg2); // plus \0
}

test(new_testcase_shall_return_pointer_to_the_new_string_if_all_is_ok)
{
  m.malloc.retval = 0x1234;
  assert_eq(0x1234, new_testcase(0x1234));
}

test(new_testcase_shall_return_null_if_something_went_wrong)
{
  assert_eq(NULL, new_testcase(0x1234));
}

/*****************************************************************************
 * delete_testcase()
 */

test(delete_testcase_free_the_testcase)
{
  delete_testcase(0x1234);
  assert_eq(1, m.free.call_count);
  assert_eq(0x1234, m.free.args.arg0);
}

/*****************************************************************************
 * new_testcase_node()
 */

test(new_testcase_node_shall_allocate_testcase_node)
{
  new_testcase_node(NULL, 5);
  assert_eq(1, m.allocate_testcase_node.call_count);
}

test(new_testcase_node_shall_return_null_if_allocation_of_node_failed)
{
  assert_eq(NULL, new_testcase_node(NULL, 5));
}

test(new_testcase_node_shall_create_a_testcase_string)
{
  m.allocate_testcase_node.retval = 0x5678;
  new_testcase_node(0x1234, 5);
  assert_eq(1, m.new_testcase.call_count);
  assert_eq(0x1234, m.new_testcase.args.arg0);
}

test(new_testcase_node_shall_free_node_if_testcase_creation_failed)
{
  m.allocate_testcase_node.retval = 0x1234;
  new_testcase_node(NULL, 5);
  assert_eq(1, m.free_testcase_node.call_count);
  assert_eq(0x1234, m.free_testcase_node.args.arg0);
}

test(new_testcase_node_shall_return_null_if_testcase_creation_failed)
{
  m.allocate_testcase_node.retval = 0x1234;
  assert_eq(NULL, new_testcase_node(NULL, 5));
}

test(new_testcase_node_shall_set_the_testcase_of_the_node)
{
  testcase_node_t node;
  m.allocate_testcase_node.retval = &node;
  m.new_testcase.retval = "The testcase :)";
  new_testcase_node(NULL, 5);
  assert_eq("The testcase :)", node.testcase);
}

test(new_testcase_node_shall_return_the_pointer_to_the_node_if_all_is_ok)
{
  testcase_node_t node;
  m.allocate_testcase_node.retval = &node;
  m.new_testcase.retval = 0x1234;
  assert_eq(&node, new_testcase_node(NULL, 5));
}

module_test(new_testcase_node_shall_create_a_new_node_with_a_testcase)
{
  testcase_node_t* node = new_testcase_node("The testcase :)", 5);
  assert_eq("The testcase :)", node->testcase);
  delete_testcase_node(node);
}

/*****************************************************************************
 * delete_testcase_node()
 */

test(delete_testcase_node_shall_delete_testcase_if_set)
{
  testcase_node_t node;
  memset(&node, 0, sizeof(node));
  node.testcase = 0x1234;
  delete_testcase_node(&node);
  assert_eq(1, m.delete_testcase.call_count);
  assert_eq(0x1234, m.delete_testcase.args.arg0);
}

test(delete_testcase_node_shall_not_delete_testcase_if_not_set)
{
  testcase_node_t node;
  memset(&node, 0, sizeof(node));
  node.testcase = NULL;
  delete_testcase_node(&node);
  assert_eq(0, m.delete_testcase.call_count);
}

test(delete_testcase_node_shall_delete_testcase_node)
{
  testcase_node_t node;
  memset(&node, 0, sizeof(node));
  node.testcase = 0x1234;
  delete_testcase_node(&node);
  assert_eq(1, m.free_testcase_node.call_count);
  assert_eq(&node, m.free_testcase_node.args.arg0);
}
