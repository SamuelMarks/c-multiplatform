/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_ios_features {
  int is_ready;
};

/**
 * @brief cmp_ios_features_create
 *
 * @param out_features Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ios_features_create(cmp_ios_features_t **out_features) {
  int rc = CMP_SUCCESS;
  struct cmp_ios_features *ctx = NULL;

  if (!out_features) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ios_features_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_ios_features), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_ios_features_create: Out of memory\n");
    return rc;
  }

  ctx->is_ready = 1;
  *out_features = (cmp_ios_features_t *)ctx;
  return rc;
}

/**
 * @brief cmp_ios_features_destroy
 *
 * @param features_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ios_features_destroy(cmp_ios_features_t *features_opaque) {
  int rc = CMP_SUCCESS;

  if (!features_opaque) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ios_features_destroy: Invalid argument\n");
    return rc;
  }

  rc = CMP_FREE(features_opaque);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_ios_features_destroy: CMP_FREE failed\n");
  }
  return rc;
}

/**
 * @brief cmp_ios_calculate_keyboard_avoidance
 *
 * @param keyboard_height Parameter description.
 * @param input_y Parameter description.
 * @param view_height Parameter description.
 * @param out_scroll_adjustment Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ios_calculate_keyboard_avoidance(float keyboard_height, float input_y,
                                         float view_height,
                                         float *out_scroll_adjustment) {
  int rc = CMP_SUCCESS;
  float visible_height, overlap;

  if (!out_scroll_adjustment || keyboard_height < 0.0f || input_y < 0.0f ||
      view_height <= 0.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_ios_calculate_keyboard_avoidance: Invalid argument\n");
    return rc;
  }

  visible_height = view_height - keyboard_height;

  /* Add 16pt margin padding below the input */
  overlap = (input_y + 16.0f) - visible_height;

  if (overlap > 0.0f) {
    /* Push the scroll up by the exact overlap amount */
    *out_scroll_adjustment = overlap;
  } else {
    *out_scroll_adjustment = 0.0f;
  }
  return rc;
}

/**
 * @brief cmp_ios_evaluate_pull_to_refresh
 *
 * @param current_overscroll_y Parameter description.
 * @param out_spinner_opacity Parameter description.
 * @param out_should_trigger Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ios_evaluate_pull_to_refresh(float current_overscroll_y,
                                     float *out_spinner_opacity,
                                     int *out_should_trigger) {
  int rc = CMP_SUCCESS;
  float progress;

  if (!out_spinner_opacity || !out_should_trigger) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ios_evaluate_pull_to_refresh: Invalid argument\n");
    return rc;
  }

  /* HIG: threshold is around 100-120pts of negative overscroll */
  if (current_overscroll_y >= 0.0f) {
    *out_spinner_opacity = 0.0f;
    *out_should_trigger = 0;
    return rc;
  }

  /* Inverse because scrolling up is negative Y */
  progress = -current_overscroll_y / 100.0f;
  if (progress > 1.0f)
    progress = 1.0f;

  *out_spinner_opacity = progress;
  *out_should_trigger = (current_overscroll_y <= -100.0f) ? 1 : 0;

  return rc;
}

/**
 * @brief cmp_ios_evaluate_sheet_detent_snap
 *
 * @param current_y Parameter description.
 * @param screen_height Parameter description.
 * @param out_snapped_detent Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ios_evaluate_sheet_detent_snap(float current_y, float screen_height,
                                       cmp_sheet_detent_t *out_snapped_detent) {
  int rc = CMP_SUCCESS;
  float half, full, dist_half, dist_full;

  if (screen_height <= 0.0f || !out_snapped_detent) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_ios_evaluate_sheet_detent_snap: Invalid argument\n");
    return rc;
  }

  /* Y origin is top-down (0 is top of screen).
     Medium Detent rests roughly halfway down. */
  half = screen_height * 0.5f;
  full = 0.0f; /* Reaches the top */

  dist_half = (float)abs((int)(current_y - half));
  dist_full = (float)abs((int)(current_y - full));

  if (dist_full < dist_half) {
    *out_snapped_detent = CMP_SHEET_DETENT_LARGE;
  } else {
    *out_snapped_detent = CMP_SHEET_DETENT_MEDIUM;
  }

  return rc;
}

/**
 * @brief cmp_ios_mitigate_reachability_offset
 *
 * @param io_touch_y Parameter description.
 * @param reachability_offset Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ios_mitigate_reachability_offset(float *io_touch_y,
                                         float reachability_offset) {
  int rc = CMP_SUCCESS;

  if (!io_touch_y || reachability_offset < 0.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_ios_mitigate_reachability_offset: Invalid argument\n");
    return rc;
  }

  /* Apple pushes the whole root UIWindow down by half the screen height.
     Physical touches on the screen need to be mapped back up by that offset to
     hit the correct elements. */
  *io_touch_y -= reachability_offset;

  return rc;
}

/**
 * @brief cmp_ios_evaluate_context_menu_peek
 *
 * @param pressure Parameter description.
 * @param out_preview_scale Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ios_evaluate_context_menu_peek(float pressure,
                                       float *out_preview_scale) {
  int rc = CMP_SUCCESS;

  if (!out_preview_scale || pressure < 0.0f || pressure > 1.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_ios_evaluate_context_menu_peek: Invalid argument\n");
    return rc;
  }

  /* Starts at ~0.6, scales to 0.8 as pressure increases. Pops to 1.0 full
   * navigation if pressure hits max (not handled here) */
  *out_preview_scale = 0.6f + (pressure * 0.2f);

  return rc;
}
