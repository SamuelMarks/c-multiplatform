#ifndef CMP_MATERIAL3_H
#define CMP_MATERIAL3_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp_theme_core.h"
/* clang-format on */

int32_t cmp_material3_measure_button(const cmp_ui_node_t *node, float *width,
                                     float *height);
int32_t cmp_material3_measure_text_input(const cmp_ui_node_t *node,
                                         float *width, float *height);
int32_t cmp_material3_measure_slider(const cmp_ui_node_t *node, float *width,
                                     float *height);
int32_t cmp_material3_measure_toggle(const cmp_ui_node_t *node, float *width,
                                     float *height);
int32_t cmp_material3_measure_checkbox(const cmp_ui_node_t *node, float *width,
                                       float *height);
int32_t cmp_material3_measure_radio(const cmp_ui_node_t *node, float *width,
                                    float *height);
int32_t cmp_material3_measure_progress(const cmp_ui_node_t *node, float *width,
                                       float *height);
int32_t cmp_material3_measure_dropdown(const cmp_ui_node_t *node, float *width,
                                       float *height);
int32_t cmp_material3_layout_nav_bar(cmp_ui_node_t *node);
int32_t cmp_material3_layout_tab_bar(cmp_ui_node_t *node);
int32_t cmp_material3_layout_dialog_content(cmp_ui_node_t *node);
int32_t cmp_material3_layout_sidebar(cmp_ui_node_t *node);
int32_t cmp_material3_draw_button(const cmp_ui_node_t *node);
int32_t cmp_material3_draw_text_input(const cmp_ui_node_t *node);
int32_t cmp_material3_draw_slider(const cmp_ui_node_t *node);
int32_t cmp_material3_draw_toggle(const cmp_ui_node_t *node);
int32_t cmp_material3_draw_checkbox(const cmp_ui_node_t *node);
int32_t cmp_material3_draw_radio(const cmp_ui_node_t *node);
int32_t cmp_material3_draw_progress(const cmp_ui_node_t *node);
int32_t cmp_material3_draw_card(const cmp_ui_node_t *node);
int32_t cmp_material3_draw_tooltip(const cmp_ui_node_t *node);
int32_t cmp_material3_draw_menu(const cmp_ui_node_t *node);
int32_t cmp_material3_draw_focus_ring(const cmp_ui_node_t *node);
int32_t cmp_material3_get_spring_config(const cmp_ui_node_t *node, float *mass,
                                        float *stiffness, float *damping);
int32_t cmp_material3_get_ripple_config(const cmp_ui_node_t *node,
                                        float *duration, float *opacity);
int32_t cmp_material3_get_state_layer_opacity(const cmp_ui_node_t *node,
                                              int32_t state, float *opacity);
int32_t cmp_material3_get_transition_duration(const cmp_ui_node_t *node,
                                              float *duration);

#ifndef CMP_THEME_MODE_SINGLE_STATIC
extern const cmp_theme_vtable_t cmp_material3_vtable;

#endif /* CMP_THEME_MODE_SINGLE_STATIC */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_MATERIAL3_H */