/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_sandbox_exec {
  int is_initialized;
};

int cmp_sandbox_exec_create(cmp_sandbox_exec_t **out_sandbox) {
  cmp_sandbox_exec_t *sandbox;
  if (!out_sandbox)
    return CMP_ERROR_INVALID_ARG;

  sandbox = (cmp_sandbox_exec_t *)malloc(sizeof(cmp_sandbox_exec_t));
  if (!sandbox)
    return CMP_ERROR_OOM;

  sandbox->is_initialized = 1;
  *out_sandbox = sandbox;
  return CMP_SUCCESS;
}

int cmp_sandbox_exec_destroy(cmp_sandbox_exec_t *sandbox) {
  if (!sandbox)
    return CMP_ERROR_INVALID_ARG;
  free(sandbox);
  return CMP_SUCCESS;
}

int cmp_sandbox_exec_run(cmp_sandbox_exec_t *sandbox, const char *language,
                         const char *code, char **out_output) {
  const char *dummy_output = "Hello world (mocked output)\n";
  char *res;
  size_t len;

  if (!sandbox || !language || !code || !out_output) {
    return CMP_ERROR_INVALID_ARG;
  }

  /* This is a mock. Real integration uses platform specific process launching
     (fork/exec on POSIX, CreateProcess on Win32, capturing pipes). */
  len = strlen(dummy_output);
  res = (char *)malloc(len + 1);
  if (!res) {
    return CMP_ERROR_OOM;
  }

  strncpy(res, dummy_output, len);
  res[len] = '\0';
  *out_output = res;

  return CMP_SUCCESS;
}

int cmp_sandbox_exec_free_output(char *output) {
  if (output) {
    free(output);
  }
  return CMP_SUCCESS;
}
