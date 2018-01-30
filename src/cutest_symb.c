#include <stdio.h>

#include "helpers.h"

typedef enum file_type_e {
  ELF32,
  ELF64,
  UNSUPPORTED
} file_type_t;

static void usage(const char* program_name)
{
  printf("USAGE: %s <object-file>\n"
         "\n"
         "This tool prints all symbols (function names) used in the\n"
         "specified object file\n"
         "\n",
         program_name);
}

static file_type_t get_file_type(const char* file_name) {
  FILE* fd = fopen(file_name, "r");
  unsigned char magic_number[4] = {0, 0, 0, 0};
  unsigned char bits[0];
  char* n = &magic_number[0];
  file_type_t retval = UNSUPPORTED;
  fread(magic_number, 5, 1, fd);

  if ((0x74 == n[0]) && ('E' == n[1]) && ('L' == n[2]) && ('F' == n[3])) {
    fread(bits, 1, 1, fd);
    if (1 == bits[0]) {
      retval = ELF32;
    }
    else if (2 == bits[0]) {
      retval = ELF64;
    }
    else {
      fprintf(stderr, "ERROR: Invalid ELF format\n");
    }
  }
  fclose(fd);
  return retval;
}

static void print_elf_symbols(const char* file_name, unsigned char bits)
{
  FILE* fd = fopen(file_name, "r");
  size_t section_header_offset = 0;
  size_t section_header_ent_size = 0;
  size_t section_header_cnt = 0;
  if (32 == bits) {
    unsigned char b[4];
    fseek(fd, 0x20, SEEK_SET);
    fread(b, 4, 1, fd);
    section_header_offset = (b[3] << 24) + (b[2] << 16) + (b[1] << 8) + b[0];
    fseek(fd, 0x2e, SEEK_SET);
    fread(b, 2, 1, fd);
    section_header_ent_size = (b[1] << 8) + b[0];
    fread(b, 2, 1, fd);
    section_header_cnt = (b[1] << 8) + b[0];
  }
  else if (64 == bits) {
    unsigned char b[8];
    fseek(fd, 0x28, SEEK_SET);
    fread(b, 8, 1, fd);
    section_header_offset = ((b[7] << 56) + (b[6] << 48) + (b[5] << 40) + (b[4] << 32) +
                             (b[3] << 24) + (b[2] << 16) + (b[1] << 8) + b[0]);
    fseek(fd, 0x3a, SEEK_SET);
    fread(b, 2, 1, fd);
    section_header_ent_size = (b[1] << 8) + b[0];
    fread(b, 2, 1, fd);
    section_header_cnt = (b[1] << 8) + b[0];
  }
}

int main(int argc, char* argv[])
{
  const char* program_name = argv[0];
  char* file_name = NULL;

  if (argc != 1) {
    fprintf(stderr, "Required argument missing\n");
    usage(program_name);
  }

  file_name = argv[1];

  if (!file_exists(file_name)) {
    fprintf(stderr, "ERROR: File \"%s\" not found.\n", file_name);
    return EXIT_FAILURE;
  }

  switch (get_file_type(file_name)) {
  case ELF32:
    print_elf_symbols(file_name, 32);
    break;
  case ELF64:
    print_elf_symbols(file_name, 64);
    break;
  default:
    fprintf(stderr, "ERROR: Unrecognized object file \"%s\".");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
