/*
 * This is the test-suite for the cutest_run program. Please take a look at it
 * for educational purposes on how to use this testing framework.
 */
#include <stdlib.h>

#define CUTEST_DEP ../helpers.o

#include "cutest.h"

/* For convenience 'm' is shorter to write than 'cutest_mock' */
#define m cutest_mock
/* The main() function is by default renamed but can be renamed again */
/*#define main MAIN */

/*****************************************************************************
 * usage()
 */
test(usage_shall_print_something)
{
  usage("some_program_name");
  assert_eq(1, m.printf.call_count);
}

/*****************************************************************************
 * add_new_mockable_node()
 */
test(add_new_mockable_node_shall_return_NULL_if_out_of_memory)
{
  mockable_node node;
  assert_eq(NULL, add_new_mockable_node(&node, "foo"));
}

test(add_new_mockable_node_shall_allocate_a_new_node_for_each_file_item)
{
  mockable_node node;
  add_new_mockable_node(&node, "foo");

  assert_eq(1, m.malloc.call_count);
  assert_eq(sizeof(mockable_node), m.malloc.args.arg0);
}

test(add_new_mockable_node_shall_allocate_bytes_for_mockable_name)
{
  mockable_node node;
  mockable_node new_node;

  m.malloc.retval = &new_node;
  m.strlen.func = strlen;

  add_new_mockable_node(&node, "testing");

  assert_eq(2, m.malloc.call_count); /* Also for the node it self */
  assert_eq(strlen("testing") + 1, m.malloc.args.arg0);
  assert_eq(&new_node, new_node.name);
}

int malloc_twice_stub_cnt = 0;
static void* malloc_twice_stub(size_t size)
{
  (void)size;
  if (malloc_twice_stub_cnt == 1) {
    return NULL;
  }
  malloc_twice_stub_cnt++;
  return m.malloc.retval;
}

test(add_new_mockable_node_shall_return_NULL_if_out_of_mem_for_mockable_name)
{
  mockable_node node;
  mockable_node new_node;
  malloc_twice_stub_cnt = 0;

  m.malloc.func = malloc_twice_stub;
  m.malloc.retval = &new_node;
  m.strlen.func = strlen;

  assert_eq(NULL, add_new_mockable_node(&node, "testing"));

  malloc_twice_stub_cnt = 0;
}

test(add_new_mockable_node_shall_copy_the_mockable_name_to_the_node)
{
  mockable_node node;
  mockable_node new_node;

  m.malloc.retval = &new_node;

  add_new_mockable_node(&node, "testing");

  assert_eq(1, m.strcpy.call_count); /* Also for the node it self */
  assert_eq(new_node.name, m.strcpy.args.arg0);
  assert_eq("testing", m.strcpy.args.arg1);
}

test(add_new_mockable_node_shall_set_next_of_current_node_to_the_new_node)
{
  mockable_node node;
  mockable_node new_node;

  m.malloc.retval = &new_node;

  add_new_mockable_node(&node, "testing");

  assert_eq(&new_node, node.next)
}

test(add_new_mockable_node_shall_return_the_new_node)
{
  mockable_node node;
  mockable_node new_node;

  m.malloc.retval = &new_node;

  assert_eq(&new_node, add_new_mockable_node(&node, "testing"));
}


/*****************************************************************************
 * read_mockables_list_file()
 */
test(raed_mockables_list_file_shall_open_the_correct_file_for_reading)
{
  read_mockables_list_file(NULL, "file_name");
  assert_eq(1, m.fopen.call_count);
  assert_eq("file_name", m.fopen.args.arg0);
  assert_eq("r", m.fopen.args.arg1);
}

test(read_mockables_list_file_shall_return_0_if_file_could_not_be_opened)
{
  assert_eq(0, read_mockables_list_file(NULL, "file_name"));
}

test(read_mockables_shall_close_the_correct_file_if_opened)
{
  m.fopen.retval = 0x1234;
  read_mockables_list_file(NULL, "file_name");
  assert_eq(1, m.fclose.call_count);
  assert_eq(0x1234, m.fclose.args.arg0);
}

