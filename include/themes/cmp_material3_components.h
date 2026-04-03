#ifndef CMP_MATERIAL3_COMPONENTS_H
#define CMP_MATERIAL3_COMPONENTS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_sys.h"
/* clang-format on */

/**
 * @brief Material 3 Button Variants
 */
typedef enum cmp_m3_button_variant {
  CMP_M3_BUTTON_ELEVATED,
  CMP_M3_BUTTON_FILLED,
  CMP_M3_BUTTON_FILLED_TONAL,
  CMP_M3_BUTTON_OUTLINED,
  CMP_M3_BUTTON_TEXT
} cmp_m3_button_variant_t;

/**
 * @brief Evaluated constraints for a Material 3 Button
 */
typedef struct cmp_m3_button_metrics {
  float height;
  float padding_left;
  float padding_right;
  float padding_leading_with_icon;
  float gap_with_icon;
  float icon_size;
  cmp_m3_elevation_level_t elevation_base;
  cmp_m3_elevation_level_t elevation_hover;
  cmp_m3_elevation_level_t elevation_pressed;
  cmp_m3_elevation_level_t elevation_disabled;
  cmp_m3_shape_family_t shape;
  float border_thickness; /* 0 if none */
} cmp_m3_button_metrics_t;

/**
 * @brief Resolve metrics for a Material 3 Button variant
 */
CMP_API int cmp_m3_button_resolve(cmp_m3_button_variant_t variant,
                                  int has_leading_icon,
                                  cmp_m3_button_metrics_t *out_metrics);

/**
 * @brief Material 3 FAB Variants
 */
typedef enum cmp_m3_fab_variant {
  CMP_M3_FAB_STANDARD,
  CMP_M3_FAB_SMALL,
  CMP_M3_FAB_LARGE,
  CMP_M3_FAB_EXTENDED
} cmp_m3_fab_variant_t;

/**
 * @brief Evaluated constraints for a Material 3 FAB
 */
typedef struct cmp_m3_fab_metrics {
  float container_width; /* 0 means dynamic (for extended) */
  float container_height;
  float icon_size;
  float padding_left;
  float padding_right;
  cmp_m3_elevation_level_t elevation_base;
  cmp_m3_elevation_level_t elevation_hover;
  cmp_m3_elevation_level_t elevation_pressed;
  cmp_m3_shape_family_t shape;
} cmp_m3_fab_metrics_t;

/**
 * @brief Resolve metrics for a Material 3 FAB variant
 */
CMP_API int cmp_m3_fab_resolve(cmp_m3_fab_variant_t variant, int is_lowered,
                               cmp_m3_fab_metrics_t *out_metrics);

/**
 * @brief Material 3 Icon Button Variants
 */
typedef enum cmp_m3_icon_button_variant {
  CMP_M3_ICON_BUTTON_STANDARD,
  CMP_M3_ICON_BUTTON_FILLED,
  CMP_M3_ICON_BUTTON_FILLED_TONAL,
  CMP_M3_ICON_BUTTON_OUTLINED
} cmp_m3_icon_button_variant_t;

/**
 * @brief Evaluated constraints for a Material 3 Icon Button
 */
typedef struct cmp_m3_icon_button_metrics {
  float target_size;
  float footprint_size;
  float icon_size;
  float border_thickness;
  cmp_m3_shape_family_t shape;
} cmp_m3_icon_button_metrics_t;

/**
 * @brief Resolve metrics for a Material 3 Icon Button
 */
CMP_API int
cmp_m3_icon_button_resolve(cmp_m3_icon_button_variant_t variant,
                           cmp_m3_icon_button_metrics_t *out_metrics);

/**
 * @brief Material 3 Segmented Button Metrics
 */
typedef struct cmp_m3_segmented_button_metrics {
  float height;
  float border_thickness;
  float checkmark_size;
  float icon_size;
} cmp_m3_segmented_button_metrics_t;

/**
 * @brief Resolve metrics for a Material 3 Segmented Button container
 */
CMP_API int
cmp_m3_segmented_button_resolve(cmp_m3_segmented_button_metrics_t *out_metrics);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_MATERIAL3_COMPONENTS_H */