/*********************************************************************
   ---    ____ ____ _____ ____ ____ _____   ____   ____ ____ ____ ---
   ---   / __// / //_  _// __// __//_  _/  /    \ /   // / // __/ ---
   ---  / /_ / / /  / / / __//_  /  / /   / / / // / //  <'/ __/  ---
   --- /___//___/  /_/ /___//___/  /_/   /_/_/_//_/_//_/_//___/   ---
   ---                                                            ---
 *
 * CUTest make build system
 * ========================
 *
 * This is a tool that is written in pure C and should be fairly easy
 * to compile to be able to build/rebuild the cutest tools and test
 * suites as fast as possible and as portable as possible. Earlier
 * versions of CUTest rely heavily on GNU Make as build system,
 * however this has proven to be a bad decision when porting CUTest
 * to other platforms than Linux or other Posix:es.
 *
 * Requirements
 * ------------
 *
 * To be able to compile this static build system you will need a
 * fairly compliant C compiler, namely the compiler you use for.
 * your software project. Make sure to change directory to your
 * cutest-folder for your project. In most cases (or rather the
 * way the GNU Make system provided by earlier versions of cutest
 * you would just type ``make cutest`` to get a copy of it and it
 * would compile the required tools) you should clone the cutest
 * repository to you test-folder (but not add it to your own
 * repository).
 *
 * So, you will need Git installed in your system, and also you
 * will need a C-compiler in your path.
 *
 * Example::
 *
 *  $ gcc -O2 -flto cutest_make.c -o cutest_make
 *
 * Usage
 * -----
 *
 * From your folder containing your tests just invoke this command
 * by a relative path::
 *
 *  $ cutest/cutet_make
 *
 * And all your test-suites (and the other cutest tools) will be built
 * with as good performance as one could wish for using your compiler
 * of choice (hopefully).
 *
 * When compiling cutest_make with the example above it will assume
 * that you are using a flat directory structure, with your test
 * code in the same folder as your source code and a cloned or
 * un-packed cutest-folder inside that directory.
 *
 * But if you can compiler cutest_make to support basically any
 * directory structure. As long as one test-suite correlates to
 * one source file in your product.
 *
 * Flat directory structure
 * ------------------------
 * ::
 *
 *  some/path/your_project              <- product code and tests
 *                    |
 *                    '- cutest         <- git clone/unpacked cutest
 *
 *  $ gcc cutest_make.c -o cutest_make
 *
 * Test-folder inside your code folder
 * -----------------------------------
 * ::
 *
 *  some/path/your_project
 *                    |
 *                    '- src            <- product code
 *                        |
 *                        '- test       <- tests
 *                            |
 *                            '- cutest <- cit clone/unpacked cutest
 *
 *  $ gcc cutest_make.c -o cutest_make -D"CUTEST_SRC_PATH=../"
 *
 * Separate test and source folders
 * --------------------------------
 * ::
 *
 *  some/path/your_project
 *                    |
 *                    |- src            <- product code
 *                    |
 *                    '- test           <- tests
 *                        |
 *                        '- cutest     <- git clone/unpacked cutest
 *
 *  $ gcc cutest_make.c -o cutest_make -D"CUTEST_SRC_PATH=../src"
 *
 * Supported C compilers
 * ---------------------
 *
 * * GCC (2.7..5.4)
 * * clang (3.8)
 *
 */
/*
#define _BSD_SOURCE
#define _POSIX_C_SOURCE 200112L
*/
#define _DEFAULT_SOURCE

#ifdef AMIGAOS
#else
#include <sys/stat.h>
#include <sys/types.h>
/*
#include <sys/wait.h>
*/
#include <dirent.h>
#include <unistd.h>
/* typedef unsigned long int size_t; */
#endif

#include <stdio.h>
#include <stdlib.h>
/* #include <wchar.h> */
#include <string.h>

#include "cutest_make.h"

#define min(a,b) ((a < b) ? a : b)
#define count(ARRAY) sizeof(ARRAY)/sizeof(artifact_t*)

static void usage(const char* program_name)
{
  printf("USAGE: %s <-v|-d> <target>\n"
         "\n"
         "This tool is a build system for CUTest-related files and test suites\n"
         "to keep CUTest as portable as possible.\n"
         "\n"
         " -v, --verbose   Print commands being executed to stdout.\n"
         " -d, --debug     Output debuging information to stdout.\n"
         " -C, --clean-all Clean all artifacts.\n"
         " -c, --clean     Clean all test-related artifacts.\n"
         "\n",
         program_name);
}

/*
static int get_number_of_cores()
{
#ifdef _SC_NPROCESSORS_ONLN
  return sysconf(_SC_NPROCESSORS_ONLN);
#else
  return 1;
#endif
}
*/

static file_node_t* new_file_node(const char* dirname, char* path)
{
  size_t dirnamelen = 0;
  char* dst = NULL;
  file_node_t* node = malloc(sizeof(*node));

  if (NULL == node) {
    return NULL;
  }

  if (NULL != dirname) {
    dirnamelen = strlen(dirname);
  }

  dst = malloc(dirnamelen + strlen(path) + 1);
  if (NULL == dst) {
    free(node);
    return NULL;
  }
  if (NULL != dirname) {
    strcpy(dst, dirname);
    strcat(dst, path);
  }
  else {
    strcpy(dst, path);
  }
  dst[dirnamelen + strlen(path)] = 0;

  node->file.file_name = dst;
  node->skip = 0;
  node->next = NULL;

  return node;
}

static void delete_file_node(file_node_t* node)
{
  free(node->file.file_name);
  free(node);
}

static void file_list_add_node(file_list_t* list, file_node_t* node)
{
  if (NULL == list->first) {
    list->first = node;
  }
  if (NULL != list->last) {
    list->last->next = node;
  }
  list->last = node;
}

