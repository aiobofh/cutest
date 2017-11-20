#include <stdio.h>
#include <stdlib.h>

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
