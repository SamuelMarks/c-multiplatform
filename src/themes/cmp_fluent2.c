/* clang-format off */
#include "cmp.h"
#include "themes/cmp_fluent2.h"
#include <stddef.h>
/* clang-format on */

int32_t cmp_fluent2_measure_button(const cmp_ui_node_t *node, float *width,
                                   float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_measure_text_input(const cmp_ui_node_t *node, float *width,
                                       float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_measure_slider(const cmp_ui_node_t *node, float *width,
                                   float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_measure_toggle(const cmp_ui_node_t *node, float *width,
                                   float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_measure_checkbox(const cmp_ui_node_t *node, float *width,
                                     float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_measure_radio(const cmp_ui_node_t *node, float *width,
                                  float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_measure_progress(const cmp_ui_node_t *node, float *width,
                                     float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_measure_dropdown(const cmp_ui_node_t *node, float *width,
                                     float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_layout_nav_bar(cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_layout_tab_bar(cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_layout_dialog_content(cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_layout_sidebar(cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_button(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_text_input(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_slider(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_toggle(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_checkbox(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_radio(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_progress(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_card(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_tooltip(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_menu(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_focus_ring(const cmp_ui_node_t *node) {
  cmp_rect_t bounds;
  cmp_rect_t outer_focus_bounds;
  cmp_color_t color;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;

  bounds.x = node->layout->computed_rect.x;
  bounds.y = node->layout->computed_rect.y;
  bounds.width = node->layout->computed_rect.width;
  bounds.height = node->layout->computed_rect.height;

  /* Fluent 2 standard FocusVisual is drawn *outside* the bounds by 2px, and is
   * 1px thick. */
  outer_focus_bounds.x = bounds.x - 2.0f;
  outer_focus_bounds.y = bounds.y - 2.0f;
  outer_focus_bounds.width = bounds.width + 4.0f;
  outer_focus_bounds.height = bounds.height + 4.0f;

  /* Standard FocusVisual is solid black/white depending on theme, using generic
   * grey here */
  color.r = 0.5f;
  color.g = 0.5f;
  color.b = 0.5f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  return CMP_SUCCESS;
}

int32_t cmp_fluent2_get_spring_config(const cmp_ui_node_t *node, float *mass,
                                      float *stiffness, float *damping) {
  (void)node;
  (void)mass;
  (void)stiffness;
  (void)damping;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_get_ripple_config(const cmp_ui_node_t *node,
                                      float *duration, float *opacity) {
  (void)node;
  (void)duration;
  (void)opacity;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_get_state_layer_opacity(const cmp_ui_node_t *node,
                                            int32_t state, float *opacity) {
  (void)node;
  (void)state;
  (void)opacity;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_get_transition_duration(const cmp_ui_node_t *node,
                                            float *duration) {
  (void)node;
  if (duration)
    *duration = 167.0f; /* Standard fast duration in milliseconds for Fluent */
  return CMP_SUCCESS;
}

#ifndef CMP_THEME_MODE_SINGLE_STATIC
const cmp_theme_vtable_t cmp_fluent2_vtable = {
    cmp_fluent2_measure_button,
    cmp_fluent2_measure_text_input,
    cmp_fluent2_measure_slider,
    cmp_fluent2_measure_toggle,
    cmp_fluent2_measure_checkbox,
    cmp_fluent2_measure_radio,
    cmp_fluent2_measure_progress,
    cmp_fluent2_measure_dropdown,
    cmp_fluent2_layout_nav_bar,
    cmp_fluent2_layout_tab_bar,
    cmp_fluent2_layout_dialog_content,
    cmp_fluent2_layout_sidebar,
    cmp_fluent2_draw_button,
    cmp_fluent2_draw_text_input,
    cmp_fluent2_draw_slider,
    cmp_fluent2_draw_toggle,
    cmp_fluent2_draw_checkbox,
    cmp_fluent2_draw_radio,
    cmp_fluent2_draw_progress,
    cmp_fluent2_draw_card,
    cmp_fluent2_draw_tooltip,
    cmp_fluent2_draw_menu,
    cmp_fluent2_draw_focus_ring,
    cmp_fluent2_get_spring_config,
    cmp_fluent2_get_ripple_config,
    cmp_fluent2_get_state_layer_opacity,
    cmp_fluent2_get_transition_duration};

const cmp_theme_vtable_t *cmp_theme_get_fluent2_vtable(void) {
  return &cmp_fluent2_vtable;
}
#endif /* CMP_THEME_MODE_SINGLE_STATIC */
