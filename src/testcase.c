#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "testcase.h"
#include "list.h"

static testcase_node_t* allocate_testcase_node()
{
  testcase_node_t* node = malloc(sizeof(testcase_node_t));
  if (NULL == node) {
    fprintf(stderr, "ERROR: Out of memory while allocating new testcase node\n");
    return NULL;
  }
  memset(node, 0, sizeof(*node));
  return node;
}

static void free_testcase_node(testcase_node_t* node)
{
  free(node);
}

static char* new_testcase(const char* src)
{
  const size_t len = strlen(src);
  char* arg = malloc(len + 1);
  if (NULL == arg) {
    fprintf(stderr, "ERROR: Out of memory while allocating testcase\n");
    return NULL;
  }
  strncpy(arg, src, len + 1);
  return arg;
}

static void delete_testcase(char* testcase)
{
  free(testcase);
}

testcase_node_t* new_testcase_node(const char* testcase_name, int reset)
{
  testcase_node_t* node = allocate_testcase_node();
  if (NULL == node) {
    return NULL;
  }

  char* testcase = new_testcase(testcase_name);
  if (NULL == testcase) {
    free_testcase_node(node);
    return NULL;
  }

  node->testcase = testcase;
  node->reset = reset;

  return node;
}

void delete_testcase_node(testcase_node_t* node)
{
  if (NULL != node->testcase) {
    delete_testcase(node->testcase);
  }

  free_testcase_node(node);
}

LIST_IMPL(testcase)