static size_t make_list_of_files(file_list_t* list, char* path,
                                 const char* suffix,
                                 const char* filt1, const char* filt2,
                                 int verbose, const char* reasonstr)
{
#ifdef AMIGAOS
#else
  DIR* dir;
  struct dirent* ent;
  const size_t suffix_len = strlen(suffix);
  const size_t filt1_len = (NULL != filt1) ? strlen(filt1) : 0;
  const size_t filt2_len = (NULL != filt2) ? strlen(filt2) : 0;
#endif

  if (1 < verbose) {
    printf("DEBUG: Searching for %s in '%s'\n", reasonstr, path);
  }

#ifdef AMIGAOS
#else
  dir = opendir((char*)path);

  if (NULL == dir) {
    fprintf(stderr, "ERROR: Could not open directory \"%s\"\n", path);
    return 0;
  }
#endif

#ifdef AMIGAOS
#else
  while (NULL != (ent = readdir(dir))) {
    char* name = ent->d_name;
    const size_t ent_len = strlen(name);
    file_node_t* node;
    if (ent_len < suffix_len) {
      continue;
    }
    if ((0 != filt1_len) && (0 == strcmp(filt1, &name[ent_len - filt1_len]))) {
      continue;
    }
    if ((0 != filt2_len) && (0 == strcmp(filt2, &name[ent_len - filt2_len]))) {
      if (0 != strcmp("cutest_run.c", name)) {
        continue;
      }
    }
    if (0 != strcmp(suffix, &name[ent_len - suffix_len])) {
      continue;
    }

    node = new_file_node(path, name);
    if (NULL == node) {
      fprintf(stderr, "ERROR: Out of memory while listing files\n");
      return 0;
    }
    file_list_add_node(list, node);
  }
#endif
  return 0;
}

static void free_files(file_list_t* list)
{
  file_node_t* node = list->first;

  while (NULL != node) {
    file_node_t* next = node->next;
    delete_file_node(node);
    node = next;
  }
}

static void strreplace(char* dst, const char* src, const char* replace,
                       const char* with)
{
  char *pos = strstr(src, replace);

  if (NULL == pos) {
    return;
  }
  memcpy(dst, src, pos - src);
  dst[pos - src] = 0;
  strcat(dst, with);
}

static char* gen_filename(const char* from,
                          const char* newpath,
                          const char* replace,
                          const char* with)
{
  char* oldpath = NULL;
  size_t filenamepos = 0;
  size_t oldpathlen = strlen(from);
  char* n = malloc(strlen(from) -
                   strlen(replace) +
                   strlen(with) +
                   1);

  strreplace(n, from, replace, with);

  while (oldpathlen > 0) {
    if (OS_PATH_SEPARATOR[0] == from[oldpathlen]) {
      oldpath = malloc(oldpathlen + 2);
      memcpy(oldpath, from, oldpathlen);
      oldpath[oldpathlen] = 0;
      filenamepos = oldpathlen + 1;
      break;
    }
    oldpathlen--;
  }

  if ((NULL != oldpath) && (NULL != newpath)) {
    const size_t newpathlen = strlen(newpath);
    char* d = malloc(newpathlen + strlen(&n[filenamepos]) + 1);
    strcpy(d, newpath);
    if (OS_PATH_SEPARATOR[0] != newpath[newpathlen - 1]) {
      strcat(d, OS_PATH_SEPARATOR);
    }
    strcat(d, &n[filenamepos]);
    free(n);
    n = d;
  }

  return n;
}

static file_node_t* find_file_node_in_file_list(file_list_t* list,
                                                const char* name,
                                                int verbose)
{
  file_node_t* node;

  for (node = list->first; NULL != node; node = node->next) {
    if (1 < verbose) {
      printf("DEBUG: Matching '%s' and '%s'.\n", node->file.file_name, name);
    }
    if (0 == strcmp(name, node->file.file_name)) {
      return node;
    }
  }

  return NULL;
}

static void print_missing(file_list_t* list1, file_list_t* list2,
                          const char* ext1, const char* ext2, const char* name)
{
  file_node_t* first = list1->first;
  file_node_t* node;

  for (node = first; NULL != node; node = node->next) {
    const char* name = node->file.file_name;
    char* expected = malloc(strlen(name) - strlen(ext1) + strlen(ext2) + 1);

    if (NULL == expected) {
      fprintf(stderr, "ERROR: Out of memory allocating expeced %s filename.\n", name);
      return;
    }

    strreplace(expected, node->file.file_name, ext1, ext2);

    if (NULL == find_file_node_in_file_list(list2, expected, 0)) {
      fprintf(stderr, "ERROR: Missing test '%s' for source '%s'\n",
              expected, name);
      node->skip = 1;
    }

    free(expected);
  }
}

static void print_missing_tests(file_list_t* source_list,
                                file_list_t* test_source_list)
{
  print_missing(source_list, test_source_list, ".c", "_test.c", "test");
}

static void print_missing_sources(file_list_t* source_list,
                                  file_list_t* test_source_list)
{
  print_missing(test_source_list, source_list, "_test.c", ".c", "source");
}

static int file_exists(const char* filename)
{
  FILE *fd = fopen(filename, "r");

  if (NULL == fd) {
    return 0;
  }
  fclose(fd);

  return 1;
}

#ifdef AMIGAOS
#define should_rebuild(TARGET, DEPS, VERBOSE) 1
static int _should_rebuild(artifact_t* target, size_t num,
                           const artifact_t** dep, int verbose)
{
  return 1;
}
#else
#define should_rebuild(TARGET, DEPS, VERBOSE)                           \
  _should_rebuild((artifact_t*)TARGET, count(DEPS), DEPS, VERBOSE)

static int _should_rebuild(artifact_t* target, size_t num,
                           const artifact_t** dep, int verbose)
{
  struct stat target_stats;
  size_t i;

  if (!file_exists(target->str)) {
    if (verbose >=2) printf("DEBUG: Could not find %s, need to rebuild\n", target->str);
    return 1;
  }

  stat(target->str, &target_stats);

  for (i = 0; i < num; i++) {
    struct stat dep_stats;

    stat(dep[i]->str, &dep_stats);
    /*
    if ((dep_stats.st_mtime.tv_sec > target_stats.st_mtime.tv_sec) ||
        ((dep_stats.st_mtime.tv_sec == target_stats.st_mtime.tv_sec) &&
         (dep_stats.st_mtim.etv_nsec > target_stats.st_mtime.tv_nsec))) {
    */
    if (dep_stats.st_mtime > target_stats.st_mtime) {
      if (verbose >= 2) printf("DEBUG: %s changed, need to rebuild %s\n", dep[i]->str, target->str);
      return 1;
    }
  }

  if (verbose >= 2) printf("DEBUG: %s is up-to-date.\n", target->str);

  return 0;
}
#endif

static int make_mockables_lst(mockables_lst_t* mockables_lst,
                              mockables_o_t* mockables_o, int verbose)
{
  int retval = 0;
  const char* fmt =
    "nm %s | sed 's/.* //g' | grep -v '__stack_' | sort -u > %s && "
    "grep 'gcc2_compiled.' %s > /dev/null && sed -i 's/^_//g' %s || true";
  char* command = NULL;
  artifact_t* deps[1];

  deps[0] = (artifact_t*)mockables_o;

  if (0 == should_rebuild(mockables_lst, (const artifact_t**)deps, verbose)) {
    return 0;
  }

  /* GCC */
  command = malloc(strlen(fmt) - strlen("%s") * 4 +
                   strlen(mockables_o->str) +
                   strlen(mockables_lst->str) +
                   strlen(mockables_lst->str) +
                   strlen(mockables_lst->str) +
                   1);

  sprintf(command, fmt,
          mockables_o->str,
          mockables_lst->str,
          mockables_lst->str,
          mockables_lst->str);

  if (verbose >= 1) printf("%s\n", command);

  retval = system(command);

  free(command);

  return retval;
}

