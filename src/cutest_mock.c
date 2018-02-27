/*********************************************************************
   ---    ____ ____ _____ ____ ____ _____   ____   ____ ____ ____ ---
   ---   / __// / //_  _// __// __//_  _/  /    \ /   // __// / / ---
   ---  / /_ / / /  / / / __//_  /  / /   / / / // / // /_ /  <'  ---
   --- /___//___/  /_/ /___//___/  /_/   /_/_/_//___//___//_/_/   ---
   ---                                                            ---
 *
 * CUTest mock generator
 * =====================
 *
 * This is a tool that can be used to generate mock-up functions. It
 * inspects a specified source-code file (written i C language) and
 * looks for uses of the functions listed in a file which list all
 * function that is replaceable with a mock when developing code using
 * test-driven design.
 *
 * The author is fully aware that the methodology used to parse the C
 * prototypes from the ``cproto`` output is quite brute. There is a
 * lot of assumptions on how various prototypes can be written, but
 * in the end. The ``cutest_mock`` tool has proven itself over time
 * to be quite clever. But, remember - There are no guarantees that
 * it will work on any random code so far.
 *
 * Requirements
 * ------------
 *
 * To be able to generate well formatted function declarations to
 * mutate into mock-ups this tool make use of the ``cproto`` tool.
 *
 * How to compile the tool
 * -----------------------
 *
 * Just include the ``cutest.mk`` makefile in your own ``Makefile``
 * in your folder containing the source code for the ``*_test.c``
 * files.
 *
 * The tool is automatically compiled when making the check target
 * But if you want to make the tool explicitly just call::
 *
 *  $ make cutest_mock
 *
 * Usage
 * -----
 *
 * If you *need* to run the tool manually this is how::
 *
 *  $ ./cutest_mock design_under_test.c mockables.lst /path/to/cutest
 *
 * And it will scan the source-code for mockable functions and
 * output a header file-style text, containing everything needed to
 * test your code alongside with the ``cutest.h`` file.
 *
 * The ``mockables.lst`` is produced by ``nm dut.o | sed 's/.* //g'``.
 *
 * However, if you use the ``Makefile`` targets specified in the
 * beginning of this document you will probably not need to run it
 * manually.
 *
 */

#define _XOPEN_SOURCE
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "cutest_mock.h"
#include "arg.h"
#include "mockable.h"
#include "helpers.h"

static void usage(const char* program_name)
{
  printf("USAGE: %s <path-to-cproto> <dut-source-file.c> <mockables.lst> <path-to-cutest>"
         " [-I flags]\n",
         program_name);
}

static size_t find_name_pos(const char* buf) {
  /*
   * Search for a potential function name in a function prototype row.
   *
   * For example, it assumes that a function starts with some kind of data
   * type (return value) and the last space in the data type name is used
   * as a starting point to scan for a left parenthesis. This is assumed
   * to be the function name to scan for.
   */
  int i;
  const int len = strlen(buf);
  size_t start = 0;
  for (i = 0; i < len; i++) {
    if (buf[i] == ' ') {
      start = i + 1;
    }
    if (buf[i] == '[') {
      break;
    }
    if (buf[i] == '(') {
      return start;
    }
  }
  return -1;
}

static size_t get_prefix_attributes(return_type_t* return_type,
                                    const char* buf, const size_t namepos)
{
  /*
   * Get as many keywords related to the return data type as possible and
   * remember them in the return_type struct for future use.
   *
   * This function brute-forcefully string-match a few known keywords that
   * are often used along with the return type of a function. For example;
   * "static " or "struct " and such things.
   *
   * The function tries to do so until the position where function name is
   * assumed to be and will consider that the end of the search.
   */
  size_t pos = 0;
  int offs = 0;
  char* prefixes[4] = {"static ",
                       "struct ",
                       "inline ",
                       "__extension__ "};
  int* attribs[4];

  attribs[0] = &return_type->is_static;
  attribs[1] = &return_type->is_struct;
  attribs[2] = &return_type->is_inline;
  attribs[3] = NULL;

  while (pos + offs <= namepos - 1) {
    size_t i = 0;
    size_t len = 0;
    for (i = 0; i < sizeof(prefixes)/sizeof(char*); i++) {
      if (0 == strncmp(&buf[pos], prefixes[i], strlen(prefixes[i]))) {
        len = strlen(prefixes[i]);
        pos += len;
        if (NULL != attribs[i]) {
          *attribs[i] = 1;
        }
        pos += count_white_spaces(&buf[pos]);
        offs = 0;
        break;
      }
    }
    offs++;
  }
  pos += count_white_spaces(&buf[pos]);
  return pos;
}

static size_t copy_return_type_name(char* dst, const char* src,
                                    size_t namepos)
{
  /*
   * Copies the return data type name from the src-array to the dst-array
   * and include asterisks in the type name, since it makes it easier to
   * reuse the data type name later in the generation of C code, namely
   * the mocks-file.
   *
   * However sometimes the analysis of the cproto output result in false
   * positives. For example stderr and stdout are believed to be function
   * prototypes sometimes. And since no return type can be found on these
   * wrongfully identified "function declarations" the return type name
   * scanning will return in an empty string "", and this function clears
   * up the mistake by return 0 instead of the offset position to the
   * character after the data type name found.
   */
  size_t dst_pos = 0;
  size_t pos = 0;

  memcpy(dst, src, namepos);

  dst_pos = pos = namepos;

  dst[dst_pos] = 0;
  pos += count_white_spaces(&src[pos]);

  while ('*' == src[pos]) {
    dst[dst_pos++] = src[pos++];
  }

  pos += count_white_spaces(&src[pos]);

  dst[dst_pos] = 0;

  if (0 == strcmp("", dst)) {
    return 0;
  }
  return pos;
}

