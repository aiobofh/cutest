#include "cutest.h"

#define CUTEST_DEP arg.o helpers.o mockable.o

#include <assert.h>

#include "mockable.h"
#include "cutest_mock.h"

#define m cutest_mock
/* #define main MAIN */

/*****************************************************************************
 * usage()
 */

test(usage_shall_print_something)
{
  usage(0x1234);
  assert_eq(1, m.printf.call_count);
}

/*****************************************************************************
 * find_name_pos()
 */

test(find_name_pos_shall_return_negative_1_if_no_first_word_was_found)
{
  assert_eq(-1, find_name_pos(NULL));
}

test(find_name_pos_shall_return_negative_1_if_left_bracket_was_found)
{
  m.strlen.func = strlen;
  assert_eq(-1, find_name_pos("static int[]"));
}

test(find_name_pos_shall_return_position_of_the_end_of_return_type)
{
  m.strlen.func = strlen;
  assert_eq(18, find_name_pos("static inline int returning_int("));
}

/*****************************************************************************
 * get_prefix_attributes()
 */

module_test(get_prefix_attributes_shall_set_the_static_return_attribute)
{
  return_type_t return_type;
  memset(&return_type, 0, sizeof(return_type));
  get_prefix_attributes(&return_type, "static int foo()",
                        strlen("static int "));
  assert_eq(1, return_type.is_static);
}

module_test(get_prefix_attributes_shall_set_the_struct_return_attribute)
{
  return_type_t return_type;
  memset(&return_type, 0, sizeof(return_type));
  get_prefix_attributes(&return_type, "struct olle foo()",
                        strlen("struct olle "));
  assert_eq(1, return_type.is_struct);
}

module_test(get_prefix_attributes_shall_set_the_inline_return_attribute)
{
  return_type_t return_type;
  memset(&return_type, 0, sizeof(return_type));
  get_prefix_attributes(&return_type, "inline int foo()",
                        strlen("inline int "));
  assert_eq(1, return_type.is_inline);
}

module_test(get_prefix_attributes_shall_not_set_any_attribute_if_extension)
{
  return_type_t return_type;
  memset(&return_type, 0, sizeof(return_type));
  get_prefix_attributes(&return_type, "__extension__ int foo()",
                        strlen("inline int "));
  assert_eq(0, return_type.is_static);
  assert_eq(0, return_type.is_struct);
  assert_eq(0, return_type.is_inline);
}

module_test(get_prefix_attributes_shall_set_all_return_attribute)
{
  return_type_t return_type;
  memset(&return_type, 0, sizeof(return_type));
  get_prefix_attributes(&return_type, "static inline struct olle foo()",
                        strlen("static inline struct olle "));
  assert_eq(1, return_type.is_static);
  assert_eq(1, return_type.is_struct);
  assert_eq(1, return_type.is_inline);
}

module_test(get_prefix_attributes_shall_return_the_position_of_the_datatype)
{
  return_type_t return_type;
  memset(&return_type, 0, sizeof(return_type));
  assert_eq(strlen("static inline struct "),
            get_prefix_attributes(&return_type,
                                  "static inline struct olle foo()",
                                  strlen("static inline struct olle ")));
}

/*****************************************************************************
 * copy_return_type_name()
 */

module_test(copy_return_type_name_shall_copy_return_type_name_in_all_forms)
{
  char dst[12];
  copy_return_type_name(dst, "data_type function_name(", strlen("datatype "));
  assert_eq("data_type", dst);

  copy_return_type_name(dst, "data_type* function_name(", strlen("datatype "));
  assert_eq("data_type*", dst);

  copy_return_type_name(dst, "data_type *function_name(", strlen("datatype "));
  assert_eq("data_type*", dst);

  copy_return_type_name(dst, "data_type **function_name(", strlen("datatype "));
  assert_eq("data_type**", dst);
}

module_test(copy_return_type_shall_return_the_position_to_the_func_name)
{
  char dst[12];
  assert_eq(strlen("data_type **"),
            copy_return_type_name(dst, "data_type **function_name(", strlen("datatype ")));
}

test(copy_return_type_shall_return_0_if_return_type_was_only_an_empty_string)
{
  /* This can happen if a stderr, stdout or similar symbol is misinterpreted */
  char dst[12];
  assert_eq(0, copy_return_type_name(dst, "function_name(", strlen("")));
}

/*****************************************************************************
 * get_return_type()
 */

test(get_return_type_shall_return_zero_if_no_return_type_was_found)
{
  return_type_t return_type;
  memset(&return_type, 0, sizeof(return_type));
  m.find_name_pos.retval = -1;
  assert_eq(0, get_return_type(&return_type, "blabba"));
}

test(get_return_type_shall_allocate_enough_memory_to_contain_the_type_name)
{
  return_type_t return_type;
  memset(&return_type, 0, sizeof(return_type));
  m.find_name_pos.retval = 1234;
  get_return_type(&return_type, "blabba");
  assert_eq(1, m.malloc.call_count);
  assert_eq(1237, m.malloc.args.arg0);
}

test(get_return_type_shall_return_null_if_out_of_memory)
{
  return_type_t return_type;
  memset(&return_type, 0, sizeof(return_type));
  m.find_name_pos.retval = 1234;
  assert_eq(NULL, get_return_type(&return_type, "blabba"));
}

test(get_return_type_shall_call_get_prefix_attributes_correctly)
{
  return_type_t return_type;
  memset(&return_type, 0, sizeof(return_type));

  const char* str = "int foo()";

  m.find_name_pos.retval = strlen(str);
  m.malloc.retval = 0x1234;

  get_return_type(&return_type, str);

  assert_eq(1, m.get_prefix_attributes.call_count);
  assert_eq(&return_type, m.get_prefix_attributes.args.arg0);
  assert_eq(str, m.get_prefix_attributes.args.arg1);
}

test(get_return_type_shall_call_copy_return_type_name_correctly)
{
  return_type_t return_type;
  memset(&return_type, 0, sizeof(return_type));

  const char* str = "int foo()";
  m.find_name_pos.retval = strlen(str);
  m.malloc.retval = 0x1234;

  get_return_type(&return_type, str);

  assert_eq(1, m.copy_return_type_name.call_count);
  assert_eq(0x1234, m.copy_return_type_name.args.arg0);
  assert_eq(str, m.copy_return_type_name.args.arg1);
}

module_test(get_return_type_shall_get_the_return_type_from_a_func_prototype)
{
  return_type_t return_type;
  memset(&return_type, 0, sizeof(return_type));

  get_return_type(&return_type, "static  inline   struct olle **func();");

  assert_eq(1, return_type.is_static);
  assert_eq(1, return_type.is_inline);
  assert_eq(1, return_type.is_struct);
  assert_eq("olle**", return_type.name);

  free(return_type.name);
}

/*****************************************************************************
 * get_function_name()
 */

test(get_function_name_shall_search_until_left_parenthesis_and_copy_name)
{
  char dst[80];
  m.strlen.func = strlen;

  memset(dst, 0, sizeof(dst));
  get_function_name(dst, "this_is_a_function(and here are args);");

  assert_eq("this_is_a_function", dst);
}

test(get_function_name_shall_give_up_search_if_found_a_semi_colon_ret_minus)
{
  char dst[80];
  m.strlen.func = strlen;

  memset(dst, 0, sizeof(dst));
  assert_eq(-1, get_function_name(dst, "this_is_not_a_func_its_a_variable;"));
}

test(get_function_name_shall_give_up_search_if_parentheis_nor_smicolon_was_found)
{
  char dst[80];
  m.strlen.func = strlen;

  memset(dst, 0, sizeof(dst));
  assert_eq(-1, get_function_name(dst, "this_is_not_a_func_its_a_variable"));
}

test(get_function_shall_return_the_position_after_the_parenthesis_if_found)
{
  char dst[80];
  m.strlen.func = strlen;

  memset(dst, 0, sizeof(dst));
  assert_eq(strlen("this_is_a_function("),
            get_function_name(dst, "this_is_a_function(and args);"));
}

/*****************************************************************************
 * find_next_comma_in_args()
 */

test(find_next_comma_in_args_should_return_the_pos_of_next_comma)
{
  m.strlen.func = strlen;
  assert_eq(7, find_next_comma_in_args("int bar)"));
}

test(find_next_comma_in_args_should_return_the_pos_of_last_parenthesis)
{
  m.strlen.func = strlen;
  assert_eq(7, find_next_comma_in_args("int foo, int bar"));
}

test(find_next_comma_in_args_should_return_the_pos_after_func_ptrs_too)
{
  m.strlen.func = strlen;
  assert_eq(24, find_next_comma_in_args("int (*foo)(int i, int j), int bar"));
}

test(find_next_comman_in_arg_should_return_0_if_no_comma_was_found)
{
  m.strlen.func = strlen;
  assert_eq(0, find_next_comma_in_args("habblahubbla"));
}

/*****************************************************************************
 * is_basic_type()
 */

module_test(is_basic_type_shall_return_0_if_input_string_is_not_a_type)
{
  assert_eq(0, is_basic_type("oogabooga"));
}

module_test(is_basic_type_shall_return_1_if_input_string_is_a_type)
{
  assert_eq(1, is_basic_type("_Bool"));
  assert_eq(1, is_basic_type("char"));
  assert_eq(1, is_basic_type("int"));
  assert_eq(1, is_basic_type("float"));
  assert_eq(1, is_basic_type("double"));
  assert_eq(1, is_basic_type("long"));
  assert_eq(1, is_basic_type("void"));
}

/*****************************************************************************
 * find_type_len()
 */
module_test(find_type_len_shall_scan_input_for_the_end_pos_of_a_simple_type)
{
  //                    v
  const char* str = "int foo";
  assert_eq(3, find_type_len(str, strlen(str)));
}

module_test(find_type_len_shall_scan_input_for_the_end_pos_of_a_datatype)
{
  //                                         v
  const char* str = "const unsigned long long *pointer_to_long";
  assert_eq(24, find_type_len(str, strlen(str)));
}

