/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_ios_features {
  int is_ready;
};

int cmp_ios_features_create(cmp_ios_features_t **out_features) {
  struct cmp_ios_features *ctx;
  if (!out_features)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_ios_features), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->is_ready = 1;
  *out_features = (cmp_ios_features_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_ios_features_destroy(cmp_ios_features_t *features_opaque) {
  if (features_opaque)
    CMP_FREE(features_opaque);
  return CMP_SUCCESS;
}

int cmp_ios_calculate_keyboard_avoidance(float keyboard_height, float input_y,
                                         float view_height,
                                         float *out_scroll_adjustment) {
  float visible_height, overlap;

  if (!out_scroll_adjustment || keyboard_height < 0.0f || input_y < 0.0f ||
      view_height <= 0.0f)
    return CMP_ERROR_INVALID_ARG;

  visible_height = view_height - keyboard_height;

  /* Add 16pt margin padding below the input */
  overlap = (input_y + 16.0f) - visible_height;

  if (overlap > 0.0f) {
    /* Push the scroll up by the exact overlap amount */
    *out_scroll_adjustment = overlap;
  } else {
    *out_scroll_adjustment = 0.0f;
  }
  return CMP_SUCCESS;
}

int cmp_ios_evaluate_pull_to_refresh(float current_overscroll_y,
                                     float *out_spinner_opacity,
                                     int *out_should_trigger) {
  float progress;
  if (!out_spinner_opacity || !out_should_trigger)
    return CMP_ERROR_INVALID_ARG;

  /* HIG: threshold is around 100-120pts of negative overscroll */
  if (current_overscroll_y >= 0.0f) {
    *out_spinner_opacity = 0.0f;
    *out_should_trigger = 0;
    return CMP_SUCCESS;
  }

  /* Inverse because scrolling up is negative Y */
  progress = -current_overscroll_y / 100.0f;
  if (progress > 1.0f)
    progress = 1.0f;

  *out_spinner_opacity = progress;
  *out_should_trigger = (current_overscroll_y <= -100.0f) ? 1 : 0;

  return CMP_SUCCESS;
}

int cmp_ios_evaluate_sheet_detent_snap(float current_y, float screen_height,
                                       cmp_sheet_detent_t *out_snapped_detent) {
  float half, full, dist_half, dist_full;

  if (screen_height <= 0.0f || !out_snapped_detent)
    return CMP_ERROR_INVALID_ARG;

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

  return CMP_SUCCESS;
}

int cmp_ios_mitigate_reachability_offset(float *io_touch_y,
                                         float reachability_offset) {
  if (!io_touch_y || reachability_offset < 0.0f)
    return CMP_ERROR_INVALID_ARG;

  /* Apple pushes the whole root UIWindow down by half the screen height.
     Physical touches on the screen need to be mapped back up by that offset to
     hit the correct elements. */
  *io_touch_y -= reachability_offset;

  return CMP_SUCCESS;
}

int cmp_ios_evaluate_context_menu_peek(float pressure,
                                       float *out_preview_scale) {
  if (!out_preview_scale || pressure < 0.0f || pressure > 1.0f)
    return CMP_ERROR_INVALID_ARG;

  /* Starts at ~0.6, scales to 0.8 as pressure increases. Pops to 1.0 full
   * navigation if pressure hits max (not handled here) */
  *out_preview_scale = 0.6f + (pressure * 0.2f);

  return CMP_SUCCESS;
}
