#ifndef CMP_MATERIAL3_PICKERS_MENUS_H
#define CMP_MATERIAL3_PICKERS_MENUS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_sys.h"
/* clang-format on */

/**
 * @brief Material 3 Menu Metrics
 */
typedef struct cmp_m3_menu_metrics {
  cmp_m3_elevation_level_t elevation;
  cmp_m3_shape_family_t shape;
} cmp_m3_menu_metrics_t;

CMP_API int cmp_m3_menu_resolve(cmp_m3_menu_metrics_t *out_metrics);

/**
 * @brief Material 3 Sheet Variants
 */
typedef enum cmp_m3_sheet_variant {
  CMP_M3_SHEET_BOTTOM_STANDARD,
  CMP_M3_SHEET_BOTTOM_MODAL,
  CMP_M3_SHEET_SIDE_STANDARD,
  CMP_M3_SHEET_SIDE_MODAL,
  CMP_M3_SHEET_SIDE_DETACHED
} cmp_m3_sheet_variant_t;

/**
 * @brief Evaluated constraints for a Material 3 Sheet
 */
typedef struct cmp_m3_sheet_metrics {
  cmp_m3_elevation_level_t elevation;
  cmp_m3_shape_family_t shape;
  int has_scrim;
  int has_drag_handle;
} cmp_m3_sheet_metrics_t;

CMP_API int cmp_m3_sheet_resolve(cmp_m3_sheet_variant_t variant,
                                 cmp_m3_sheet_metrics_t *out_metrics);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_MATERIAL3_PICKERS_MENUS_H */