static int make_mockables_o(mockables_o_t* mockables_o, c_t* c, int verbose)
{
  /* GCC */
  int retval = 0;
#ifdef AMIGAOS
  const char* fmt =
    "vc %s -g -O0 -o %s -c %s -D\"static=\" -D\"inline=\" -I\"" CUTEST_INC_PATH "\"";
#else
  const char* fmt =
    "$CC %s -fno-inline -g -O0 -o %s -c %s -D\"static=\" -D\"inline=\" -I\"" CUTEST_INC_PATH "\"";
#endif
  char* command = NULL;
  artifact_t* deps[1];

  deps[0] = (artifact_t*)c;

  if (0 == should_rebuild(mockables_o, (const artifact_t**)deps, verbose)) {
    return 0;
  }

  command = malloc(strlen(fmt) - strlen("%s") * 3 +
                   strlen(CUTEST_CC_VISIBILITY_HIDDEN) +
                   strlen(mockables_o->str) +
                   strlen(c->str) +
                   1);

  sprintf(command, fmt,
          CUTEST_CC_VISIBILITY_HIDDEN,
          mockables_o->str,
          c->str);

  if (verbose >= 1) printf("%s\n", command);

  retval = system(command);

  free(command);

  return retval;
}

static int make_mockables_s(mockables_s_t* mockables_s, c_t* c, int verbose)
{
  /* GCC */
  int retval = 0;
#ifdef AMIGAOS
  const char* fmt =
    "vc -S %s -g -O0 -o %s -c %s -D\"static=\" -D\"inline=\" -D\"main=MAIN\" -I\"" CUTEST_INC_PATH "\"";
#else
  const char* fmt =
    "$CC -S -fverbose-asm %s -fno-inline -g -O0 -o %s -c %s -D\"static=\" -D\"inline=\" -D\"main=MAIN\" -I\"" CUTEST_INC_PATH "\"";
#endif
  char* command = NULL;

  artifact_t* deps[1];

  deps[0] = (artifact_t*)c;

  if (0 == should_rebuild(mockables_s, (const artifact_t**)deps, verbose)) {
    return 0;
  }

  command = malloc(strlen(fmt) - strlen("%s") * 3 +
                   strlen(CUTEST_CC_VISIBILITY_HIDDEN) +
                   strlen(mockables_s->str) +
                   strlen(c->str) +
                   1);

  sprintf(command, fmt,
          CUTEST_CC_VISIBILITY_HIDDEN,
          mockables_s->str,
          c->str);

  if (verbose >= 1) printf("%s\n", command);

  retval = system(command);

  free(command);

  return retval;
}

static int make_proxified_s(proxified_s_t* proxified_s,
                            mockables_s_t* mockables_s,
                            mockables_lst_t* mockables_lst,
                            cutest_prox_t* cutest_prox,
                            int verbose)
{
  int retval = 0;
  const char* fmt = "%s %s %s > %s";
  char* command = NULL;

  artifact_t* deps[3];

  deps[0] = (artifact_t*)mockables_s;
  deps[1] = (artifact_t*)mockables_lst;
  deps[2] = (artifact_t*)cutest_prox;

  if (0 == should_rebuild(proxified_s, (const artifact_t**)deps, verbose)) {
    return 0;
  }

  command = malloc(strlen(fmt) - strlen("%s") * 4 +
                   strlen(cutest_prox->str) +
                   strlen(mockables_s->str) +
                   strlen(mockables_lst->str) +
                   strlen(proxified_s->str) +
                   1);

  sprintf(command, fmt,
          cutest_prox->str,
          mockables_s->str,
          mockables_lst->str,
          proxified_s->str);

  if (verbose >= 1) printf("%s\n", command);

  retval = system(command);

  free(command);

  return retval;
}

static int make_mocks_h(mocks_h_t* mocks_h,
                        c_t* c,
                        mockables_lst_t* mockables_lst,
                        cutest_mock_t* cutest_mock,
                        cproto_t *cproto,
                        int verbose)
{
  int retval = 0;
  const char* fmt = "%s %s %s %s %s > %s";
  char* command;

  artifact_t* deps[4];

  deps[0] = (artifact_t*)c;
  deps[1] = (artifact_t*)mockables_lst;
  deps[2] = (artifact_t*)cutest_mock;
  deps[3] = (artifact_t*)cproto;

  if (0 == should_rebuild(mocks_h, (const artifact_t**)deps, verbose)) {
    return 0;
  }

  command = malloc(strlen(fmt) - strlen("%s") * 6 +
                   strlen(cutest_mock->str) +
                   strlen(cproto->str) +
                   strlen(c->str) +
                   strlen(mockables_lst->str) +
                   strlen(CUTEST_INC_PATH) +
                   strlen(mocks_h->str) +
                   1);

  sprintf(command, fmt,
          cutest_mock->str,
          cproto->str,
          c->str,
          mockables_lst->str,
          CUTEST_INC_PATH,
          mocks_h->str);

  if (verbose >= 1) printf("%s\n", command);

  retval = system(command);

  free(command);

  return retval;
}

static int make_test_run_c(test_run_c_t* test_run_c,
                           test_c_t* test_c,
                           mocks_h_t* mocks_h,
                           cutest_run_t *cutest_run,
                           int verbose)
{
  int retval = 0;
  const char* fmt = "%s %s %s > %s";
  char* command;

  artifact_t* deps[3];

  deps[0] = (artifact_t*)test_c;
  deps[1] = (artifact_t*)mocks_h;
  deps[2] = (artifact_t*)cutest_run;

  if (0 == should_rebuild(test_run_c, (const artifact_t**)deps, verbose)) {
    return 0;
  }

  command = malloc(strlen(fmt) - strlen("%s") * 4 +
                   strlen(cutest_run->str) +
                   strlen(test_c->str) +
                   strlen(mocks_h->str) +
                   strlen(test_run_c->str) +
                   1);

  sprintf(command, fmt,
          cutest_run->str,
          test_c->str,
          mocks_h->str,
          test_run_c->str);

  if (verbose >= 1) printf("%s\n", command);

  retval = system(command);

  free(command);

  return retval;
}

