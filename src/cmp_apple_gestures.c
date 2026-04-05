/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_apple_gestures {
  int pinch_enabled;
  int rotation_enabled;
  int swipe_enabled;
};

int cmp_apple_gestures_create(cmp_apple_gestures_t **out_gestures) {
  cmp_apple_gestures_t *gest;

  if (!out_gestures)
    return CMP_ERROR_INVALID_ARG;

  gest = (cmp_apple_gestures_t *)malloc(sizeof(cmp_apple_gestures_t));
  if (!gest)
    return CMP_ERROR_OOM;

  gest->pinch_enabled = 0;
  gest->rotation_enabled = 0;
  gest->swipe_enabled = 0;

  *out_gestures = gest;
  return CMP_SUCCESS;
}

int cmp_apple_gestures_destroy(cmp_apple_gestures_t *gestures) {
  if (!gestures)
    return CMP_ERROR_INVALID_ARG;

  free(gestures);
  return CMP_SUCCESS;
}

int cmp_apple_gestures_enable(cmp_apple_gestures_t *gestures,
                              cmp_window_t *window, int enable_pinch,
                              int enable_rotation, int enable_swipe) {
  int err;
  if (!gestures || !window)
    return CMP_ERROR_INVALID_ARG;

  err = cmp_window_apple_enable_gestures(window, enable_pinch, enable_rotation,
                                         enable_swipe);
  if (err != CMP_SUCCESS)
    return err;

  gestures->pinch_enabled = enable_pinch;
  gestures->rotation_enabled = enable_rotation;
  gestures->swipe_enabled = enable_swipe;

  return CMP_SUCCESS;
}
