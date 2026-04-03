/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_communication.h"
#include <stddef.h>
/* clang-format on */

int cmp_m3_badge_resolve(cmp_m3_badge_variant_t variant,
                         cmp_m3_badge_metrics_t *out_metrics) {
  if (!out_metrics) {
    return CMP_ERROR_INVALID_ARG;
  }

  switch (variant) {
  case CMP_M3_BADGE_SMALL:
    out_metrics->width = 6.0f;
    out_metrics->height = 6.0f;
    out_metrics->padding_left_right = 0.0f;
    out_metrics->shape = CMP_M3_SHAPE_FULL; /* Circular */
    break;
  case CMP_M3_BADGE_LARGE:
    out_metrics->width = 0.0f; /* Dynamic, based on content */
    out_metrics->height = 16.0f;
    out_metrics->padding_left_right = 4.0f;
    out_metrics->shape = CMP_M3_SHAPE_FULL; /* Pill */
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}

int cmp_m3_progress_resolve(cmp_m3_progress_variant_t variant,
                            cmp_m3_progress_metrics_t *out_metrics) {
  if (!out_metrics) {
    return CMP_ERROR_INVALID_ARG;
  }

  switch (variant) {
  case CMP_M3_PROGRESS_LINEAR:
    out_metrics->track_thickness = 4.0f;
    out_metrics->default_size = 0.0f; /* Typically stretches to parent */
    break;
  case CMP_M3_PROGRESS_CIRCULAR:
    out_metrics->track_thickness = 4.0f;
    out_metrics->default_size =
        48.0f; /* Container 48x48, indicator inside usually 40x40 or 48x48 */
    break;
  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}

int cmp_m3_snackbar_resolve(cmp_m3_snackbar_metrics_t *out_metrics) {
  if (!out_metrics) {
    return CMP_ERROR_INVALID_ARG;
  }

  out_metrics->padding_left_right = 16.0f;
  out_metrics->elevation = CMP_M3_ELEVATION_LEVEL_3;
  out_metrics->shape = CMP_M3_SHAPE_EXTRA_SMALL;

  return CMP_SUCCESS;
}

int cmp_m3_pull_to_refresh_resolve(
    cmp_m3_pull_to_refresh_metrics_t *out_metrics) {
  if (!out_metrics) {
    return CMP_ERROR_INVALID_ARG;
  }

  out_metrics->container_size = 48.0f;
  out_metrics->indicator_size = 24.0f;

  return CMP_SUCCESS;
}