/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_keyboard_avoidance {
  int is_active;
  float current_offset;
};

int cmp_keyboard_avoidance_create(cmp_keyboard_avoidance_t **out_avoider) {
  cmp_keyboard_avoidance_t *avoider;
  if (!out_avoider)
    return CMP_ERROR_INVALID_ARG;

  avoider =
      (cmp_keyboard_avoidance_t *)malloc(sizeof(cmp_keyboard_avoidance_t));
  if (!avoider)
    return CMP_ERROR_OOM;

  avoider->is_active = 0;
  avoider->current_offset = 0.0f;
  *out_avoider = avoider;

  return CMP_SUCCESS;
}

int cmp_keyboard_avoidance_destroy(cmp_keyboard_avoidance_t *avoider) {
  if (!avoider)
    return CMP_ERROR_INVALID_ARG;
  free(avoider);
  return CMP_SUCCESS;
}

int cmp_keyboard_avoidance_compute_offset(cmp_keyboard_avoidance_t *avoider,
                                          float keyboard_height,
                                          float input_bottom_y,
                                          float screen_height,
                                          float *out_y_offset) {
  int err;

  if (!avoider || !out_y_offset)
    return CMP_ERROR_INVALID_ARG;

  if (keyboard_height <= 0.0f) {
    /* Keyboard is hidden */
    *out_y_offset = 0.0f;
    avoider->is_active = 0;
    avoider->current_offset = 0.0f;
    return CMP_SUCCESS;
  }

  err = cmp_ios_calculate_keyboard_avoidance(keyboard_height, input_bottom_y,
                                             screen_height, out_y_offset);
  if (err != CMP_SUCCESS)
    return err;

  avoider->is_active = (*out_y_offset != 0.0f) ? 1 : 0;
  avoider->current_offset = *out_y_offset;

  return CMP_SUCCESS;
}
