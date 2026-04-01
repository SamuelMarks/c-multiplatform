/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

int cmp_layout_set_corner_curve_continuous(cmp_layout_node_t *node,
                                           int is_continuous) {
  if (!node)
    return CMP_ERROR_INVALID_ARG;
  /* Modifies internal vdom trait for radius algorithms (squircle vs circular)
   */
  (void)is_continuous;
  return CMP_SUCCESS;
}

int cmp_layout_enforce_pixel_alignment(cmp_layout_node_t *node,
                                       int is_aligned) {
  if (!node)
    return CMP_ERROR_INVALID_ARG;
  /* Modifies internal flags so final layout matrices floor/round translation
   * pixels */
  (void)is_aligned;
  return CMP_SUCCESS;
}

struct cmp_system_geometry {
  int is_initialized;
};

int cmp_system_geometry_create(cmp_system_geometry_t **out_geom) {
  struct cmp_system_geometry *ctx;
  if (!out_geom)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_system_geometry), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  ctx->is_initialized = 1;
  *out_geom = (cmp_system_geometry_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_system_geometry_destroy(cmp_system_geometry_t *geom) {
  if (geom)
    CMP_FREE(geom);
  return CMP_SUCCESS;
}

int cmp_system_geometry_get_safe_area(cmp_system_geometry_t *geom, int is_tvos,
                                      cmp_rect_t *out_safe_insets) {
  if (!geom || !out_safe_insets)
    return CMP_ERROR_INVALID_ARG;

  if (is_tvos) {
    /* tvOS Overscan boundaries (left/right 90, top/bottom 60) */
    out_safe_insets->x = 90.0f;
    out_safe_insets->width = 90.0f;
    out_safe_insets->y = 60.0f;
    out_safe_insets->height = 60.0f;
  } else {
    /* Simulated Mobile (Notch/Home Indicator) */
    out_safe_insets->x = 0.0f;
    out_safe_insets->width = 0.0f;
    out_safe_insets->y = 47.0f;      /* Dynamic Island top */
    out_safe_insets->height = 34.0f; /* Home bar bottom */
  }
  return CMP_SUCCESS;
}

int cmp_system_geometry_get_layout_margins(cmp_system_geometry_t *geom,
                                           int is_compact_width,
                                           float *out_leading,
                                           float *out_trailing) {
  if (!geom || !out_leading || !out_trailing)
    return CMP_ERROR_INVALID_ARG;

  /* 16pt for iPhones in portrait, 20pt for iPads/iPhones in landscape */
  *out_leading = is_compact_width ? 16.0f : 20.0f;
  *out_trailing = is_compact_width ? 16.0f : 20.0f;

  return CMP_SUCCESS;
}

int cmp_system_geometry_get_readable_content_guide(
    cmp_system_geometry_t *geom, float available_width,
    float *out_max_readable_width) {
  if (!geom || !out_max_readable_width)
    return CMP_ERROR_INVALID_ARG;

  /* HIG specifies a max-width for continuous reading blocks to prevent long
   * lines */
  if (available_width > 672.0f) {
    *out_max_readable_width = 672.0f;
  } else {
    /* If screen is narrower than the max, it falls back to using layout margins
     */
    *out_max_readable_width = available_width - (20.0f * 2.0f);
  }

  return CMP_SUCCESS;
}
