#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arg.h"
#include "list.h"


/*
static void* my_malloc_ptr = NULL;
//#define malloc(bytes)                                                   \
  my_malloc_ptr = malloc(bytes); fprintf(stderr, "malloc: %s:%d %p\n", __FILE__, __LINE__, my_malloc_ptr)

//#define free(ptr) \
  free((void*)ptr); fprintf(stderr, "free: %s:%d %p\n", __FILE__, __LINE__, ptr)
*/

static arg_node_t* allocate_arg_node()
{
  arg_node_t* node = malloc(sizeof(arg_node_t));
  if (NULL == node) {
    fprintf(stderr, "ERROR: Out of memory while allocating new arg node\n");
    return NULL;
  }
  memset(node, 0, sizeof(*node));
  return node;
}

static void free_arg_node(arg_node_t* node)
{
  free(node);
}

static char* new_arg(const char* src)
{
  const size_t len = strlen(src);
  char* arg = malloc(len + 1);
  if (NULL == arg) {
    fprintf(stderr, "ERROR: Out of memory while allocating arg\n");
    return NULL;
  }
  strncpy(arg, src, len + 1);
  return arg;
}

static void delete_arg(char* arg)
{
  free(arg);
}

static void delete_type(char* type)
{
  free(type);
}

static void delete_mock_declaration_name(char* name)
{
  free(name);
}

static void delete_args_control_name(char* name)
{
  free(name);
}

static void delete_assignment_name(char* name)
{
  free(name);
}

static void delete_assignment_variable(char* name)
{
  free(name);
}

static void delete_caller_name(char* name)
{
  free(name);
}

static void delete_assignment_type_cast(char* type_cast)
{
  free(type_cast);
}

arg_node_t* new_arg_node(const char* arg_string)
{
  arg_node_t* node = allocate_arg_node();
  if (NULL == node) {
    return NULL;
  }

  char* arg = new_arg(arg_string);
  if (NULL == arg) {
    free_arg_node(node);
    return NULL;
  }

  node->arg = arg;

  return node;
}

void delete_arg_node(arg_node_t* node)
{
  if (NULL != node->arg) {
    delete_arg(node->arg);
  }
  if (NULL != node->type) {
    delete_type(node->type);
  }
  if (NULL != node->mock_declaration.name) {
    delete_mock_declaration_name(node->mock_declaration.name);
  }
  if (NULL != node->args_control.name) {
    delete_args_control_name(node->args_control.name);
  }
  if (NULL != node->assignment.name) {
    delete_assignment_name(node->assignment.name);
  }
  if (NULL != node->assignment.variable) {
    delete_assignment_variable(node->assignment.variable);
  }
  if (NULL != node->caller.name) {
    delete_caller_name(node->caller.name);
  }
  if (NULL != node->assignment.type_cast) {
    delete_assignment_type_cast(node->assignment.type_cast);
  }

  free(node);
}

LIST_IMPL(arg)
