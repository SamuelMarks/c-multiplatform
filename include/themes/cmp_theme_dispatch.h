#ifndef CMP_THEME_DISPATCH_H
#define CMP_THEME_DISPATCH_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "cmp_theme_core.h"
#include "cmp_cupertino.h"
#include "cmp_fluent2.h"
#include "cmp_material3.h"
#include "cmp_unstyled.h"
/* clang-format on */

#ifdef CMP_THEME_MODE_SINGLE_STATIC

#ifndef CMP_STATIC_THEME_PREFIX
#error                                                                         \
    "CMP_THEME_MODE_SINGLE_STATIC is defined but CMP_STATIC_THEME_PREFIX is not set!"
#endif

#define CMP_THEME_CONCAT_(a, b) a##_##b
#define CMP_THEME_CONCAT(a, b) CMP_THEME_CONCAT_(a, b)

#define cmp_theme_measure_button                                               \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, measure_button)
#define cmp_theme_measure_text_input                                           \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, measure_text_input)
#define cmp_theme_measure_slider                                               \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, measure_slider)
#define cmp_theme_measure_toggle                                               \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, measure_toggle)
#define cmp_theme_measure_checkbox                                             \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, measure_checkbox)
#define cmp_theme_measure_radio                                                \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, measure_radio)
#define cmp_theme_measure_progress                                             \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, measure_progress)
#define cmp_theme_measure_dropdown                                             \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, measure_dropdown)
#define cmp_theme_layout_nav_bar                                               \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, layout_nav_bar)
#define cmp_theme_layout_tab_bar                                               \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, layout_tab_bar)
#define cmp_theme_layout_dialog_content                                        \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, layout_dialog_content)
#define cmp_theme_layout_sidebar                                               \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, layout_sidebar)
#define cmp_theme_draw_button                                                  \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, draw_button)
#define cmp_theme_draw_text_input                                              \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, draw_text_input)
#define cmp_theme_draw_slider                                                  \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, draw_slider)
#define cmp_theme_draw_toggle                                                  \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, draw_toggle)
#define cmp_theme_draw_checkbox                                                \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, draw_checkbox)
#define cmp_theme_draw_radio                                                   \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, draw_radio)
#define cmp_theme_draw_progress                                                \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, draw_progress)
#define cmp_theme_draw_card CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, draw_card)
#define cmp_theme_draw_tooltip                                                 \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, draw_tooltip)
#define cmp_theme_draw_menu CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, draw_menu)
#define cmp_theme_draw_focus_ring                                              \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, draw_focus_ring)
#define cmp_theme_get_spring_config                                            \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, get_spring_config)
#define cmp_theme_get_ripple_config                                            \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, get_ripple_config)
#define cmp_theme_get_state_layer_opacity                                      \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, get_state_layer_opacity)
#define cmp_theme_get_transition_duration                                      \
  CMP_THEME_CONCAT(CMP_STATIC_THEME_PREFIX, get_transition_duration)

#else /* Dynamic dispatch mode */

static inline int32_t cmp_theme_measure_button(const cmp_ui_node_t *node,
                                               float *width, float *height) {
  return cmp_resolve_vtable(node)->measure_button(node, width, height);
}

static inline int32_t cmp_theme_measure_text_input(const cmp_ui_node_t *node,
                                                   float *width,
                                                   float *height) {
  return cmp_resolve_vtable(node)->measure_text_input(node, width, height);
}

static inline int32_t cmp_theme_measure_slider(const cmp_ui_node_t *node,
                                               float *width, float *height) {
  return cmp_resolve_vtable(node)->measure_slider(node, width, height);
}

static inline int32_t cmp_theme_measure_toggle(const cmp_ui_node_t *node,
                                               float *width, float *height) {
  return cmp_resolve_vtable(node)->measure_toggle(node, width, height);
}

static inline int32_t cmp_theme_measure_checkbox(const cmp_ui_node_t *node,
                                                 float *width, float *height) {
  return cmp_resolve_vtable(node)->measure_checkbox(node, width, height);
}

static inline int32_t cmp_theme_measure_radio(const cmp_ui_node_t *node,
                                              float *width, float *height) {
  return cmp_resolve_vtable(node)->measure_radio(node, width, height);
}