test(read_mockables_list_file_shall_call_fgets_correctly)
{
  m.fopen.retval = 0x1234;
  read_mockables_list_file(NULL, "file_name");
  assert_eq(1, m.fgets.call_count);
  assert_eq(0x1234, m.fgets.args.arg2);
}

static int fgets_only_once_stub_cnt = 0;
static char* fgets_only_once_stub(char* s, int size, FILE *stream)
{
  (void)s;
  (void)size;
  (void)stream;
  if (fgets_only_once_stub_cnt > 0) {
    return NULL;
  }
  fgets_only_once_stub_cnt++;
  return 0x5678;
}

test(read_mockables_list_file_shall_return_0_if_out_of_memory)
{
  mockable_node node;
  fgets_only_once_stub_cnt = 0;

  m.strcmp.retval = 1;
  m.fopen.retval = 0x1234;
  m.fgets.func = fgets_only_once_stub;
  m.fgets.retval = 0x5678;

  assert_eq(0, read_mockables_list_file(&node, "file_name"));

  fgets_only_once_stub_cnt = 0;
}

test(read_mockables_list_file_shall_return_count_if_all_is_ok)
{
  mockable_node node;
  mockable_node new_node;
  fgets_only_once_stub_cnt = 0;

  m.fopen.retval = 0x1234;
  m.fgets.func = fgets_only_once_stub;
  m.fgets.retval = 0x5678;
  m.strcmp.retval = 1;
  m.add_new_mockable_node.retval = &new_node;

  assert_eq(1, read_mockables_list_file(&node, "file_name"));

  fgets_only_once_stub_cnt = 0;
}

/*****************************************************************************
 * rstrip()
 */
test(rstrip_shall_make_string_one_byte_shorter)
{
  m.strlen.func = strlen;
  char* string = malloc(strlen("a string\n") + 1);
  strcpy(string, "a string\n");
  rstrip(string);
  assert_eq("a string", string);
  free(string);
}

/*****************************************************************************
 * assembler_row_contain_mockable_name()
 */
test(mockable_name_pos_shall_return_0_if_not_found_at_all)
{
  m.strstr.func = strstr;
  assert_eq(0, mockable_name_pos("  foo", "bar"));
}

test(mockable_name_pos_shall_return_0_if_too_early_on_row)
{
  m.strstr.func = strstr;
  assert_eq(0, mockable_name_pos("foo:", "foo"));
}

test(mockable_name_pos_shall_return_position_if_found)
{
  m.strstr.func = strstr;
  char *str = "  mnemonic foo";
  assert_eq(strlen("  mnemonic "), mockable_name_pos(str, "foo"));
}

/*****************************************************************************
 * is_space()
 */
test(is_space_shall_return_1_if_character_is_a_valid_whitespace)
{
  assert_eq(1, is_space('\0'));
  assert_eq(1, is_space(' '));
  assert_eq(1, is_space('\n'));
  assert_eq(1, is_space('\r'));
  assert_eq(1, is_space('\t'));
}

test(is_space_shall_return_0_if_character_is_not_a_valid_whitespace)
{
  assert_eq(0, is_space('a'));
  assert_eq(0, is_space('!'));
  assert_eq(0, is_space('b'));
  assert_eq(0, is_space('1'));
}

/*****************************************************************************
 * mockable_is_surrounded_by_whitespaces()
 */
test(mockable_is_surrounded_by_whitespaces_shall_return_1_if_left_and_right)
{
  m.is_space.func = is_space;
  assert_eq(1, mockable_is_surrounded_by_whitespaces(" a ", 1, 1));
  assert_eq(1, mockable_is_surrounded_by_whitespaces(" a ", 1, 1));
}

test(mockable_is_surrounded_by_whitespaces_shall_return_0_if_not)
{
  m.is_space.func = is_space;
  assert_eq(0, mockable_is_surrounded_by_whitespaces(" ab", 1, 1));
  assert_eq(0, mockable_is_surrounded_by_whitespaces("ba ", 1, 1));
}