static size_t get_return_type(return_type_t* return_type,
                              const char* buf)
{
  /*
   * Try to find out as much as possible about the function return type at
   * hand.
   *
   * By searching for the function name position on the text line this
   * algorithm will determine if it should search and analyze the return
   * type part of the text line.
   *
   * TODO: This function should dynamically allocate the return type instead
   *       of just filling out a statically allocated string for type name
   *       and such things. - Just to be more consistent with the rest of
   *       the code style in this program.
   */
  /* Remove this when the TODO described above is done. */
  int pos = 0;
  int namepos = 0;
  pos += count_white_spaces(&buf[pos]);

  namepos = find_name_pos(&buf[pos]);
  if (-1 == namepos) {
    return 0;
  }

  return_type->name = malloc(namepos + 3); /* Plus some asterisks and \0 */
  if (NULL == return_type->name) {
    return 0;
  }
  memset(return_type->name, 0, namepos + 3);

  pos += count_white_spaces(&buf[pos]);

  pos += get_prefix_attributes(return_type, &buf[pos], namepos);

  pos += count_white_spaces(&buf[pos]);

  pos += copy_return_type_name(return_type->name, &buf[pos], namepos - pos - 1);

  if (0 == strcmp("void", return_type->name)) {
    return_type->is_void = 1;
  }
  else {
    return_type->is_void = 0;
  }

  if (0 == pos) {
    free(return_type->name);
    return_type->name = NULL;
  }

  return (size_t)pos;
}

static size_t get_function_name(char* name, const char* buf)
{
  /*
   * Grab the name of a function from the function prototype text line.
   *
   * Some hefty assumptions are being done here as well. For example the
   * search parameters include the knowledge that a function name precedes
   * a left parenthesis '('. This is what will be considered a function name.
   */
  size_t pos = 0;
  const size_t len = strlen(buf);

  name[0] = 0;

  while (pos < len) {
    if (';' == buf[pos]) {
      break;
    }
    name[pos] = buf[pos];
    pos++;
    if ('(' == buf[pos]) {
      name[pos] = 0;
      pos += 1;
      pos += count_white_spaces(&buf[pos]);
      return pos;
    }
  }
  return -1;
}

static size_t find_next_comma_in_args(const char* buf) {
  /*
   * Scan the function prototype after the function name and left parenthesis
   * to find all the arguments that the function can take.
   *
   * The function tries to keep track of function-pointers with argument-
   * parenthesis but in the end, it only scans for ',' and it considers ')'
   * to be the end of the list of arguments, which seem to work OK.
   *
   * Speaking of function pointers, the whole function pointer argument, with
   * return type, name and its arguments are considered ONE argument to the
   * function prototype being analyzed.
   */
  const size_t buf_len = strlen(buf);
  size_t len = 0;
  int paren = 0;

  while (len < buf_len) {
    if ((')' == buf[len]) && (0 == paren)) { /* last arg */
      return len;
    }
    paren += ('(' == buf[len]);
    paren -= (')' == buf[len]);
    if ((',' == buf[len]) && (0 == paren)) { /* comma outside funcptr */
      return len;
    }
    len++;
  }

  return 0;
}

static int is_basic_type(const char* buffer) {
  if ((0 != strncmp("_Bool", buffer, 5)) &&
      (0 != strncmp("char", buffer, 4)) &&
      (0 != strncmp("int", buffer, 3)) &&
      (0 != strncmp("float", buffer, 5)) &&
      (0 != strncmp("double", buffer, 6)) &&
      (0 != strncmp("long", buffer, 4)) &&
      (0 != strncmp("void", buffer, 4))) {
    return 0;
  }
  return 1;
}

/*
   Find the end of the argument datatype name.

   If the buffer points to:

                                           Scan starts here
                                           |
                                           v
   const unsigned long long *pointer_to_long
                          ^
                          |
                          This is where the end is.

                                          Scan starts here
                                          |
                                          v
   const unsigned long long a_long_variable
                          ^
                          |
                          This is where the end is.

   If it for some reason only is a type name as primitive declaration
   the end position is returned, since there are no white spaces to
   scan for.
 */
static size_t find_type_len(const char* buf, size_t end) {
  size_t last_type_name_character = end;
  int paren = 0;

  while (0 < last_type_name_character) {
    char c = buf[last_type_name_character];
    paren += (')' == c);
    paren -= ('(' == c);
    if ((' ' == c) && (0 == paren)) {
      if (0 == is_basic_type(&buf[last_type_name_character + 1])) {
        return last_type_name_character;
      }
      /* TODO: Take this back?
      else {
        return end + 1;
      }
      */
    }
    last_type_name_character--;
  }
  return end;
}

#define MAX_ARG_LEN 128