module_test(find_type_len_shall_scan_input_for_the_end_pos_of_a_unkown_datatype)
{
  //                                         v
  const char* str = "hobbalabobba fjoppa moof *pointer_to_long";
  assert_eq(24, find_type_len(str, strlen(str)));
}

module_test(find_type_len_shall_scan_input_for_the_end_pos_of_func_ptr_type)
{
  //                                         v
  const char* str = "const unsigned long long (*pointer_to_long)(int, int)";
  assert_eq(24, find_type_len(str, strlen(str)));
}

module_test(find_type_len_shall_scan_input_and_allow_own_types)
{
  //                           v
  const char* str = "const FILE *file";
  assert_eq(10, find_type_len(str, strlen(str)));
}

module_test(find_type_len_shall_scan_input_and_anything_as_last_resort)
{
  //                                  v
  const char* str = "very_strange_thing";
  assert_eq(18, find_type_len(str, strlen(str)));
}

module_test(find_type_len_shall_scan_input_and_handle_variadic_args)
{
  //                   v
  const char* str = "...";
  assert_eq(3, find_type_len(str, strlen(str)));
}

/*****************************************************************************
 * add_new_arg_node()
 */

test(add_new_arg_node_shall_return_null_if_new_arg_node_failed)
{
  assert_eq(NULL, add_new_arg_node(NULL, NULL, 0));
}

test(add_new_arg_node_shall_allocate_a_new_node_for_each_file_item)
{
  m.memcpy.func = memcpy;
  m.memset.func = memset;

  add_new_arg_node(NULL, "foo", 3);

  assert_eq(1, m.new_arg_node.call_count);
  /*
   * This can not be checked 100% safely, since the argument is on the stack
   * belonging to the add_new_arg_node() function. Valgrind will complain.
   *
   * assert_eq("foo", m.new_arg_node.args.arg0);
   */
}

test(add_new_arg_node_shall_add_the_node_to_the_list)
{
  arg_list_t list;
  arg_node_t new_node;

  m.new_arg_node.retval = &new_node;

  add_new_arg_node(&list, "testing", 7);

  assert_eq(1, m.arg_list_add_node.call_count);
  assert_eq(&list, m.arg_list_add_node.args.arg0);
  assert_eq(&new_node, m.arg_list_add_node.args.arg1);
}

test(add_new_arg_node_shall_return_the_new_node)
{
  arg_list_t list;
  arg_node_t new_node;

  m.new_arg_node.retval = &new_node;

  assert_eq(&new_node, add_new_arg_node(&list, "testing", 7));
}

/*****************************************************************************
 * split2args()
 */

test(split2args_shall_use_find_next_comma_in_args_correctly)
{
  const char* src = "foobar";
  arg_list_t list;
  split2args(&list, src);
  assert_eq(1, m.find_next_comma_in_args.call_count);
  assert_eq(src, m.find_next_comma_in_args.args.arg0);
}

int find_next_comma_in_args_stub_cnt = 0;
static size_t find_next_comma_in_args_stub(const char* buf)
{
  size_t retval = 0;
  (void)buf;
  if (4 > find_next_comma_in_args_stub_cnt) {
    retval = 2;
  }
  find_next_comma_in_args_stub_cnt++;
  return retval;
}

test(split2args_shall_add_4_nodes_if_4_arguments_were_found)
{
  arg_list_t list;
  find_next_comma_in_args_stub_cnt = 0;

  m.find_next_comma_in_args.func = find_next_comma_in_args_stub;

  split2args(&list, "mumbojumbo");

  assert_eq(4, m.add_new_arg_node.call_count);

  find_next_comma_in_args_stub_cnt = 0;
}

test(split2args_shall_return_end_of_input_string)
{
  arg_list_t list;
  find_next_comma_in_args_stub_cnt = 0;

  m.find_next_comma_in_args.func = find_next_comma_in_args_stub;

  assert_eq(8, split2args(&list, "mumbojumbo"));

  find_next_comma_in_args_stub_cnt = 0;
}

/*****************************************************************************
 * extract_type_to_string()
 */
test(extract_type_to_string_shall_calculate_length_of_source_string)
{
  char* dst;
  char* src = 0x1234;
  extract_type_to_string(&dst, src);
  assert_eq(1, m.strlen.call_count);
  assert_eq(0x1234, m.strlen.args.arg0);
}

test(extract_type_to_string_shall_call_find_type_len_correctly)
{
  char* dst;
  char* src = 0x1234;
  m.strlen.retval = 5678;
  extract_type_to_string(&dst, src);
  assert_eq(0x1234, m.find_type_len.args.arg0);
  assert_eq(5678, m.find_type_len.args.arg1);
}

test(extract_type_to_string_shall_allocate_memory_for_the_str)
{
  char* dst;
  char* src = 0x1234;
  m.find_type_len.retval = 5678;
  extract_type_to_string(&dst, src);
  assert_eq(1, m.malloc.call_count);
  assert_eq(5678 + 1, m.malloc.args.arg0);
}

test(extract_type_to_string_shall_print_error_and_return_0_if_out_of_memory)
{
  char* dst;
  char* src = 0x1234;
  m.find_type_len.retval = 5678;
  m.malloc.retval = NULL;
  assert_eq(0, extract_type_to_string(&dst, src));
#ifdef CUTEST_GCC
  assert_eq(1, m.fwrite.call_count);
#else
  assert_eq(1, m.fprintf.call_count);
#endif
}

test(extract_type_to_string_shall_copy_string_to_the_allocated_memory)
{
  char* dst;
  char* src = 0x1234;
  m.find_type_len.retval = 5678;
  m.malloc.retval = 0x8765;
  extract_type_to_string(&dst, src);
  assert_eq(1, m.strncpy.call_count);
  assert_eq(0x8765, m.strncpy.args.arg0);
  assert_eq(0x1234, m.strncpy.args.arg1);
  assert_eq(5678, m.strncpy.args.arg2);
}

test(extract_type_shall_return_the_length_of_the_type_part_of_the_arg)
{
  char* dst;
  char* src = 0x1234;
  m.find_type_len.retval = 5678;
  m.malloc.retval = 0x8765;
  assert_eq(5678, extract_type_to_string(&dst, src));
}

module_test(extract_type_to_string_shall_extract_the_type_part_of_arg)
{
  char* dst;
  assert_eq(strlen("const char"),
            extract_type_to_string(&dst, "const char argument"));
  assert_eq("const char", dst);
  free(dst);
  assert_eq(strlen("const char"),
            extract_type_to_string(&dst, "const char *argument"));
  assert_eq("const char", dst);
  free(dst);
  assert_eq(strlen("const struct foo"),
            extract_type_to_string(&dst, "const struct foo argument"));
  assert_eq("const struct foo", dst);
  free(dst);
  assert_eq(strlen("const struct foo"),
            extract_type_to_string(&dst, "const struct foo *argument"));
  assert_eq("const struct foo", dst);
  free(dst);
  assert_eq(strlen("char"),
            extract_type_to_string(&dst, "char *argument"));
  assert_eq("char", dst);
  free(dst);
  assert_eq(strlen("char"),
            extract_type_to_string(&dst, "char *argument[]"));
  assert_eq("char", dst);
  free(dst);
  assert_eq(strlen("char"),
            extract_type_to_string(&dst, "char **argument"));
  assert_eq("char", dst);
  free(dst);
  assert_eq(strlen("int"),
            extract_type_to_string(&dst, "int (*func)(int a, int b)"));
  assert_eq("int", dst);
  free(dst);
  assert_eq(strlen("char"),
            extract_type_to_string(&dst, "char"));
  assert_eq("char", dst);
  free(dst);
  assert_eq(strlen("..."),
            extract_type_to_string(&dst, "..."));
  assert_eq("...", dst);
  free(dst);
  assert_eq(strlen("int"),
            extract_type_to_string(&dst, "int array[]"));
  assert_eq("int", dst);
  free(dst);
}

/*****************************************************************************
 * is_variadic_argument()
 */

test(is_variadic_argument_shall_return_1_if_input_looks_like_a_variadic_arg)
{
  m.strncmp.func = strncmp;
  assert_eq(1, is_variadic_argument("..."));
}

test(is_variadic_argument_shall_return_0_if_input_does_not_look_like_var_arg)
{
  m.strncmp.func = strncmp;
  assert_eq(0, is_variadic_argument("normal_arg_name"));
}

/*****************************************************************************
 * is_function_pointer()
 */

test(is_function_pointer_shall_return_1_if_input_looks_like_a_function_ptr)
{
  m.strncmp.func = strncmp;
  assert_eq(1, is_function_pointer("(*foo)"));
}

test(is_function_pointer_shall_return_0_if_input_does_not_look_like_a_ptr)
{
  m.strncmp.func = strncmp;
  assert_eq(0, is_function_pointer("normal_arg_name"));
}

/*****************************************************************************
 * is_pointer()
 */
test(is_pointer_shall_return_0_if_not_a_pointer)
{
  assert_eq(0, is_pointer("foobar"));
}

test(is_pointer_shall_return_1_if_it_is_pointer)
{
  assert_eq(1, is_pointer("*foobar"));
}

test(is_pointer_shall_return_2_if_it_is_pointer_to_pointer)
{
  assert_eq(2, is_pointer("**foobar"));
}

/*****************************************************************************
 * make_mock_arg_name()
 */
test(make_mock_arg_name_shall_allocate_the_correct_number_of_bytes_for_0_9)
{
  m.strlen.func = strlen;
  m.malloc.func = malloc;
  char* retval = make_mock_arg_name(0, 0, "Nonsense", 0, 0);
  assert_eq(1, m.malloc.call_count);
  assert_eq(strlen("arg0") + 1, m.malloc.args.arg0);
  free(retval);
}

test(make_mock_arg_name_shall_allocate_the_correct_number_of_bytes_for_10_99)
{
  m.strlen.func = strlen;
  m.malloc.func = malloc;
  char* retval = make_mock_arg_name(14, 0, "Nonsense", 0, 0);
  assert_eq(1, m.malloc.call_count);
  assert_eq(strlen("arg14") + 1, m.malloc.args.arg0);
  free(retval);
}

