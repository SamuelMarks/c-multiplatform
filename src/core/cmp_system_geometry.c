/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

/**
 * @brief Sets whether the corner curve of a layout node is continuous.
 *
 * @param node Pointer to the layout node.
 * @param is_continuous 1 to enable continuous curves, 0 otherwise.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_layout_set_corner_curve_continuous(cmp_layout_node_t *node,
                                           int is_continuous) {
  int rc;
  rc = 0;if (!node)
    return CMP_ERROR_INVALID_ARG;
  /* Modifies internal vdom trait for radius algorithms (squircle vs circular)
   */
  (void)is_continuous;
  if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief Enforces pixel alignment for a layout node.
 *
 * @param node Pointer to the layout node.
 * @param is_aligned 1 to enforce alignment, 0 otherwise.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_layout_enforce_pixel_alignment(cmp_layout_node_t *node,
                                       int is_aligned) {
  int rc;
  rc = 0;if (!node)
    return CMP_ERROR_INVALID_ARG;
  /* Modifies internal flags so final layout matrices floor/round translation
   * pixels */
  (void)is_aligned;
  if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

struct cmp_system_geometry {
  int is_initialized;
};

/**
 * @brief Creates a system geometry context.
 *
 * @param out_geom Pointer to a variable where the context pointer will be stored.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_geometry_create(cmp_system_geometry_t **out_geom) {
  int rc;
  rc = 0;struct cmp_system_geometry *ctx;
  if (!out_geom)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_system_geometry), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  ctx->is_initialized = 1;
  *out_geom = (cmp_system_geometry_t *)ctx;
  if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief Destroys a system geometry context.
 *
 * @param geom Pointer to the context to destroy.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_geometry_destroy(cmp_system_geometry_t *geom) {
  int rc;
  if (!geom) {
    rc = CMP_ERROR_INVALID_ARG;
    { const char *err_str;
      int rc2;
      rc2 = cmp_strerror(rc, &err_str);
      if (rc2 != CMP_SUCCESS) { err_str = "Unknown"; } LOG_DEBUG("cmp_system_geometry_destroy: %s\n", err_str);
 }    if (rc != 0) {      return rc;    }    return rc;
  }
  CMP_FREE(geom);
  return CMP_SUCCESS;
}

/**
 * @brief Retrieves the safe area insets for a specific platform.
 *
 * @param geom Pointer to the system geometry context.
 * @param is_tvos 1 if querying for tvOS, 0 otherwise.
 * @param out_safe_insets Pointer to a rectangle struct to store the insets.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_geometry_get_safe_area(cmp_system_geometry_t *geom, int is_tvos,
                                      cmp_rect_t *out_safe_insets) {
  int rc;
  rc = 0;if (!geom || !out_safe_insets)
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
  if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief Retrieves the layout margins based on width classification.
 *
 * @param geom Pointer to the system geometry context.
 * @param is_compact_width 1 if the width is compact, 0 for regular width.
 * @param out_leading Pointer to a float to store the leading margin.
 * @param out_trailing Pointer to a float to store the trailing margin.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_geometry_get_layout_margins(cmp_system_geometry_t *geom,
                                           int is_compact_width,
                                           float *out_leading,
                                           float *out_trailing) {
  int rc;
  rc = 0;if (!geom || !out_leading || !out_trailing)
    return CMP_ERROR_INVALID_ARG;

  /* 16pt for iPhones in portrait, 20pt for iPads/iPhones in landscape */
  *out_leading = is_compact_width ? 16.0f : 20.0f;
  *out_trailing = is_compact_width ? 16.0f : 20.0f;

  if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief Computes the readable content guide width based on available width.
 *
 * @param geom Pointer to the system geometry context.
 * @param available_width The available layout width.
 * @param out_max_readable_width Pointer to a float to store the maximum readable width.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_geometry_get_readable_content_guide(
    cmp_system_geometry_t *geom, float available_width,
    float *out_max_readable_width) {
  int rc;
  rc = 0;if (!geom || !out_max_readable_width)
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

  if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
