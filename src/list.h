#ifndef _LIST_H_
#define _LIST_H_

#define LIST_DECL(PREFIX)                                               \
  typedef struct PREFIX##_list_s {                                      \
    PREFIX##_node_t* first;                                             \
    PREFIX##_node_t* last;                                              \
  } PREFIX##_list_t;                                                    \
                                                                        \
  PREFIX##_list_t* new_##PREFIX##_list();                               \
  void PREFIX##_list_add_node(PREFIX##_list_t* list,                    \
                              PREFIX##_node_t* node);                   \
  void delete_##PREFIX##_list(PREFIX##_list_t* list);

#define LIST_IMPL(PREFIX)                                               \
  PREFIX##_list_t* new_##PREFIX##_list()                                \
  {                                                                     \
    PREFIX##_list_t* list = malloc(sizeof(PREFIX##_list_t));            \
    if (NULL == list) {                                                 \
      fprintf(stderr,                                                   \
              "ERROR: Out of memory while allocating arg list\n");      \
      return NULL;                                                      \
    }                                                                   \
    list->first = list->last = NULL;                                    \
    return list;                                                        \
  }                                                                     \
                                                                        \
  void PREFIX##_list_add_node(PREFIX##_list_t* list,                    \
                              PREFIX##_node_t* node)                    \
  {                                                                     \
    if (NULL == list->first) {                                          \
      list->first = node;                                               \
    }                                                                   \
    if (NULL != list->last) {                                           \
      list->last->next = node;                                          \
    }                                                                   \
    list->last = node;                                                  \
  }                                                                     \
                                                                        \
  void delete_##PREFIX##_list(PREFIX##_list_t* list)                    \
  {                                                                     \
    PREFIX##_node_t* node = list->first;                                \
    while (NULL != node) {                                              \
      PREFIX##_node_t* n = node->next;                                  \
      delete_##PREFIX##_node(node);                                     \
      node = n;                                                         \
    }                                                                   \
    free(list);                                                         \
  }

typedef struct just_for_test_node_s {
  struct just_for_test_node_s* next;
} just_for_test_node_t;

LIST_DECL(just_for_test)

#endif
