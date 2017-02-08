#include <stdio.h>

static int bar(int argument) {
  return argument * 4;
}

void foo(int* argument)
{
  if (NULL == argument) {
    return;
  }
  *argument = call(bar(*argument));
}