static arg_node_t* add_new_arg_node(arg_list_t* list, const char* src,
                                    size_t len)
{
  char buf[MAX_ARG_LEN];
  arg_node_t* node = NULL;

  memset(buf, 0, sizeof(buf));
  memcpy(buf, src, len);
  buf[len] = 0;

  node = new_arg_node(buf);
  if (NULL == node) {
    return NULL;
  }
  arg_list_add_node(list, node);
  return node;
}

static size_t split2args(arg_list_t* list, const char* buf) {
  /*
   * Add new argument nodes in the list of arguments in the internal
   * representation of the function prototype arguments being scanned.
   */
  size_t pos = 0;
  size_t len = 0;

  while (0 != (len = find_next_comma_in_args(&buf[pos]))) {
    add_new_arg_node(list, &buf[pos], len);
    pos += len;
    if (',' == buf[pos]) {
      pos++;
    }
    pos += count_white_spaces(&buf[pos]);
  }
  return pos;
}

static size_t extract_type_to_string(char** dst, const char* src)
{
  /*
   * Grab the data type part of a scanned function prototype argument into
   * a new string.
   */
  size_t len = find_type_len(src, strlen(src));
  *dst = malloc(len + 1);
  if (NULL == *dst) {
    fprintf(stderr, "ERROR: Out of memory while allocating argument type\n");
    return 0;
  }
  memset(*dst, 0, len + 1);
  strncpy(*dst, src, len);
  return len;
}

static int is_variadic_argument(const char* buf)
{
  /*
   * Variadic arguments are always at the end of a function declaration and
   * are easily found by identifying the three periods.
   */
  if (0 == strncmp(buf, "...", 3)) {
    return 1;
  }
  return 0;
}

static int is_function_pointer(const char* buf)
{
  /*
   * Function pointers are quite easy to find by just searching for the left
   * parenthesis followed by an asterisk.
   */
  if (0 == strncmp(buf, "(*", 2)) {
    return 1;
  }
  return 0;
}

static int is_pointer(const char* buf) {
  /*
   * Consider one or more asterisks to indicate a pointer argument.
   */
  if (('*' == buf[0])) {
    if (('*' == buf[1])) {
      return 2;
    }
    return 1;
  }
  return 0;
}

static char* make_mock_arg_name(size_t idx, int function_pointer,
                                const char* function_pointer_args,
                                int asterisks, int array)
{
  /*
   * Mock-args are the name of the members of the CUTest mock control
   * structure, these are simply named arg0..argN so that you do not have
   * to re-factor the test-cases every time you change an argument name.
   *
   * They are being assigned by the generated mock-implementation of the
   * function that is currently being scanned by functions.
   */
  char* arg = NULL;
  size_t numbers = 1;
  size_t fpchars = 0;
  const unsigned long int idx_lu = idx;

  if (9 < idx) {
    numbers = 2;
  }
  if (1 == function_pointer) {
    fpchars = 3; /* (*...) */
    fpchars += strlen(function_pointer_args);
  }
  arg = malloc(numbers + fpchars + asterisks + strlen("arg") + 1 + array);
  if (1 == function_pointer) {
    sprintf(arg, "(*arg%lu)%s", idx_lu, function_pointer_args);
  }
  else {
    if (0 == array) {
      if (0 == asterisks) {
        sprintf(arg, "arg%lu", idx_lu);
      }
      else if (1 == asterisks) {
        sprintf(arg, "*arg%lu", idx_lu);
      }
      else {
        sprintf(arg, "**arg%lu", idx_lu);
      }
    }
    else { /* Array arguments can be considered a lever of pointer-nes :) */
      if (0 == asterisks) {
        sprintf(arg, "*arg%lu", idx_lu);
      }
      else if (1 == asterisks) {
        sprintf(arg, "**arg%lu", idx_lu);
      }
      else {
        fprintf(stderr, "ERROR: Too many asterisks for an array ref\n");
      }
    }
  }
  return arg;
}

static char* make_assignment_type_cast(char* args_control_type, int asterisks)
{
  /*
   * Generate a type-cast string that can be put in the generated code for
   * the mock-implementation. This helps when arguments are "const", since
   * the mock control structure need to be run-time writable the const-part
   * can not be there, hence a type-cast is needed to not get compiler
   * warnings when the code is generated compiled for assigning the
   * arg0..argN members of the control structure.
   */
  int i;
  const size_t len = strlen(args_control_type);
  char* buf = malloc(1 + len + 1 + asterisks + 1 + 1);

  if (NULL == buf) {
    return NULL;
  }

  strcpy(buf, "(");
  strcat(buf, args_control_type);
  if (0 != asterisks) {
    strcat(buf, " ");
    for (i = 0; i < asterisks; i++) {
      strcat(buf, "*");
    }
  }
  strcat(buf, ")");
  return buf;
}

static char* make_assignment_name(size_t idx)
{
  /*
   * The assignment of the control structure arguments do not need to
   * consider for example if it's a pointer or a value being copied to
   * the control structure, hence the arg0..argN members can be assigned
   * without having asterisks or type-information.
   */
  const unsigned long int idx_lu = idx;
  char* arg = NULL;
  size_t numbers = 1;

  if (9 < idx) {
    numbers = 2;
  }
  arg = malloc(numbers + strlen("arg") + 1);
  if (NULL == arg) {
    return NULL;
  }
  sprintf(arg, "arg%lu", idx_lu);
  return arg;
}