test(make_mock_arg_name_shall_allocate_the_correct_number_of_bytes_for_funcp)
{
  m.strlen.func = strlen;
  m.malloc.func = malloc;
  char* retval = make_mock_arg_name(14, 1, "(int, int)", 0, 0);
  assert_eq(1, m.malloc.call_count);
  assert_eq(strlen("(*arg14)(int, int)") + 1, m.malloc.args.arg0);
  free(retval);
}

test(make_mock_arg_name_shall_use_the_correct_formatting_for_func_ptr)
{
  m.strlen.func = strlen;
  m.malloc.retval = 0x1234;
  make_mock_arg_name(0, 1, "(int*, char*)", 0, 0);
  assert_eq(1, m.sprintf.call_count);
  assert_eq(0x1234, m.sprintf.args.arg0);
  assert_eq("(*arg%llu)%s", m.sprintf.args.arg1);
}

test(make_mock_arg_name_shall_use_the_correct_formatting_for_non_func_ptr)
{
  m.strlen.func = strlen;
  m.malloc.retval = 0x1234;
  make_mock_arg_name(0, 0, NULL, 0, 0);
  assert_eq(1, m.sprintf.call_count);
  assert_eq(0x1234, m.sprintf.args.arg0);
  assert_eq("arg%llu", m.sprintf.args.arg1);
}

test(make_mock_arg_name_shall_use_the_correct_formatting_pointers)
{
  m.strlen.func = strlen;
  m.malloc.retval = 0x1234;
  make_mock_arg_name(0, 0, NULL, 1, 0);
  assert_eq(1, m.sprintf.call_count);
  assert_eq(0x1234, m.sprintf.args.arg0);
  assert_eq("*arg%llu", m.sprintf.args.arg1);
}

test(make_mock_arg_name_shall_use_the_correct_formatting_double_pointers)
{
  m.strlen.func = strlen;
  m.malloc.retval = 0x1234;
  make_mock_arg_name(0, 0, NULL, 2, 0);
  assert_eq(1, m.sprintf.call_count);
  assert_eq(0x1234, m.sprintf.args.arg0);
  assert_eq("**arg%llu", m.sprintf.args.arg1);
}

test(make_mock_arg_name_shall_use_the_correct_formatting_for_non_func_ptr_ar)
{
  m.strlen.func = strlen;
  m.malloc.retval = 0x1234;
  make_mock_arg_name(0, 0, NULL, 0, 1);
  assert_eq(1, m.sprintf.call_count);
  assert_eq(0x1234, m.sprintf.args.arg0);
  assert_eq("*arg%llu", m.sprintf.args.arg1);
}

test(make_mock_arg_name_shall_use_the_correct_formatting_pointers_array)
{
  m.strlen.func = strlen;
  m.malloc.retval = 0x1234;
  make_mock_arg_name(0, 0, NULL, 1, 1);
  assert_eq(1, m.sprintf.call_count);
  assert_eq(0x1234, m.sprintf.args.arg0);
  assert_eq("**arg%llu", m.sprintf.args.arg1);
}

test(make_mock_arg_name_shall_output_an_error_message_if_too_many_asterisks)
{
  m.strlen.func = strlen;
  m.malloc.retval = 0x1234;
  make_mock_arg_name(0, 0, NULL, 3, 1);
#ifdef CUTEST_GCC
  assert_eq(1, m.fwrite.call_count);
#else
  assert_eq(1, m.fprintf.call_count);
#endif
}

/*****************************************************************************
 * make_assignment_type_cast
 */

test(make_assignment_type_cast_shall_allocate_enought_memory_for_longer_str)
{
  m.strlen.retval = strlen("char");
  make_assignment_type_cast("char", 1);
  assert_eq(1, m.malloc.call_count);
  /* '(' type ' ' '*' ')' '\0' */
  assert_eq(strlen("(") + strlen("char") + strlen(" ") + strlen("*") + strlen(")") + 1, m.malloc.args.arg0);
}

test(make_assignment_type_cast_shall_return_null_if_out_of_memory)
{
  assert_eq(NULL, make_assignment_type_cast("char", 1));
}

module_test(make_assignment_type_cast_shall_make_right_hand_side_assignment)
{
  char* rhs = make_assignment_type_cast("char", 1);
  assert_eq("(char *)", rhs);
  free(rhs);
}

/*****************************************************************************
 * make_assignment_name()
 */

test(make_assignment_name_shall_allocate_correct_number_of_bytes_for_0_to_9)
{
  make_assignment_name(5);
  assert_eq(1, m.malloc.call_count);
  assert_eq(strlen("argX") + 1, m.malloc.args.arg0);
}

test(make_assignment_name_shall_allocate_correct_number_of_bytes_for_10_to_99)
{
  make_assignment_name(50);
  assert_eq(1, m.malloc.call_count);
  assert_eq(strlen("argXX") + 1, m.malloc.args.arg0);
}

test(make_assignment_name_shall_return_null_if_out_of_memory)
{
  assert_eq(NULL, make_assignment_name(5));
}

test(make_assignment_name_shall_return_the_pointer_to_the_string_on_success)
{
  m.malloc.retval = 0x1234;
  assert_eq(0x1234, make_assignment_name(5));
}

module_test(make_assignment_name_shall_produce_valid_names)
{
  char* name;
  name = make_assignment_name(5);
  assert_eq("arg5", name);
  free(name);
  name = make_assignment_name(50);
  assert_eq("arg50", name);
  free(name);
}

/*****************************************************************************
 * extract_variable_name_to_ptr()
 */

test(extract_variable_name_to_ptr_shall_simply_return_the_offsetted_pointer)
{
  assert_eq(0x1238, extract_variable_name_to_ptr(0x1234, 4));
}

/*****************************************************************************
 * make_args_control_type()
 */
module_test(make_args_control_type_shall_strip_const_followed_by_space)
{
  assert_eq("char* foo()", make_args_control_type("const char* foo()"));
}

/*****************************************************************************
 * is_array_argument()
 */

test(is_array_argument_shall_return_1_if_the_variable_look_like_an_array)
{
  m.strlen.func = strlen;
  assert_eq(1, is_array_argument("foo[]"));
  assert_eq(1, is_array_argument("foo[1024]"));
}

test(is_array_argument_shall_return_0_if_the_variable_look_like_an_array)
{
  m.strlen.func = strlen;
  assert_eq(0, is_array_argument("foo"));
}

/*****************************************************************************
 * get_length_without_asterisks_brackets_or_parenthesis()
 */

module_test(get_length_without_asterisks_brackets_or_parenthesis_shall_return_the_length_without_these_characters)
{
  assert_eq(4, get_length_without_asterisks_brackets_or_parenthesis("**foob["));
  assert_eq(4, get_length_without_asterisks_brackets_or_parenthesis("**foob("));
  assert_eq(6, get_length_without_asterisks_brackets_or_parenthesis("*foobar"));
}

/*****************************************************************************
 * strip_asterisks_and_backts()
 */
test(strip_asterisks_and_brackets_shall_call_get_length_without_asterisks_brackets_or_parenthesis)
{
  strip_asterisks_and_brackets(0x1234);
  assert_eq(1, m.get_length_without_asterisks_brackets_or_parenthesis.call_count);
  assert_eq(0x1234, m.get_length_without_asterisks_brackets_or_parenthesis.args.arg0);
}

test(strip_asterisks_and_brackets_shall_allocate_mem)
{
  m.get_length_without_asterisks_brackets_or_parenthesis.retval = 1234;
  strip_asterisks_and_brackets(NULL);
  assert_eq(1, m.malloc.call_count);
  assert_eq(1235, m.malloc.args.arg0);
}

test(strip_asterisks_and_brackets_shall_return_null_if_out_of_memory)
{
  m.get_length_without_asterisks_brackets_or_parenthesis.retval = 1234;
  assert_eq(NULL, strip_asterisks_and_brackets(NULL));
}

module_test(strip_asterisks_and_brackets_shall_strip_leading_asterisks)
{
  char* buf = strip_asterisks_and_brackets("**here");
  assert_eq("here", buf);
  free(buf);
}

/*****************************************************************************
 * is_const()
 */
module_test(is_const_shall_return_true_if_the_string_contains_const_and_space)
{
  assert_eq((1 == 1), is_const("something const something"));
}

module_test(is_const_shall_return_false_if_the_string_contains_const_and_space)
{
  assert_eq((1 == 0), is_const("something something"));
}

/*****************************************************************************
 * parse_argument_to_info()
 */

test(parse_argument_to_info_shall_call_extract_type_to_string_correctly)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  m.strlen.func = strlen;
  node.arg = 0x1234;
  node.type = 0x5678;
  parse_argument_to_info(&node, 0);
  assert_eq(1, m.extract_type_to_string.call_count);
  assert_eq(&node.type, m.extract_type_to_string.args.arg0);
  assert_eq(0x1234, m.extract_type_to_string.args.arg1);
}

test(parse_argument_shall_pass_the_node_type_to_is_variadic_argument)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  m.strlen.func = strlen;
  node.arg = 0x1234;
  node.type = 0x5678;
  m.extract_type_to_string.retval = 1;
  parse_argument_to_info(&node, 0);
  assert_eq(1, m.is_variadic_argument.call_count);
  assert_eq(0x5678, m.is_variadic_argument.args.arg0);
}

test(parse_argument_shall_remember_variadic_argument)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  m.strlen.func = strlen;
  node.arg = 0x1234;
  node.type = 0x5678;
  m.extract_type_to_string.retval = 1;
  m.is_variadic_argument.retval = 4321;
  parse_argument_to_info(&node, 0);
  assert_eq(4321, node.variadic);
}

test(parse_argument_shall_not_store_a_variable_name_for_variadic_arguments)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  m.strlen.func = strlen;
  node.arg = 0x1234;
  node.type = 0x5678;
  m.extract_type_to_string.retval = 1;
  m.is_variadic_argument.retval = 4321;
  parse_argument_to_info(&node, 0);
  assert_eq(NULL, node.variable);
}

