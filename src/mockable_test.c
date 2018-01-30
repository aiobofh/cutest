#include "cutest.h"

#define CUTEST_DEP arg.o helpers.o

/* For convenience 'm' is shorter to write than 'cutest_mock' */
#define m cutest_mock

#include "mockable.h"

/*****************************************************************************
 * allocate_mockable_node()
 */

test(allocate_mockable_node_shall_allocate_memory_for_a_new_node)
{
  (void)allocate_mockable_node();
  assert_eq(1, m.malloc.call_count);
  assert_eq(sizeof(mockable_node_t), m.malloc.args.arg0);
}

test(allocate_mockable_node_shall_output_an_error_if_allocation_failed)
{
  allocate_mockable_node();
#ifdef CUTEST_GCC
  assert_eq(1, m.fwrite.call_count);
  assert_eq(stderr, m.fwrite.args.arg3);
#else
  assert_eq(1, m.fprintf.call_count);
  assert_eq(stderr, m.fprintf.args.arg0);
#endif
}

test(allocate_mockable_node_shall_return_pointer_to_new_node_if_all_is_ok)
{
  mockable_node_t node;
  m.malloc.retval = &node;
  assert_eq(&node, allocate_mockable_node());
}

test(allocate_mockable_node_shall_set_the_next_pointer_to_null)
{
  mockable_node_t node;
  node.next = (mockable_node_t*)0x1234;
  m.malloc.retval = &node;
  allocate_mockable_node();
  assert_eq(NULL, node.next);
}

test(allocate_mockable_node_shall_set_allocate_a_list_for_function_arguments)
{
  mockable_node_t node;
  node.next = (mockable_node_t*)0x1234;
  m.malloc.retval = &node;
  m.new_arg_list.retval = (arg_list_t*)0x5678;
  allocate_mockable_node();
  assert_eq(0x5678, node.args);
}

test(allocate_mockable_node_shall_return_null_if_something_wrong)
{
  assert_eq(NULL, allocate_mockable_node());
}

/*****************************************************************************
 * free_mockable_node()
 */

test(free_mockable_node_shall_free_node)
{
  free_mockable_node((mockable_node_t*)0x1234);
  assert_eq(1, m.free.call_count);
  assert_eq(0x1234, m.free.args.arg0);
}

/*****************************************************************************
 * new_symbol_name()
 */

test(new_symbol_shall_grab_the_string_length_of_the_input_symbol_name)
{
  new_symbol_name((const char*)0x1234);
  assert_eq(1, m.strlen.call_count);
  assert_eq(0x1234, m.strlen.args.arg0);
}

test(new_symbol_name_shall_allocate_correct_amount_of_memory_for_name)
{
  m.strlen.retval = 10;
  new_symbol_name((const char*)0x1234);
  assert_eq(1, m.malloc.call_count);
  assert_eq(10 + 1, m.malloc.args.arg0); /* plus \0 */
}

test(new_symbol_name_shall_output_an_error_if_allocation_failed)
{
  new_symbol_name((const char*)0x1234);
#ifdef CUTEST_GCC
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
  m.malloc.retval = (char*)0x1234;
  new_symbol_name((const char*)0x5678);
  assert_eq(1, m.strncpy.call_count);
  assert_eq(0x1234, m.strncpy.args.arg0);
  assert_eq(0x5678, m.strncpy.args.arg1);
  assert_eq(10 + 1, m.strncpy.args.arg2); /* plus \0 */
}

test(new_symbol_name_shall_return_pointer_to_the_new_string_if_all_is_ok)
{
  m.malloc.retval = (char*)0x1234;
  assert_eq(0x1234, new_symbol_name((const char*)0x1234));
}

test(new_symbol_name_shall_return_null_if_something_went_wrong)
{
  assert_eq(NULL, new_symbol_name((const char*)0x1234));
}

/*****************************************************************************
 * delete_symbol_name()
 */

test(delete_symbol_name_free_the_symbol_name)
{
  delete_symbol_name((char*)0x1234);
  assert_eq(1, m.free.call_count);
  assert_eq(0x1234, m.free.args.arg0);
}

