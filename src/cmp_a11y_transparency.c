/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdio.h>
/* clang-format on */

struct cmp_a11y_transparency {
  int reduced_transparency_enabled;
};

/**
 * @brief cmp_a11y_transparency_create
 *
 * @param out_trans Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_transparency_create(cmp_a11y_transparency_t **out_trans) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_a11y_transparency_t *trans = NULL;

  if (out_trans == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_a11y_transparency_create: Invalid argument (out_trans=NULL): %s\n",
        err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_a11y_transparency_t), (void **)&trans);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_transparency_create: Out of memory: %s\n", err_str);

    return rc;
  }

  trans->reduced_transparency_enabled = 0;
  *out_trans = trans;
  cmp_log_debug("cmp_a11y_transparency_create: Successfully created "
                "transparency context\n");

  return rc;
}

/**
 * @brief cmp_a11y_transparency_destroy
 *
 * @param trans Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_transparency_destroy(cmp_a11y_transparency_t *trans) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (trans == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_a11y_transparency_destroy: Invalid argument (trans=NULL): %s\n",
        err_str);

    return rc;
  }
  CMP_FREE(trans);
  cmp_log_debug("cmp_a11y_transparency_destroy: Successfully destroyed "
                "transparency context\n");

  return rc;
}

/**
 * @brief cmp_a11y_transparency_set
 *
 * @param trans Parameter description.
 * @param enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_transparency_set(cmp_a11y_transparency_t *trans, int enabled) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (trans == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_a11y_transparency_set: Invalid argument (trans=NULL): %s\n",
        err_str);

    return rc;
  }
  trans->reduced_transparency_enabled = enabled != 0 ? 1 : 0;
  cmp_log_debug("cmp_a11y_transparency_set: Set reduced transparency to %d\n",
                trans->reduced_transparency_enabled);

  return rc;
}

/**
 * @brief cmp_a11y_transparency_apply
 *
 * @param trans Parameter description.
 * @param out_opacity Parameter description.
 * @param fallback_opacity Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_transparency_apply(cmp_a11y_transparency_t *trans,
                                float *out_opacity, float fallback_opacity) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (trans == NULL || out_opacity == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_transparency_apply: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  if (trans->reduced_transparency_enabled) {
    *out_opacity = fallback_opacity;
    cmp_log_debug("cmp_a11y_transparency_apply: Reduced transparency applied, "
                  "opacity set to %f\n",
                  fallback_opacity);
  } else {
    cmp_log_debug("cmp_a11y_transparency_apply: Reduced transparency not "
                  "enabled, opacity unchanged\n");
  }

  return rc;
}