test(parse_argument_to_info_shall_return_minus_1_if_out_of_memeory)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  m.extract_type_to_string.retval = 0;
  assert_eq(-1, parse_argument_to_info(&node, 0));
}

test(parse_argument_to_info_shall_call_is_function_pointer_correctly)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.arg = "int (*func)(int, int)";
  m.strlen.func = strlen;
  m.extract_type_to_string.retval = 4;
  parse_argument_to_info(&node, 0);
  assert_eq(1, m.is_function_pointer.call_count);
  assert_eq("(*func)(int, int)", m.is_function_pointer.args.arg0);
}

test(parse_argument_to_info_shall_rememember_function_pointers)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.arg = "int (*func)(int, int)";
  m.strlen.func = strlen;
  m.extract_type_to_string.retval = 4;
  m.is_function_pointer.retval = 1;
  parse_argument_to_info(&node, 0);
  assert_eq(1, node.function_pointer);
}

test(parse_argument_to_info_shall_rememember_function_pointers_arguments)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.arg = "int (*func)(int, int)";
  m.strlen.func = strlen;
  m.strchr.func = strchr;
  m.extract_type_to_string.retval = 4;
  m.is_function_pointer.retval = 1;
  parse_argument_to_info(&node, 0);
  assert_eq("(int, int)", node.function_pointer_args);
}

test(parse_argument_to_info_shall_remember_the_variable_name) {
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.arg = "int (*func)(int, int)";
  m.strlen.func = strlen;
  m.extract_type_to_string.retval = 4;
  m.is_function_pointer.retval = 1;
  m.extract_variable_name_to_ptr.func = extract_variable_name_to_ptr;
  parse_argument_to_info(&node, 0);
  assert_eq("(*func)(int, int)", node.variable);
}

module_test(parse_argument_to_info_shall_produce_relevant_info_for_simple)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.arg = "const char *buffer";
  parse_argument_to_info(&node, 1);
  assert_eq("const char", node.type);
  assert_eq("buffer", node.variable);
  assert_eq(0, node.variadic);
  assert_eq(0, node.function_pointer);
  assert_eq(NULL, node.function_pointer_args);
  assert_eq("arg1", node.mock_declaration.name);
  assert_eq("char", node.args_control.type);
  assert_eq("arg1", node.args_control.name);
  assert_eq("buffer", node.args_control.comment);
  free(node.type);
  free(node.mock_declaration.name);
}

module_test(parse_argument_to_info_shall_produce_relevant_info_for_array)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));

  node.arg = "const char buffer[1024]";

  parse_argument_to_info(&node, 1);

  assert_eq("const char", node.type);
  assert_eq("buffer[1024]", node.variable);
  assert_eq(0, node.variadic);
  assert_eq(0, node.function_pointer);
  assert_eq(NULL, node.function_pointer_args);
  assert_eq("*arg1", node.mock_declaration.name);
  assert_eq("char", node.args_control.type);
  assert_eq("*arg1", node.args_control.name);
  assert_eq("buffer[1024]", node.args_control.comment);

  free(node.type);
  free(node.mock_declaration.name);
}

module_test(parse_argument_to_info_shall_produce_relevant_info_for_funcptr)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));

  node.arg = "int (*func)(int, int b)";

  parse_argument_to_info(&node, 1);

  assert_eq("int", node.type);
  assert_eq("(*func)(int, int b)", node.variable);
  assert_eq(0, node.variadic);
  assert_eq(1, node.function_pointer);
  assert_eq("(int, int b)", node.function_pointer_args);
  assert_eq("(*arg1)(int, int b)", node.mock_declaration.name);
  assert_eq("int", node.args_control.type);
  assert_eq("(*arg1)(int, int b)", node.args_control.name);
  assert_eq("(*func)(int, int b)", node.args_control.comment);

  free(node.type);
  free(node.mock_declaration.name);
}

module_test(parse_argument_to_info_shall_produce_relevant_info_for_variadic)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));

  node.arg = "...";

  parse_argument_to_info(&node, 1);

  assert_eq("...", node.type);
  assert_eq(NULL, node.variable);
  assert_eq(1, node.variadic);
  assert_eq(0, node.function_pointer);
  assert_eq(NULL, node.function_pointer_args);
  assert_eq(NULL, node.mock_declaration.name);
  assert_eq(NULL, node.args_control.type);
  assert_eq(NULL, node.args_control.name);
  assert_eq(NULL, node.args_control.comment);

  free(node.type);
  free(node.mock_declaration.name);
}

/*****************************************************************************
 * get_function_args()
 */

test(get_function_args_shall_call_split2args_correctly)
{
  arg_list_t list;
  memset(&list, 0, sizeof(list));
  get_function_args(&list, 0x1234);
  assert_eq(1, m.split2args.call_count);
  assert_eq(&list, m.split2args.args.arg0);
  assert_eq(0x1234, m.split2args.args.arg1);
}

test(get_function_shall_call_parse_argument_to_info_for_all_arguments)
{
  arg_list_t list;
  arg_node_t node[3];
  memset(&list, 0, sizeof(list));
  memset(node, 0, sizeof(node));

  node[0].next = &node[1];
  node[1].next = &node[2];
  node[2].next = NULL;
  list.first = &node[0];

  get_function_args(&list, 0x1234);

  assert_eq(3, m.parse_argument_to_info.call_count);
}

test(get_function_args_shall_return_the_position_of_the_end_of_last_arg)
{
  arg_list_t list;
  memset(&list, 0, sizeof(list));
  m.split2args.retval = 20;
  assert_eq(20, get_function_args(&list, 0x1234));
}

/*****************************************************************************
 * get_mockables()
 */

test(get_mockables_shall_open_the_correct_file)
{
  get_mockables(NULL, 0x1234);
  assert_eq(1, m.fopen.call_count);
  assert_eq(0x1234, m.fopen.args.arg0);
  assert_eq("r", m.fopen.args.arg1);
}

test(get_mockables_shall_output_an_error_message_if_file_could_not_be_opened)
{
  get_mockables(NULL, 0x1234);
  assert_eq(1, m.fprintf.call_count);
  assert_eq(stderr, m.fprintf.args.arg0);
}

test(get_mockables_shall_return_0_if_file_could_not_be_openend)
{
  assert_eq(0, get_mockables(NULL, 0x1234));
}

static int fgets_only_5_times_stub_cnt = 0;
static char* fgets_only_5_times_stub(char* s, int size, FILE *stream)
{
  (void)s;
  (void)size;
  (void)stream;
  if (fgets_only_5_times_stub_cnt >= 5) {
    return NULL;
  }
  strcpy(s, "Nonsese");
  fgets_only_5_times_stub_cnt++;
  return 0x5678;
}

test(get_mockables_shall_read_every_line_of_the_nm_file)
{
  fgets_only_5_times_stub_cnt = 0;
  m.fgets.func = fgets_only_5_times_stub;
  m.fopen.retval = 0x5678;

  get_mockables(NULL, 0x1234);
  assert_eq(5, m.new_mockable_node.call_count);

  fgets_only_5_times_stub_cnt = 0;
}

test(get_mockables_shall_add_all_rows_to_the_list_of_mockables)
{
  fgets_only_5_times_stub_cnt = 0;
  m.fgets.func = fgets_only_5_times_stub;
  m.fopen.retval = 0x5678;
  m.new_mockable_node.retval = 0x8765;
  get_mockables(0x4321, 0x1234);

  assert_eq(5, m.mockable_list_add_node.call_count);
  assert_eq(0x4321, m.mockable_list_add_node.args.arg0);
  assert_eq(0x8765, m.mockable_list_add_node.args.arg1);

  fgets_only_5_times_stub_cnt = 0;
}

test(get_mockbables_shall_close_the_correct_file)
{
  m.fopen.retval = 0x5678;

  get_mockables(NULL, 0x1234);
  assert_eq(1, m.fclose.call_count);
  assert_eq(0x5678, m.fclose.args.arg0);
}

/*****************************************************************************
 * get_include_flags()
 */

module_test(get_include_flags_shall_always_use_arg_number_3_as_cutest_path)
{
  char dst[1024];
  char* argv[] = {"program name", "cproto", "other arg", "other arg", "cutest path"};
  get_include_flags(dst, 4, argv);
  assert_eq("-I\"cutest path\"", dst);
}

module_test(get_include_flags_shall_always_append_the_rest_of_the_args)
{
  char dst[1024];
  char* argv[] = {"program name", "cproto", "other arg", "other arg", "cutest path",
                  "-Ifoo", "-Ibar"};
  get_include_flags(dst, 7, argv);
  assert_eq("-I\"cutest path\" -Ifoo -Ibar", dst);
}

/*****************************************************************************
 * node_symbol_match()
 */

module_test(node_symbol_match_shall_return_0_if_input_does_not_match)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  node.symbol_name = "Cant find this";

  const char* symbol = "Looking for this";
  const size_t len = strlen(symbol);

  assert_eq(0, node_symbol_match(&node, symbol, len));
}

module_test(node_symbol_match_shall_return_1_if_input_does_match)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  node.symbol_name = "Looking for this";

  const char* symbol = "Looking for this";
  const size_t len = strlen(symbol);

  assert_eq(1, node_symbol_match(&node, symbol, len));
}

/*****************************************************************************
 * sumbil_is_in_list()
 */
test(symbol_is_in_list_shall_traverse_the_whole_list_if_to_find_no_match)
{
  mockable_list_t list;
  mockable_node_t node[3];
  memset(&list, 0, sizeof(list));
  memset(node, 0, sizeof(node));

  list.first = &node[0];
  node[0].next = &node[1];
  node[1].next = &node[2];
  list.last = &node[2];
  node[2].next = NULL;

  symbol_is_in_list(&list, "not in there");

  assert_eq(3, m.node_symbol_match.call_count);
}

