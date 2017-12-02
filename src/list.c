#include "list.h"

#include <stdio.h>
#include <stdlib.h>

static void delete_just_for_test_node(just_for_test_node_t* node) {
  (void)node;
}

LIST_IMPL(just_for_test)
