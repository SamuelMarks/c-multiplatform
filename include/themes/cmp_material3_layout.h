#ifndef CMP_MATERIAL3_LAYOUT_H
#define CMP_MATERIAL3_LAYOUT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
/* clang-format on */

/**
 * @brief Window Size Classes for Material 3
 */
typedef enum cmp_m3_window_size_class {
  CMP_M3_WINDOW_CLASS_COMPACT,  /* < 600dp */
  CMP_M3_WINDOW_CLASS_MEDIUM,   /* 600 - 839dp */
  CMP_M3_WINDOW_CLASS_EXPANDED, /* 840 - 1199dp */
  CMP_M3_WINDOW_CLASS_LARGE     /* 1200+ dp */
} cmp_m3_window_size_class_t;

/**
 * @brief Window Layout Configuration based on Size Class
 */
typedef struct cmp_m3_window_layout_config {
  int columns;
  float margins;
  float gutters;
} cmp_m3_window_layout_config_t;

/**
 * @brief Resolve Window Size Class from width
 */
CMP_API int
cmp_m3_window_size_class_resolve(float width_dp,
                                 cmp_m3_window_size_class_t *out_class);

/**
 * @brief Get grid configuration for a size class
 */
CMP_API int
cmp_m3_window_layout_config_get(cmp_m3_window_size_class_t size_class,
                                cmp_m3_window_layout_config_t *out_config);

/**
 * @brief Layout Pattern Categories
 */
typedef enum cmp_m3_layout_pattern {
  CMP_M3_PATTERN_LIST_DETAIL_STACK,
  CMP_M3_PATTERN_LIST_DETAIL_SIDE_BY_SIDE,
  CMP_M3_PATTERN_SUPPORTING_PANE,
  CMP_M3_PATTERN_FEED
} cmp_m3_layout_pattern_t;

/**
 * @brief Evaluates best layout pattern based on size class
 */
CMP_API int cmp_m3_layout_pattern_resolve(cmp_m3_window_size_class_t size_class,
                                          int is_feed, int has_supporting_pane,
                                          cmp_m3_layout_pattern_t *out_pattern);

/**
 * @brief Hinge Posture States
 */
typedef enum cmp_m3_posture {
  CMP_M3_POSTURE_FLAT,
  CMP_M3_POSTURE_TABLETOP,
  CMP_M3_POSTURE_BOOK
} cmp_m3_posture_t;

/**
 * @brief Evaluate Foldable/Hinge adaptation logic
 */
CMP_API int cmp_m3_foldable_posture_resolve(int has_hinge,
                                            float hinge_angle_degrees,
                                            int is_vertical_hinge,
                                            cmp_m3_posture_t *out_posture);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_MATERIAL3_LAYOUT_H */