test(symbol_is_in_list_shall_return_matching_node)
{
  mockable_list_t list;
  mockable_node_t node[3];
  memset(&list, 0, sizeof(list));
  memset(node, 0, sizeof(node));

  m.node_symbol_match.retval = 1;

  list.first = &node[0];
  node[0].next = &node[1];
  node[1].next = &node[2];
  list.last = &node[2];
  node[2].next = NULL;

  symbol_is_in_list(&list, "bogus hit");

  assert_eq(1, m.node_symbol_match.call_count);
}

/*****************************************************************************
 * parse_cproto_row()
 */

test(parse_cproto_row_shall_get_the_return_type_correctly)
{
  parse_cproto_row(0x1234, 0x5678);
  assert_eq(1, m.get_return_type.call_count);
  assert_eq(0x5678, m.get_return_type.args.arg1);
}

test(parse_cproto_row_shall_return_null_if_no_return_type_was_found)
{
  assert_eq(NULL, parse_cproto_row(0x1234, 0x5678));
}


test(parse_cproto_row_shall_get_the_function_name_correctly)
{
  m.get_return_type.retval = 2;
  parse_cproto_row(0x1234, 0x5678);
  assert_eq(1, m.get_function_name.call_count);
  assert_eq(0x567a, m.get_function_name.args.arg1);
}

test(parse_cproto_row_shall_call_symbol_is_in_list_correctly)
{
  m.get_return_type.retval = 2;
  parse_cproto_row(0x1234, 0x5678);
  assert_eq(1, m.symbol_is_in_list.call_count);
  assert_eq(0x1234, m.symbol_is_in_list.args.arg0);
  /*
   * This can not be checked 100% safely, since the argument is on the stack
   * belonging to the parse_cproto_row() function. Valgrind will complain.
   *
   * assert_eq(m.get_function_name.args.arg0, m.symbol_is_in_list.args.arg1);
   */
}

test(parse_cproto_row_shall_return_null_if_symbol_should_not_be_mocked)
{
  m.symbol_is_in_list.retval = NULL;
  assert_eq(NULL, parse_cproto_row(0x1234, 0x5678));
}

test(parse_cproto_row_shall_not_call_get_function_args_if_already_handeled)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  node.args = 0x4321;
  node.legit = 1;

  m.symbol_is_in_list.retval = &node;
  m.get_return_type.retval = 2;
  m.get_function_name.retval = 3;

  parse_cproto_row(0x1234, 0x5678);

  assert_eq(0, m.get_function_args.call_count);
}

test(parse_cproto_row_shall_return_null_if_already_handeled)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  node.args = 0x4321;
  node.legit = 1;

  m.symbol_is_in_list.retval = &node;
  m.get_return_type.retval = 2;
  m.get_function_name.retval = 3;

  assert_eq(NULL, parse_cproto_row(0x1234, 0x5678));
}

test(parse_cproto_row_shall_call_get_function_args_correctly)
{
  mockable_node_t node;
  arg_list_t args;
  memset(&node, 0, sizeof(node));

  node.args = &args;

  m.symbol_is_in_list.retval = &node;
  m.get_return_type.retval = 2;
  m.get_function_name.retval = 3;

  parse_cproto_row(0x1234, 0x5678);

  assert_eq(1, m.get_function_args.call_count);
  assert_eq(&args, m.get_function_args.args.arg0);
  assert_eq(0x567d, m.get_function_args.args.arg1);
}

test(parse_cproto_row_shall_return_the_pointer_to_a_mockable_node)
{
  mockable_node_t node;
  arg_list_t args;
  arg_node_t arg;
  memset(&node, 0, sizeof(node));
  memset(&args, 0, sizeof(args));
  memset(&arg, 0, sizeof(arg));

  args.first = &arg;
  node.args = &args;
  m.strcmp.retval = 1;
  node.legit = 0;

  m.get_return_type.retval = 2;
  m.symbol_is_in_list.retval = &node;

  assert_eq(&node, parse_cproto_row(0x1234, 0x5678));
}

/*****************************************************************************
 * construct_cproto_command_line()
 */

module_test(construct_cproto_command_line_shall_produce_a_valid_command)
{
  char command[1024];
  char* argv[] = {"program name", "cproto", "design.c", "mockables.lst",
                  "/path/to/cutest", "-Iinc"};
  construct_cproto_command_line(command, "cproto", 6, argv);
  assert_eq("\"cproto\" -i -s -x -I\"/path/to/cutest\" -Iinc \"design.c\" 2>/dev/null",
            command);
}

/*****************************************************************************
 * execute_cproto()
 */

test(execute_cproto_shall_forward_args_to_command_line_constructor)
{
  execute_cproto(NULL, 0x4321, 5678, 0x1234);
  assert_eq(1, m.construct_cproto_command_line.call_count);
  assert_eq(0x4321, m.construct_cproto_command_line.args.arg1);
  assert_eq(5678, m.construct_cproto_command_line.args.arg2);
  assert_eq(0x1234, m.construct_cproto_command_line.args.arg3);
}

test(execute_cproto_shall_execute_the_constructed_command)
{
  execute_cproto(NULL, 0x4321, 5678, 0x1234);
  assert_eq(1, m.popen.call_count);
  /*
   * This can not be checked 100% safely, since the argument is on the stack
   * belonging to the execute_cproto() function. Valgrind will complain.
   *
   * assert_eq(m.construct_cproto_command_line.args.arg0, m.popen.args.arg0);
   */
}

test(execute_cproto_shall_output_an_error_if_the_command_could_not_execute)
{
  execute_cproto(NULL, 0x4321, 5678, 0x1234);
  assert_eq(1, m.fprintf.call_count);
  assert_eq(stderr, m.fprintf.args.arg0);
}

test(execute_cproto_shall_return_0_if_the_command_could_not_execute)
{
  assert_eq(0, execute_cproto(NULL, 0x4321, 5678, 0x1234));
}

test(execute_cproto_shall_call_fgets_to_read_the_stdout_from_cproto)
{
  m.strncmp.func = strncmp;
  m.popen.retval = 0x4321;
  execute_cproto(NULL, 0x4321, 5678, 0x1234);
  assert_eq(1, m.fgets.call_count);
  assert_eq(0x4321, m.fgets.args.arg2);
}

test(execute_cproto_shall_call_fgets_until_the_end_of_the_file_was_reached)
{
  m.strncmp.func = strncmp;
  fgets_only_5_times_stub_cnt = 0;
  m.fgets.func = fgets_only_5_times_stub;
  m.popen.retval = 0x4321;
  execute_cproto(NULL, 0x4321, 5678, 0x1234);
  assert_eq(6, m.fgets.call_count);
  fgets_only_5_times_stub_cnt = 0;
}

static int fgets_no_comment_5_times_stub_cnt = 0;
static char* fgets_no_comment_5_times_stub(char* s, int size, FILE *stream)
{
  (void)s;
  (void)size;
  (void)stream;
  strcpy(s, "static long get_file_size(FILE *fd);");
  if (fgets_no_comment_5_times_stub_cnt >= 5) {
    return NULL;
  }
  fgets_no_comment_5_times_stub_cnt++;
  return s;
}

test(execute_cproto_shall_call_parse_cproto_row_for_each_non_comment_row)
{
  m.strncmp.func = strncmp;
  fgets_no_comment_5_times_stub_cnt = 0;
  m.fgets.func = fgets_no_comment_5_times_stub;
  m.popen.retval = 0x4321;
  execute_cproto(NULL, 0x4321, 5678, 0x1234);
  assert_eq(5, m.parse_cproto_row.call_count);
  fgets_no_comment_5_times_stub_cnt = 0;
}

static int fgets_read_comment_stub_cnt = 0;
static char* fgets_read_comment_stub(char* s, int size, FILE *stream)
{
  (void)s;
  (void)size;
  (void)stream;
  strcpy(s, "/* A comment */");
  if (fgets_read_comment_stub_cnt >= 1) {
    return NULL;
  }
  fgets_read_comment_stub_cnt++;
  return s;
}

test(execute_cproto_shall_not_call_parse_cproto_row_if_comment_row)
{
  m.strncmp.func = strncmp;
  fgets_read_comment_stub_cnt = 0;
  m.fgets.func = fgets_read_comment_stub;
  m.popen.retval = 0x4321;
  execute_cproto(NULL, 0x4321, 5678, 0x1234);
  assert_eq(0, m.parse_cproto_row.call_count);
  fgets_read_comment_stub_cnt = 0;
}

/*****************************************************************************
 * print_function_args()
 */

test(print_function_args_shall_construct_argument_line)
{
  arg_list_t list;
  arg_node_t node[3];
  memset(&list, 0, sizeof(list));
  memset(node, 0, sizeof(node));

  node[0].arg = "int argc";
  node[1].arg = "char* argv[]";
  node[2].arg = "...";
  list.first = &node[0];
  node[0].next = &node[1];
  node[1].next = &node[2];
  list.last = &node[2];
  node[2].next = NULL;
  print_function_args(&list);
  assert_eq(3, m.printf.call_count);
}

/*****************************************************************************
 * print_mock_control_struct_with_return_type()
 */

test(print_mock_control_struct_with_return_type_shall_print_something)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  print_mock_control_struct_with_return_type(&node);
#ifdef CUTEST_GCC
  assert_eq(1, m.printf.call_count);
#else
  assert_eq(2, m.printf.call_count);
#endif
}

/*****************************************************************************
 * print_mock_control_struct_with_void_type()
 */

test(print_mock_control_struct_with_void_type_shall_print_something)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  print_mock_control_struct_with_void_type(&node);
#ifdef CUTEST_GCC
  assert_eq(1, m.printf.call_count);
#else
  assert_eq(2, m.printf.call_count);
#endif
}

/*****************************************************************************
 * print_mock_control_struct_arg()
 */

test(print_mock_control_struct_arg_shall_print_an_argument_sampler_storage)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));

  print_mock_control_struct_arg(&node);
  assert_eq(1, m.printf.call_count);
}

/*****************************************************************************
 * print_mock_control_struct_args()
 */

