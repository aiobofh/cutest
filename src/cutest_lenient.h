#ifndef _CUTEST_LENIENT_H_
#define _CUTEST_LENIENT_H_

#pragma GCC diagnostic ignored "-Wint-conversion"

#ifdef CUTEST_CLANG
/* Since __VA_ARGS__ are used in a GNU:ish way */
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

#ifndef CUTEST_MY_OWN_EQ_COMPARATORS
#define CUTEST_MY_OWN_EQ_COMPARATORS(EXP, REF)
#endif

int cutest_assert_eq_char(const char a, const char b, char* output);
int cutest_assert_eq_str(const char* a, const char *b, char* output);
int cutest_compare_float(float f1, float f2);
int cutest_compare_double(double d1, double d2);
int cutest_compare_long_double(long double ld1, long double ld2);
int cutest_assert_eq_float(const float a, const float b, char* output);
int cutest_assert_eq_double(const double a, const double b, char* output);
int cutest_assert_eq_long_double(const long double a, const long double b,
                                 char* output);
int cutest_assert_eq_default(const unsigned long long a,
                             const unsigned long long b,
                             char* output);

#define assert_eq_comp(EXP, REF)                             \
  _Generic((EXP),                                            \
           float: cutest_assert_eq_float,                    \
           const float: cutest_assert_eq_float,              \
           double: cutest_assert_eq_double,                  \
           const double: cutest_assert_eq_double,            \
           long double: cutest_assert_eq_long_double,        \
           const long double: cutest_assert_eq_long_double,  \
           char: cutest_assert_eq_char,                      \
           const char: cutest_assert_eq_char,                \
           char*: cutest_assert_eq_str,                      \
           const char*: cutest_assert_eq_str,                \
           CUTEST_MY_OWN_EQ_COMPARATORS(EXP, REF)            \
  default: cutest_assert_eq_default)

#define assert_eq_3(EXP, REF, STR)                                      \
  {                                                                     \
    char cutest_o[1024];                                                \
    cutest_o[0] = 0;                                                    \
    const int cutest_assert_retval =                                    \
      assert_eq_comp((EXP), (REF))((EXP), (REF), cutest_o);             \
    if (0 != cutest_assert_retval) {                                    \
      char error_output_buf[1024];                                      \
      sprintf(error_output_buf,                                         \
              " %s:%d assert_eq(%s, " STR ") failed\n",                 \
              __FILE__,                                                 \
              __LINE__,                                                 \
              cutest_o);                                                \
      cutest_increment_fails(error_output_buf);                         \
    }                                                                   \
  }

#define assert_eq_2(EXP, REF)                                           \
  {                                                                     \
    char cutest_o[1024];                                                \
    cutest_o[0] = 0;                                                    \
    const int cutest_assert_retval =                                    \
      assert_eq_comp((EXP), (REF))((EXP), (REF), cutest_o);             \
    if (0 != cutest_assert_retval) {                                    \
      char error_output_buf[1024];                                      \
      sprintf(error_output_buf,                                         \
              " %s:%d assert_eq(%s) failed\n",                          \
              __FILE__,                                                 \
              __LINE__,                                                 \
              cutest_o);                                                \
      cutest_increment_fails(error_output_buf);                         \
    }                                                                   \
  }

#define assert_eq_1(EXP)                                           \
  if (!(EXP)) {                                                    \
    char error_output_buf[1024];                                   \
    sprintf(error_output_buf,                                      \
            " %s:%d assert_eq(" #EXP ") "                          \
            "failed\n",                                            \
            __FILE__, __LINE__);                                   \
    cutest_increment_fails(error_output_buf);                      \
  }

#endif