/*****************************************************************************
 * delete_symbol_name()
 */

test(delete_return_type_name_free_the_return_type_name)
{
  delete_return_type_name((char*)0x1234);
  assert_eq(1, m.free.call_count);
  assert_eq(0x1234, m.free.args.arg0);
}

/*****************************************************************************
 * new_mockable_node()
 */

test(new_mockable_node_shall_allocate_mockable_node)
{
  new_mockable_node(NULL);
  assert_eq(1, m.allocate_mockable_node.call_count);
}

test(new_mockable_node_shall_return_null_if_allocation_of_node_failed)
{
  assert_eq(NULL, new_mockable_node(NULL));
}

test(new_mockable_node_shall_create_a_symbol_name_string)
{
  m.allocate_mockable_node.retval = (mockable_node_t*)0x5678;
  new_mockable_node((const char*)0x1234);
  assert_eq(1, m.new_symbol_name.call_count);
  assert_eq(0x1234, m.new_symbol_name.args.arg0);
}

test(new_mockable_node_shall_free_node_if_symbol_name_creation_failed)
{
  m.allocate_mockable_node.retval = (mockable_node_t*)0x1234;
  new_mockable_node(NULL);
  assert_eq(1, m.free_mockable_node.call_count);
  assert_eq(0x1234, m.free_mockable_node.args.arg0);
}

test(new_mockable_node_shall_return_null_if_symbol_name_creation_failed)
{
  m.allocate_mockable_node.retval = (mockable_node_t*)0x1234;
  assert_eq(NULL, new_mockable_node(NULL));
}

test(new_mockable_node_shall_set_the_symbol_name_of_the_node)
{
  mockable_node_t node;
  m.allocate_mockable_node.retval = &node;
  m.new_symbol_name.retval = "The symbol :)";
  new_mockable_node(NULL);
  assert_eq("The symbol :)", node.symbol_name);
}

test(new_mockable_node_shall_return_the_pointer_to_the_node_if_all_is_ok)
{
  mockable_node_t node;
  m.allocate_mockable_node.retval = &node;
  m.new_symbol_name.retval = (char*)0x1234;
  assert_eq(&node, new_mockable_node(NULL));
}

module_test(new_mockable_node_shall_create_a_new_node_with_a_symbol_name)
{
  mockable_node_t* node = new_mockable_node("The symbol :)");
  assert_eq("The symbol :)", node->symbol_name);
  delete_mockable_node(node);
}

/*****************************************************************************
 * delete_mockable_node()
 */

test(delete_mockable_node_shall_delete_symbol_name_if_set)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));
  node.symbol_name = (char*)0x1234;
  delete_mockable_node(&node);
  assert_eq(1, m.delete_symbol_name.call_count);
  assert_eq(0x1234, m.delete_symbol_name.args.arg0);
}

test(delete_mockable_node_shall_not_delete_symbol_name_if_not_set)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));
  node.symbol_name = NULL;
  delete_mockable_node(&node);
  assert_eq(0, m.delete_symbol_name.call_count);
}

test(delete_mockable_node_shall_argument_list_if_set)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));
  node.args = (arg_list_t*)0x1234;
  delete_mockable_node(&node);
  assert_eq(1, m.delete_arg_list.call_count);
  assert_eq(0x1234, m.delete_arg_list.args.arg0);
}

test(delete_mockable_node_shall_not_delete_argument_list_if_not_set)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));
  node.args = NULL;
  delete_mockable_node(&node);
  assert_eq(0, m.delete_arg_list.call_count);
}

test(delete_mockable_node_shall_delete_return_type_name_if_set)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));
  node.return_type.name = (char*)0x1234;
  delete_mockable_node(&node);
  assert_eq(1, m.delete_return_type_name.call_count);
  assert_eq(0x1234, m.delete_return_type_name.args.arg0);
}

test(delete_mockable_node_shall_not_delete_return_type_name_if_not_set)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));
  node.return_type.name = NULL;
  delete_mockable_node(&node);
  assert_eq(0, m.delete_return_type_name.call_count);
}
