#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if defined(WIN32) || defined(_WIN32)
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

struct file_node_s {
  char* file_name;
  struct file_node_s* next;
};

typedef struct file_node_s file_node_t;

struct file_list_s {
  file_node_t* first;
  file_node_t* last;
};

typedef struct file_list_s file_list_t;

static void usage(const char* program_name)
{
  printf("USAGE: %s <path-to-cutest_make> <path-to-cutest_work>\n",
         program_name);
}

static file_node_t* new_file_node(char* path)
{
  file_node_t* node = malloc(sizeof(*node));
  if (NULL == node) {
    return NULL;
  }
  char* dst = malloc(strlen(path) + 1);
  if (NULL == dst) {
    free(node);
    return NULL;
  }
  strcpy(dst, path);
  dst[strlen(path)] = 0;

  node->file_name = dst;
  node->next = NULL;

  return node;
}

static void delete_file_node(file_node_t* node)
{
  free(node->file_name);
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

static int make_test_suite_list(file_list_t* list)
{
  DIR* dir;
  struct dirent* ent;
  char cwd[1024];

  if (NULL == getcwd(cwd, sizeof(cwd))) {
    fprintf(stderr, "ERROR: Could not get current working directory\n");
    return -1;
  }

  dir = opendir(cwd);

  if (NULL == dir) {
    fprintf(stderr, "ERROR: Could not open current directory for reading\n");
    return -1;
  }

  const size_t suffix_len = strlen("_test");

  while (NULL != (ent = readdir(dir))) {
    char* name = ent->d_name;
    char* full_path;
    const size_t full_len = strlen(cwd) + strlen(name) + 1;
    const size_t ent_len = strlen(name);
    if (ent_len < suffix_len) {
      continue;
    }
    if (0 != strcmp("_test", &name[ent_len - suffix_len])) {
      continue;
    }

    full_path = malloc(full_len);
    if (NULL == full_path) {
      fprintf(stderr, "ERROR: Out of memory while allocating absolute path to test suite\n");
      return -1;
    }
    memset(full_path, 0, full_len);
    strcpy(full_path, cwd);
    strcat(full_path, PATH_SEPARATOR);
    strcat(full_path, name);
    file_node_t* node = new_file_node(full_path);
    free(full_path);
    if (NULL == node) {
      fprintf(stderr, "ERROR: Out of memory while allocating new test suite entry.\n");
      return -1;
    }
    file_list_add_node(list, node);
  }

  closedir(dir);

  return 0;
}

int main(int argc, char* argv[]) {
  char* cutest_worker;
  char* cutest_make;

  if (argc != 3) {
    fprintf(stderr, "ERROR: Too few arguments\n");
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  file_list_t list = {NULL, NULL};
  make_test_suite_list(&list);

  size_t cutest_worker_len = strlen(argv[2]) + strlen(" -n ") + 1;
  size_t cutest_make_len = strlen(argv[1]) + strlen(" ") + 1;

  cutest_make = malloc(cutest_make_len);
  if (NULL == cutest_make) {
    fprintf(stderr, "ERROR: Out of memory while allocating command\n");
    exit(EXIT_FAILURE);
  }
  memset(cutest_make, 0, cutest_make_len);

  strcat(cutest_make, argv[1]);
  strcat(cutest_make, " ");

  file_node_t* node;
  for (node = list.first; NULL != node; node = node->next) {
    cutest_worker_len += strlen(node->file_name) + 1;
  }

  cutest_worker = malloc(cutest_worker_len);
  if (NULL == cutest_worker) {
    fprintf(stderr, "ERROR: Out of memory while allocating command\n");
    exit(EXIT_FAILURE);
  }
  memset(cutest_worker, 0, cutest_worker_len);

  strcat(cutest_worker, argv[2]);
  strcat(cutest_worker, " -n ");

  for (node = list.first; NULL != node; node = node->next) {
    strcat(cutest_worker, node->file_name);
    strcat(cutest_worker, " ");
  }

  int retval = EXIT_SUCCESS;

  if (0 != system(cutest_make)) {
    retval = EXIT_FAILURE;
    goto end;
  }

  if (0 != system(cutest_worker)) {
    retval = EXIT_FAILURE;
    goto end;
  }

 end:

  node = list.first;
  while (NULL != node) {
    file_node_t* next = node->next;
    delete_file_node(node);
    node = next;
  }

  return retval;
}
