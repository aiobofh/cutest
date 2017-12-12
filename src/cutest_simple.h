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

#endif
