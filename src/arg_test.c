#include "cutest.h"

/* For convenience 'm' is shorter to write than 'cutest_mock' */
#define m cutest_mock

#include "arg.h"

/*****************************************************************************
 * allocate_arg_node()
 */

test(allocate_arg_node_shall_allocate_memory_for_a_new_node)
{
  (void)allocate_arg_node();
  assert_eq(1, m.malloc.call_count);
  assert_eq(sizeof(arg_node_t), m.malloc.args.arg0);
}

test(allocate_arg_node_shall_output_an_error_if_allocation_failed)
{
  allocate_arg_node();
#ifdef CUTEST_GCC
  assert_eq(1, m.fwrite.call_count);
  assert_eq(stderr, m.fwrite.args.arg3);
#else
  assert_eq(1, m.fprintf.call_count);
  assert_eq(stderr, m.fprintf.args.arg0);
#endif
}

test(allocate_arg_node_shall_return_pointer_to_new_node_if_all_is_ok)
{
  m.malloc.retval = 0x1234;
  assert_eq(0x1234, allocate_arg_node());
}

test(allocate_arg_node_shall_return_null_if_something_wrong)
{
  assert_eq(NULL, allocate_arg_node());
}


/*****************************************************************************
 * free_arg_node()
 */

test(free_arg_node_shall_free_node)
{
  free_arg_node(0x1234);
  assert_eq(1, m.free.call_count);
  assert_eq(0x1234, m.free.args.arg0);
}

/*****************************************************************************
 * new_arg()
 */

test(new_arg_shall_grab_the_string_length_of_the_input_arg)
{
  new_arg(0x1234);
  assert_eq(1, m.strlen.call_count);
  assert_eq(0x1234, m.strlen.args.arg0);
}

test(new_arg_shall_allocate_correct_amount_of_memory_for_name)
{
  m.strlen.retval = 10;
  new_arg(NULL);
  assert_eq(1, m.malloc.call_count);
  assert_eq(10 + 1, m.malloc.args.arg0); // plus \0
}

test(new_arg_shall_output_an_error_if_allocation_failed)
{
  new_arg(NULL);
#ifdef CUTEST_GCC
  assert_eq(1, m.fwrite.call_count);
  assert_eq(stderr, m.fwrite.args.arg3);
#else
  assert_eq(1, m.fprintf.call_count);
  assert_eq(stderr, m.fprintf.args.arg0);
#endif
}

test(new_arg_shall_new_the_full_string)
{
  m.strlen.retval = 10;
  m.malloc.retval = 0x1234;
  new_arg(0x5678);
  assert_eq(1, m.strncpy.call_count);
  assert_eq(0x1234, m.strncpy.args.arg0);
  assert_eq(0x5678, m.strncpy.args.arg1);
  assert_eq(10 + 1, m.strncpy.args.arg2); // plus \0
}

test(new_arg_shall_return_pointer_to_the_new_string_if_all_is_ok)
{
  m.malloc.retval = 0x1234;
  assert_eq(0x1234, new_arg(NULL));
}

test(new_arg_shall_return_null_if_something_went_wrong)
{
  assert_eq(NULL, new_arg(NULL));
}

/*****************************************************************************
 * delete_arg()
 */

test(delete_arg_free_the_arg)
{
  delete_arg(0x1234);
  assert_eq(1, m.free.call_count);
  assert_eq(0x1234, m.free.args.arg0);
}

/*****************************************************************************
 * delete_type()
 */
test(delete_type_free_the_type)
{
  delete_type(0x1234);
  assert_eq(1, m.free.call_count);
  assert_eq(0x1234, m.free.args.arg0);
}

/*****************************************************************************
 * delete_mock_declaration_name()
 */
test(delete_mock_declaration_name_free_the_mock_declaration_name)
{
  delete_mock_declaration_name(0x1234);
  assert_eq(1, m.free.call_count);
  assert_eq(0x1234, m.free.args.arg0);
}

/*****************************************************************************
 * delete_args_control_name()
 */
test(delete_args_control_name_free_the_args_control_name)
{
  delete_args_control_name(0x1234);
  assert_eq(1, m.free.call_count);
  assert_eq(0x1234, m.free.args.arg0);
}

/*****************************************************************************
 * delete_assignment_name()
 */
test(delete_assignment_name_free_the_assignment_name)
{
  delete_assignment_name(0x1234);
  assert_eq(1, m.free.call_count);
  assert_eq(0x1234, m.free.args.arg0);
}

/*****************************************************************************
 * delete_assignment_variable()
 */
test(delete_assignment_variable_free_the_assignment_variable)
{
  delete_assignment_variable(0x1234);
  assert_eq(1, m.free.call_count);
  assert_eq(0x1234, m.free.args.arg0);
}

/*****************************************************************************
 * delete_caller_name()
 */
test(delete_caller_name_free_the_caller_name)
{
  delete_caller_name(0x1234);
  assert_eq(1, m.free.call_count);
  assert_eq(0x1234, m.free.args.arg0);
}

/*****************************************************************************
 * delete_assignment_type_cast()
 */
test(delete_assignment_type_cast_free_the_assignment_type_cast)
{
  delete_assignment_type_cast(0x1234);
  assert_eq(1, m.free.call_count);
  assert_eq(0x1234, m.free.args.arg0);
}

/*****************************************************************************
 * new_arg_node()
 */

test(new_arg_node_shall_allocate_arg_node)
{
  new_arg_node(NULL);
  assert_eq(1, m.allocate_arg_node.call_count);
}

test(new_arg_node_shall_return_null_if_allocation_of_node_failed)
{
  assert_eq(NULL, new_arg_node(NULL));
}

