/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>

#include "cmp_log.h"
/* clang-format on */

#define STATUS_TEXT_MAX 128

struct cmp_status_bar {
  char backend_status[STATUS_TEXT_MAX];
  int prompt_tokens;
  int completion_tokens;
  float system_memory_mb;
  float vram_used_mb;
};

/**
 * @brief Create a status bar.
 *
 * @param out_bar Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_status_bar_create(cmp_status_bar_t **out_bar) {
  int rc;
  cmp_status_bar_t *bar;

  rc = CMP_SUCCESS;

  if (out_bar == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_status_bar_t), (void **)&bar);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  bar->backend_status[0] = '\0';
  bar->prompt_tokens = 0;
  bar->completion_tokens = 0;
  bar->system_memory_mb = 0.0f;
  bar->vram_used_mb = 0.0f;

  *out_bar = bar;
  return CMP_SUCCESS;
}

/**
 * @brief Destroy a status bar.
 *
 * @param bar Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_status_bar_destroy(cmp_status_bar_t *bar) {
  int rc;

  rc = CMP_SUCCESS;

  if (bar == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(bar);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
    return rc;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Set the backend status text.
 *
 * @param bar Parameter description.
 * @param status Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_status_bar_set_backend_status(cmp_status_bar_t *bar,
                                      const char *status) {
  if (bar == NULL || status == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_MSC_VER)
  if (strncpy_s(bar->backend_status, STATUS_TEXT_MAX, status, STATUS_TEXT_MAX - 1) != 0) {
      LOG_DEBUG("strncpy_s failed\n");
      return CMP_ERROR_GENERAL;
  }
#else
  strncpy(bar->backend_status, status, STATUS_TEXT_MAX - 1);
  bar->backend_status[STATUS_TEXT_MAX - 1] = '\0';
#endif

  return CMP_SUCCESS;
}

/**
 * @brief Update token usage metrics.
 *
 * @param bar Parameter description.
 * @param prompt_tokens Parameter description.
 * @param completion_tokens Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_status_bar_update_token_usage(cmp_status_bar_t *bar, int prompt_tokens,
                                      int completion_tokens) {
  if (bar == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  bar->prompt_tokens = prompt_tokens;
  bar->completion_tokens = completion_tokens;

  return CMP_SUCCESS;
}

/**
 * @brief Update memory metrics.
 *
 * @param bar Parameter description.
 * @param system_memory_mb Parameter description.
 * @param vram_used_mb Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_status_bar_update_memory_metrics(cmp_status_bar_t *bar,
                                         float system_memory_mb,
                                         float vram_used_mb) {
  if (bar == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  bar->system_memory_mb = system_memory_mb;
  bar->vram_used_mb = vram_used_mb;

  return CMP_SUCCESS;
}
