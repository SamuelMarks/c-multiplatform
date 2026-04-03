/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_layout.h"
#include <stddef.h>
/* clang-format on */

int cmp_m3_window_size_class_resolve(float width_dp,
                                     cmp_m3_window_size_class_t *out_class) {
  if (!out_class || width_dp < 0.0f) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (width_dp < 600.0f) {
    *out_class = CMP_M3_WINDOW_CLASS_COMPACT;
  } else if (width_dp < 840.0f) {
    *out_class = CMP_M3_WINDOW_CLASS_MEDIUM;
  } else if (width_dp < 1200.0f) {
    *out_class = CMP_M3_WINDOW_CLASS_EXPANDED;
  } else {
    *out_class = CMP_M3_WINDOW_CLASS_LARGE;
  }

  return CMP_SUCCESS;
}

int cmp_m3_window_layout_config_get(cmp_m3_window_size_class_t size_class,
                                    cmp_m3_window_layout_config_t *out_config) {
  if (!out_config) {
    return CMP_ERROR_INVALID_ARG;
  }

  switch (size_class) {
  case CMP_M3_WINDOW_CLASS_COMPACT:
    out_config->columns = 4;
    out_config->margins = 16.0f;
    out_config->gutters = 16.0f;
    break;
  case CMP_M3_WINDOW_CLASS_MEDIUM:
    out_config->columns = 8;
    out_config->margins = 24.0f;
    out_config->gutters = 24.0f;
    break;
  case CMP_M3_WINDOW_CLASS_EXPANDED:
    out_config->columns = 12;
    out_config->margins = 24.0f;
    out_config->gutters = 24.0f;
    break;
  case CMP_M3_WINDOW_CLASS_LARGE:
    out_config->columns = 12;
    out_config->margins = 24.0f; /* Max body width logic typically bounds max
                                    size rather than margins strictly */
    out_config->gutters = 24.0f;
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}

int cmp_m3_layout_pattern_resolve(cmp_m3_window_size_class_t size_class,
                                  int is_feed, int has_supporting_pane,
                                  cmp_m3_layout_pattern_t *out_pattern) {
  if (!out_pattern) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (is_feed) {
    *out_pattern = CMP_M3_PATTERN_FEED;
    return CMP_SUCCESS;
  }

  if (has_supporting_pane) {
    *out_pattern = CMP_M3_PATTERN_SUPPORTING_PANE;
    return CMP_SUCCESS;
  }

  /* Default list detail logic */
  if (size_class == CMP_M3_WINDOW_CLASS_COMPACT ||
      size_class == CMP_M3_WINDOW_CLASS_MEDIUM) {
    *out_pattern = CMP_M3_PATTERN_LIST_DETAIL_STACK;
  } else {
    *out_pattern = CMP_M3_PATTERN_LIST_DETAIL_SIDE_BY_SIDE;
  }

  return CMP_SUCCESS;
}

int cmp_m3_foldable_posture_resolve(int has_hinge, float hinge_angle_degrees,
                                    int is_vertical_hinge,
                                    cmp_m3_posture_t *out_posture) {
  if (!out_posture) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (!has_hinge) {
    *out_posture = CMP_M3_POSTURE_FLAT;
    return CMP_SUCCESS;
  }

  /* Fully open or closed doesn't create a special posture typically, assuming
   * 180 is flat */
  if (hinge_angle_degrees > 170.0f || hinge_angle_degrees < 10.0f) {
    *out_posture = CMP_M3_POSTURE_FLAT;
    return CMP_SUCCESS;
  }

  /* Half-folded postures */
  if (is_vertical_hinge) {
    *out_posture = CMP_M3_POSTURE_BOOK;
  } else {
    *out_posture = CMP_M3_POSTURE_TABLETOP;
  }

  return CMP_SUCCESS;
}