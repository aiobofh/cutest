#include <limits.h>
#include <float.h>
#include <stdio.h>

extern int float_compare(float a, float b);
extern int double_compare(double a, double b);
extern int long_double_compare(long double a, long double b);

/*
 * Used for testing user-defined types
 */
typedef struct my_own_type_s {
  int a;
  char b[3];
  long c;
} my_own_type_t;

/*
 * compare function for our own type
 */
static int compare_my_own_type_t(my_own_type_t a, my_own_type_t b, char* output)
{
  if ((a.a == b.a) &&
      (a.b[0] == b.b[0]) &&
      (a.b[1] == b.b[1]) &&
      (a.b[2] == b.b[2]) &&
      (a.c == b.c)) {
    return 0;
  }
  sprintf(output, "{%d, \"%c%c%c\", %ld}, {%d, \"%c%c%c\", %ld}",
          a.a, a.b[0], a.b[1], a.b[2], a.c,
          b.a, b.b[0], b.b[1], b.b[2], b.c);
  return 1;
}

/*
 * This macro is inserted into the cutest assert_eq comparator
 */
#define CUTEST_MY_OWN_EQ_COMPARATORS(EXP, REF) \
  my_own_type_t: compare_my_own_type_t,

/*
#define CUTEST_LENIENT_ASSERTS 1
*/
#include "cutest.h"

/*
 * Some really naive tests for internal testing of CUTest lenient asserts
 */
test(cutest_shall_compare_bools_correctly)
{
  _Bool my_bool = (1 == 1);
  assert_eq((1 == 1), my_bool);
  my_bool = (1 != 1);
  assert_eq((1 != 1), my_bool);
}

test(cutest_shall_compare_char_values_correctly)
{
  const char my_char_min = CHAR_MIN;
  const char my_char_max = CHAR_MAX;

  assert_eq(CHAR_MIN == my_char_min);
  assert_eq(CHAR_MIN, my_char_min);
  assert_eq(CHAR_MIN, my_char_min, "Text");

  assert_eq(CHAR_MAX == my_char_max);
  assert_eq(CHAR_MAX, my_char_max);
  assert_eq(CHAR_MAX, my_char_max, "Text");
}

test(cutest_shall_compare_unsigned_char_values_correctly)
{
  const unsigned char my_unsigned_char_min = 0;
  const unsigned char my_unsigned_char_max = UCHAR_MAX;

  assert_eq(0 == my_unsigned_char_min);
  assert_eq(0, my_unsigned_char_min);
  assert_eq(0, my_unsigned_char_min, "Text");

  assert_eq(UCHAR_MAX == my_unsigned_char_max);
  assert_eq(UCHAR_MAX, my_unsigned_char_max);
  assert_eq(UCHAR_MAX, my_unsigned_char_max, "Text");
}

test(cutest_shall_compare_signed_char_values_correctly)
{
  const signed char my_signed_char_min = SCHAR_MIN;
  const signed char my_signed_char_max = SCHAR_MAX;

  assert_eq(SCHAR_MIN == my_signed_char_min);
  assert_eq(SCHAR_MIN, my_signed_char_min);
  assert_eq(SCHAR_MIN, my_signed_char_min, "Text");

  assert_eq(SCHAR_MAX == my_signed_char_max);
  assert_eq(SCHAR_MAX, my_signed_char_max);
  assert_eq(SCHAR_MAX, my_signed_char_max, "Text");
}

test(cutest_shall_compare_short_int_values_correctly)
{
  const short int my_short_int_min = SHRT_MIN;
  const short int my_short_int_max = SHRT_MAX;

  assert_eq(SHRT_MIN == my_short_int_min);
  assert_eq(SHRT_MIN, my_short_int_min);
  assert_eq(SHRT_MIN, my_short_int_min, "Text");

  assert_eq(SHRT_MAX == my_short_int_max);
  assert_eq(SHRT_MAX, my_short_int_max);
  assert_eq(SHRT_MAX, my_short_int_max, "Text");
}

test(cutest_shall_compare_unsigned_short_int_values_correctly)
{
  const unsigned short int my_unsigned_short_int_min = 0;
  const unsigned short int my_unsigned_short_int_max = USHRT_MAX;

  assert_eq(0 == my_unsigned_short_int_min);
  assert_eq(0, my_unsigned_short_int_min);
  assert_eq(0, my_unsigned_short_int_min, "Text");

  assert_eq(USHRT_MAX == my_unsigned_short_int_max);
  assert_eq(USHRT_MAX, my_unsigned_short_int_max);
  assert_eq(USHRT_MAX, my_unsigned_short_int_max, "Text");
}

test(cutest_shall_compare_int_values_correctly)
{
  const int my_int_min = INT_MIN;
  const int my_int_max = INT_MAX;

  assert_eq(INT_MIN ==  my_int_min);
  assert_eq(INT_MIN, my_int_min);
  assert_eq(INT_MIN, my_int_min, "Text");

  assert_eq(INT_MAX ==  my_int_max);
  assert_eq(INT_MAX, my_int_max);
  assert_eq(INT_MAX, my_int_max, "Text");
}