static inline int32_t cmp_theme_measure_progress(const cmp_ui_node_t *node,
                                                 float *width, float *height) {
  return cmp_resolve_vtable(node)->measure_progress(node, width, height);
}

static inline int32_t cmp_theme_measure_dropdown(const cmp_ui_node_t *node,
                                                 float *width, float *height) {
  return cmp_resolve_vtable(node)->measure_dropdown(node, width, height);
}

static inline int32_t cmp_theme_layout_nav_bar(cmp_ui_node_t *node) {
  return cmp_resolve_vtable(node)->layout_nav_bar(node);
}

static inline int32_t cmp_theme_layout_tab_bar(cmp_ui_node_t *node) {
  return cmp_resolve_vtable(node)->layout_tab_bar(node);
}

static inline int32_t cmp_theme_layout_dialog_content(cmp_ui_node_t *node) {
  return cmp_resolve_vtable(node)->layout_dialog_content(node);
}

static inline int32_t cmp_theme_layout_sidebar(cmp_ui_node_t *node) {
  return cmp_resolve_vtable(node)->layout_sidebar(node);
}

static inline int32_t cmp_theme_draw_button(const cmp_ui_node_t *node) {
  return cmp_resolve_vtable(node)->draw_button(node);
}

static inline int32_t cmp_theme_draw_text_input(const cmp_ui_node_t *node) {
  return cmp_resolve_vtable(node)->draw_text_input(node);
}

static inline int32_t cmp_theme_draw_slider(const cmp_ui_node_t *node) {
  return cmp_resolve_vtable(node)->draw_slider(node);
}

static inline int32_t cmp_theme_draw_toggle(const cmp_ui_node_t *node) {
  return cmp_resolve_vtable(node)->draw_toggle(node);
}

static inline int32_t cmp_theme_draw_checkbox(const cmp_ui_node_t *node) {
  return cmp_resolve_vtable(node)->draw_checkbox(node);
}

static inline int32_t cmp_theme_draw_radio(const cmp_ui_node_t *node) {
  return cmp_resolve_vtable(node)->draw_radio(node);
}

static inline int32_t cmp_theme_draw_progress(const cmp_ui_node_t *node) {
  return cmp_resolve_vtable(node)->draw_progress(node);
}

static inline int32_t cmp_theme_draw_card(const cmp_ui_node_t *node) {
  return cmp_resolve_vtable(node)->draw_card(node);
}

static inline int32_t cmp_theme_draw_tooltip(const cmp_ui_node_t *node) {
  return cmp_resolve_vtable(node)->draw_tooltip(node);
}

static inline int32_t cmp_theme_draw_menu(const cmp_ui_node_t *node) {
  return cmp_resolve_vtable(node)->draw_menu(node);
}

static inline int32_t cmp_theme_draw_focus_ring(const cmp_ui_node_t *node) {
  return cmp_resolve_vtable(node)->draw_focus_ring(node);
}

static inline int32_t cmp_theme_get_spring_config(const cmp_ui_node_t *node,
                                                  float *mass, float *stiffness,
                                                  float *damping) {
  return cmp_resolve_vtable(node)->get_spring_config(node, mass, stiffness,
                                                     damping);
}

static inline int32_t cmp_theme_get_ripple_config(const cmp_ui_node_t *node,
                                                  float *duration,
                                                  float *opacity) {
  return cmp_resolve_vtable(node)->get_ripple_config(node, duration, opacity);
}

static inline int32_t
cmp_theme_get_state_layer_opacity(const cmp_ui_node_t *node, int32_t state,
                                  float *opacity) {
  return cmp_resolve_vtable(node)->get_state_layer_opacity(node, state,
                                                           opacity);
}

static inline int32_t
cmp_theme_get_transition_duration(const cmp_ui_node_t *node, float *duration) {
  return cmp_resolve_vtable(node)->get_transition_duration(node, duration);
}

#endif /* CMP_THEME_MODE_SINGLE_STATIC */

#ifdef __cplusplus
}
#endif

#endif /* CMP_THEME_DISPATCH_H */
