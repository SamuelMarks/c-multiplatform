/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3.h"
#include <stddef.h>
/* clang-format on */

int32_t cmp_material3_measure_button(const cmp_ui_node_t *node, float *width,
                                     float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_material3_measure_text_input(const cmp_ui_node_t *node,
                                         float *width, float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_material3_measure_slider(const cmp_ui_node_t *node, float *width,
                                     float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_material3_measure_toggle(const cmp_ui_node_t *node, float *width,
                                     float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_material3_measure_checkbox(const cmp_ui_node_t *node, float *width,
                                       float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_material3_measure_radio(const cmp_ui_node_t *node, float *width,
                                    float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_material3_measure_progress(const cmp_ui_node_t *node, float *width,
                                       float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_material3_measure_dropdown(const cmp_ui_node_t *node, float *width,
                                       float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_material3_layout_nav_bar(cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_material3_layout_tab_bar(cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_material3_layout_dialog_content(cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_material3_layout_sidebar(cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_material3_draw_button(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_material3_draw_text_input(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_material3_draw_slider(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_material3_draw_toggle(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_material3_draw_checkbox(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_material3_draw_radio(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_material3_draw_progress(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_material3_draw_card(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_material3_draw_tooltip(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_material3_draw_menu(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_material3_draw_focus_ring(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_material3_get_spring_config(const cmp_ui_node_t *node, float *mass,
                                        float *stiffness, float *damping) {
  (void)node;
  (void)mass;
  (void)stiffness;
  (void)damping;
  return CMP_SUCCESS;
}

int32_t cmp_material3_get_ripple_config(const cmp_ui_node_t *node,
                                        float *duration, float *opacity) {
  (void)node;
  if (duration)
    *duration = 200.0f; /* Standard ripple expansion duration */
  if (opacity)
    *opacity = 0.12f; /* Standard ripple opacity */
  return CMP_SUCCESS;
}

int32_t cmp_material3_get_state_layer_opacity(const cmp_ui_node_t *node,
                                              int32_t state, float *opacity) {
  (void)node;
  if (!opacity)
    return CMP_SUCCESS;

  switch (state) {
  case 1: /* Hover */
    *opacity = 0.08f;
    break;
  case 2: /* Focus */
  case 3: /* Pressed */
    *opacity = 0.12f;
    break;
  case 4: /* Dragged */
    *opacity = 0.16f;
    break;
  default:
    *opacity = 0.0f;
    break;
  }
  return CMP_SUCCESS;
}

int32_t cmp_material3_get_transition_duration(const cmp_ui_node_t *node,
                                              float *duration) {
  (void)node;
  /* Material standard transitions range from 150ms to 500ms based on emphasis
   */
  if (duration)
    *duration = 300.0f;
  return CMP_SUCCESS;
}

#ifndef CMP_THEME_MODE_SINGLE_STATIC
const cmp_theme_vtable_t cmp_material3_vtable = {
    cmp_material3_measure_button,
    cmp_material3_measure_text_input,
    cmp_material3_measure_slider,
    cmp_material3_measure_toggle,
    cmp_material3_measure_checkbox,
    cmp_material3_measure_radio,
    cmp_material3_measure_progress,
    cmp_material3_measure_dropdown,
    cmp_material3_layout_nav_bar,
    cmp_material3_layout_tab_bar,
    cmp_material3_layout_dialog_content,
    cmp_material3_layout_sidebar,
    cmp_material3_draw_button,
    cmp_material3_draw_text_input,
    cmp_material3_draw_slider,
    cmp_material3_draw_toggle,
    cmp_material3_draw_checkbox,
    cmp_material3_draw_radio,
    cmp_material3_draw_progress,
    cmp_material3_draw_card,
    cmp_material3_draw_tooltip,
    cmp_material3_draw_menu,
    cmp_material3_draw_focus_ring,
    cmp_material3_get_spring_config,
    cmp_material3_get_ripple_config,
    cmp_material3_get_state_layer_opacity,
    cmp_material3_get_transition_duration};

const cmp_theme_vtable_t *cmp_theme_get_material3_vtable(void) {
  return &cmp_material3_vtable;
}
#endif /* CMP_THEME_MODE_SINGLE_STATIC */
