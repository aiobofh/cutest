#include <stdlib.h>
/*
 * The practice of test-driven design
 * ----------------------------------
 *
 * Written after the first test was defined:
 *
 * int add(int a, int b)
 * {
 *   return 0;
 * }
 *
 * Written after the second test was defined:
 *
 * long long add(int a, int b)
 * {
 *   return a + b;
 * }
 *
 * Written after the third and fourth test was defined:
 *
 * long long add(int a, int b)
 * {
 *   return (long long)a + (long)b;
 * }
 *
 */

long long add(int a, int b)
{
  return (long long)a + (long long)b;
}