static char* extract_variable_name_to_ptr(char* buf, size_t pos)
{
  return (char*)buf + pos;
}

static int is_array_argument(char* buf)
{
  const size_t len = strlen(buf);

  if (']' == buf[len - 1]) {
    return 1;
  }
  return 0;
}

static char* make_args_control_type(char* type)
{
  if (type == strstr(type, "const ")) {
    type += strlen("const ");
  }
  return type;
}

static size_t get_length_without_asterisks_brackets_or_parenthesis(char* s)
{
  /*
   * Extrapolate the length of a argument variable name but strip pointer-
   * asterisks, array-brackets and function pointer parentheses.
   *
   * This length enables the calling code to know hos much memory is needed
   * for an allocation of a string for a copy of only the name.
   */
  size_t len = strlen(s);
  size_t pos = 0;
  int chars_to_remove = 0;
  const int function_pointer = is_function_pointer(s);

  while (pos < len) {
    if ('*' == s[pos]) {
      chars_to_remove++;
    }
    else if ('[' == s[pos]) {
      chars_to_remove += len - pos;
      break;
    }
    else if (('(' == s[pos]) && (pos != 0)) {
      chars_to_remove += len - pos;
      break;
    }
    pos++;
  }
  if (function_pointer) {
    chars_to_remove += 1;
  }
  return len - chars_to_remove;
}

static char* strip_asterisks_and_brackets(char* variable_name)
{
  /*
   * Allocate memory for and copy a stripped version of the function prototype
   * argument variable name. Pointer asterisks, array brackets and funcion
   * pointer parentheses will not be copied to the new string.
   */
  const size_t len =
    get_length_without_asterisks_brackets_or_parenthesis(variable_name);
  size_t pos = 0;
  const int function_pointer = is_function_pointer(variable_name);
  size_t dst_pos = 0;
  char* buf = malloc(len + 1);

  if (NULL == buf) {
    return NULL;
  }

  if (function_pointer) {
    variable_name++; /* skip the '(' in a function pointer */
  }

  while (dst_pos < len) {
    if ('*' == variable_name[pos]) {
      pos++;
      continue;
    }
    if ((1 == function_pointer) && (')' == variable_name[pos])) {
      break;
    }
    buf[dst_pos++] = variable_name[pos++];
  }
  buf[dst_pos] = '\0';
  return buf;
}

static int is_const(const char* arg)
{
  return (NULL != strstr(arg, "const "));
}

static int parse_argument_to_info(arg_node_t* node, size_t idx)
{
  /*
   * Read the argument stored in an argument node and elaborate it into more
   * information in the argument node structure member variables.
   *
   * Brute-parse a complete argument to find pointer asterisks, array brackets
   * and function pointer parentheses to try to get a better understanding on
   * exactly what any argument represent.
   *
   * This function also prepare a lot of strings that be raw-printed in the
   * code-generation step of cutest_mock, to reduce confusion in that part
   * of the code - since most similar operations are done here.
   *
   * TODO: This function is too big, it need to be re-factored.
   */
  int asterisks = 0;
  size_t pos = 0;
  size_t param_name_start = extract_type_to_string(&node->type, node->arg);

  if (0 == param_name_start) {
    return -1;
  }

  node->variadic = is_variadic_argument(node->type);

  pos = param_name_start;
  pos += count_white_spaces(&node->arg[pos]);

  node->mock_declaration.type = node->type;

  if (0 != node->variadic) {
    node->mock_declaration.comment = "Unsupported in cutest";
    return 0;
  }

  node->function_pointer = is_function_pointer(&node->arg[pos]);
  node->pointer = is_pointer(&node->arg[pos]);
  pos += node->pointer;
  node->variable = extract_variable_name_to_ptr(node->arg, pos);

  node->array = is_array_argument(node->variable);

  if (node->function_pointer) {
    node->function_pointer_args = strchr(&node->arg[pos + 1], '(');
  }
  /* TODO: Dead code? Take this back?
  else if (NULL != node->variable) {
    while ('*' == node->variable[asterisks]) {
      asterisks++;
    }
  }
  */

  node->mock_declaration.name =
    make_mock_arg_name(idx, node->function_pointer,
                       node->function_pointer_args,
                       asterisks, node->array);

  node->args_control.name =
    make_mock_arg_name(idx, node->function_pointer,
                       node->function_pointer_args,
                       asterisks, node->array);

  node->args_control.type = make_args_control_type(node->type);
  node->args_control.comment = node->variable;

  node->assignment.name = make_assignment_name(idx);
  node->assignment.variable = strip_asterisks_and_brackets(node->variable);
  node->caller.name = strip_asterisks_and_brackets(node->variable);

  if (0 != node->pointer) {
    asterisks++;
  }
  node->asterisks = asterisks;

  if (is_const(node->arg)) {
    node->assignment.type_cast = make_assignment_type_cast(node->args_control.type, asterisks);
  }

  return 0;
}

/*
   This function should take a pointer to a buffer pointing to the
   end of the function name in a string from a line produced by cproto.

   unsigned long my_function(const unsigned char value);
                             ^
                             |
                             This is where buf should point at
 */
