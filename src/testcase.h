#ifndef _TESTCASE_H_
#define _TESTCASE_H_

#include "list.h"

typedef struct testcase_node_s {
  char* testcase;
  int reset;
  struct testcase_node_s* next;
} testcase_node_t;

testcase_node_t* new_testcase_node(const char* name, int reset);
void delete_testcase_node(testcase_node_t* node);

LIST_DECL(testcase)

#endif