test(print_mock_control_struct_arg_shall_traverse_and_print_all_arguments)
{
  arg_list_t list;
  arg_node_t node[3];
  memset(&list, 0, sizeof(list));
  memset(node, 0, sizeof(node));

  list.first = &node[0];
  node[0].next = &node[1];
  node[1].next = &node[2];
  list.last = &node[2];
  node[2].next = NULL;
  print_mock_control_struct_args(&list);
  assert_eq(3, m.print_mock_control_struct_arg.call_count);
}

test(print_mock_control_struct_arg_shall_not_print_all_variadic_arguments)
{
  arg_list_t list;
  arg_node_t node[3];
  memset(&list, 0, sizeof(list));
  memset(node, 0, sizeof(node));

  node[1].variadic = 1;
  list.first = &node[0];
  node[0].next = &node[1];
  node[1].next = &node[2];
  list.last = &node[2];
  node[2].next = NULL;

  print_mock_control_struct_args(&list);

  assert_eq(2, m.print_mock_control_struct_arg.call_count);
}

/*****************************************************************************
 * print_mock_control_struct()
 */

test(print_mock_control_struct_shall_call_return_type_printing_if_not_void)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  m.strstr.func = strstr;

  node.return_type.is_void = 0;
  print_mock_control_struct(&node);
  assert_eq(1, m.print_mock_control_struct_with_return_type.call_count);
  assert_eq(&node, m.print_mock_control_struct_with_return_type.args.arg0);
}

test(print_mock_control_struct_shall_call_void_type_printing_void)
{
  m.strstr.func = strstr;
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  node.return_type.is_void = 1;
  print_mock_control_struct(&node);
  assert_eq(1, m.print_mock_control_struct_with_void_type.call_count);
  assert_eq(&node, m.print_mock_control_struct_with_void_type.args.arg0);
}

test(print_mock_control_struct_shall_print_struct_args)
{
  m.strstr.func = strstr;
  mockable_node_t node;
  arg_node_t arg_node;
  arg_list_t arg_list;
  memset(&node, 0, sizeof(node));
  memset(&arg_node, 0, sizeof(arg_node));
  memset(&arg_list, 0, sizeof(arg_list));

  arg_list.first = &arg_node;
  node.args = &arg_list;

  node.return_type.name = "void";

  print_mock_control_struct(&node);

  assert_eq(1, m.print_mock_control_struct_args.call_count);
  assert_eq(&arg_list, m.print_mock_control_struct_args.args.arg0);
}

test(print_mock_control_struct_shall_not_print_struct_args_if_no_args)
{
  m.strstr.func = strstr;
  mockable_node_t node;
  arg_list_t arg_list;
  memset(&node, 0, sizeof(node));
  memset(&arg_list, 0, sizeof(arg_list));

  arg_list.first = NULL;
  node.args = &arg_list;
  node.return_type.name = "void";

  print_mock_control_struct(&node);

  assert_eq(0, m.print_mock_control_struct_args.call_count);
}

test(print_mock_control_struct_shall_not_print_struct_args_if_only_variadic)
{
  m.strstr.func = strstr;
  mockable_node_t node;
  arg_node_t arg_node;
  arg_list_t arg_list;
  memset(&node, 0, sizeof(node));
  memset(&arg_node, 0, sizeof(arg_node));
  memset(&arg_list, 0, sizeof(arg_list));

  arg_list.first = &arg_node;
  arg_node.variadic = 1;
  node.args = &arg_list;
  node.return_type.name = "void";

  print_mock_control_struct(&node);

  assert_eq(0, m.print_mock_control_struct_args.call_count);
}

/*****************************************************************************
 * print_mock_control_structs()
 */

test(print_mock_control_structs_shall_print_and_traverse_the_mockables_list)
{
  mockable_list_t list;
  mockable_node_t node[3];
  memset(&list, 0, sizeof(list));
  memset(node, 0, sizeof(node));

  list.first = &node[0];
  node[0].next = &node[1];
  node[1].next = &node[2];
  list.last = &node[2];
  node[2].next = NULL;

  node[0].legit = node[1].legit = node[2].legit = 1;

  print_mock_control_structs(&list);
  assert_eq(3, m.print_mock_control_struct.call_count);
}

test(print_mock_control_structs_shall_print_struct_header_and_footer)
{
  mockable_list_t list;
  mockable_node_t node[3];
  memset(&list, 0, sizeof(list));
  memset(node, 0, sizeof(node));

  list.first = &node[0];
  node[0].next = &node[1];
  node[1].next = &node[2];
  list.last = &node[2];
  node[2].next = NULL;
  print_mock_control_structs(&list);
#ifdef CUTEST_GCC
  assert_eq(2, m.puts.call_count); // printf optimized to puts
#else
  assert_eq(2, m.printf.call_count);
#endif
}

/*****************************************************************************
 * copy_pre_processor_directives_from_dut()
 */

test(copy_pre_processor_directives_from_dut_shall_open_the_file_correctly)
{
  copy_pre_processor_directives_from_dut(0x1234);
  assert_eq(1, m.fopen.call_count);
  assert_eq(0x1234, m.fopen.args.arg0);
  assert_eq("r", m.fopen.args.arg1);
}

test(copy_pre_processor_directives_from_dut_shall_output_error_on_fail_open)
{
  copy_pre_processor_directives_from_dut(0x1234);
  assert_eq(1, m.fprintf.call_count);
}

test(copy_pre_processor_directives_from_dut_shall_read_all_rows)
{
  fgets_only_5_times_stub_cnt = 0;
  m.fgets.func = fgets_only_5_times_stub;
  m.fopen.retval = 0x5678;

  copy_pre_processor_directives_from_dut(0x1234);

  assert_eq(6, m.fgets.call_count);
  assert_eq(0x5678, m.fgets.args.arg2);

  fgets_only_5_times_stub_cnt = 0;
}

test(copy_pre_processor_directives_from_dut_shall_not_copy_noncpp_lines)
{
  fgets_only_5_times_stub_cnt = 0;
  m.fgets.func = fgets_only_5_times_stub;
  m.fopen.retval = 0x5678;

  copy_pre_processor_directives_from_dut(0x1234);

  assert_eq(0, m.fprintf.call_count);

  fgets_only_5_times_stub_cnt = 0;
}

static int fgets_read_preprocessor_stub_cnt = 0;
static char* fgets_read_preprocessor_stub(char* s, int size, FILE *stream)
{
  (void)s;
  (void)size;
  (void)stream;
  if (fgets_read_preprocessor_stub_cnt >= 1) {
    return NULL;
  }
  strcpy(s, "#inclue <foo.h>");
  fgets_read_preprocessor_stub_cnt++;
  return s;
}

test(copy_pre_processor_directives_from_dut_shall_copy_cpp_lines)
{
  fgets_read_preprocessor_stub_cnt = 0;
  m.fgets.func = fgets_read_preprocessor_stub;
  m.fopen.retval = 0x5678;

  copy_pre_processor_directives_from_dut(0x1234);

#ifdef CUTEST_GCC
  assert_eq(1, m.printf.call_count);
  assert_eq(2, m.puts.call_count);
#else
  assert_eq(1, m.printf.call_count);
  assert_eq(2, m.puts.call_count);
#endif

  fgets_read_preprocessor_stub_cnt = 0;
}

/*****************************************************************************
 * print_declaration()
 */

test(print_declarations_shall_print_start_and_end_of_declaration)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  print_declaration(NULL, NULL, &node);
#ifdef CUTEST_GCC
  assert_eq(1, m.printf.call_count);
#else
  assert_eq(2, m.printf.call_count);
#endif

  node.return_type.is_struct = 1;

  m.printf.call_count = 0;
  print_declaration(NULL, NULL, &node);
#ifdef CUTEST_GCC
  assert_eq(1, m.printf.call_count);
#else
  assert_eq(2, m.printf.call_count);
#endif
}

test(print_declarations_shall_print_print_funciton_args)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  node.return_type.is_struct = 0;
  print_declaration(NULL, NULL, &node);
  assert_eq(1, m.print_function_args.call_count);
  assert_eq(node.args, m.print_function_args.args.arg0);
}

test(print_declarations_shall_print_print_funciton_args_if_func_ret_struct)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  node.return_type.is_struct = 1;
  print_declaration(NULL, NULL, &node);
  assert_eq(1, m.print_function_args.call_count);
  assert_eq(node.args, m.print_function_args.args.arg0);
}

/*****************************************************************************
 * print_declarations()
 */

test(print_declarations_shall_print_and_traverse_all_declarations)
{
  mockable_list_t list;
  mockable_node_t node[3];
  memset(&list, 0, sizeof(list));
  memset(node, 0, sizeof(node));

  list.first = &node[0];
  node[0].next = &node[1];
  node[1].next = &node[2];
  list.last = &node[2];
  node[2].next = NULL;

  node[0].legit = node[1].legit = node[2].legit = 1;

  print_declarations(0x1234, 0x5678, &list);

  assert_eq(3, m.print_declaration.call_count);
}

/*****************************************************************************
 * print_dut_declarations()
 */

test(print_dut_declarations_shall_print_extern_declarations_of_mockables)
{
  print_dut_declarations(0x1234);
  assert_eq(1, m.print_declarations.call_count);
  assert_eq("extern ", m.print_declarations.args.arg0);
  assert_eq("", m.print_declarations.args.arg1);
  assert_eq(0x1234, m.print_declarations.args.arg2);
}

/*****************************************************************************
 * print_mock_declarations()
 */

test(print_mock_declarations_shall_print_prefixed_declarations_of_mockables)
{
  print_mock_declarations(0x1234);
  assert_eq(1, m.print_declarations.call_count);
  assert_eq("", m.print_declarations.args.arg0);
  assert_eq("cutest_", m.print_declarations.args.arg1);
  assert_eq(0x1234, m.print_declarations.args.arg2);
}

/*****************************************************************************
 * strip_array_part()
 */
test(strip_array_part_shall_do_nothing_if_src_is_null)
{
  strip_array_part(0x1234, NULL);
  assert_eq(0, m.strlen.call_count);
}

