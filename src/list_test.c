#include "list.h"
#include "cutest.h"

#define m cutest_mock

/*****************************************************************************
 * delete_just_fort_test_node()
 */
test(delete_just_for_test_node_dummy_function_should_be_covered)
{
  delete_just_for_test_node(NULL);
}

/*****************************************************************************
 * new_just_for_test_list()
 */

test(new_just_for_test_list_shall_allocate_a_new_just_for_test_list)
{
  new_just_for_test_list();
  assert_eq(1, m.malloc.call_count);
  assert_eq(sizeof(just_for_test_list_t), m.malloc.args.arg0);
}

test(new_just_for_test_list_shall_return_null_if_out_of_memory)
{
  assert_eq(NULL, new_just_for_test_list());
}

test(new_just_for_test_list_shall_output_an_error_message_if_out_of_memory)
{
  new_just_for_test_list();
#ifdef CUTEST_GCC
  assert_eq(1, m.fwrite.call_count);
  assert_eq(stderr, m.fwrite.args.arg3);
#else
  assert_eq(1, m.fprintf.call_count);
  assert_eq(stderr, m.fprintf.args.arg0);
#endif
}

test(new_just_for_test_list_shall_set_the_members_to_null)
{
  just_for_test_list_t list;
  m.malloc.retval = &list;
  new_just_for_test_list();
  assert_eq(NULL, list.first);
  assert_eq(NULL, list.last);
}

test(new_just_for_test_list_shall_return_the_address_to_the_list)
{
  just_for_test_list_t list;
  m.malloc.retval = &list;
  assert_eq(&list, new_just_for_test_list());
}

/*****************************************************************************
 * just_for_test_list_add_node()
 */

test(just_for_test_list_add_node_shall_be_able_to_add_the_first_node_to_the_list)
{
  just_for_test_list_t list = {NULL, NULL};
  just_for_test_list_add_node(&list, 0x1234);
  assert_eq(list.first, 0x1234);
  assert_eq(list.last, 0x1234);
}

test(just_for_test_list_add_node_shall_add_a_node_to_the_end_of_the_list)
{
  just_for_test_node_t first;
  just_for_test_list_t list = {&first, &first};
  first.next = NULL;
  just_for_test_list_add_node(&list, 0x1234);
  assert_eq(&first, list.first);
  assert_eq(0x1234, list.last);
  assert_eq(0x1234, first.next);
}

/*****************************************************************************
 * delete_just_for_test_list()
 */

test(delete_just_for_test_list_shall_delete_all_nodes)
{
  just_for_test_node_t first;
  just_for_test_node_t middle;
  just_for_test_node_t last;
  first.next = &middle;
  middle.next = &last;
  last.next = NULL;
  just_for_test_list_t list = {&first, &last};
  delete_just_for_test_list(&list);
  assert_eq(3, m.delete_just_for_test_node.call_count);
}

test(delete_just_for_test_list_shall_free_the_list_allocation)
{
  just_for_test_list_t list = {NULL, NULL};
  delete_just_for_test_list(&list);
  assert_eq(1, m.free.call_count);
}
