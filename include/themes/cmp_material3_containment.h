#ifndef CMP_MATERIAL3_CONTAINMENT_H
#define CMP_MATERIAL3_CONTAINMENT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_sys.h"
/* clang-format on */

/**
 * @brief Material 3 Card Variants
 */
typedef enum cmp_m3_card_variant {
  CMP_M3_CARD_ELEVATED,
  CMP_M3_CARD_FILLED,
  CMP_M3_CARD_OUTLINED
} cmp_m3_card_variant_t;

typedef struct cmp_m3_card_metrics {
  cmp_m3_elevation_level_t elevation;
  cmp_m3_shape_family_t shape;
  float border_thickness;
} cmp_m3_card_metrics_t;

CMP_API int cmp_m3_card_resolve(cmp_m3_card_variant_t variant,
                                cmp_m3_card_metrics_t *out_metrics);

/**
 * @brief Material 3 Dialog Variants
 */
typedef enum cmp_m3_dialog_variant {
  CMP_M3_DIALOG_BASIC,
  CMP_M3_DIALOG_FULL_SCREEN
} cmp_m3_dialog_variant_t;

typedef struct cmp_m3_dialog_metrics {
  cmp_m3_elevation_level_t elevation;
  cmp_m3_shape_family_t shape;
  float padding_all;
} cmp_m3_dialog_metrics_t;

CMP_API int cmp_m3_dialog_resolve(cmp_m3_dialog_variant_t variant,
                                  cmp_m3_dialog_metrics_t *out_metrics);

/**
 * @brief Material 3 Divider Variants
 */
typedef enum cmp_m3_divider_variant {
  CMP_M3_DIVIDER_FULL_BLEED,
  CMP_M3_DIVIDER_INSET,
  CMP_M3_DIVIDER_MIDDLE_INSET
} cmp_m3_divider_variant_t;

typedef struct cmp_m3_divider_metrics {
  float thickness;
  float inset_start;
  float inset_end;
} cmp_m3_divider_metrics_t;

CMP_API int cmp_m3_divider_resolve(cmp_m3_divider_variant_t variant,
                                   cmp_m3_divider_metrics_t *out_metrics);

/**
 * @brief Material 3 Tooltip Variants
 */
typedef enum cmp_m3_tooltip_variant {
  CMP_M3_TOOLTIP_PLAIN,
  CMP_M3_TOOLTIP_RICH
} cmp_m3_tooltip_variant_t;

typedef struct cmp_m3_tooltip_metrics {
  float height; /* 0 for auto */
  float padding_all;
  cmp_m3_elevation_level_t elevation;
  cmp_m3_shape_family_t shape;
} cmp_m3_tooltip_metrics_t;

CMP_API int cmp_m3_tooltip_resolve(cmp_m3_tooltip_variant_t variant,
                                   cmp_m3_tooltip_metrics_t *out_metrics);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_MATERIAL3_CONTAINMENT_H */