static size_t get_function_args(arg_list_t* list, const char* buf) {
  size_t pos = 0;
  size_t idx = 0;
  arg_node_t* node;

  pos = split2args(list, buf);

  node = list->first;
  while (NULL != node) {
    parse_argument_to_info(node, idx++);
    node = node->next;
  }

  return pos;
}

static int get_mockables(mockable_list_t* list, const char* nm_filename) {
  /*
   * Open the output text file from the 'nm' command and read all the symbols
   * that are _used_ in the compiled binary version of the design under test.
   *
   * This list of symbols is the template for exactly what functions can be
   * automatically mocked and need a corresponding implementation AS a mock.
   *
   * The cutest_prox tool make use of the same list to change the assembly
   * jumps to callable functions into calls to the mocks generated by this
   * tool.
   */
  char buf[1024];
  int mockable_cnt = 0;

  /*
   * Run nm on the object file in order to get a list of functions that
   * are actually used in the file.
   */
  FILE* fd = fopen(nm_filename, "r");
  if (NULL == fd) {
    fprintf(stderr, "ERROR: Unable to open '%s'\n", nm_filename);
    return 0;
  }

  while (fgets(buf, sizeof(buf), fd)) {
    mockable_node_t* node;
    buf[strlen(buf) - 1] = 0;
    node = new_mockable_node(buf);
    mockable_list_add_node(list, node);
  }

  fclose(fd);

  return mockable_cnt;
}

static void get_include_flags(char* dst, int argc, char* argv[]) {
  /*
   * Every command line option to cutest_mock after the last one option,
   * (as described in the usage) are being passed forward to the 'cproto'
   * tool. This is useful to give it access to include-paths for example, so
   * that it can find functions to create function prototypes for them.
   */
  const char* cutest_path = argv[4];
  int i;

  strcpy(dst, "-I\"");
  strcat(dst, cutest_path);
  strcat(dst, "\"");

  if (argc > 5) {
    strcat(dst, " ");
  }

  for (i = 5; i < argc; i++) {
    strcat(dst, argv[i]);
    if (i < argc - 1) {
      strcat(dst, " ");
    }
  }
}

static int node_symbol_match(mockable_node_t* node, const char* symbol,
                             size_t symbol_len)
{
  /*
   * Just match the given symbol with the node list parsed from the 'nm'
   * command to determine if the symbol even should be mocked.
   */
  const size_t mockable_len = strlen(node->symbol_name);

  return ((symbol_len == mockable_len) &&
          (0 == strcmp(node->symbol_name, symbol)));
}

static mockable_node_t* symbol_is_in_list(mockable_list_t* list,
                                          const char* symbol)
{
  mockable_node_t* node;
  const size_t symbol_len = strlen(symbol);

  for (node = list->first; NULL != node; node = node->next) {
    if (node_symbol_match(node, symbol, symbol_len)) {
      return node;
    }
  }
  return NULL;
}

static mockable_node_t* parse_cproto_row(mockable_list_t* list, char* buf)
{
  /*
   * Grab symbol names from the 'cproto' output and match against the list
   * of symbols from the 'nm' output and only analyze the symbols that are
   * to be mocked.
   *
   * Grab function names, return types, and arguments for each symbol that
   * is to be considered to produce a mock-up.
   */
  size_t pos = 0;
  return_type_t return_type;
  mockable_node_t* node = NULL;
  char function_name[1024];

  memset(&return_type, 0, sizeof(return_type));
  pos += get_return_type(&return_type, &buf[pos]);
  if (0 == pos) {
    return NULL;
  }
  pos += get_function_name(function_name, &buf[pos]);
  if (NULL == (node = symbol_is_in_list(list, function_name))) {
    free(return_type.name);
    return NULL;
  }
  if (1 == node->legit) {
    free(return_type.name);
    return NULL;
  }
  pos += get_function_args(node->args, &buf[pos]);

  node->return_type = return_type;
  node->legit = 1;
  return node;
}

static void construct_cproto_command_line(char* dst, const char* cproto,
                                          int argc, char* argv[], char** tmpfile)
{
  char iflags[1024];
  const char* filename = argv[2];
  *tmpfile = malloc(strlen(filename) + strlen(".cproto") + 1);
  strcpy(*tmpfile, filename);
  strcat(*tmpfile, ".cproto");

  get_include_flags(iflags, argc, argv);
  sprintf(dst, "\"%s\" -i -s -x %s \"%s\" 2>/dev/null >%s", cproto, iflags, filename, *tmpfile);

  /* Remember to free the tmpfile */
}

static int execute_cproto(mockable_list_t* list, const char* cproto,
                          int argc, char* argv[])
{
  /*
   * Execute the 'cproto' binary and parse all output rows.
   */
  char buf[1024];
  char command[1024];
  FILE* fd = NULL;
  char* tmpfile;

  construct_cproto_command_line(command, cproto, argc, argv, &tmpfile);

  if (0 != system(command)) {
    fprintf(stderr, "ERROR: cproto failed\n");
    return 0;
  }

  fd = fopen(tmpfile, "r");

  free(tmpfile);

  if (NULL == fd) {
    fprintf(stderr, "ERROR: Unable to execute command '%s'\n", command);
    return 0;
  }

  while (fgets(buf, sizeof(buf), fd)) {
    if (0 == strncmp(buf, "/*", 2)) {
      continue;
    }
    buf[strlen(buf) - 1] = 0;
    parse_cproto_row(list, buf);
  }


  fclose(fd);

  unlink(tmpfile);

  return 1;
}