module_test(strip_array_part_shall_copy_all_chars_upuntil_the_left_bracket)
{
  char dst[10];
  strip_array_part(dst, "abc[123]");
  assert_eq("abc", dst);
}

/*****************************************************************************
 * print_arg_assignment()
 */

test(print_arg_assignment_shall_not_produce_assignment_statement_if_variadic)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.variadic = 1;

  print_arg_assignment(&node, "foobar");

#ifdef CUTEST_GCC
  assert_eq(1, m.puts.call_count);
#else
  assert_eq(1, m.printf.call_count);
#endif
}

test(print_arg_assignment_shall_not_produce_assignment_statement_if_void_arg)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));
  node.assignment.variable = "";

  print_arg_assignment(&node, "foobar");

#ifdef CUTEST_GCC
  assert_eq(1, m.puts.call_count);
#else
  assert_eq(1, m.printf.call_count);
#endif
}

test(print_arg_assignment_shall_produce_the_correct_assignment)
{
  arg_node_t node;
  memset(&node, 0, sizeof(node));

  node.assignment.variable = "foobar";
  node.args_control.name = "foobar";
  node.mock_declaration.name = "arg0";
  node.args_control.comment = "foobar";
  print_arg_assignment(&node, "foobar");
}

/*****************************************************************************
 * print_arg_assignments()
 */

test(print_arg_assignments_shall_print_all_argument_assignments)
{
  arg_list_t list;
  arg_node_t node[3];
  memset(&list, 0, sizeof(list));
  memset(node, 0, sizeof(node));

  node[0].next = &node[1];
  node[1].next = &node[2];
  node[2].next = NULL;
  list.first = &node[0];

  print_arg_assignments(&list, 0x1234);

  assert_eq(3, m.print_arg_assignment.call_count);
}

/*****************************************************************************
 * print_stub_caller_args()
 */

test(print_stub_caller_args_shall_printf_all_arguments) {
  arg_list_t list;
  arg_node_t node[3];
  memset(&list, 0, sizeof(list));
  memset(node, 0, sizeof(node));

  node[0].variadic = 0;
  node[0].next = &node[1];
  node[1].variadic = 0;
  node[1].next = &node[2];
  node[2].variadic = 0;
  node[2].next = NULL;
  list.first = &node[0];

  print_stub_caller_args(&list);

  assert_eq(3, m.printf.call_count);
}

/*****************************************************************************
 * last_unvariadic_arg()
 */
test(last_unvariadic_arg_shall_return_the_last_variable_name_before_variadic)
{
  arg_list_t list;
  arg_node_t node[3];
  memset(&list, 0, sizeof(list));
  memset(node, 0, sizeof(node));

  m.fprintf.func = fprintf;

  node[0].variadic = 0;
  node[0].variable = "first";
  node[0].next = &node[1];
  node[1].variadic = 0;
  node[1].variable = "second";
  node[1].next = &node[2];
  node[2].variadic = 1;
  node[2].variable = "...";
  node[2].next = NULL;
  list.first = &node[0];

  assert_eq("second", last_unvariadic_arg(&list));
}

test(last_unvariadic_arg_shall_return_null_if_no_variadic_argument_is_found)
{
  arg_list_t list;
  arg_node_t node[3];
  memset(&list, 0, sizeof(list));
  memset(node, 0, sizeof(node));

  node[0].variadic = 0;
  node[0].variable = "first";
  node[0].next = &node[1];
  node[1].variadic = 0;
  node[1].variable = "second";
  node[1].next = &node[2];
  node[2].variadic = 0;
  node[2].variable = "third";
  node[2].next = NULL;
  list.first = &node[0];

  assert_eq(NULL, last_unvariadic_arg(&list));
}

/*****************************************************************************
 * print_printf_caller()
 */

test(print_printf_caller_shall_print_if_statement_and_caller)
{
  print_printf_caller(0x1234);
#ifdef CUTEST_GCC
  assert_eq(2, m.puts.call_count);
#else
  assert_eq(3, m.printf.call_count);
#endif
}

test(print_printf_caller_shall_return_1)
{
  assert_eq(1, print_printf_caller(0x1234));
}

/*****************************************************************************
 * print_sprintf_caller()
 */

test(print_sprintf_caller_shall_print_if_statement_and_caller)
{
  print_sprintf_caller(0x1234);
#ifdef CUTEST_GCC
  assert_eq(2, m.puts.call_count);
#else
  assert_eq(3, m.printf.call_count);
#endif
}

test(print_sprintf_caller_shall_return_1)
{
  assert_eq(1, print_sprintf_caller(0x1234));
}

/*****************************************************************************
 * print_snprintf_caller()
 */

test(print_snprintf_caller_shall_print_if_statement_and_caller)
{
  print_snprintf_caller(0x1234);
#ifdef CUTEST_GCC
  assert_eq(2, m.puts.call_count);
#else
  assert_eq(3, m.printf.call_count);
#endif
}

test(print_snprintf_caller_shall_return_1)
{
  assert_eq(1, print_snprintf_caller(0x1234));
}

/*****************************************************************************
 * print_fprintf_caller()
 */

test(print_fprintf_caller_shall_print_if_statement_and_caller)
{
  print_fprintf_caller(0x1234);
#ifdef CUTEST_GCC
  assert_eq(2, m.puts.call_count);
#else
  assert_eq(3, m.printf.call_count);
#endif
}

test(print_fprintf_caller_shall_return_1)
{
  assert_eq(1, print_fprintf_caller(0x1234));
}

/*****************************************************************************
 * print_call_without_return_value()
 */

test(print_call_without_return_value_shall_print_something)
{
  print_call_without_return_value(0x1234);
  assert_eq(1, m.printf.call_count);
}

/*****************************************************************************
 * print_call_with_return_value()
 */

test(print_call_with_return_value_shall_print_something)
{
  print_call_with_return_value(0x1234);
  assert_eq(1, m.printf.call_count);
}

/*****************************************************************************
 * print_stub_caller()
 */

test(print_stub_caller_shall_call_print_printf_caller_if_is_printf)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  node.args = 0x1234;
  m.strcmp.func = strcmp;
  node.symbol_name = "printf";
  print_stub_caller(&node);
  assert_eq(1, m.print_printf_caller.call_count);
  assert_eq(0x1234, m.print_printf_caller.args.arg0);
}

test(print_stub_caller_shall_forward_print_printf_caller_return_value)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  m.strcmp.func = strcmp;
  m.print_printf_caller.retval = 1234;
  node.symbol_name = "printf";
  assert_eq(1234, print_stub_caller(&node));
}

test(print_stub_caller_shall_call_print_sprintf_caller_if_is_sprintf)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  m.strcmp.func = strcmp;
  node.symbol_name = "sprintf";
  print_stub_caller(&node);
  assert_eq(1, m.print_sprintf_caller.call_count);
}

test(print_stub_caller_shall_forward_print_sprintf_caller_return_value)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  m.strcmp.func = strcmp;
  m.print_sprintf_caller.retval = 1234;
  node.symbol_name = "sprintf";
  assert_eq(1234, print_stub_caller(&node));
}

test(print_stub_caller_shall_call_print_snprintf_caller_if_is_snprintf)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  m.strcmp.func = strcmp;
  node.symbol_name = "snprintf";
  print_stub_caller(&node);
  assert_eq(1, m.print_snprintf_caller.call_count);
}

test(print_stub_caller_shall_forward_print_snprintf_caller_return_value)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  m.strcmp.func = strcmp;
  m.print_snprintf_caller.retval = 1234;
  node.symbol_name = "snprintf";
  assert_eq(1234, print_stub_caller(&node));
}

test(print_stub_caller_shall_call_print_fprintf_caller_if_is_fprintf)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  m.strcmp.func = strcmp;
  node.symbol_name = "fprintf";
  print_stub_caller(&node);
  assert_eq(1, m.print_fprintf_caller.call_count);
}

test(print_stub_caller_shall_forward_print_fprintf_caller_return_value)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  m.strcmp.func = strcmp;
  m.print_fprintf_caller.retval = 1234;
  node.symbol_name = "fprintf";
  assert_eq(1234, print_stub_caller(&node));
}

test(print_stub_caller_shall_output_warning_print_out_if_arg_is_variadic)
{
  mockable_node_t node;
  arg_list_t list;
  arg_node_t arg;
  memset(&node, 0, sizeof(node));
  memset(&list, 0, sizeof(list));
  memset(&arg, 0, sizeof(arg));

  m.strcmp.retval = 1;
  arg.variadic = 1;
  list.last = list.first = &arg;
  node.args = &list;
  print_stub_caller(&node);
#ifdef CUTEST_GCC
  assert_eq(2, m.printf.call_count);
#else
  assert_eq(3, m.printf.call_count);
#endif
}

test(print_stub_caller_shall_return_1_if_arg_is_variadic)
{
  mockable_node_t node;
  arg_list_t list;
  arg_node_t arg;
  memset(&node, 0, sizeof(node));
  memset(&list, 0, sizeof(list));
  memset(&arg, 0, sizeof(arg));

  m.strcmp.retval = 1;
  arg.variadic = 1;
  list.last = list.first = &arg;
  node.args = &list;
  assert_eq(1, print_stub_caller(&node));
}

test(print_stub_caller_shall_print_caller_without_return_value)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  node.return_type.name = "void";
  node.symbol_name = "my_func";

  m.strcmp.func = strcmp;

  print_stub_caller(&node);
  assert_eq(1, m.print_call_without_return_value.call_count);
  assert_eq("my_func", m.print_call_without_return_value.args.arg0);
}

test(print_stub_caller_shall_print_caller_with_return_value)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));

  node.return_type.name = "int";
  node.symbol_name = "my_func";

  m.strcmp.func = strcmp;

  print_stub_caller(&node);
  assert_eq(1, m.print_call_with_return_value.call_count);
  assert_eq("my_func", m.print_call_with_return_value.args.arg0);
}

