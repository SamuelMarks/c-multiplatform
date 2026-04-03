#ifndef CMP_MATERIAL3_COMMUNICATION_H
#define CMP_MATERIAL3_COMMUNICATION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_sys.h"
/* clang-format on */

/**
 * @brief Material 3 Badge Variants
 */
typedef enum cmp_m3_badge_variant {
  CMP_M3_BADGE_SMALL,
  CMP_M3_BADGE_LARGE
} cmp_m3_badge_variant_t;

/**
 * @brief Evaluated constraints for a Material 3 Badge
 */
typedef struct cmp_m3_badge_metrics {
  float width; /* 0 if dynamic */
  float height;
  float padding_left_right;
  cmp_m3_shape_family_t shape;
} cmp_m3_badge_metrics_t;

CMP_API int cmp_m3_badge_resolve(cmp_m3_badge_variant_t variant,
                                 cmp_m3_badge_metrics_t *out_metrics);

/**
 * @brief Material 3 Progress Indicator Variants
 */
typedef enum cmp_m3_progress_variant {
  CMP_M3_PROGRESS_LINEAR,
  CMP_M3_PROGRESS_CIRCULAR
} cmp_m3_progress_variant_t;

/**
 * @brief Evaluated constraints for a Material 3 Progress Indicator
 */
typedef struct cmp_m3_progress_metrics {
  float track_thickness;
  float default_size; /* Width for linear, diameter for circular */
} cmp_m3_progress_metrics_t;

CMP_API int cmp_m3_progress_resolve(cmp_m3_progress_variant_t variant,
                                    cmp_m3_progress_metrics_t *out_metrics);

/**
 * @brief Evaluated constraints for a Material 3 Snackbar
 */
typedef struct cmp_m3_snackbar_metrics {
  float padding_left_right;
  cmp_m3_elevation_level_t elevation;
  cmp_m3_shape_family_t shape;
} cmp_m3_snackbar_metrics_t;

CMP_API int cmp_m3_snackbar_resolve(cmp_m3_snackbar_metrics_t *out_metrics);

/**
 * @brief Evaluated constraints for a Material 3 Pull to Refresh
 */
typedef struct cmp_m3_pull_to_refresh_metrics {
  float container_size;
  float indicator_size;
} cmp_m3_pull_to_refresh_metrics_t;

CMP_API int
cmp_m3_pull_to_refresh_resolve(cmp_m3_pull_to_refresh_metrics_t *out_metrics);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_MATERIAL3_COMMUNICATION_H */