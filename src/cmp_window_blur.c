/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

#define CMP_MAX_BLURRED_WINDOWS 32

struct cmp_window_blur {
  cmp_window_t *blurred_windows[CMP_MAX_BLURRED_WINDOWS];
  int count;
};

int cmp_window_blur_create(cmp_window_blur_t **out_blur) {
  cmp_window_blur_t *blur;

  if (!out_blur) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_window_blur_t), (void **)&blur) != 0) {
    return CMP_ERROR_OOM;
  }

  memset(blur, 0, sizeof(cmp_window_blur_t));

  *out_blur = blur;
  return CMP_SUCCESS;
}

int cmp_window_blur_destroy(cmp_window_blur_t *blur) {
  if (!blur) {
    return CMP_ERROR_INVALID_ARG;
  }

  CMP_FREE(blur);
  return CMP_SUCCESS;
}

int cmp_window_blur_set_enabled(cmp_window_blur_t *blur, cmp_window_t *window,
                                int enabled) {
  int i;
  int found = 0;

  if (!blur || !window) {
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < blur->count; i++) {
    if (blur->blurred_windows[i] == window) {
      found = 1;
      if (!enabled) {
        blur->blurred_windows[i] = blur->blurred_windows[blur->count - 1];
        blur->count--;
        return CMP_SUCCESS;
      }
      break;
    }
  }

  if (enabled && !found) {
    if (blur->count >= CMP_MAX_BLURRED_WINDOWS) {
      return CMP_ERROR_BOUNDS;
    }
    blur->blurred_windows[blur->count++] = window;
  }

  return CMP_SUCCESS;
}

int cmp_window_blur_is_enabled(const cmp_window_blur_t *blur,
                               const cmp_window_t *window, int *out_enabled) {
  int i;

  if (!blur || !window || !out_enabled) {
    return CMP_ERROR_INVALID_ARG;
  }

  *out_enabled = 0;
  for (i = 0; i < blur->count; i++) {
    if (blur->blurred_windows[i] == window) {
      *out_enabled = 1;
      break;
    }
  }

  return CMP_SUCCESS;
}