static void print_function_args(arg_list_t* list) {
  /*
   * Helper to print each function prototype argument as a raw copy from the
   * 'cproto' output.
   */
  arg_node_t* node;

  for (node = list->first; NULL != node; node = node->next) {
    if (NULL != node->next) {
      printf("%s, ", node->arg);
    }
    else {
      printf("%s", node->arg);
    }
  }
}

static void print_mock_control_struct_with_return_type(mockable_node_t* node)
{
  /*
   * Output a mock-control structure including the retval member (e.g. non-
   * void return types can be controlled via the mock-control structure).
   */
  printf("  struct {\n"
         "    int call_count;\n"
         "    %s%s retval;\n"
         "    %s%s (*func)(",
         (node->return_type.is_struct ? "struct " : ""),
         node->return_type.name,
         (node->return_type.is_struct ? "struct " : ""),
         node->return_type.name);
  print_function_args(node->args);
  printf(");\n");
}

static void print_mock_control_struct_with_void_type(mockable_node_t* node)
{
  /*
   * Output a mock-control structure without the retval member (for functions
   * that does not return anything).
   */
  printf("  struct {\n"
         "    int call_count;\n"
         "    void (*func)(");
  print_function_args(node->args);
  printf(");\n");
}

static void print_mock_control_struct_arg(arg_node_t* node)
{
  /*
   * Try to represent each argument allowed to a function call into a storage
   * inside the mock-control structure with "similar" data type and storage
   * size.
   */
  char buf[3];
  buf[0] = '*';
  buf[1] = '*';
  buf[node->pointer] = '\0';

  printf("      %s %s%s; /* %s */\n",
         node->args_control.type,
         buf,
         node->args_control.name,
         node->args_control.comment);
}

static void print_mock_control_struct_args(arg_list_t* list)
{
  /*
   * Output the mock-control data structure for storing arguments for test
   * case assertions.
   */
  arg_node_t* node;

  printf("    struct {\n");
  printf("      int this_variable_is_only_here_to_allow_empty_arg_struct;\n");
  for (node = list->first; NULL != node; node = node->next) {
    if (node->variadic) {
      printf("      /* Skipping variadic arguments */\n");
      continue;
    }
    if ((0 == node->pointer) && /* void-arguments can't be stored */
        (NULL != node->args_control.type) &&
        (0 == strcmp(node->args_control.type, "void")) &&
        (0 == node->function_pointer)) {
      printf("      /* Skipping void arguments */\n");
      continue;
    }
    print_mock_control_struct_arg(node);
  }
  printf("    } args;\n");
}

static void print_mock_control_struct(mockable_node_t* node)
{
  if (0 == node->return_type.is_void) {
    print_mock_control_struct_with_return_type(node);
  }
  else {
    print_mock_control_struct_with_void_type(node);
  }

  if (NULL != node->args) {
    if ((NULL != node->args->first) && (0 == node->args->first->variadic)) {
      print_mock_control_struct_args(node->args);
    }
  }

  printf("  } %s;\n", node->symbol_name);
}

static void print_mock_control_structs(mockable_list_t* list)
{
  mockable_node_t* node;
  printf("/*\n"
         " * Mock control structures, can be read or written from your tests.\n"
         " */\n"
         "\n"
         "struct {\n\n"
         "  int keep_the_struct_with_contents_if_no_mocks_generated;\n"
         "\n");
  for (node = list->first; NULL != node; node = node->next) {
    if (0 == node->legit) {
      continue;
    }
    print_mock_control_struct(node);
    if (NULL != node->next) {
      printf("\n");
    }
  }
  printf("} cutest_mock;\n"
         "\n");
}

static void copy_pre_processor_directives_from_dut(const char* filename)
{
  /*
   * Output all the pre-processor directives found in the source code of
   * the design under test and put them in the mocks header file as well
   * so that #includes and #defines are part of the artifact.
   */
  char buf[1024];
  FILE* f = fopen(filename, "r");

  if (NULL == f) {
    fprintf(stderr, "ERROR: Unable to open '%s'\n", filename);
    return;
  }

  while (fgets(buf, sizeof(buf), f)) {
    if ('#' != buf[0]) {
      continue;
    }
    printf("%s", buf);
  }
  fclose(f);

  puts("#include <stdarg.h>");
  puts("");
}

static void print_declaration(const char* pretype, const char* prefix,
                              mockable_node_t* node)
{
  /*
   * Print a function declaration (controlled from wrapper functions to
   * prefix with "extern" and such things).
   */
  if (node->return_type.is_struct) {
    printf("%sstruct /*type:*/ %s /*name:*/ %s%s(", pretype, node->return_type.name, prefix,
           node->symbol_name);
  }
  else {
    printf("%s%s %s%s(", pretype, node->return_type.name, prefix,
           node->symbol_name);
  }
  print_function_args(node->args);
  printf(")");
}

