#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arg.h"
#include "mockable.h"

/*
static void* my_malloc_ptr = NULL;

//#define malloc(bytes) \
  my_malloc_ptr = malloc(bytes); fprintf(stderr, "malloc: %s:%d %p\n", __FILE__, __LINE__, my_malloc_ptr)

//#define free(ptr) \
  free((void*)ptr); fprintf(stderr, "free: %s:%d %p\n", __FILE__, __LINE__, ptr)
*/

static mockable_node_t *allocate_mockable_node()
{
  mockable_node_t* node = malloc(sizeof(mockable_node_t));
  if (NULL == node) {
    fprintf(stderr, "ERROR: Out of memory while allocating new mockable\n");
    return NULL;
  }
  memset(node, 0, sizeof(*node));
  node->args = new_arg_list();
  node->next = NULL;
  return node;
}

static void free_mockable_node(mockable_node_t* node)
{
  free(node);
}

static char* new_symbol_name(const char* src)
{
  const size_t len = strlen(src);
  char* symbol_name = malloc(len + 1);
  if (NULL == symbol_name) {
    fprintf(stderr, "ERROR: Out of memory while allocating mockable name\n");
    return NULL;
  }
  strncpy(symbol_name, src, len + 1);
  return symbol_name;
}

static void delete_symbol_name(char* symbol_name)
{
  free(symbol_name);
}

static void delete_return_type_name(char* name) {
  free(name);
}

mockable_node_t* new_mockable_node(const char* symbol_name)
{
  mockable_node_t* node = allocate_mockable_node();
  if (NULL == node) {
    return NULL;
  }

  char* sn = new_symbol_name(symbol_name);
  if (NULL == sn) {
    free_mockable_node(node);
    return NULL;
  }

  node->symbol_name = sn;

  return node;
}

void delete_mockable_node(mockable_node_t* node)
{
  if (NULL != node->symbol_name) {
    delete_symbol_name(node->symbol_name);
  }
  if (NULL != node->args) {
    delete_arg_list(node->args);
  }
  if (NULL != node->return_type.name) {
    delete_return_type_name(node->return_type.name);
  }
  free_mockable_node(node);
}

LIST_IMPL(mockable)
