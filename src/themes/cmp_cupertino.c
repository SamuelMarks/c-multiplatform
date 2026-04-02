/* clang-format off */
#include "cmp.h"
#include "themes/cmp_cupertino.h"
#include <stddef.h>
/* clang-format on */

int32_t cmp_cupertino_measure_button(const cmp_ui_node_t *node, float *width,
                                     float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_measure_text_input(const cmp_ui_node_t *node,
                                         float *width, float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_measure_slider(const cmp_ui_node_t *node, float *width,
                                     float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_measure_toggle(const cmp_ui_node_t *node, float *width,
                                     float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_measure_checkbox(const cmp_ui_node_t *node, float *width,
                                       float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_measure_radio(const cmp_ui_node_t *node, float *width,
                                    float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_measure_progress(const cmp_ui_node_t *node, float *width,
                                       float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_measure_dropdown(const cmp_ui_node_t *node, float *width,
                                       float *height) {
  (void)node;
  (void)width;
  (void)height;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_layout_nav_bar(cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_layout_tab_bar(cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_layout_dialog_content(cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_layout_sidebar(cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_draw_button(const cmp_ui_node_t *node) {
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;

  bounds.x = node->layout->computed_rect.x;
  bounds.y = node->layout->computed_rect.y;
  bounds.width = node->layout->computed_rect.width;
  bounds.height = node->layout->computed_rect.height;

  /* systemBlue standard HIG mapping */
  color.r = 0.0f;
  color.g = 0.478f;
  color.b = 1.0f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_draw_text_input(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_draw_slider(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_draw_toggle(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_draw_checkbox(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_draw_radio(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_draw_progress(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_draw_card(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_draw_tooltip(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_draw_menu(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_draw_focus_ring(const cmp_ui_node_t *node) {
  (void)node;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_get_spring_config(const cmp_ui_node_t *node, float *mass,
                                        float *stiffness, float *damping) {
  (void)node;
  /* Equivalent to CASpringAnimation defaults */
  if (mass)
    *mass = 1.0f;
  if (stiffness)
    *stiffness = 100.0f;
  if (damping)
    *damping = 10.0f;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_get_ripple_config(const cmp_ui_node_t *node,
                                        float *duration, float *opacity) {
  (void)node;
  (void)duration;
  (void)opacity;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_get_state_layer_opacity(const cmp_ui_node_t *node,
                                              int32_t state, float *opacity) {
  (void)node;
  (void)state;
  (void)opacity;
  return CMP_SUCCESS;
}

int32_t cmp_cupertino_get_transition_duration(const cmp_ui_node_t *node,
                                              float *duration) {
  (void)node;
  if (duration)
    *duration = 250.0f; /* Standard quarter-second default */
  return CMP_SUCCESS;
}

#ifndef CMP_THEME_MODE_SINGLE_STATIC
const cmp_theme_vtable_t cmp_cupertino_vtable = {
    cmp_cupertino_measure_button,
    cmp_cupertino_measure_text_input,
    cmp_cupertino_measure_slider,
    cmp_cupertino_measure_toggle,
    cmp_cupertino_measure_checkbox,
    cmp_cupertino_measure_radio,
    cmp_cupertino_measure_progress,
    cmp_cupertino_measure_dropdown,
    cmp_cupertino_layout_nav_bar,
    cmp_cupertino_layout_tab_bar,
    cmp_cupertino_layout_dialog_content,
    cmp_cupertino_layout_sidebar,
    cmp_cupertino_draw_button,
    cmp_cupertino_draw_text_input,
    cmp_cupertino_draw_slider,
    cmp_cupertino_draw_toggle,
    cmp_cupertino_draw_checkbox,
    cmp_cupertino_draw_radio,
    cmp_cupertino_draw_progress,
    cmp_cupertino_draw_card,
    cmp_cupertino_draw_tooltip,
    cmp_cupertino_draw_menu,
    cmp_cupertino_draw_focus_ring,
    cmp_cupertino_get_spring_config,
    cmp_cupertino_get_ripple_config,
    cmp_cupertino_get_state_layer_opacity,
    cmp_cupertino_get_transition_duration};

const cmp_theme_vtable_t *cmp_theme_get_cupertino_vtable(void) {
  return &cmp_cupertino_vtable;
}
#endif /* CMP_THEME_MODE_SINGLE_STATIC */
