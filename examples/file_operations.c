#include <assert.h>
#include <stdio.h>

int write_file(const char* filename)
{
  int i = 10;
  int retval = 0;
  FILE* file = fopen(filename, "w");

  if (NULL == file) { /* Something went wrong opening the file */
    return 1;
  }

  while (i-- > 0) { /* Write ten simple lines of text to the file */
    if (0 == fputs("A text row\n", file)) {
      retval = 2;
      break;
    }
  }

  if (0 != fclose(file)) { /* Make sure it's possible to close the file */
    retval = 3;
  }

  return retval;
}
