#ifndef _MOCKABLE_H_
#define _MOCKABLE_H_

#include "list.h"

#include "arg.h"

typedef struct return_type_s {
  int is_static;
  int is_const;
  int is_inline;
  int is_struct;
  int is_void;
  char* name;
} return_type_t;

typedef struct mockable_node_s {
  return_type_t return_type;
  char* symbol_name;
  arg_list_t* args;
  int legit;
  struct mockable_node_s* next;
} mockable_node_t;

mockable_node_t* new_mockable_node(const char* symbol_name);
void delete_mockable_node(mockable_node_t* node);

LIST_DECL(mockable)

#endif
