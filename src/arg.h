#ifndef _ARG_H_
#define _ARG_H_

#include <stdlib.h>
#include "list.h"

typedef struct arg_node_s {
  char* arg;
  char* type;
  char* variable;
  int function_pointer;
  char* function_pointer_args;
  int variadic;
  int array;
  int pointer;
  int asterisks;
  struct {
    char* type;
    char* name; /* arg0, arg1 ... */
    char* comment; /* Warnings about variadic macros */
  } mock_declaration;
  struct {
    char* type; /* Typically without const */
    char* name;
    char* comment; /* Original variable name */
  } args_control;
  struct {
    char* name;
    char* variable; /* Original variable name */
    char* type_cast;
  } assignment;
  struct {
    char* name;
  } caller;
  struct arg_node_s* next;
} arg_node_t;

arg_node_t* new_arg_node(const char* src);
void delete_arg_node(arg_node_t* node);

LIST_DECL(arg)

#endif