static int make_test(test_t* test,
                     proxified_s_t* proxified_s,
                     test_run_c_t* test_run_c,
                     cutest_impl_o_t *cutest_impl_o,
                     file_list_t* dep_list,
                     int verbose)
{
  int retval = 0;
#ifdef AMIGAOS
  const char* fmt = "vc %s%s %s %s -o %s -DNDEBUG -D\"inline=\" -D\"CUTEST_GCC\" -D\"CUTEST_LENIENT_ASSERTS\" -I\"" CUTEST_INC_PATH "\"";
#else
  const char* fmt = "$CC %s%s %s %s -o %s -DNDEBUG -D\"inline=\" -D\"CUTEST_GCC\" -D\"CUTEST_LENIENT_ASSERTS\" -I\"" CUTEST_INC_PATH "\"";
#endif
  char* command;
  file_node_t* file;
  size_t dep_len = 0;
  char* extra_deps;
  artifact_t* deps[3];

  for (file = dep_list->first; NULL != file; file = file->next) {
    dep_len += strlen(file->file.file_name);
    dep_len += strlen(" ");
  }

  extra_deps = malloc(dep_len + 1);
  memset(extra_deps, 0, dep_len + 1);

  for (file = dep_list->first; NULL != file; file = file->next) {
    strcat(extra_deps, file->file.file_name);
    strcat(extra_deps, " ");
  }

  deps[0] = (artifact_t*)proxified_s;
  deps[1] = (artifact_t*)test_run_c;
  deps[2] = (artifact_t*)cutest_impl_o;

  if (0 == should_rebuild(test, (const artifact_t**)deps, verbose)) {
    return 0;
  }

  command = malloc(strlen(fmt) - strlen("%s") * 5 +
                   dep_len +
                   strlen(proxified_s->str) +
                   strlen(test_run_c->str) +
                   strlen(cutest_impl_o->str) +
                   strlen(test->str) +
                   1);

  sprintf(command, fmt,
          extra_deps,
          proxified_s->str,
          test_run_c->str,
          cutest_impl_o->str,
          test->str);

  if (verbose >= 1) printf("%s\n", command);

  retval = system(command);

  free(command);

  return retval;
}

#define make_o_from_c(O, C, VERBOSE)                            \
  _make_o_from_c((artifact_t*)O, (artifact_t*)C, VERBOSE)

static int _make_o_from_c(artifact_t* o, artifact_t* c, int verbose)
{
  int retval = 0;
#ifdef AMIGAOS
  const char* fmt = "vc -c %s -o %s -O2 -D\"CUTEST_GCC\" -D\"CUTEST_LENIENT_ASSERTS\" -I\"" CUTEST_INC_PATH "\"";
#else
  const char* fmt = "$CC -c %s -o %s -O2 -D\"CUTEST_GCC\" -D\"CUTEST_LENIENT_ASSERTS\" -I\"" CUTEST_INC_PATH "\"";
#endif
  char* command = NULL;
  artifact_t* deps[1];

  deps[0] = (artifact_t*)c;

  if (0 == _should_rebuild(o, 1, (const artifact_t**)deps, verbose)) {
    return 0;
  }

  command = malloc(strlen(fmt) - strlen("%s") * 2 +
                   strlen(c->str) +
                   strlen(o->str) +
                   1);

  sprintf(command, fmt,
          c->str,
          o->str);

  if (verbose >= 1) printf("%s\n", command);

  retval = system(command);

  return retval;
}

static int make_helpers_o(helpers_o_t* helpers_o, helpers_c_t* helpers_c, int verbose)
{
  return make_o_from_c(helpers_o, helpers_c, verbose);
}

static int make_mockable_o(mockable_o_t* mockable_o, mockable_c_t* mockable_c,
                           int verbose)
{
  return make_o_from_c(mockable_o, mockable_c, verbose);
}

static int make_arg_o(arg_o_t* arg_o, arg_c_t* arg_c, int verbose)
{
  return make_o_from_c(arg_o, arg_c, verbose);
}

static int make_testcase_o(testcase_o_t* testcase_o,
                           testcase_c_t* testcase_c, int verbose)
{
  return make_o_from_c(testcase_o, testcase_c, verbose);
}

static int make_cutest_impl_o(cutest_impl_o_t* cutest_impl_o,
                              cutest_impl_c_t* cutest_impl_c, int verbose)
{
  return make_o_from_c(cutest_impl_o, cutest_impl_c, verbose);
}

static int make_cutest_prox_o(cutest_prox_o_t* cutest_prox_o,
                              cutest_prox_c_t* cutest_prox_c,
                              int verbose)
{
  return make_o_from_c(cutest_prox_o, cutest_prox_c, verbose);
}

#define make_exe_from_o(EXE, OBJS, VERBOSE)                             \
  _make_exe_from_o((artifact_t*)(EXE), count(OBJS), OBJS, VERBOSE)

static int _make_exe_from_o(artifact_t* exe, size_t num,
                            const artifact_t** obj, int verbose)
{
  size_t obj_len = 0;
  size_t i = 0;
  int retval = 0;
  char *command = NULL;
  int command_len = 0;

  if (0 == _should_rebuild(exe, num, obj, verbose)) {
    return 0;
  }

  for (i = 0; i < num; i++) {
    obj_len += strlen(obj[i]->str);
  }

#ifdef AMIGAOS
  command_len = (strlen("vc ") +
                 strlen(CUTEST_CC_LTO) +
                 strlen(" ") +
                 obj_len +
                 num +
                 strlen("-o ") +
                 strlen(exe->str) +
                 1);
#else
  command_len = (strlen("$CC ") +
                 strlen(CUTEST_CC_LTO) +
                 strlen(" ") +
                 obj_len +
                 num +
                 strlen("-o ") +
                 strlen(exe->str) +
                 1);
#endif
  command = malloc(command_len);

  memset(command, 0, command_len);
#ifdef AMIGAOS
  strcpy(command, "vc ");
#else
  strcpy(command, "$CC ");
#endif
  strcat(command, CUTEST_CC_LTO);
  strcat(command, " ");
  for (i = 0; i < num; i++) {
    strcat(command, obj[i]->str);
    strcat(command, " ");
  }
  strcat(command, "-o ");
  strcat(command, exe->str);

  if (verbose >= 1) printf("%s\n", command);

  retval = system(command);

  free(command);

  if (verbose >= 2) {
    if (retval != 0) {
      printf("DEBUG: Retval %d\n", retval);
    }
    else {
      printf("DEBUG: OK\n");
    }
  }

  return retval;
}

