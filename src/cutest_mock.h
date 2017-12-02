#ifndef _CUTEST_MOCK_H_
#define _CUTEST_MOCK_H_

#define MAX_CUTEST_MOCKS 1024
#define MAX_CUTEST_MOCK_ARGS 128
#define MAX_CUTEST_MOCK_NAME_LENGTH 128

#include "arg.h"

typedef struct cutest_mock_arg_type_s {
  char name[MAX_CUTEST_MOCK_NAME_LENGTH + 1];
  char struct_member_type_name[MAX_CUTEST_MOCK_NAME_LENGTH + 1];
} cutest_mock_arg_type_t;

typedef struct cutest_mock_arg_s {
  char name[MAX_CUTEST_MOCK_NAME_LENGTH + 1];
  char function_pointer_name[MAX_CUTEST_MOCK_NAME_LENGTH + 1];
  char arg_text[MAX_CUTEST_MOCK_NAME_LENGTH + 1];
  char struct_text[MAX_CUTEST_MOCK_NAME_LENGTH + 1];
  char assignment_code[2 * MAX_CUTEST_MOCK_NAME_LENGTH + 1];
  cutest_mock_arg_type_t type;
  int is_function_pointer;
} cutest_mock_arg_t;

typedef struct cutest_mock_return_type_s {
  int is_static;
  int is_const;
  int is_inline;
  int is_struct;
  char name[MAX_CUTEST_MOCK_NAME_LENGTH + 1];
} cutest_mock_return_type_t;

typedef struct cutest_mock_s {
  char name[MAX_CUTEST_MOCK_NAME_LENGTH + 1];
  cutest_mock_return_type_t return_type;
  int arg_cnt;
  cutest_mock_arg_t arg[MAX_CUTEST_MOCK_ARGS];
  char func_text[MAX_CUTEST_MOCK_NAME_LENGTH + 1];

  arg_node_t arg_list;
} cutest_mock_t;

typedef struct cutest_mocks_s {
  int mock_cnt;
  cutest_mock_t mock[MAX_CUTEST_MOCKS];
} cutest_mocks_t;

#endif
