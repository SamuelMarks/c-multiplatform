#ifndef CMP_MATERIAL3_INFORMATION_H
#define CMP_MATERIAL3_INFORMATION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_sys.h"
/* clang-format on */

/**
 * @brief Material 3 Chip Variants
 */
typedef enum cmp_m3_chip_variant {
  CMP_M3_CHIP_ASSIST,
  CMP_M3_CHIP_FILTER,
  CMP_M3_CHIP_INPUT,
  CMP_M3_CHIP_SUGGESTION
} cmp_m3_chip_variant_t;

typedef struct cmp_m3_chip_metrics {
  float height;
  float padding_left;
  float padding_right;
  float icon_size;
  float avatar_size;
  cmp_m3_shape_family_t shape;
  float border_thickness;
  cmp_m3_elevation_level_t elevation;
} cmp_m3_chip_metrics_t;

CMP_API int cmp_m3_chip_resolve(cmp_m3_chip_variant_t variant, int is_elevated,
                                cmp_m3_chip_metrics_t *out_metrics);

/**
 * @brief Material 3 List Variants
 */
typedef enum cmp_m3_list_variant {
  CMP_M3_LIST_ONE_LINE,
  CMP_M3_LIST_TWO_LINE,
  CMP_M3_LIST_THREE_LINE
} cmp_m3_list_variant_t;

typedef struct cmp_m3_list_metrics {
  float height;
  float padding_left;
  float padding_right;
} cmp_m3_list_metrics_t;

CMP_API int cmp_m3_list_resolve(cmp_m3_list_variant_t variant,
                                cmp_m3_list_metrics_t *out_metrics);

/**
 * @brief Material 3 Carousel Variants
 */
typedef enum cmp_m3_carousel_variant {
  CMP_M3_CAROUSEL_MULTI_BROWSE,
  CMP_M3_CAROUSEL_HERO,
  CMP_M3_CAROUSEL_UNCONTAINED
} cmp_m3_carousel_variant_t;

typedef struct cmp_m3_carousel_metrics {
  float item_spacing;
  cmp_m3_shape_family_t contained_shape;
} cmp_m3_carousel_metrics_t;

CMP_API int cmp_m3_carousel_resolve(cmp_m3_carousel_variant_t variant,
                                    cmp_m3_carousel_metrics_t *out_metrics);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_MATERIAL3_INFORMATION_H */