static int make_cutest_prox(cutest_prox_t* cutest_prox,
                            cutest_prox_o_t* cutest_prox_o,
                            helpers_o_t* helpers_o,
                            int verbose)
{
  artifact_t* objs[2];

  objs[0] = (artifact_t*)cutest_prox_o;
  objs[1] = (artifact_t*)helpers_o;

  return make_exe_from_o(cutest_prox, (const artifact_t**)objs, verbose);
}

static int make_cutest_mock_o(cutest_mock_o_t* cutest_mock_o,
                              cutest_mock_c_t* cutest_mock_c,
                              int verbose)
{
  return make_o_from_c(cutest_mock_o, cutest_mock_c, verbose);
}

static int make_cutest_mock(cutest_mock_t* cutest_mock,
                            cutest_mock_o_t* cutest_mock_o,
                            helpers_o_t* helpers_o,
                            mockable_o_t* mockable_o,
                            arg_o_t* arg_o,
                            int verbose)
{
  artifact_t* objs[4];

  objs[0] = (artifact_t*)cutest_mock_o;
  objs[1] = (artifact_t*)helpers_o;
  objs[2] = (artifact_t*)mockable_o;
  objs[3] = (artifact_t*)arg_o;

  return make_exe_from_o(cutest_mock, (const artifact_t**)objs, verbose);
}

static int make_cutest_run_o(cutest_run_o_t* cutest_run_o,
                             cutest_run_c_t* cutest_run_c,
                             int verbose)
{
  return make_o_from_c(cutest_run_o, cutest_run_c, verbose);
}

static int make_cutest_run(cutest_run_t* cutest_run,
                           cutest_run_o_t* cutest_run_o,
                           helpers_o_t* helpers_o,
                           testcase_o_t* testcase_o,
                           int verbose)
{
  artifact_t* objs[3];

  objs[0] = (artifact_t*)cutest_run_o;
  objs[1] = (artifact_t*)helpers_o;
  objs[2] = (artifact_t*)testcase_o;

  return make_exe_from_o(cutest_run, (const artifact_t**)objs, verbose);
}

static int make_cutest_work_o(cutest_work_o_t* cutest_work_o,
                              cutest_work_c_t* cutest_work_c,
                              int verbose)
{
  return make_o_from_c(cutest_work_o, cutest_work_c, verbose);
}

static int make_cutest_work(cutest_work_t* cutest_work,
                            cutest_work_o_t* cutest_work_o,
                            helpers_o_t* helpers_o,
                            int verbose)
{
  artifact_t* objs[2];

  objs[0] = (artifact_t*)cutest_work_o;
  objs[1] = (artifact_t*)helpers_o;

  return make_exe_from_o(cutest_work, (const artifact_t**)objs, verbose);
}

static int make_cutest_o(cutest_o_t* cutest_o,
                         cutest_c_t* cutest_c,
                         int verbose)
{
  return make_o_from_c(cutest_o, cutest_c, verbose);
}

static int make_cutest(cutest_t* cutest,
                       cutest_o_t* cutest_o,
                       int verbose)
{
  artifact_t* objs[1];

  objs[0] = (artifact_t*)cutest_o;

  return make_exe_from_o(cutest, (const artifact_t**)objs, verbose);
}

/*
static int make_cproto(cproto_t* cproto, int verbose)
{
  char command[1000];

  cproto_t cproto_c = {"./cproto-4.7m/cproto.c", 0, 0};
  const artifact_t* deps[1] = {(artifact_t*)&cproto_c};

  if (0 == should_rebuild(cproto, deps, verbose)) {
    return 0;
  }

  sprintf(command, "make -f cproto.mk %s", cproto->str);
  if (0 != verbose) {
    puts(command);
  }
  int retval = system(command);
  if (0 != retval) {
    return -1;
  }
  return 0;
}
*/

#define DEPTOOKEN "#define CUTEST_DEP "

int split_deptooken(file_list_t* dep_list, char* buf)
{
  char *b = &buf[strlen(DEPTOOKEN)];
  const size_t len = strlen(b);
  size_t i;
  char *start = b;
  for (i = 0; i < len; i++) {
    if ((' ' == b[i]) || (len - 1 == i)) {
      file_node_t* file = NULL;
      b[i] = '\0';
      file = new_file_node(NULL, start);
      file_list_add_node(dep_list, file);
      start = &b[i + 1];
    }
  }
  return 0;
}

int read_required_linkable_objects_from_test_suite(file_list_t* dep_list, test_c_t* test_c)
{
  char buf[1024];
  FILE* fd = fopen(test_c->str, "r");
  if (NULL == fd) {
    fprintf(stderr, "ERROR: Could not open '%s' for parsing.\n", test_c->str);
    return -1;
  }

  while (NULL != fgets(buf, sizeof(buf), fd)) {
    if (0 != strncmp(buf, DEPTOOKEN, strlen(DEPTOOKEN))) {
      continue;
    }
    split_deptooken(dep_list, buf);
    break;
  }

  fclose(fd);

  return 0;
}

#define clean(ARTIFACTPTR, VERBOSE) _clean((artifact_t*)ARTIFACTPTR, VERBOSE);

void _clean(artifact_t* artifact, int verbose)
{
  if (0 != verbose) {
    printf("Removing %s\n", artifact->str);
  }
#ifdef AMIGAOS
#else
  unlink(artifact->str);
#endif
}

static int test_in_target_list(file_list_t* target_list, const char* file_name, int verbose) {
  if (NULL == target_list->first) {
    return 1;
  }
  if (NULL == find_file_node_in_file_list(target_list, file_name, verbose)) {
    return 0;
  }
  return 1;
}

#define ASSIGN_ARTIFACT(ARTIFACT, STR, FAKE_MAIN) \
  ARTIFACT.str = STR;                             \
  ARTIFACT.timestamp = 0;                         \
  ARTIFACT.fake_main = FAKE_MAIN