module_test(mockable_is_surrounded_by_whitespaces_shall_return_true_if_so)
{
  m.printf.func = printf;
  const int pos = strlen("  mnemonic ");
  const int end = pos + strlen("mockable") - 1;
  assert_eq(1, mockable_is_surrounded_by_whitespaces("  mnemonic mockable ",
                                                     pos, end));
  assert_eq(1, mockable_is_surrounded_by_whitespaces("  mnemonic mockable",
                                                     pos, end));
  assert_eq(1, mockable_is_surrounded_by_whitespaces("  mnemonic\tmockable ",
                                                     pos, end));
  assert_eq(1, mockable_is_surrounded_by_whitespaces("  mnemonic\tmockable\n",
                                                     pos, end));
}

/*****************************************************************************
 * replace_jump_destination()
 */
test(replace_jump_destination_shall_call_mockable_is_surrounded_correctly)
{
  m.strlen.retval = 3; /* End-position of the mockable on a row */
  replace_jump_destination(0x1234, 0x5678, 9);
  assert_eq(1, m.mockable_is_surrounded_by_whitespaces.call_count);
  assert_eq(0x1234, m.mockable_is_surrounded_by_whitespaces.args.arg0);
  assert_eq(9, m.mockable_is_surrounded_by_whitespaces.args.arg1);
  assert_eq((9 + m.strlen.retval - 1), m.mockable_is_surrounded_by_whitespaces.args.arg2);
}

test(replace_jump_destination_shall_return_0_if_not_a_valid_branch)
{
  assert_eq(0, replace_jump_destination(0x1234, 0x5678, 9));
}

test(replace_jump_destination_shall_return_1_if_valid_branch)
{
  char buf[10];
  m.mockable_is_surrounded_by_whitespaces.retval = 1;
  assert_eq(1, replace_jump_destination(buf, "foo", 6));
}

module_test(replace_jump_destination_shall_transform_the_branch_label)
{
  char buf[100];
  strcpy(buf, "  jmp foo");
  replace_jump_destination(buf, "foo", 6);
  assert_eq("  jmp cutest_foo", buf);
}

/*****************************************************************************
 * traverse_all_nodes()
 */

#define traverse_all_fixture()                  \
  mockable_node node[3];                        \
  node[0].next = &node[1];                      \
  node[1].next = &node[2];                      \
  node[2].next = NULL

test(traverse_all_nodes_shall_traverse_through_all_nodes)
{
  traverse_all_fixture();
  traverse_all_nodes(0x1234, &node[0]);
  assert_eq(3, m.mockable_name_pos.call_count);
}

test(traverse_all_nodes_shall_stop_traversion_after_found_pos_and_replace)
{
  traverse_all_fixture();
  m.mockable_name_pos.retval = 1;
  m.replace_jump_destination.retval = 1;
  traverse_all_nodes(0x1234, &node[0]);
  assert_eq(1, m.mockable_name_pos.call_count);
}

#undef traverse_all_fixture

#define traverse_all_fixture()                  \
  mockable_node node;                           \
  node.name = 0x5678;                           \
  node.next = NULL

test(traverse_all_nodes_shall_call_mockable_name_pos_correctly)
{
  traverse_all_fixture();
  traverse_all_nodes(0x1234, &node);
  assert_eq(1, m.mockable_name_pos.call_count);
  assert_eq(0x1234, m.mockable_name_pos.args.arg0);
  assert_eq(0x5678, m.mockable_name_pos.args.arg1);
}

test(traverse_all_nodes_shall_call_replace_jump_destination_correctly_if_pos)
{
  traverse_all_fixture();
  m.mockable_name_pos.retval = 9;
  traverse_all_nodes(0x1234, &node);
  assert_eq(1, m.replace_jump_destination.call_count);
  assert_eq(0x1234, m.replace_jump_destination.args.arg0);
  assert_eq(0x5678, m.replace_jump_destination.args.arg1);
  assert_eq(9, m.replace_jump_destination.args.arg2);
}