static void print_declarations(const char* pretype, const char* prefix,
                               mockable_list_t* list)
{
  mockable_node_t* node;

  for (node = list->first; NULL != node; node = node->next) {
    if (0 == node->legit) {
      continue;
    }
    print_declaration(pretype, prefix, node);
    printf(";\n");
  }
}

static void print_dut_declarations(mockable_list_t* list)
{
  printf("/*\n"
         " * These extern declarations are here to help the compiler to find\n"
         " * your design under test.\n"
         " */\n"
         "\n");
  print_declarations("extern ", "", list);
  printf("\n");
}

static void print_mock_declarations(mockable_list_t* list)
{
  printf("/*\n"
         " * Function prototypes to all mock-up functions that are to\n"
         " * replace function calls within your design under test.\n"
         " */\n"
         "\n");
  print_declarations("", "cutest_", list);
  printf("\n");
}

static void strip_array_part(char* dst, char* src) {
  size_t src_len = 0;
  size_t pos = 0;

  /* Could this be reused in stripping of asterisks and brackets? */
  if (NULL == src) {
    return;
  }
  src_len = strlen(src);

  while (('[' != src[pos]) && (pos < src_len)) {
    dst[pos] = src[pos];
    pos++;
  }
  dst[pos] = 0;
}

static void print_arg_assignment(arg_node_t* node, const char* name)
{
  /*
   * Output the code to assign the storage variables for arguments passed
   * to a mock-up for later analysis in the test-cases.
   */
  if (node->variadic) {
    printf("  /* Can not inspect variadic argument with cutest */\n");
    return;
  }
  if (0 == strcmp("", node->assignment.variable)) {
    printf("  /* Can not assign void arguments to anything */\n");
    return;
  }
  printf("  cutest_mock.%s.args.%s = %s%s;\n",
         name, node->assignment.name,
         (NULL == node->assignment.type_cast ? "" : node->assignment.type_cast),
         node->assignment.variable);
}

static void print_arg_assignments(arg_list_t* list, const char* name)
{
  arg_node_t* node;

  for (node = list->first; NULL != node; node = node->next) {
    print_arg_assignment(node, name);
  }
}

static void print_stub_caller_args(arg_list_t* list)
{
  /*
   * Output the argument list as variable names ONLY to mock-ups that pretend
   * to be other implementations or real implementations by calling the
   * function set to the .func-member of the mock-control structure.
   */
  arg_node_t* node = NULL;

  for (node = list->first; NULL != node; node = node->next) {
    if (node->variadic) {
      continue;
    }
    if (NULL == node->next) {
      printf("%s", node->caller.name);
    }
    else {
      printf("%s, ", node->caller.name);
    }
  }
}

static char* last_unvariadic_arg(arg_list_t* list)
{
  arg_node_t* node = NULL;

  /* Loop until the next to last node */
  for (node = list->first; NULL != node && NULL != node->next; node = node->next) {
    const int variadic = node->next->variadic;

    if (1 == variadic) {
      return node->variable;
    }
  }
  return NULL;
}

static int print_printf_caller(arg_list_t* list)
{
  char lastarg[128];

  strip_array_part(lastarg, last_unvariadic_arg(list));
  printf("    if (printf != cutest_mock.printf.func) {\n"
         "      fprintf(stderr, \"WARNING: You can only set "
         "cutest_mock.printf.func to printf if you want to call real "
         "code, due to lack of support for variadic arguments.\\n\");\n"
         "      return cutest_mock.printf.retval;\n"
         "    }\n");
  printf("    va_list arg;\n"
         "    int done;\n"
         "    va_start (arg, %s);\n"
         "    done = vprintf(", lastarg);
  print_stub_caller_args(list);
  printf("arg);\n"
         "    va_end (arg);\n"
         "    return done;\n"
         "  }\n");
  return 1;
}

static int print_sprintf_caller(arg_list_t* list)
{
  char lastarg[128];

  strip_array_part(lastarg, last_unvariadic_arg(list));
  printf("    if (sprintf != cutest_mock.sprintf.func) {\n"
         "      fprintf(stderr, \"WARNING: You can only set "
         "cutest_mock.sprintf.func to sprintf if you want to call real "
         "code, due to lack of support for variadic arguments.\\n\");\n"
         "      return cutest_mock.sprintf.retval;\n"
         "    }\n");
  printf("    va_list arg;\n"
         "    int done;\n"
         "    va_start (arg, %s);\n"
         "    done = vsprintf(", lastarg);
  print_stub_caller_args(list);
  printf("arg);\n"
         "    va_end (arg);\n"
         "    return done;\n"
         "  }\n");
  return 1;
}

static int print_snprintf_caller(arg_list_t* list)
{
  char lastarg[128];

  strip_array_part(lastarg, last_unvariadic_arg(list));
  printf("    if (snprintf != cutest_mock.snprintf.func) {\n"
         "      fprintf(stderr, \"WARNING: You can only set "
         "cutest_mock.snprintf.func to snprintf if you want to call real "
         "code, due to lack of support for variadic arguments.\\n\");\n"
         "      return cutest_mock.sprintf.retval;\n"
         "    }\n");
  printf("    va_list arg;\n"
         "    int done;\n"
         "    va_start (arg, %s);\n"
         "    done = vsprintf(", lastarg);
  print_stub_caller_args(list);
  printf("arg);\n"
         "    va_end (arg);\n"
         "    return done;\n"
         "  }\n");
  return 1;
}

