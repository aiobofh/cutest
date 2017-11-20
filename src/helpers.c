#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"

int file_exists(const char *filename)
{
  FILE *fd = fopen(filename, "r");
  if (NULL == fd) {
    fprintf(stderr, "ERROR: File '%s' does not exist\n", filename);
    return 0;
  }
  fclose(fd);
  return 1;
}

size_t count_white_spaces(const char* buf) {
  size_t pos = 0;
  const size_t len = strlen(buf);
  while (((buf[pos] == ' ') || (buf[pos] == '\t')) && (pos < len)) {
    pos++;
  }
  return pos;
}
