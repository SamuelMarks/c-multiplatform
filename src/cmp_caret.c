/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

struct cmp_caret {
  double elapsed_ms;
  int is_visible;
  double blink_rate_ms;
};

/**
 * @brief cmp_caret_create
 *
 * @param out_caret Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_caret_create(cmp_caret_t **out_caret) {
  int rc = CMP_SUCCESS;
  struct cmp_caret *caret = NULL;

  if (!out_caret) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_caret_create: Invalid argument (out_caret=NULL): %s\n",
                err_str);
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_caret), (void **)&caret);
  if (rc != CMP_SUCCESS) {
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_caret_create: Out of memory: %s\n", err_str);
    }
    return rc;
  }

  memset(caret, 0, sizeof(struct cmp_caret));
  caret->blink_rate_ms = 500.0; /* 500ms on, 500ms off */
  caret->is_visible = 1;

  *out_caret = (cmp_caret_t *)caret;
  return rc;
}

/**
 * @brief cmp_caret_destroy
 *
 * @param caret Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_caret_destroy(cmp_caret_t *caret) {
  int rc = CMP_SUCCESS;
  struct cmp_caret *internal_caret = (struct cmp_caret *)caret;

  if (!internal_caret) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_caret_destroy: Invalid argument (caret=NULL): %s\n",
                err_str);
    }
    return rc;
  }

  CMP_FREE(internal_caret);
  return rc;
}

/**
 * @brief cmp_caret_update_blink
 *
 * @param caret Parameter description.
 * @param dt_ms Parameter description.
 * @param out_is_visible Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_caret_update_blink(cmp_caret_t *caret, double dt_ms,
                           int *out_is_visible) {
  int rc = CMP_SUCCESS;
  struct cmp_caret *internal_caret = (struct cmp_caret *)caret;

  if (!internal_caret || !out_is_visible || dt_ms < 0.0) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_caret_update_blink: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  internal_caret->elapsed_ms += dt_ms;

  if (internal_caret->elapsed_ms >= internal_caret->blink_rate_ms) {
    internal_caret->elapsed_ms -= internal_caret->blink_rate_ms;
    internal_caret->is_visible = !internal_caret->is_visible;
  }

  *out_is_visible = internal_caret->is_visible;

  return rc;
}