int build_dep(const char* source_file_name, file_list_t* target_list,
              cutest_h_t* cutest_h, cutest_impl_o_t* cutest_impl_o,
              cutest_prox_t* cutest_prox, cutest_mock_t* cutest_mock,
              cutest_run_t* cutest_run, cproto_t* cproto,
              int verbose, int clean)
{
  const int v = verbose;
  c_t c;

  mockables_o_t mockables_o;
  mockables_s_t mockables_s;
  mockables_lst_t mockables_lst;
  proxified_s_t proxified_s;
  mocks_h_t mocks_h;
  test_c_t test_c;
  test_run_c_t test_run_c;
  test_t test;

  file_list_t dep_list;

  ASSIGN_ARTIFACT(c,
                  (char*)source_file_name,
                  1);
  ASSIGN_ARTIFACT(mockables_o,
                  gen_filename(c.str, CUTEST_TMP_PATH, ".c", "_mockables.o"),
                  1);
  ASSIGN_ARTIFACT(mockables_s,
                  gen_filename(mockables_o.str, CUTEST_TMP_PATH, ".o", ".s"),
                  1);
  ASSIGN_ARTIFACT(mockables_lst,
                  gen_filename(mockables_s.str, CUTEST_TMP_PATH, ".s", ".lst"),
                  1);
  ASSIGN_ARTIFACT(proxified_s,
                  gen_filename(mockables_s.str, CUTEST_TMP_PATH, "_mockables.s", "_proxified.s"),
                  1);
  ASSIGN_ARTIFACT(mocks_h,
                  gen_filename(c.str, CUTEST_TMP_PATH, ".c", "_mocks.h"),
                  1);
  ASSIGN_ARTIFACT(test_c,
                  gen_filename(c.str, CUTEST_TST_PATH, ".c", "_test.c"),
                  1);
  ASSIGN_ARTIFACT(test_run_c,
                  gen_filename(c.str, CUTEST_TMP_PATH, ".c", "_test_run.c"),
                  1);
  ASSIGN_ARTIFACT(test,
                  gen_filename(c.str, CUTEST_TST_PATH, ".c", "_test"),
                  0);

  if (NULL != target_list->first) {
    if (1 < verbose) {
      printf("DEBUG: Searching for '%s' in requested target list.\n", test.str);
    }
    if (!test_in_target_list(target_list, test.str, verbose)) {
      goto cleanup;
    }
    if (1 < verbose) {
      printf("DEBUG: Will build requested target '%s'\n", test.str);
    }
  }

  if (0 == clean) {
    if (0 != make_mockables_o(&mockables_o, &c, v)) {
      return -1;
    }
    if (0 != make_mockables_s(&mockables_s, &c, v)) {
      return -2;
    }
    if (0 != make_mockables_lst(&mockables_lst, &mockables_o, v)) {
      return -3;
    }
    if (0 != make_proxified_s(&proxified_s, &mockables_s, &mockables_lst, cutest_prox, v)) {
      return -5;
    }
    if (0 != make_mocks_h(&mocks_h, &c, &mockables_lst, cutest_mock, cproto, v)) {
      return -5;
    }
    if (0 != make_test_run_c(&test_run_c, &test_c, &mocks_h, cutest_run, v)) {
      return -5;
    }

    memset(&dep_list, 0, sizeof(dep_list));

    read_required_linkable_objects_from_test_suite(&dep_list, &test_c);

    if (1 < verbose) {
      printf("DEBUG: Will try to build '%s'.\n", test.str);
    }

    /* TODO: Maybe proxified_o and test_run_o would save build time */
    if (0 != make_test(&test, &proxified_s, &test_run_c, cutest_impl_o, &dep_list, v)) {
      return -5;
    }
  }
  else {
    clean(&mockables_o, verbose);
    clean(&mockables_s, verbose);
    clean(&mockables_lst, verbose);
    clean(&proxified_s, verbose);
    clean(&mocks_h, verbose);
    clean(&test_run_c, verbose);
    clean(&test, verbose);
  }
 cleanup:
  free(test.str);
  free(test_run_c.str);
  free(test_c.str);
  free(mocks_h.str);
  free(proxified_s.str);
  free(mockables_lst.str);
  free(mockables_s.str);
  free(mockables_o.str);

  return 0;
}

/*
static int slight_delay()
{
  int a = 0;
  int i = 0;

  for (i = 0; i < 10; i++) {
    a += i * 2;
  }
  return a;
}
*/

/*
static int wait_for_child_processes(int allocated_cores, int verbose)
{
  int retval = 0;

  while (allocated_cores > 0) {
    int status = 0;
    (void)waitpid(0, &status, 0);
    --allocated_cores;
    retval |= WEXITSTATUS(status);
  }
  slight_delay();
  if (-1 == verbose) {
    puts("");
  }

  return retval;
}
*/

static size_t file_list_len(file_list_t* list)
{
  size_t cnt = 0;
  file_node_t* node = NULL;
  for (node = list->first; NULL != node; node = node->next) {
    cnt++;
  }
  return cnt;
}

file_node_t* file_list_node_by_idx(file_list_t* list, const size_t idx)
{
  size_t cnt = 0;
  file_node_t* node = NULL;
  for (node = list->first; NULL != node; node = node->next) {
    if (idx == cnt) {
      return node;
    }
    cnt++;
  }
  return NULL;
}

int build_deps(int core_idx,
               int cores,
               file_list_t* list,
               file_list_t* target_list,
               cutest_h_t* cutest_h,
               cutest_impl_o_t* cutest_impl_o,
               cutest_prox_t* cutest_prox,
               cutest_mock_t* cutest_mock,
               cutest_run_t* cutest_run,
               cproto_t* cproto, int verbose, int clean)
{
  int retval = 0;
  const int all_target_len = file_list_len(list);
  const int req_target_len = file_list_len(target_list);
  size_t target_len = all_target_len;
  size_t target_idx = core_idx;
  if (0 != req_target_len) {
    target_len = min(all_target_len, req_target_len);
  }

  while (target_idx < target_len) {
    file_node_t* node = file_list_node_by_idx(list, target_idx);

    if (0 == node->skip) {
      const int r = build_dep(node->file.file_name,
                              target_list,
                              cutest_h, cutest_impl_o,
                              cutest_prox, cutest_mock,
                              cutest_run, cproto,
                              verbose, clean);
      if (0 != r) {
        fprintf(stderr, "ERROR: %s failed\n", node->file.file_name);
        return -1;
      }
      retval |= r;
    }
    target_idx += cores;
  }
  /*
  for (node = list->first; NULL != node; node = node->next) {
    if (0 != node->skip) {
      continue;
    }
    retval |= build_dep(node->file.file_name,
                        target_list,
                        cutest_h, cutest_o,
                        cutest_prox, cutest_mock,
                        cutest_run, cproto,
                        verbose, clean);
    if (0 != retval) {
      fprintf(stderr, "ERROR: Failed\n");
      return retval;
    }
  }
  */
  return retval;
}

