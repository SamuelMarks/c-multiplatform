/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#define CMP_MAX_BLURRED_WINDOWS 32

struct cmp_window_blur {
  cmp_window_t *blurred_windows[CMP_MAX_BLURRED_WINDOWS];
  int count;
};

/**
 * @brief cmp_window_blur_create
 *
 * @param out_blur Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_window_blur_create(cmp_window_blur_t **out_blur) {
  cmp_window_blur_t *blur;
  int rc = CMP_SUCCESS;

  if (!out_blur) {
    LOG_DEBUG("cmp_window_blur_create: out_blur is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_window_blur_t), (void **)&blur);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_window_blur_create: OOM\n");
    return CMP_ERROR_OOM;
  }

  memset(blur, 0, sizeof(cmp_window_blur_t));

  *out_blur = blur;
  return rc;
}

/**
 * @brief cmp_window_blur_destroy
 *
 * @param blur Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_window_blur_destroy(cmp_window_blur_t *blur) {
  int rc = CMP_SUCCESS;
  if (!blur) {
    LOG_DEBUG("cmp_window_blur_destroy: blur is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(blur);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_window_blur_destroy: CMP_FREE failed\n");

    return rc;
  }
  return rc;
}

/**
 * @brief cmp_window_blur_set_enabled
 *
 * @param blur Parameter description.
 * @param window Parameter description.
 * @param enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_window_blur_set_enabled(cmp_window_blur_t *blur, cmp_window_t *window,
                                int enabled) {
  int rc = CMP_SUCCESS;
  int i;
  int found = 0;

  if (!blur || !window) {
    LOG_DEBUG("cmp_window_blur_set_enabled: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < blur->count; i++) {
    if (blur->blurred_windows[i] == window) {
      found = 1;
      if (!enabled) {
        blur->blurred_windows[i] = blur->blurred_windows[blur->count - 1];
        blur->count--;
        return rc;
      }
      break;
    }
  }

  if (enabled && !found) {
    if (blur->count >= CMP_MAX_BLURRED_WINDOWS) {
      LOG_DEBUG("cmp_window_blur_set_enabled: Bounds error\n");
      return CMP_ERROR_BOUNDS;
    }
    blur->blurred_windows[blur->count++] = window;
  }

  return rc;
}

/**
 * @brief cmp_window_blur_is_enabled
 *
 * @param blur Parameter description.
 * @param window Parameter description.
 * @param out_enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_window_blur_is_enabled(const cmp_window_blur_t *blur,
                               const cmp_window_t *window, int *out_enabled) {
  int rc = CMP_SUCCESS;
  int i;

  if (!blur || !window || !out_enabled) {
    LOG_DEBUG("cmp_window_blur_is_enabled: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  *out_enabled = 0;
  for (i = 0; i < blur->count; i++) {
    if (blur->blurred_windows[i] == window) {
      *out_enabled = 1;
      break;
    }
  }

  return rc;
}