test(cutest_shall_compare_unsigned_int_values_correctlu)
{
  const unsigned int my_unsigned_int_min = 0;
  const unsigned int my_unsigned_int_max = UINT_MAX;

  assert_eq(0 == my_unsigned_int_min);
  assert_eq(0, my_unsigned_int_min);
  assert_eq(0, my_unsigned_int_min, "Text");

  assert_eq(UINT_MAX == my_unsigned_int_max);
  assert_eq(UINT_MAX, my_unsigned_int_max);
  assert_eq(UINT_MAX, my_unsigned_int_max, "Text");
}

test(cutest_shall_compare_long_int_values_correctly)
{
  const long int my_long_int_min = LONG_MIN;
  const long int my_long_int_max = LONG_MAX;

  assert_eq(LONG_MIN == my_long_int_min);
  assert_eq(LONG_MIN, my_long_int_min);
  assert_eq(LONG_MIN, my_long_int_min, "Text");

  assert_eq(LONG_MAX == my_long_int_max);
  assert_eq(LONG_MAX, my_long_int_max);
  assert_eq(LONG_MAX, my_long_int_max, "Text");
}

test(cutest_shall_compare_unsigned_long_int_values_correctly)
{
  const unsigned long int my_unsigned_long_int_min = 0;
  const unsigned long int my_unsigned_long_int_max = ULONG_MAX;

  assert_eq(0 == my_unsigned_long_int_min);
  assert_eq(0, my_unsigned_long_int_min);
  assert_eq(0, my_unsigned_long_int_min, "Text");

  assert_eq(ULONG_MAX == my_unsigned_long_int_max);
  assert_eq(ULONG_MAX, my_unsigned_long_int_max);
  assert_eq(ULONG_MAX, my_unsigned_long_int_max, "Text");
}

test(cutst_shall_cmpare_long_long_int_values_correctly)
{
  const long long int my_long_long_int_min = LLONG_MIN;
  const long long int my_long_long_int_max = LLONG_MAX;

  assert_eq(LLONG_MIN == my_long_long_int_min);
  assert_eq(LLONG_MIN, my_long_long_int_min);
  assert_eq(LLONG_MIN, my_long_long_int_min, "Text");

  assert_eq(LLONG_MAX == my_long_long_int_max);
  assert_eq(LLONG_MAX, my_long_long_int_max);
  assert_eq(LLONG_MAX, my_long_long_int_max, "Text");
}

test(cutest_shall_compare_unsigned_long_long_int_values_correctly)
{
  const unsigned long long int my_unsigned_long_long_int_min = 0;
  const unsigned long long int my_unsigned_long_long_int_max = ULLONG_MAX;

  assert_eq(0 == my_unsigned_long_long_int_min);
  assert_eq(0, my_unsigned_long_long_int_min);
  assert_eq(0, my_unsigned_long_long_int_min, "Text");

  assert_eq(ULLONG_MAX == my_unsigned_long_long_int_max);
  assert_eq(ULLONG_MAX, my_unsigned_long_long_int_max);
  assert_eq(ULLONG_MAX, my_unsigned_long_long_int_max, "Text");
}


test(cutest_shall_compare_float_values_correctly)
{
  const float my_float_min = FLT_MIN;
  const float my_float_max = FLT_MAX;

  assert_eq(float_compare(FLT_MIN, my_float_min));
  assert_eq(FLT_MIN, my_float_min);
  assert_eq(FLT_MIN, my_float_min, "Text");

  assert_eq(float_compare(FLT_MAX, my_float_max));
  assert_eq(FLT_MAX, my_float_max);
  assert_eq(FLT_MAX, my_float_max, "Text");
}

test(cutest_shall_compare_double_values_correctly)
{
  const double my_double_min = DBL_MIN;
  const double my_double_max = DBL_MAX;

  assert_eq(double_compare(DBL_MIN, my_double_min));
  assert_eq(DBL_MIN, my_double_min);
  assert_eq(DBL_MIN, my_double_min, "Text");

  assert_eq(double_compare(DBL_MAX, my_double_max));
  assert_eq(DBL_MAX, my_double_max);
  assert_eq(DBL_MAX, my_double_max, "Text");
}

test(cutest_shall_compare_long_double_values_correctly)
{
  const long double my_long_double_min = LDBL_MIN;
  const long double my_long_double_max = LDBL_MAX;

  assert_eq(long_double_compare(LDBL_MIN, my_long_double_min));
  assert_eq(LDBL_MIN, my_long_double_min);
  assert_eq(LDBL_MIN, my_long_double_min, "Text");

  assert_eq(long_double_compare(LDBL_MAX, my_long_double_max));
  assert_eq(LDBL_MAX, my_long_double_max);
  assert_eq(LDBL_MAX, my_long_double_max, "Text");
}

test(cutest_shall_compare_strings_correctly)
{
  const char* text = "My text";
  assert_eq("My text", text);
}

test(cutest_shall_compare_own_data_types_correctly)
{
  my_own_type_t values = {1, "234", 5};
  my_own_type_t gold = {1, "234", 5};
  assert_eq(gold, values);
}