/*
static void launch_child_processes(int allocated_cores,
                                   file_list_t* list,
                                   file_list_t* target_list,
                                   cutest_h_t* cutest_h,
                                   cutest_impl_o_t* cutest_impl_o,
                                   cutest_prox_t* cutest_prox,
                                   cutest_mock_t* cutest_mock,
                                   cutest_run_t* cutest_run,
                                   cproto_t* cproto, int verbose, int clean)
{
  int idx = 0;
  pid_t pid[128];

  for (idx = 0; idx < allocated_cores; idx++) {
    if (0 > (pid[idx] = fork())) {
      fprintf(stderr, "ERROR: Failed to fork\n");
      exit(EXIT_FAILURE);
    }
    else if (pid[idx] == 0) {
      int r = build_deps(idx,
                         allocated_cores,
                         list,
                         target_list,
                         cutest_h,
                         cutest_impl_o,
                         cutest_prox,
                         cutest_mock,
                         cutest_run,
                         cproto, verbose, clean);
      if (0 != r) {
        r = EXIT_FAILURE;
      }
      else {
        r = EXIT_SUCCESS;
      }
      exit(r);
    }
  }
}
*/

static int launch_process(file_list_t* list,
                          file_list_t* target_list,
                          cutest_h_t* cutest_h,
                          cutest_impl_o_t* cutest_impl_o,
                          cutest_prox_t* cutest_prox,
                          cutest_mock_t* cutest_mock,
                          cutest_run_t* cutest_run,
                          cproto_t* cproto, int verbose, int clean)
{
  int r = build_deps(0,
                     1,
                     list,
                     target_list,
                     cutest_h,
                     cutest_impl_o,
                     cutest_prox,
                     cutest_mock,
                     cutest_run,
                     cproto,
                     verbose,
                     clean);
  return r;
}

int build(file_list_t* source_list, file_list_t* target_list, int verbose, int clean)
{
  int retval = 0;

  helpers_c_t helpers_c;
  helpers_o_t helpers_o;
  mockable_c_t mockable_c;
  mockable_o_t mockable_o;
  arg_c_t arg_c;
  arg_o_t arg_o;
  testcase_c_t testcase_c;
  testcase_o_t testcase_o;
  cutest_h_t cutest_h;
  cutest_impl_c_t cutest_impl_c;
  cutest_impl_o_t cutest_impl_o;
  cutest_prox_c_t cutest_prox_c;
  cutest_prox_o_t cutest_prox_o;
  cutest_prox_t cutest_prox;
  cutest_mock_c_t cutest_mock_c;
  cutest_mock_o_t cutest_mock_o;
  cutest_mock_t cutest_mock;
  cutest_run_c_t cutest_run_c;
  cutest_run_o_t cutest_run_o;
  cutest_run_t cutest_run;
  cutest_work_c_t cutest_work_c;
  cutest_work_o_t cutest_work_o;
  cutest_work_t cutest_work;
  cutest_c_t cutest_c;
  cutest_o_t cutest_o;
  cutest_t cutest;
  cproto_t cproto;

  ASSIGN_ARTIFACT(helpers_c,
                  HELPERS_C,
                  0);
  ASSIGN_ARTIFACT(helpers_o,
                  gen_filename(helpers_c.str, CUTEST_TMP_PATH, ".c", ".o"),
                  0);
  ASSIGN_ARTIFACT(mockable_c,
                  MOCKABLE_C,
                  0);
  ASSIGN_ARTIFACT(mockable_o,
                  gen_filename(mockable_c.str, CUTEST_TMP_PATH, ".c", ".o"),
                  0);
  ASSIGN_ARTIFACT(arg_c,
                  ARG_C,
                  0);
  ASSIGN_ARTIFACT(arg_o,
                  gen_filename(arg_c.str, CUTEST_TMP_PATH, ".c", ".o"),
                  0);
  ASSIGN_ARTIFACT(testcase_c,
                  TESTCASE_C,
                  0);
  ASSIGN_ARTIFACT(testcase_o,
                  gen_filename(testcase_c.str, CUTEST_TMP_PATH, ".c", ".o"),
                  0);
  ASSIGN_ARTIFACT(cutest_h,
                  CUTEST_H,
                  0);
  ASSIGN_ARTIFACT(cutest_impl_c,
                  CUTEST_IMPL_C,
                  0);
  ASSIGN_ARTIFACT(cutest_impl_o,
                  gen_filename(cutest_impl_c.str, CUTEST_TMP_PATH, ".c", ".o"),
                  0);
  ASSIGN_ARTIFACT(cutest_prox_c,
                  CUTEST_PROX_C,
                  0);
  ASSIGN_ARTIFACT(cutest_prox_o,
                  gen_filename(cutest_prox_c.str, CUTEST_TMP_PATH, ".c", ".o"),
                  0);
  ASSIGN_ARTIFACT(cutest_prox,
                  gen_filename(cutest_prox_c.str, CUTEST_TMP_PATH, ".c", ""),
                  0);
  ASSIGN_ARTIFACT(cutest_mock_c,
                  CUTEST_MOCK_C,
                  0);
  ASSIGN_ARTIFACT(cutest_mock_o,
                  gen_filename(cutest_mock_c.str, CUTEST_TMP_PATH, ".c", ".o"),
                  0);
  ASSIGN_ARTIFACT(cutest_mock,
                  gen_filename(cutest_mock_c.str, CUTEST_TMP_PATH, ".c", ""),
                  0);
  ASSIGN_ARTIFACT(cutest_run_c,
                  CUTEST_RUN_C,
                  0);
  ASSIGN_ARTIFACT(cutest_run_o,
                  gen_filename(cutest_run_c.str, CUTEST_TMP_PATH, ".c", ".o"),
                  0);
  ASSIGN_ARTIFACT(cutest_run,
                  gen_filename(cutest_run_c.str, CUTEST_TMP_PATH, ".c", ""),
                  0);
  ASSIGN_ARTIFACT(cutest_work_c,
                  CUTEST_WORK_C,
                  0);
  ASSIGN_ARTIFACT(cutest_work_o,
                  gen_filename(cutest_work_c.str, CUTEST_TMP_PATH, ".c", ".o"),
                  0);
  ASSIGN_ARTIFACT(cutest_work,
                  gen_filename(cutest_work_c.str, CUTEST_TMP_PATH, ".c", ""),
                  0);
  ASSIGN_ARTIFACT(cutest_c,
                  CUTEST_C,
                  0);
  ASSIGN_ARTIFACT(cutest_o,
                  gen_filename(cutest_c.str, CUTEST_TMP_PATH, ".c", ".o"),
                  0);
  ASSIGN_ARTIFACT(cutest,
                  gen_filename(cutest_c.str, CUTEST_TMP_PATH, ".c", ""),
                  0);
  ASSIGN_ARTIFACT(cproto,
                  CPROTO,
                  0);

  if (1 < verbose) {
    printf("DEBUG: Making the CUTest framework and tools\n");
  }

  if (2 != clean) {
    /*
     * This should only be needed if cutest is used from source. If it is
     * installed in the user's system rebuilding the cutest tools is irrelevant.
     */
    if (0 != make_helpers_o(&helpers_o, &helpers_c, verbose)) {
      return -4;
    }
    if (0 != make_mockable_o(&mockable_o, &mockable_c, verbose)) {
      return -4;
    }
    if (0 != make_arg_o(&arg_o, &arg_c, verbose)) {
      return -4;
    }
    if (0 != make_testcase_o(&testcase_o, &testcase_c, verbose)) {
      return -4;
    }
    if (0 != make_cutest_impl_o(&cutest_impl_o, &cutest_impl_c, verbose)) {
      return -4;
    }
    if (0 != make_cutest_prox_o(&cutest_prox_o, &cutest_prox_c, verbose)) {
      return -4;
    }
    if (0 != make_cutest_prox(&cutest_prox, &cutest_prox_o, &helpers_o, verbose)) {
      return -4;
    }
    if (0 != make_cutest_mock_o(&cutest_mock_o, &cutest_mock_c, verbose)) {
      return -4;
    }
    if (0 != make_cutest_mock(&cutest_mock, &cutest_mock_o, &helpers_o, &mockable_o, &arg_o, verbose)) {
      return -4;
    }
    if (0 != make_cutest_run_o(&cutest_run_o, &cutest_run_c, verbose)) {
      return -4;
    }
    if (0 != make_cutest_run(&cutest_run, &cutest_run_o, &helpers_o, &testcase_o, verbose)) {
      return -4;
    }
    if (0 != make_cutest_work_o(&cutest_work_o, &cutest_work_c, verbose)) {
      return -4;
    }
    if (0 != make_cutest_work(&cutest_work, &cutest_work_o, &helpers_o, verbose)) {
      return -4;
    }
    if (0 != make_cutest_o(&cutest_o, &cutest_c, verbose)) {
      return -4;
    }
    if (0 != make_cutest(&cutest, &cutest_o, verbose)) {
      return -4;
    }
    /*
    if (0 != make_cproto(&cproto, verbose)) {
      return -4;
    }
    */
  }
  else {
    clean(&helpers_o, verbose);
    clean(&mockable_o, verbose);
    clean(&arg_o, verbose);
    clean(&testcase_o, verbose);
    clean(&cutest_impl_o, verbose);
    clean(&cutest_prox_o, verbose);
    clean(&cutest_prox, verbose);
    clean(&cutest_mock_o, verbose);
    clean(&cutest_mock, verbose);
    clean(&cutest_run_o, verbose);
    clean(&cutest_run, verbose);
  }

  if (1 < verbose) {
    printf("DEBUG: Done with the CUTest framework and tools dependencies.\n");
  }

  /*
  const int allocated_cores = get_number_of_cores();
  */
  /*
  const int allocated_cores = 1;

  if (allocated_cores > 1) {
    if (1 < verbose) {
      printf("DEBUG: Launching sub-processes\n");
    }
    launch_child_processes(allocated_cores,
                           source_list,
                           target_list,
                           &cutest_h,
                           &cutest_impl_o,
                           &cutest_prox,
                           &cutest_mock,
                           &cutest_run,
                           &cproto, verbose, clean);
    retval = wait_for_child_processes(allocated_cores, verbose);
  }
  else {
  */
    retval = launch_process(source_list,
                            target_list,
                            &cutest_h,
                            &cutest_impl_o,
                            &cutest_prox,
                            &cutest_mock,
                            &cutest_run,
                            &cproto, verbose, clean);
    /*
  }
    */
  /*
  retval = build_deps(source_list, target_list, &cutest_h, &cutest_o,
                      &cutest_prox, &cutest_mock,
                      &cutest_run,
                      &cproto, verbose, clean);
  */
  free(helpers_o.str);
  free(mockable_o.str);
  free(arg_o.str);
  free(testcase_o.str);
  free(cutest_impl_o.str);
  free(cutest_prox_o.str);
  free(cutest_prox.str);
  free(cutest_mock_o.str);
  free(cutest_mock.str);
  free(cutest_run_o.str);
  free(cutest_run.str);

  return retval;
}

