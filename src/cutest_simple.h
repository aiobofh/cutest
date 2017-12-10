#ifndef _CUTEST_SIMPLE_H_
#define _CUTEST_SIMPLE_H_

#define assert_eq_3(EXP, REF, STR)                                 \
  if ((EXP) != (REF)) {                                            \
    char error_output_buf[1024];                                   \
    sprintf(error_output_buf,                                      \
            " %s:%d assert_eq(" #EXP ", " #REF ", " STR ") "       \
            "failed\n",                                            \
            __FILE__, __LINE__);                                   \
    cutest_increment_fails(error_output_buf);                      \
  }

#define assert_eq_2(EXP, REF)                                      \
  if ((EXP) != (REF)) {                                            \
    char error_output_buf[1024];                                   \
    sprintf(error_output_buf,                                      \
            " %s:%d assert_eq(" #EXP ", " #REF ") "                \
            "failed\n",                                            \
            __FILE__, __LINE__);                                   \
    cutest_increment_fails(error_output_buf);                      \
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

#define assert_eq_select(X, A1, A2, A3, MACRO, ...) MACRO

#define assert_eq_chooser(...)                  \
  assert_eq_select(,##__VA_ARGS__,              \
                   assert_eq_3,                 \
                   assert_eq_2,                 \
                   assert_eq_1,)

#define assert_eq(...) assert_eq_chooser(__VA_ARGS__)(__VA_ARGS__)

#endif
