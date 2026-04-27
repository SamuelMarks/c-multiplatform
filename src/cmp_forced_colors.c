/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_forced_colors {
  int active;
};

/**
 * @brief cmp_forced_colors_create
 *
 * @param out_ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_forced_colors_create(cmp_forced_colors_t **out_ctx) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_forced_colors_t *ctx = NULL;

  if (out_ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_forced_colors_create: Invalid argument (out_ctx=NULL): %s\n",
        err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_forced_colors_t), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_forced_colors_create: Out of memory: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->active = 0;
  *out_ctx = ctx;
  cmp_log_debug(
      "cmp_forced_colors_create: Successfully created forced colors context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_forced_colors_destroy
 *
 * @param ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_forced_colors_destroy(cmp_forced_colors_t *ctx) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_forced_colors_destroy: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_forced_colors_destroy: CMP_FREE failed\n");
  }

  cmp_log_debug("cmp_forced_colors_destroy: Successfully destroyed forced "
                "colors context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_forced_colors_set
 *
 * @param ctx Parameter description.
 * @param active Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_forced_colors_set(cmp_forced_colors_t *ctx, int active) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_forced_colors_set: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->active = active ? 1 : 0;
  cmp_log_debug("cmp_forced_colors_set: Set forced colors active=%d\n",
                ctx->active);
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_forced_colors_strip_background
 *
 * @param ctx Parameter description.
 * @param out_strip Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_forced_colors_strip_background(const cmp_forced_colors_t *ctx,
                                       int *out_strip) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (ctx == NULL || out_strip == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_forced_colors_strip_background: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  *out_strip = ctx->active;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_forced_colors_strip_box_shadow
 *
 * @param ctx Parameter description.
 * @param out_strip Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_forced_colors_strip_box_shadow(const cmp_forced_colors_t *ctx,
                                       int *out_strip) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (ctx == NULL || out_strip == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_forced_colors_strip_box_shadow: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  *out_strip = ctx->active;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
