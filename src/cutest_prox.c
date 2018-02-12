/*********************************************************************
   ----    ____ ____ _____ ____ ____ _____   ____ ____ ____ __   -----
   ----   / __// / //_  _// __// __//_  _/  / _ // _ //   /| /.' -----
   ----  / /_ / / /  / / / __//_  /  / /   / __//   \/ / /_' \   -----
   ---- /___//___/  /_/ /___//___/  /_/   /_/  /_/_//___//_/_/   -----
 *
 * CUTest proxification tool
 * =========================
 *
 * The ``cutest_prox`` tool reads an elaborated assembler source file
 * and a file containing a list of mockable functions to produce a new
 * assembler output with all calls to local (or other) functions
 * replaced by CUTest mocks.
 *
 * How to build the tool
 * ---------------------
 *
 * Just include the ``cutest.mk`` makefile in your own ``Makefile`` in
 * your folder containing the source code for the ``*_test.c`` files.
 *
 * The tool is automatically compiled when making the check target.
 * But if you want to make the tool explicitly just call::
 *
 *  $ make cutest_prox
 *
 * Usage
 * -----
 *
 * If you *need* to run the tool manually this is how::
 *
 *  $ ./cutest_prox dut_mockables.s dut_mockables.lst
 *
 * And an assembler file will be outputted to stdout.
 *
 * */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "cutest_prox.h"

#include "helpers.h"

#define CHUNK_SIZE 1024

static void usage(const char* program_name)
{
  printf("USAGE: %s <dut-asm-source-file> <mockables-list-file>\n",
         program_name);
}

static mockable_node* add_new_mockable_node(mockable_node* node,
                                            const char* mockable_name)
{
  mockable_node *n = NULL;
  size_t len = 0;

  n = malloc(sizeof(mockable_node));
  if (NULL == n) {
    return NULL;
  }
  memset(n, 0, sizeof(*n));
  len = strlen(mockable_name);
  n->name = malloc(len + 1);
  if (NULL == n->name) {
    return NULL;
  }
  memset(n->name, 0, len + 1);
  strcpy(n->name, mockable_name);

  node->next = n;
  return n;
}

static void rstrip(char* buf)
{
  buf[strlen(buf) - 1] = 0;
}

static size_t read_mockables_list_file(mockable_node* node,
                                       const char* mockables_list_file_name)
{
  size_t cnt = 0;
  char buf[CHUNK_SIZE];
  FILE* fd = fopen(mockables_list_file_name, "r");

  if (NULL == fd) {
    return 0;
  }

  while (fgets(buf, sizeof(buf), fd)) {
    rstrip(buf);
    if (0 == strcmp(buf, "stderr")) {
      continue;
    }
    if (NULL == (node = add_new_mockable_node(node, buf))) {
      cnt = 0;
      break;
    }
    cnt++;
  }

  fclose(fd);
  return cnt;
}

static int mockable_name_pos(const char* buf, const char* mockable_name)
{
  char* s = strstr(buf, mockable_name);
  int pos = 0;

  if (NULL == s) {
    return 0;
  }
  pos = s - buf;
  if (pos < 3) { /* probably not a mnemonic in-front of the label */
    return 0;
  }
  return pos;
}

static int is_space(char c)
{
  switch (c) {
  case '\0':
  case ' ':
  case '\n':
  case '\r':
  case '\t':
  case ',':
  case '$':
  case '#':
  case '@':
    return 1;
  default:
    return 0;
  }
}

static int is_space_underscore(char c1, char c2)
{
  return (is_space(c1) && ('_' == c2));
}

static int mockable_is_surrounded_by_whitespaces_but_old_gcc(const char* buf,
                                                             int pos,
                                                             int end)
{
  const int left = is_space_underscore(buf[pos - 2], buf[pos - 1]);
  const int right = is_space(buf[end + 1]);

  return (left && right);
}

static int mockable_is_surrounded_by_whitespaces(const char* buf,
                                                 int pos,
                                                 int end)
{
  const int left = is_space(buf[pos - 1]);
  const int right = is_space(buf[end + 1]);

  return (left && right);
}

static int replace_jump_destination(char* buf, char* mockable_name, int pos)
{
  const int mocklen = strlen(mockable_name);
  const int end = pos + mocklen - 1;
  char newbuf[CHUNK_SIZE];
  /*
  char fnurp[1024];
  */
  const int new_school = mockable_is_surrounded_by_whitespaces(buf, pos, end);
  const int old_school = mockable_is_surrounded_by_whitespaces_but_old_gcc(buf, pos, end);

  if (!new_school && !old_school) {
    return 0;
  }
  buf[pos] = 0;
  if (new_school) {
    sprintf(newbuf, "%scutest_%s%s", buf, mockable_name, &buf[end + 1]);
  }
  else {
    sprintf(newbuf, "%scutest_%s%s", buf, mockable_name, &buf[end + 1]);
  }
  strcpy(buf, newbuf);

  /*
  fnurp[0] = 0;
  if (new_school) {
    sprintf(fnurp, " ; DEBUG: NEW '%s' %s\n", mockable_name, buf);
  }
  if (old_school) {
    sprintf(fnurp, " ; DEBUG: OLD '%s' %s\n", mockable_name, buf);
  }
  strcat(buf, fnurp);
  */
  return 1;
}

static void traverse_all_nodes(char* buf, mockable_node* node)
{
  while (node) {
    const int pos = mockable_name_pos(buf, node->name);

    if ((0 != pos) && replace_jump_destination(buf, node->name, pos)) {
      break;
    }
    node = node->next;
  }
}

static int row_starts_with_valid_char(const char* buf)
{
  const size_t len = strlen(buf);
  const size_t i = count_white_spaces(buf);

  if ((i == len) || (buf[i] == '.')) {
    return 0;
  }
  return 1;
}

static void replace_assembler_jumps(mockable_node* node,
                                    const char* asm_file_name)
{
  FILE* fd = fopen(asm_file_name, "r");
  char buf[CHUNK_SIZE];

  while (NULL != fgets(buf, sizeof(buf), fd)) {
    rstrip(buf);
    if (row_starts_with_valid_char(buf)) {
      traverse_all_nodes(buf, node);
    }
    puts(buf);
  }

  fclose(fd);
}

static void free_mockables_list(mockable_node* node)
{
  while (node) {
    mockable_node* next = node->next;
    free(node->name);
    free(node);
    node = next;
  }
}

int main(int argc, char* argv[]) {
  const char* program_name = argv[0];
  const char* dut_asm_source_file_name = argv[1];
  const char* mockables_list_file_name = argv[2];
  mockable_node* node = NULL;

  if (argc < 3) {
    fprintf(stderr, "ERROR: Missing argument\n");
    usage(program_name);
    return EXIT_FAILURE;
  }

  if (!file_exists(dut_asm_source_file_name) ||
      !file_exists(mockables_list_file_name)) {
    return EXIT_FAILURE;
  }

  node = malloc(sizeof(mockable_node));
  memset(node, 0, sizeof(*node));

  read_mockables_list_file(node, mockables_list_file_name);

  replace_assembler_jumps(node->next, dut_asm_source_file_name);

  free_mockables_list(node);

  return 0;
}