test(print_stub_caller_shall_append_sub_caller_args)
{
  mockable_node_t node;
  arg_list_t args;
  memset(&node, 0, sizeof(node));
  memset(&args, 0, sizeof(args));

  node.return_type.name = "int";
  node.symbol_name = "my_func";
  node.args = &args;

  m.strcmp.func = strcmp;

  print_stub_caller(&node);

  assert_eq(1, m.print_stub_caller_args.call_count);
  assert_eq(&args, m.print_stub_caller_args.args.arg0);
}

/*****************************************************************************
 * print_mock_implementation()
 */

test(print_mock_implementation_shall_print_declaration_for_function_head)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));
  print_mock_implementation(&node);
  assert_eq(1, m.print_declaration.call_count);
  assert_eq("", m.print_declaration.args.arg0);
  assert_eq("cutest_", m.print_declaration.args.arg1);
  assert_eq(&node, m.print_declaration.args.arg2);
}

test(print_mock_implementation_shall_print_arg_assignments)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));
  node.args = 0x1234;
  node.symbol_name = 0x5678;
  print_mock_implementation(&node);
  assert_eq(1, m.print_arg_assignments.call_count);
  assert_eq(0x1234, m.print_arg_assignments.args.arg0);
  assert_eq(0x5678, m.print_arg_assignments.args.arg1);
}

test(print_mock_implementation_shall_call_print_stub_caller)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));
  node.args = 0x1234;
  node.symbol_name = 0x5678;
  print_mock_implementation(&node);
  assert_eq(1, m.print_stub_caller.call_count);
  assert_eq(&node, m.print_stub_caller.args.arg0);
}

test(print_mock_implementation_shall_not_print_retval_if_stub_caller_ret_0)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));
  m.print_stub_caller.retval = 0;
  node.args = 0x1234;
  node.symbol_name = 0x5678;
  print_mock_implementation(&node);
#ifdef CUTEST_GCC
  assert_eq(1, m.printf.call_count);
#else
  assert_eq(2, m.printf.call_count);
#endif
}

test(print_mock_implementation_shall_print_retval_if_stub_caller_ret_not_0)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));
  m.print_stub_caller.retval = 1;
  node.args = 0x1234;
  node.symbol_name = 0x5678;
  print_mock_implementation(&node);
#ifdef CUTEST_GCC
  assert_eq(2, m.printf.call_count);
#else
  assert_eq(3, m.printf.call_count);
#endif
}

/*****************************************************************************
 * print_mock_implementation()
 */
test(print_mock_implementations_shall_print_and_traverse_the_list)
{
  mockable_list_t list;
  mockable_node_t node[3];
  memset(&list, 0, sizeof(list));
  memset(node, 0, sizeof(node));
  list.first = &node[0];
  node[0].next = &node[1];
  node[1].next = &node[2];
  list.last = &node[2];
  node[2].next = NULL;

  node[0].legit = node[1].legit = node[2].legit = 1;

  print_mock_implementations(&list);
  assert_eq(3, m.print_mock_implementation.call_count);
}

/*****************************************************************************
 * print_mock_func_module_test_assignment()
 */

test(print_mock_func_module_test_assignment_shall_print_something)
{
  mockable_node_t node;
  memset(&node, 0, sizeof(node));
  print_mock_func_module_test_assignment(&node);
  assert_eq(1, m.printf.call_count);
}

/*****************************************************************************
 * print_mock_func_module_test_assignment()
 */

test(print_mock_func_module_test_assignments_shall_print_and_traverse_list)
{
  mockable_list_t list;
  mockable_node_t node[3];
  memset(&list, 0, sizeof(list));
  memset(node, 0, sizeof(node));

  list.first = &node[0];
  node[0].next = &node[1];
  node[1].next = &node[2];
  list.last = &node[2];
  node[2].next = NULL;

  node[0].legit = node[1].legit = node[2].legit = 1;

  print_mock_func_module_test_assignments(&list);
  assert_eq(3, m.print_mock_func_module_test_assignment.call_count);
}

/*****************************************************************************
 * main()
 */

test(main_shall_output_an_error_message_if_arguments_are_less_than_4) {
  char* argv[] = {"program_name"};
  main(1, argv);
#ifdef CUTEST_GCC
  assert_eq(1, m.fwrite.call_count);
#else
  assert_eq(1, m.fprintf.call_count);
#endif
}

test(main_shall_call_usage_if_arguments_are_less_than_4) {
  char* argv[] = {"program_name"};
  main(1, argv);
  assert_eq(1, m.usage.call_count);
  assert_eq("program_name", m.usage.args.arg0);
}

test(main_shall_return_EXIT_FAILURE_if_arguments_are_less_than_4) {
  char* argv[] = {"program_name"};
  assert_eq(EXIT_FAILURE, main(1, argv));
}

test(main_shall_call_new_mockable_list_once) {
  char* argv[] = {"program_name", "cproto", "dut_src", "nm_filename", "cutest_path"};
  main(5, argv);
  assert_eq(1, m.new_mockable_list.call_count);
}

test(main_shall_return_EXIT_FAILURE_if_argument_list_could_not_be_created)
{
  char* argv[] = {"program_name", "cproto", "dut_src", "nm_filename", "cutest_path"};
  assert_eq(EXIT_FAILURE, main(5, argv));
}

test(main_shall_get_mockables_from_the_nm_file)
{
  char* argv[] = {"program_name", "cproto", "dut_src", "nm_filename", "cutest_path"};
  m.new_mockable_list.retval = 0x1234;
  main(5, argv);
  assert_eq(1, m.get_mockables.call_count);
  assert_eq(0x1234, m.get_mockables.args.arg0);
  assert_eq("nm_filename", m.get_mockables.args.arg1);
}

test(main_shall_execute_cproto_correctly)
{
  char* argv[] = {"program_name", "cproto", "dut_src", "nm_filename", "cutest_path"};
  m.new_mockable_list.retval = 0x1234;
  main(5, argv);
  assert_eq(1, m.execute_cproto.call_count);
  assert_eq(0x1234, m.execute_cproto.args.arg0);
  assert_eq("cproto", m.execute_cproto.args.arg1);
  assert_eq(5, m.execute_cproto.args.arg2);
  assert_eq(argv, m.execute_cproto.args.arg3);
}

test(main_shall_delete_mockables_list_if_cproto_fails)
{
  char* argv[] = {"program_name", "cproto", "dut_src", "nm_filename", "cutest_path"};
  m.new_mockable_list.retval = 0x1234;
  m.execute_cproto.retval = 0;
  main(5, argv);
  assert_eq(1, m.delete_mockable_list.call_count);
  assert_eq(0x1234, m.delete_mockable_list.args.arg0);
}

test(main_shall_return_EXIT_FAILURE_if_cproto_fails)
{
  char* argv[] = {"program_name", "cproto", "dut_src", "nm_filename", "cutest_path"};
  m.new_mockable_list.retval = 0x1234;
  m.execute_cproto.retval = 0;
  assert_eq(EXIT_FAILURE, main(5, argv));
}

test(main_shall_print_file_header)
{
  char* argv[] = {"program_name", "cproto", "dut_src", "nm_filename", "cutest_path"};
  m.new_mockable_list.retval = 0x1234;
  m.execute_cproto.retval = 1;
  main(5, argv);
  assert_eq(1, m.printf.call_count);
}

test(main_shall_copy_pre_processor_directives)
{
  char* argv[] = {"program_name", "cproto", "dut_src", "nm_filename", "cutest_path"};
  m.new_mockable_list.retval = 0x1234;
  m.execute_cproto.retval = 1;
  main(5, argv);
  assert_eq(1, m.copy_pre_processor_directives_from_dut.call_count);
  assert_eq("dut_src", m.copy_pre_processor_directives_from_dut.args.arg0);
}

test(main_shall_print_dut_declarations)
{
  char* argv[] = {"program_name", "cproto", "dut_src", "nm_filename", "cutest_path"};
  m.new_mockable_list.retval = 0x1234;
  m.execute_cproto.retval = 1;
  main(5, argv);
  assert_eq(1, m.print_dut_declarations.call_count);
  assert_eq(0x1234, m.print_dut_declarations.args.arg0);
}

test(main_shall_print_mock_declarations)
{
  char* argv[] = {"program_name", "cproto", "dut_src", "nm_filename", "cutest_path"};
  m.new_mockable_list.retval = 0x1234;
  m.execute_cproto.retval = 1;
  main(5, argv);
  assert_eq(1, m.print_mock_declarations.call_count);
  assert_eq(0x1234, m.print_mock_declarations.args.arg0);
}

test(main_shall_print_mock_control_structs)
{
  char* argv[] = {"program_name", "cproto", "dut_src", "nm_filename", "cutest_path"};
  m.new_mockable_list.retval = 0x1234;
  m.execute_cproto.retval = 1;
  main(5, argv);
  assert_eq(1, m.print_mock_control_structs.call_count);
  assert_eq(0x1234, m.print_mock_control_structs.args.arg0);
}

test(main_shall_print_mock_implementations)
{
  char* argv[] = {"program_name", "cproto", "dut_src", "nm_filename", "cutest_path"};
  m.new_mockable_list.retval = 0x1234;
  m.execute_cproto.retval = 1;
  main(5, argv);
  assert_eq(1, m.print_mock_implementations.call_count);
  assert_eq(0x1234, m.print_mock_implementations.args.arg0);
}

test(main_shall_print_mock_func_module_test_assignments)
{
  char* argv[] = {"program_name", "cproto", "dut_src", "nm_filename", "cutest_path"};
  m.new_mockable_list.retval = 0x1234;
  m.execute_cproto.retval = 1;
  main(5, argv);
  assert_eq(1, m.print_mock_func_module_test_assignments.call_count);
}

test(main_shall_delete_mockable_list)
{
  char* argv[] = {"program_name", "cproto", "dut_src", "nm_filename", "cutest_path"};
  m.new_mockable_list.retval = 0x1234;
  m.execute_cproto.retval = 1;
  main(5, argv);
  assert_eq(1, m.delete_mockable_list.call_count);
  assert_eq(0x1234, m.delete_mockable_list.args.arg0);
}