test(new_arg_node_shall_create_a_arg_string)
{
  m.allocate_arg_node.retval = 0x5678;
  new_arg_node(0x1234);
  assert_eq(1, m.new_arg.call_count);
  assert_eq(0x1234, m.new_arg.args.arg0);
}

test(new_arg_node_shall_free_node_if_arg_creation_failed)
{
  m.allocate_arg_node.retval = 0x1234;
  new_arg_node(NULL);
  assert_eq(1, m.free_arg_node.call_count);
  assert_eq(0x1234, m.free_arg_node.args.arg0);
}

test(new_arg_node_shall_return_null_if_arg_creation_failed)
{
  m.allocate_arg_node.retval = 0x1234;
  assert_eq(NULL, new_arg_node(NULL));
}

test(new_arg_node_shall_set_the_arg_of_the_node)
{
  arg_node_t node;
  m.allocate_arg_node.retval = &node;
  m.new_arg.retval = "The arg :)";
  new_arg_node(NULL);
  assert_eq("The arg :)", node.arg);
}

test(new_arg_node_shall_return_the_pointer_to_the_node_if_all_is_ok)
{
  arg_node_t node;
  m.allocate_arg_node.retval = &node;
  m.new_arg.retval = 0x1234;
  assert_eq(&node, new_arg_node(NULL));
}

module_test(new_arg_node_shall_create_a_new_node_with_a_arg)
{
  arg_node_t* node = new_arg_node("The arg :)");
  assert_eq("The arg :)", node->arg);
  delete_arg_node(node);
}

/*****************************************************************************
 * delete_arg_node()
 */

test(delete_arg_node_shall_delete_arg_if_set)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.arg = 0x1234;
  delete_arg_node(&node);
  assert_eq(1, m.delete_arg.call_count);
  assert_eq(0x1234, m.delete_arg.args.arg0);
}

test(delete_arg_node_shall_not_delete_arg_if_not_set)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.arg = NULL;
  delete_arg_node(&node);
  assert_eq(0, m.delete_arg.call_count);
}

test(delete_arg_node_shall_delete_type_if_set)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.type = 0x1234;
  delete_arg_node(&node);
  assert_eq(1, m.delete_type.call_count);
  assert_eq(0x1234, m.delete_type.args.arg0);
}

test(delete_arg_node_shall_not_delete_type_if_not_set)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.type = NULL;
  delete_arg_node(&node);
  assert_eq(0, m.delete_type.call_count);
}

test(delete_arg_node_shall_delete_mock_declaration_name_if_set)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.mock_declaration.name = 0x1234;
  delete_arg_node(&node);
  assert_eq(1, m.delete_mock_declaration_name.call_count);
  assert_eq(0x1234, m.delete_mock_declaration_name.args.arg0);
}

test(delete_arg_node_shall_not_delete_mock_declaration_name_if_not_set)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.mock_declaration.name = NULL;
  delete_arg_node(&node);
  assert_eq(0, m.delete_mock_declaration_name.call_count);
}

test(delete_arg_node_shall_delete_args_control_name_if_set)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.args_control.name = 0x1234;
  delete_arg_node(&node);
  assert_eq(1, m.delete_args_control_name.call_count);
  assert_eq(0x1234, m.delete_args_control_name.args.arg0);
}

test(delete_arg_node_shall_not_delete_args_control_name_if_not_set)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.args_control.name = NULL;
  delete_arg_node(&node);
  assert_eq(0, m.delete_args_control_name.call_count);
}

test(delete_arg_node_shall_delete_assignment_name_if_set)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.assignment.name = 0x1234;
  delete_arg_node(&node);
  assert_eq(1, m.delete_assignment_name.call_count);
  assert_eq(0x1234, m.delete_assignment_name.args.arg0);
}

test(delete_arg_node_shall_not_delete_assignment_name_if_not_set)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.assignment.name = NULL;
  delete_arg_node(&node);
  assert_eq(0, m.delete_assignment_name.call_count);
}

test(delete_arg_node_shall_delete_assignment_variable_if_set)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.assignment.variable = 0x1234;
  delete_arg_node(&node);
  assert_eq(1, m.delete_assignment_variable.call_count);
  assert_eq(0x1234, m.delete_assignment_variable.args.arg0);
}

test(delete_arg_node_shall_not_delete_assignment_variable_if_not_set)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.assignment.variable = NULL;
  delete_arg_node(&node);
  assert_eq(0, m.delete_assignment_variable.call_count);
}

test(delete_arg_node_shall_delete_caller_name_if_set)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.caller.name = 0x1234;
  delete_arg_node(&node);
  assert_eq(1, m.delete_caller_name.call_count);
  assert_eq(0x1234, m.delete_caller_name.args.arg0);
}

test(delete_arg_node_shall_not_delete_caller_name_if_not_set)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.caller.name = NULL;
  delete_arg_node(&node);
  assert_eq(0, m.delete_caller_name.call_count);
}

test(delete_arg_node_shall_delete_assignment_type_cast_if_set)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.assignment.type_cast = 0x1234;
  delete_arg_node(&node);
  assert_eq(1, m.delete_assignment_type_cast.call_count);
  assert_eq(0x1234, m.delete_assignment_type_cast.args.arg0);
}

test(delete_arg_node_shall_not_delete_assignment_type_cast_if_not_set)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.assignment.type_cast = NULL;
  delete_arg_node(&node);
  assert_eq(0, m.delete_assignment_type_cast.call_count);
}

test(delete_arg_node_shall_free_the_node) {
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  delete_arg_node(&node);
  assert_eq(1, m.free.call_count);
  assert_eq(&node, m.free.args.arg0);
}
