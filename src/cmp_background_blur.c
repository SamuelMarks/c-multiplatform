/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_background_blur {
  cmp_window_blur_t *impl;
};

int cmp_background_blur_create(cmp_background_blur_t **out_blur) {
  cmp_background_blur_t *blur;
  int err;

  if (!out_blur)
    return CMP_ERROR_INVALID_ARG;

  blur = (cmp_background_blur_t *)malloc(sizeof(cmp_background_blur_t));
  if (!blur)
    return CMP_ERROR_OOM;

  err = cmp_window_blur_create(&blur->impl);
  if (err != CMP_SUCCESS) {
    free(blur);
    return err;
  }

  *out_blur = blur;
  return CMP_SUCCESS;
}

int cmp_background_blur_destroy(cmp_background_blur_t *blur) {
  int err = CMP_SUCCESS;
  if (!blur)
    return CMP_ERROR_INVALID_ARG;

  if (blur->impl) {
    err = cmp_window_blur_destroy(blur->impl);
  }
  free(blur);
  return err;
}

int cmp_background_blur_set_enabled(cmp_background_blur_t *blur,
                                    cmp_window_t *window, int enabled) {
  if (!blur || !window)
    return CMP_ERROR_INVALID_ARG;

  return cmp_window_blur_set_enabled(blur->impl, window, enabled);
}

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