int main(int argc, char* argv[]) {
  int verbose = 0;
  int clean = 0;
  int i = 0;
  file_list_t source_list = {NULL, NULL};
  file_list_t test_source_list = {NULL, NULL};
  file_list_t target_list = {NULL, NULL};
  file_node_t* target_node = NULL;

  if (!(argc == 1 || argc == 2 || argc == 3)) {
    fprintf(stderr, "ERROR: Argument number error.\n");
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  for (i = 1; i < argc; i++) {
    if ((0 == strcmp(argv[i], "-h")) || (0 == strcmp(argv[i], "--help"))) {
      usage(argv[0]);
      return EXIT_SUCCESS;
    }
    if ((0 == strcmp(argv[i], "-d")) || (0 == strcmp(argv[i], "--debug"))) {
      verbose = 2;
      continue;
    }
    if ((0 == strcmp(argv[i], "-v")) || (0 == strcmp(argv[i], "--verbose"))) {
      verbose = 1;
      continue;
    }
    if ((0 == strcmp(argv[i], "-c")) || (0 == strcmp(argv[i], "--clean"))) {
      clean = 1;
      continue;
    }
    if ((0 == strcmp(argv[i], "-C")) || (0 == strcmp(argv[i], "--clean-all"))) {
      clean = 2;
      continue;
    }

    if (0 == strcmp(argv[i], "clean")) {
      clean = 1;
      continue;
    }

    if (0 == strcmp(argv[i], "clean-all")) {
      clean = 2;
      continue;
    }

    /* Add all other arguments to the build-target list */
    target_node = new_file_node(NULL, argv[i]);
    if (NULL == target_node) {
      fprintf(stderr, "ERROR: Out of memory while adding requested target '%s'\n", argv[i]);
      return EXIT_FAILURE;
    }

    file_list_add_node(&target_list, target_node);

    if (1 < verbose) {
      printf("DEBUG: Adding '%s' to targets to build.\n", target_node->file.file_name);
    }
  }

  make_list_of_files(&source_list, CUTEST_SRC_PATH, ".c", "_test.c", "_run.c", verbose, "product code");
  make_list_of_files(&test_source_list, CUTEST_TST_PATH, "_test.c", NULL, NULL, verbose, "test suites");

  print_missing_tests(&source_list, &test_source_list);
  print_missing_sources(&source_list, &test_source_list);

  build(&source_list, &target_list, verbose, clean);

  target_node = target_list.first;
  while (NULL != target_node) {
    file_node_t* next = target_node->next;
    delete_file_node(target_node);
    target_node = next;
  }

  free_files(&source_list);
  free_files(&test_source_list);
}
