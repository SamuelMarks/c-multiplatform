/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_background_blur {
  cmp_window_blur_t *impl;
};

/**
 * @brief cmp_background_blur_create
 *
 * @param out_blur Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_background_blur_create(cmp_background_blur_t **out_blur) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_background_blur_t *blur = NULL;
  int err;

  if (out_blur == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_background_blur_create: Invalid argument (out_blur=NULL): %s\n",
        err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_background_blur_t), (void **)&blur);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_background_blur_create: Out of memory: %s\n", err_str);

    return rc;
  }

  err = cmp_window_blur_create(&blur->impl);
  if (err != CMP_SUCCESS) {
    err_rc = cmp_strerror(err, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_background_blur_create: cmp_window_blur_create failed: %s\n",
        err_str);
    rc = CMP_FREE(blur);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug(
          "cmp_background_blur_create: CMP_FREE failed during cleanup\n");
    }
    return err;
  }

  *out_blur = blur;
  cmp_log_debug("cmp_background_blur_create: Successfully created background "
                "blur context\n");
  return rc;
}

/**
 * @brief cmp_background_blur_destroy
 *
 * @param blur Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_background_blur_destroy(cmp_background_blur_t *blur) {
  int rc = CMP_SUCCESS;
  int err = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (blur == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_background_blur_destroy: Invalid argument (blur=NULL): %s\n",
        err_str);

    return rc;
  }

  if (blur->impl != NULL) {
    err = cmp_window_blur_destroy(blur->impl);
    if (err != CMP_SUCCESS) {
      err_rc = cmp_strerror(err, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug(
          "cmp_background_blur_destroy: cmp_window_blur_destroy failed: %s\n",
          err_str);
    }
  }

  rc = CMP_FREE(blur);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_background_blur_destroy: CMP_FREE failed\n");
    if (err == CMP_SUCCESS) {
      err = rc;
    }
  }

  cmp_log_debug("cmp_background_blur_destroy: Successfully destroyed "
                "background blur context\n");
  rc = err;
  return rc;
}

/**
 * @brief cmp_background_blur_set_enabled
 *
 * @param blur Parameter description.
 * @param window Parameter description.
 * @param enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_background_blur_set_enabled(cmp_background_blur_t *blur,
                                    cmp_window_t *window, int enabled) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (blur == NULL || window == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_background_blur_set_enabled: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  rc = cmp_window_blur_set_enabled(blur->impl, window, enabled);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_background_blur_set_enabled: Failed: %s\n", err_str);

    return rc;
  }

  cmp_log_debug("cmp_background_blur_set_enabled: Enabled=%d\n", enabled);

  return rc;
}

/**
 * @brief cmp_background_blur_is_enabled
 *
 * @param blur Parameter description.
 * @param window Parameter description.
 * @param out_enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_background_blur_is_enabled(const cmp_background_blur_t *blur,
                                   const cmp_window_t *window,
                                   int *out_enabled) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (blur == NULL || window == NULL || out_enabled == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    if (out_enabled != NULL) {
      *out_enabled = 0;
    }
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_background_blur_is_enabled: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  rc = cmp_window_blur_is_enabled(blur->impl, window, out_enabled);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_background_blur_is_enabled: Failed: %s\n", err_str);

    return rc;
  }

  cmp_log_debug("cmp_background_blur_is_enabled: Is enabled=%d\n",
                *out_enabled);

  return rc;
}
