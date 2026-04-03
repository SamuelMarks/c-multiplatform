#ifndef CMP_MATERIAL3_SELECTION_CONTROLS_H
#define CMP_MATERIAL3_SELECTION_CONTROLS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_sys.h"
/* clang-format on */

/**
 * @brief Material 3 Checkbox Metrics
 */
typedef struct cmp_m3_checkbox_metrics {
  float box_size;
  float touch_target_size;
  float border_thickness_unchecked;
  float border_thickness_checked;
  cmp_m3_shape_family_t shape;
} cmp_m3_checkbox_metrics_t;

CMP_API int cmp_m3_checkbox_resolve(cmp_m3_checkbox_metrics_t *out_metrics);

/**
 * @brief Material 3 Radio Button Metrics
 */
typedef struct cmp_m3_radio_metrics {
  float outer_ring_size;
  float inner_dot_size;
  float touch_target_size;
  float border_thickness;
  cmp_m3_shape_family_t shape;
} cmp_m3_radio_metrics_t;

CMP_API int cmp_m3_radio_resolve(cmp_m3_radio_metrics_t *out_metrics);

/**
 * @brief Material 3 Switch Metrics
 */
typedef struct cmp_m3_switch_metrics {
  float track_width;
  float track_height;
  float touch_target_size;
  float thumb_unselected_size;
  float thumb_unselected_border;
  float thumb_selected_size;
  float thumb_pressed_size;
  cmp_m3_shape_family_t shape;
} cmp_m3_switch_metrics_t;

CMP_API int cmp_m3_switch_resolve(cmp_m3_switch_metrics_t *out_metrics);

/**
 * @brief Material 3 Slider Metrics
 */
typedef struct cmp_m3_slider_metrics {
  float track_height_inactive;
  float track_height_active;
  float handle_size;
  float touch_target_size;
  cmp_m3_shape_family_t shape;
} cmp_m3_slider_metrics_t;

CMP_API int cmp_m3_slider_resolve(cmp_m3_slider_metrics_t *out_metrics);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_MATERIAL3_SELECTION_CONTROLS_H */