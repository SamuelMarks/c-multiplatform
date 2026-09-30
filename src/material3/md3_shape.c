/**
 * @file md3_shape.c
 * @brief Implementation of Material 3 baseline shape scale and Expressive shape
 * tokens.
 */

/* clang-format off */
#include "material3/md3_shape.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

static const float g_shape_radii[MD3_SHAPE_SCALE_COUNT] = {
    0.0f,   /* NONE */
    4.0f,   /* EXTRA_SMALL */
    8.0f,   /* SMALL */
    12.0f,  /* MEDIUM */
    16.0f,  /* LARGE */
    28.0f,  /* EXTRA_LARGE */
    9999.0f /* FULL */
};

static const char *const g_shape_scale_names[MD3_SHAPE_SCALE_COUNT] = {
    "none",         "corner-extra-small", "corner-small", "corner-medium",
    "corner-large", "corner-extra-large", "corner-full"};

struct md3_cut_shape_entry {
  const char *name;
  float radius;
};

static const struct md3_cut_shape_entry g_cut_shapes[3] = {
    {"--md-sys-shape-corner-cut-small", 8.0f},
    {"--md-sys-shape-corner-cut-medium", 16.0f},
    {"--md-sys-shape-corner-cut-large", 24.0f}};

/**
 * @brief Retrieves the corner radius in dp for a given shape scale.
 *
 * @param scale The shape scale.
 * @param out_radius_dp Pointer to receive the radius in dp.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT on bad
 * scale/pointer.
 */
ui_error_t md3_shape_get_corner_radius(enum md3_shape_scale scale,
                                       float *out_radius_dp) {
  if (!out_radius_dp) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if ((int)scale < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (scale >= MD3_SHAPE_SCALE_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_radius_dp = g_shape_radii[scale];
  return UI_ERROR_NONE;
}

/**
 * @brief Injects shape tokens into a design token dictionary.
 *
 * @param is_expressive Non-zero to inject expressive shape tokens.
 * @param dict Pointer to the design token dictionary.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t md3_shape_apply_tokens(int is_expressive,
                                  struct ui_design_token_dict *dict) {
  int i;
  char buf[64];
  ui_error_t rc;

  if (!dict) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < MD3_SHAPE_SCALE_COUNT; i++) {
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "--md-sys-shape-%s", g_shape_scale_names[i]);
#else
    snprintf(buf, sizeof(buf), "--md-sys-shape-%s", g_shape_scale_names[i]);
#endif
    rc = ui_design_token_set_number(dict, buf, g_shape_radii[i]);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  if (is_expressive) {
    for (i = 0; i < 3; i++) {
      rc = ui_design_token_set_number(dict, g_cut_shapes[i].name,
                                      g_cut_shapes[i].radius);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  }

  return UI_ERROR_NONE;
}
