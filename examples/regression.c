/*
 * This is a regression test implementation for CUTest, to make sure that
 * all the scenarios thinkable when it comes to auto-mocking is supported
 * as good as possible.
 */

int return_int() {
  return 1234;
}

int use_return_int() {
  return return_int();
}

int* return_int_ptr() {
  return (int*)1234;
}

int* use_return_int_ptr() {
  return return_int_ptr();
}

struct my_struct {
  int value;
};

struct my_struct* return_my_struct_ptr() {
  return (struct my_struct*)1234;
}

struct my_struct* use_return_my_struct_ptr() {
  return return_my_struct_ptr();
}

const char* return_const_char_ptr() {
  return "1234";
}

const char* use_return_const_char_ptr() {
  return return_const_char_ptr();
}

static inline struct my_struct* return_cumbersome_my_struct_ptr() {
  return (struct my_struct*)1234;
}

struct my_struct* use_return_cumbersome_my_struct_ptr() {
  return return_cumbersome_my_struct_ptr();
}

int arg_int(int i, int j) {
  (void)j;
  return i;
}

int use_arg_int(int i, int j) {
  return arg_int(i, j);
}

int arg_const_int(const int i, const int j) {
  (void)j;
  return i;
}

int use_arg_const_int(const int i, const int j) {
  return arg_const_int(i, j);
}

int arg_char_ptr(int i, char* str) {
  (void)str;
  return i;
}

int use_arg_char_ptr(int i, char* str) {
  return arg_char_ptr(i, str);
}

int arg_array(int i, int j[20]) {
  (void)j;
  return i;
}

int use_arg_array(int i, int j[20]) {
  return arg_array(i, j);
}

int arg_function_pointer(int i, int (*func)(int, int)) {
  (void)i;
  return func(1234, 5678);
}

int use_arg_function_pointer(int i, int (*func)(int, int)) {
  return arg_function_pointer(i, func);
}

int arg_array_ptr(int i, int *j[]) {
  (void)j;
  return i;
}

int use_arg_array_ptr(int i, int *j[]) {
  return arg_array_ptr(i, j);
}

int arg_variadic(int i, ...) {
  return i;
}
