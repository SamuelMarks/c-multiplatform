/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_keyboard_avoidance {
  int is_active;
  float current_offset;
};

/**
 * @brief cmp_keyboard_avoidance_create
 *
 * @param out_avoider Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_keyboard_avoidance_create(cmp_keyboard_avoidance_t **out_avoider) {
  int rc = CMP_SUCCESS;
  cmp_keyboard_avoidance_t *avoider = NULL;

  if (!out_avoider) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_keyboard_avoidance_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_keyboard_avoidance_t), (void **)&avoider);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_keyboard_avoidance_create: Out of memory\n");
    return rc;
  }

  avoider->is_active = 0;
  avoider->current_offset = 0.0f;
  *out_avoider = avoider;

  return rc;
}

/**
 * @brief cmp_keyboard_avoidance_destroy
 *
 * @param avoider Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_keyboard_avoidance_destroy(cmp_keyboard_avoidance_t *avoider) {
  int rc = CMP_SUCCESS;

  if (!avoider) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_keyboard_avoidance_destroy: Invalid argument\n");
    return rc;
  }

  CMP_FREE(avoider);
  return rc;
}

/**
 * @brief cmp_keyboard_avoidance_compute_offset
 *
 * @param avoider Parameter description.
 * @param keyboard_height Parameter description.
 * @param input_bottom_y Parameter description.
 * @param screen_height Parameter description.
 * @param out_y_offset Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_keyboard_avoidance_compute_offset(cmp_keyboard_avoidance_t *avoider,
                                          float keyboard_height,
                                          float input_bottom_y,
                                          float screen_height,
                                          float *out_y_offset) {
  int rc = CMP_SUCCESS;

  if (!avoider || !out_y_offset) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_keyboard_avoidance_compute_offset: Invalid argument\n");
    return rc;
  }

  if (keyboard_height <= 0.0f) {
    /* Keyboard is hidden */
    *out_y_offset = 0.0f;
    avoider->is_active = 0;
    avoider->current_offset = 0.0f;
    return rc;
  }

  rc = cmp_ios_calculate_keyboard_avoidance(keyboard_height, input_bottom_y,
                                            screen_height, out_y_offset);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_keyboard_avoidance_compute_offset: "
              "cmp_ios_calculate_keyboard_avoidance failed\n");
    return rc;
  }

  avoider->is_active = (*out_y_offset != 0.0f) ? 1 : 0;
  avoider->current_offset = *out_y_offset;

  return rc;
}
