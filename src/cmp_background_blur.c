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
  cmp_background_blur_t *blur;
  int err;
  int rc;

  if (!out_blur) {
    LOG_DEBUG("cmp_background_blur_create: out_blur is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_background_blur_t), (void **)&blur);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_background_blur_create: OOM\n");
    return CMP_ERROR_OOM;
  }

  err = cmp_window_blur_create(&blur->impl);
  if (err != CMP_SUCCESS) {
    LOG_DEBUG("cmp_background_blur_create: cmp_window_blur_create failed\n");
    rc = CMP_FREE(blur);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_background_blur_create: CMP_FREE failed\n");
    }
    return err;
  }

  *out_blur = blur;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_background_blur_destroy
 *
 * @param blur Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_background_blur_destroy(cmp_background_blur_t *blur) {
  int err = CMP_SUCCESS;
  int rc;

  if (!blur) {
    LOG_DEBUG("cmp_background_blur_destroy: blur is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (blur->impl) {
    err = cmp_window_blur_destroy(blur->impl);
    if (err != CMP_SUCCESS) {
      LOG_DEBUG(
          "cmp_background_blur_destroy: cmp_window_blur_destroy failed\n");
    }
  }

  rc = CMP_FREE(blur);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_background_blur_destroy: CMP_FREE failed\n");
    if (err == CMP_SUCCESS)
      err = rc;
  }
  return err;
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
  if (!blur || !window)
    return CMP_ERROR_INVALID_ARG;

  return cmp_window_blur_set_enabled(blur->impl, window, enabled);
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
  if (!blur || !window || !out_enabled) {
    if (out_enabled)
      *out_enabled = 0;
    return CMP_ERROR_INVALID_ARG;
  }

  return cmp_window_blur_is_enabled(blur->impl, window, out_enabled);
}
