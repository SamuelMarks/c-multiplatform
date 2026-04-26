/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include "cmp_log.h"
/* clang-format on */

struct cmp_sandbox_exec {
  int is_initialized;
};

/**
 * @brief cmp_sandbox_exec_create
 *
 * @param out_sandbox Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_sandbox_exec_create(cmp_sandbox_exec_t **out_sandbox) {
  int rc = CMP_SUCCESS;
  cmp_sandbox_exec_t *sandbox;
  if (!out_sandbox)
    return CMP_ERROR_INVALID_ARG;

  rc = CMP_MALLOC(sizeof(cmp_sandbox_exec_t), (void **)&(sandbox));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  sandbox->is_initialized = 1;
  *out_sandbox = sandbox;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_sandbox_exec_destroy
 *
 * @param sandbox Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_sandbox_exec_destroy(cmp_sandbox_exec_t *sandbox) {
  int rc = CMP_SUCCESS;
  if (!sandbox)
    return CMP_ERROR_INVALID_ARG;
  rc = CMP_FREE(sandbox);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_sandbox_exec_run
 *
 * @param sandbox Parameter description.
 * @param language Parameter description.
 * @param code Parameter description.
 * @param out_output Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_sandbox_exec_run(cmp_sandbox_exec_t *sandbox, const char *language,
                         const char *code, char **out_output) {
  int rc = CMP_SUCCESS;
  const char *dummy_output = "Hello world (mocked output)\n";
  char *res;
  size_t len;

  if (!sandbox || !language || !code || !out_output) {
    return CMP_ERROR_INVALID_ARG;
  }

  /* This is a mock. Real integration uses platform specific process launching
     (fork/exec on POSIX, CreateProcess on Win32, capturing pipes). */
  len = strlen(dummy_output);
  rc = CMP_MALLOC(len + 1, (void **)&(res));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  strncpy(res, dummy_output, len);
  res[len] = '\0';
  *out_output = res;

  return CMP_SUCCESS;
}

/**
 * @brief cmp_sandbox_exec_free_output
 *
 * @param output Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_sandbox_exec_free_output(char *output) {
  int rc = CMP_SUCCESS;
  if (output) {
    rc = CMP_FREE(output);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
  }
  return CMP_SUCCESS;
}
