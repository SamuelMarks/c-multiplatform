#ifndef CMP_MATERIAL3_NAVIGATION_H
#define CMP_MATERIAL3_NAVIGATION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_sys.h"
/* clang-format on */

/**
 * @brief Material 3 Bottom App Bar Metrics
 */
typedef struct cmp_m3_bottom_app_bar_metrics {
  float height;
  cmp_m3_elevation_level_t elevation;
  int max_action_items;
} cmp_m3_bottom_app_bar_metrics_t;

CMP_API int
cmp_m3_bottom_app_bar_resolve(cmp_m3_bottom_app_bar_metrics_t *out_metrics);

/**
 * @brief Material 3 Bottom Navigation (Nav Bar) Metrics
 */
typedef struct cmp_m3_bottom_nav_metrics {
  float height;
  float active_indicator_width;
  float active_indicator_height;
  cmp_m3_shape_family_t active_indicator_shape;
} cmp_m3_bottom_nav_metrics_t;

CMP_API int cmp_m3_bottom_nav_resolve(cmp_m3_bottom_nav_metrics_t *out_metrics);

/**
 * @brief Material 3 Navigation Drawer Variants
 */
typedef enum cmp_m3_drawer_variant {
  CMP_M3_DRAWER_MODAL,
  CMP_M3_DRAWER_STANDARD
} cmp_m3_drawer_variant_t;

typedef struct cmp_m3_drawer_metrics {
  float max_width;
  cmp_m3_elevation_level_t elevation;
  cmp_m3_shape_family_t shape;
  float active_item_padding;
  cmp_m3_shape_family_t active_item_shape;
} cmp_m3_drawer_metrics_t;

CMP_API int cmp_m3_drawer_resolve(cmp_m3_drawer_variant_t variant,
                                  cmp_m3_drawer_metrics_t *out_metrics);

/**
 * @brief Material 3 Navigation Rail Metrics
 */
typedef struct cmp_m3_nav_rail_metrics {
  float width;
  cmp_m3_shape_family_t active_indicator_shape;
} cmp_m3_nav_rail_metrics_t;

CMP_API int cmp_m3_nav_rail_resolve(cmp_m3_nav_rail_metrics_t *out_metrics);

/**
 * @brief Material 3 Top App Bar Variants
 */
typedef enum cmp_m3_top_app_bar_variant {
  CMP_M3_TOP_APP_BAR_CENTER_ALIGNED,
  CMP_M3_TOP_APP_BAR_SMALL,
  CMP_M3_TOP_APP_BAR_MEDIUM,
  CMP_M3_TOP_APP_BAR_LARGE
} cmp_m3_top_app_bar_variant_t;

typedef struct cmp_m3_top_app_bar_metrics {
  float height_collapsed;
  float height_expanded;
  cmp_m3_elevation_level_t elevation_scrolled;
} cmp_m3_top_app_bar_metrics_t;

CMP_API int
cmp_m3_top_app_bar_resolve(cmp_m3_top_app_bar_variant_t variant,
                           cmp_m3_top_app_bar_metrics_t *out_metrics);

/**
 * @brief Material 3 Tabs Variants
 */
typedef enum cmp_m3_tabs_variant {
  CMP_M3_TABS_PRIMARY,
  CMP_M3_TABS_SECONDARY
} cmp_m3_tabs_variant_t;

typedef struct cmp_m3_tabs_metrics {
  float height;
  cmp_m3_shape_family_t active_indicator_shape;
  float active_indicator_thickness; /* Valid for secondary */
} cmp_m3_tabs_metrics_t;

CMP_API int cmp_m3_tabs_resolve(cmp_m3_tabs_variant_t variant,
                                cmp_m3_tabs_metrics_t *out_metrics);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_MATERIAL3_NAVIGATION_H */