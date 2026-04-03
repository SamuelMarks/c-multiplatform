#ifndef CMP_MATERIAL3_PICKERS_MENUS_EXT_H
#define CMP_MATERIAL3_PICKERS_MENUS_EXT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_sys.h"
/* clang-format on */

/**
 * @brief Material 3 Date Picker Variants
 */
typedef enum cmp_m3_date_picker_variant {
  CMP_M3_DATE_PICKER_MODAL,
  CMP_M3_DATE_PICKER_DOCKED
} cmp_m3_date_picker_variant_t;

typedef struct cmp_m3_date_picker_metrics {
  cmp_m3_elevation_level_t elevation;
  cmp_m3_shape_family_t shape;
  float day_selection_size;
} cmp_m3_date_picker_metrics_t;

CMP_API int
cmp_m3_date_picker_resolve(cmp_m3_date_picker_variant_t variant,
                           cmp_m3_date_picker_metrics_t *out_metrics);

/**
 * @brief Material 3 Time Picker Variants
 */
typedef enum cmp_m3_time_picker_variant {
  CMP_M3_TIME_PICKER_DIAL,
  CMP_M3_TIME_PICKER_INPUT
} cmp_m3_time_picker_variant_t;

typedef struct cmp_m3_time_picker_metrics {
  cmp_m3_elevation_level_t elevation;
  cmp_m3_shape_family_t shape;
  float am_pm_segment_height;
} cmp_m3_time_picker_metrics_t;

CMP_API int
cmp_m3_time_picker_resolve(cmp_m3_time_picker_variant_t variant,
                           cmp_m3_time_picker_metrics_t *out_metrics);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_MATERIAL3_PICKERS_MENUS_EXT_H */