static int print_fprintf_caller(arg_list_t* list)
{
  char lastarg[128];

  strip_array_part(lastarg, last_unvariadic_arg(list));
  printf("    if (fprintf != cutest_mock.fprintf.func) {\n"
         "      fprintf(stderr, \"WARNING: You can only set "
         "cutest_mock.fprintf.func to fprintf if you want to call real "
         "code, due to lack of support for variadic arguments.\\n\");\n"
         "      return cutest_mock.fprintf.retval;\n"
         "    }\n");
  printf("    va_list arg;\n"
         "    int done;\n"
         "    va_start (arg, %s);\n"
         "    done = vfprintf(", lastarg);
  print_stub_caller_args(list);
  printf("arg);\n"
         "    va_end (arg);\n"
         "    return done;\n"
         "  }\n");
  return 1;
}

static void print_call_without_return_value(const char* symbol_name)
{
  printf("    cutest_mock.%s.func(", symbol_name);
}

static void print_call_with_return_value(const char* symbol_name)
{
  printf("    return cutest_mock.%s.func(", symbol_name);
}

static int print_stub_caller(mockable_node_t* node)
{
  printf("  if (NULL != cutest_mock.%s.func) {\n", node->symbol_name);
  if (0 == strcmp(node->symbol_name, "printf")) {
    return print_printf_caller(node->args);
  }
  else if (0 == strcmp(node->symbol_name, "sprintf")) {
    return print_sprintf_caller(node->args);
  }
  else if (0 == strcmp(node->symbol_name, "snprintf")) {
    return print_snprintf_caller(node->args);
  }
  else if (0 == strcmp(node->symbol_name, "fprintf")) {
    return print_fprintf_caller(node->args);
  }
  else if ((NULL != node->args) &&
           (NULL != node->args->last) &&
           (1 == node->args->last->variadic)) {
    printf("    fprintf(stderr, \"WARNING: No support for stubbing variadic "
           "function '%s'.\\n\");\n", node->symbol_name);
    printf("  }\n");
    return 1;
  }
  else if (0 == strcmp(node->return_type.name, "void")) {
    print_call_without_return_value(node->symbol_name);
  }
  else {
    print_call_with_return_value(node->symbol_name);
  }
  print_stub_caller_args(node->args);
  printf(");\n");
  printf("  }\n");
  if (0 == strcmp(node->return_type.name, "void")) {
    return 0;
  }
  return 1;
}

static void print_mock_implementation(mockable_node_t* node)
{
  print_declaration("", "cutest_", node);
  printf("\n"
         "{\n"
         "  cutest_mock.%s.call_count++;\n", node->symbol_name);
  print_arg_assignments(node->args, node->symbol_name);
  if (print_stub_caller(node)) {
    printf("\n"
           "  return cutest_mock.%s.retval;\n",
           node->symbol_name);
  }
  printf("}\n"
         "\n");
}

static void print_mock_implementations(mockable_list_t* list)
{
  mockable_node_t* node = NULL;

  printf("/*\n"
         " * Mock-up implementations of every callable function in your"
         " * design. These functions are called by default instead of the"
         " * original functions. If you use module_test() macro for your test"
         " * these mock-up functions will be called too, but the .func member"
         " * variable is automatically set to the original function in the"
         " * test set-up.\n"
         " */\n"
         "\n");

  for (node = list->first; NULL != node; node = node->next) {
    if (0 == node->legit) {
      continue;
    }
    print_mock_implementation(node);
  }
}

static void print_mock_func_module_test_assignment(mockable_node_t* node)
{
  printf("  cutest_mock.%s.func = %s;\n",
         node->symbol_name, node->symbol_name);
}

static void print_all_mock_func_module_test_assignments(mockable_list_t* list)
{
  mockable_node_t* node = NULL;

  printf("void cutest_set_mocks_to_original_functions() {\n");
  for (node = list->first; NULL != node; node = node->next) {
    if (0 == node->legit) {
      continue;
    }
    print_mock_func_module_test_assignment(node);
  }
  printf("}\n"
         "\n");
}

int main(int argc, char* argv[])
{
  const char* program_name = argv[0];
  const char* cproto = argv[1];
  const char* filename = argv[2];
  const char* nm_filename = argv[3];
  const char* cutest_path = argv[4];
  mockable_list_t* list = NULL;

  if (argc < 5) {
    fprintf(stderr, "ERROR: Missing argument\n");
    usage(program_name);
    return EXIT_FAILURE;
  }

  list = new_mockable_list();
  if (NULL == list) {
    return EXIT_FAILURE;
  }

  get_mockables(list, nm_filename);

  if (0 == execute_cproto(list, cproto, argc, argv)) {
    delete_mockable_list(list);
    return EXIT_FAILURE;
  }

  printf("/*\n"
         " * This file is generated by '%s %s %s %s'\n"
         " */\n\n", program_name, filename, nm_filename, cutest_path);

  copy_pre_processor_directives_from_dut(filename);
  print_dut_declarations(list);
  print_mock_declarations(list);
  print_mock_control_structs(list);
  print_mock_implementations(list);

  print_all_mock_func_module_test_assignments(list);

  delete_mockable_list(list);

  return EXIT_SUCCESS;
}