test(traverse_all_nodes_shall_not_call_replace_jump_destination_if_no_pos)
{
  traverse_all_fixture();
  m.mockable_name_pos.retval = 0;
  traverse_all_nodes(0x1234, &node);
  assert_eq(0, m.replace_jump_destination.call_count);
}

#undef traverse_all_fixture

/*****************************************************************************
 * row_starts_with_valid_char()
 */
module_test(row_starts_with_valid_char_shall_return_0_if_start_w_punktiation)
{
  assert_eq(0, row_starts_with_valid_char("    ."));
}

module_test(row_starts_with_valid_char_shall_return_0_if_row_is_empty)
{
  assert_eq(0, row_starts_with_valid_char("  "));
}

module_test(row_starts_with_valid_char_shall_return_1_if_row_start_is_good)
{
  assert_eq(1, row_starts_with_valid_char("    call"));
}

/*****************************************************************************
 * replace_assembler_jumps()
 */
test(replace_assembler_jumps_shall_open_the_correct_file_for_reading)
{
  replace_assembler_jumps(NULL, "foobar");
  assert_eq("foobar", m.fopen.args.arg0);
  assert_eq("r", m.fopen.args.arg1);
}

test(replace_assembler_jumps_shall_use_fgets_to_read_rows_from_the_file)
{
  m.fopen.retval = 0x1234;
  replace_assembler_jumps(NULL, "foobar");
  assert_eq(0x1234, m.fgets.args.arg2);
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
  fgets_only_5_times_stub_cnt++;
  return 0x5678;
}

test(replace_assembler_jumps_shall_use_fgets_to_read_print_all_lines)
{
  fgets_only_5_times_stub_cnt = 0;
  m.fopen.retval = 0x1234;
  m.fgets.func = fgets_only_5_times_stub;

  replace_assembler_jumps(NULL, "foobar");

  assert_eq(0x1234, m.fgets.args.arg2);
  assert_eq(5, m.puts.call_count);

  fgets_only_5_times_stub_cnt = 0;
}

static char fgets_only_once_with_buf_buf[10];
static int fgets_only_once_with_buf_stub_cnt = 0;
static char* fgets_only_once_with_buf_stub(char* s, int size, FILE *stream)
{
  (void)size;
  (void)stream;
  if (fgets_only_once_with_buf_stub_cnt > 0) {
    return NULL;
  }
  fgets_only_once_with_buf_stub_cnt++;
  strcpy(s, fgets_only_once_with_buf_buf);
  return s;
}

test(replace_assembler_jumps_shall_truncate_all_rows_since_puts_is_used)
{
  fgets_only_once_with_buf_stub_cnt = 0;
  m.fopen.retval = 0x1234;
  m.fgets.func = fgets_only_once_with_buf_stub;
  strcpy(fgets_only_once_with_buf_buf, "buffer");

  replace_assembler_jumps(NULL, "foobar");

  assert_eq(1, m.rstrip.call_count);

  fgets_only_once_with_buf_stub_cnt = 0;
}

test(replace_assembler_jumps_shall_call_traverse_all_nodes_if_valid_char)
{
  fgets_only_once_with_buf_stub_cnt = 0;
  m.fopen.retval = 0x1234;
  m.fgets.func = fgets_only_once_with_buf_stub;
  strcpy(fgets_only_once_with_buf_buf, "buffer");
  m.row_starts_with_valid_char.retval = 1;

  replace_assembler_jumps(0x1234, "foobar");

  assert_eq(1, m.traverse_all_nodes.call_count);
  assert_eq(0x1234, m.traverse_all_nodes.args.arg1);

  fgets_only_once_with_buf_stub_cnt = 0;
}

/*****************************************************************************
 * free_mockables_list()
 */
