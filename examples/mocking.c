#include <stdio.h>

/* An "obscure" function signature */
static int mii(int (*func)(int argument), int argument) {
  return func(argument);
}

/* Some statics and consts */
static int bar(const int argument) {
  return argument * 4;
}

/* Pointer arg */
void foo(int* argument)
{
  if (NULL == argument) {
    return;
  }
  *argument = bar(*argument);
}

/* Function pointer passing */
void moo(int* argument)
{
  if (NULL == argument) {
    return;
  }
  *argument = mii(bar, *argument);
}

/* Array pointer magic */
void muu(int* argument[2]) {
  *argument[0] = bar(*argument[0]);
  *argument[1] = bar(*argument[1]);
}