test(free_mockables_list_shall_free_name_and_node_for_every_node) {
  mockable_node node[3];
  node[0].next = &node[1];
  node[1].next = &node[2];
  node[2].next = NULL;
  free_mockables_list(&node[0]);
  assert_eq(6, m.free.call_count);
}

/*****************************************************************************
 * main()
 */
test(main_shall_do_a_sanity_check_of_argument_count_print_an_error_wrong_cnt)
{
  char* argv[] = {"program_name"};
  main(1, argv);
#ifdef __GNUC__
  assert_eq(1, m.fwrite.call_count);
  assert_eq(stderr, m.fwrite.args.arg3);
#else
  assert_eq(1, m.fprintf.call_count);
  assert_eq(stderr, m.fprintf.args.arg0);
#endif
}

test(main_shall_print_usage_if_argument_count_is_not_3)
{
  char* argv[] = {"program_name"};
  main(1, argv);
  assert_eq(1, m.usage.call_count);
}

test(main_shall_return_EXIT_FAILURE_if_argument_count_is_not_3)
{
  char* argv[] = {"program_name"};
  assert_eq(EXIT_FAILURE, main(1, argv));
}


test(main_shall_check_if_dut_asm_file_name_exists)
{
  char* argv[] = {"program_name", "dut_asm_file", "mockables_file"};
  main(3, argv);
  assert_eq(1, m.file_exists.call_count);
  assert_eq("dut_asm_file", m.file_exists.args.arg0);
}

test(main_shall_check_if_mockables_list_file_name_exists)
{
  char* argv[] = {"program_name", "test_file", "mockables_file"};
  m.file_exists.retval = 1;
  m.malloc.func = malloc;
  main(3, argv);
  assert_eq(2, m.file_exists.call_count);
  assert_eq("mockables_file", m.file_exists.args.arg0);
}

static int file_exists_false_test_source_file(const char* filename)
{
  if (0 == strcmp("dut_asm_file", filename)) {
    return 0;
  }
  if (0 == strcmp("mockables_file", filename)) {
    return 1;
  }
  return 0;
}

static int file_exists_false_mock_header_file(const char* filename)
{
  if (0 == strcmp("dut_asm_file", filename)) {
    return 1;
  }
  if (0 == strcmp("mockables_file", filename)) {
    return 0;
  }
  return 0;
}

test(main_shall_return_EXIT_FAILURE_if_dut_asm_file_could_not_be_found)
{
  char* argv[] = {"program_name", "dut_asm_file", "mockables_file"};
  m.file_exists.func = file_exists_false_test_source_file;
  assert_eq(EXIT_FAILURE, main(3, argv));
}

test(main_shall_return_EXIT_FAILURE_if_mockables_file_could_not_be_found)
{
  char* argv[] = {"program_name", "dut_asm_file", "mockables_file"};
  m.file_exists.func = file_exists_false_mock_header_file;
  assert_eq(EXIT_FAILURE, main(3, argv));
}

test(main_shall_call_read_mockables_list_correctly)
{
  char* argv[] = {"program_name", "dut_asm_file", "mockables_file"};
  m.malloc.func = malloc;
  m.file_exists.retval = 1;
  main(3, argv);
  assert_eq(1, m.read_mockables_list_file.call_count);
  assert_eq("mockables_file", m.read_mockables_list_file.args.arg1);
}

test(main_shall_call_replace_assembler_jumps_correctly)
{
  char* argv[] = {"program_name", "dut_asm_file", "mockables_file"};
  m.malloc.func = malloc;
  m.file_exists.retval = 1;
  main(3, argv);
  assert_eq(1, m.read_mockables_list_file.call_count);
  assert_eq("dut_asm_file", m.replace_assembler_jumps.args.arg1);
}

test(main_shall_return_EXIT_SUCCESS_if_all_is_ok)
{
  char* argv[] = {"program_name", "test_file", "mockables_file"};
  m.malloc.func = malloc;
  m.file_exists.retval = 1;
  assert_eq(EXIT_SUCCESS, main(3, argv));
}
