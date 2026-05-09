#ifndef CMP_THEME_DISPATCH_H
#define CMP_THEME_DISPATCH_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp_theme_core.h"
#include "cmp_cupertino.h"
#include "cmp_fluent2.h"
#include "cmp_material3.h"
#include "cmp_unstyled.h"

CMP_API void cmp_theme_set_default_vtable(const cmp_theme_vtable_t *vtable);
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

#else  /* Dynamic dispatch mode */

static int cmp_theme_measure_button(const cmp_ui_node_t *node, float *width,
                                    float *height) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->measure_button(node, width, height);
  return 0;
}

static int cmp_theme_measure_text_input(const cmp_ui_node_t *node, float *width,
                                        float *height) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->measure_text_input(node, width, height);
  return 0;
}

static int cmp_theme_measure_slider(const cmp_ui_node_t *node, float *width,
                                    float *height) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->measure_slider(node, width, height);
  return 0;
}

static int cmp_theme_measure_toggle(const cmp_ui_node_t *node, float *width,
                                    float *height) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->measure_toggle(node, width, height);
  return 0;
}

static int cmp_theme_measure_checkbox(const cmp_ui_node_t *node, float *width,
                                      float *height) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->measure_checkbox(node, width, height);
  return 0;
}

static int cmp_theme_measure_radio(const cmp_ui_node_t *node, float *width,
                                   float *height) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->measure_radio(node, width, height);
  return 0;
}

static int cmp_theme_measure_progress(const cmp_ui_node_t *node, float *width,
                                      float *height) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->measure_progress(node, width, height);
  return 0;
}

static int cmp_theme_measure_dropdown(const cmp_ui_node_t *node, float *width,
                                      float *height) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->measure_dropdown(node, width, height);
  return 0;
}

static int cmp_theme_layout_nav_bar(cmp_ui_node_t *node) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->layout_nav_bar(node);
  return 0;
}

static int cmp_theme_layout_tab_bar(cmp_ui_node_t *node) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->layout_tab_bar(node);
  return 0;
}

static int cmp_theme_layout_dialog_content(cmp_ui_node_t *node) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->layout_dialog_content(node);
  return 0;
}

static int cmp_theme_layout_sidebar(cmp_ui_node_t *node) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->layout_sidebar(node);
  return 0;
}

static int cmp_theme_draw_button(const cmp_ui_node_t *node) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->draw_button(node);
  return 0;
}

static int cmp_theme_draw_text_input(const cmp_ui_node_t *node) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->draw_text_input(node);
  return 0;
}

static int cmp_theme_draw_slider(const cmp_ui_node_t *node) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->draw_slider(node);
  return 0;
}

static int cmp_theme_draw_toggle(const cmp_ui_node_t *node) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->draw_toggle(node);
  return 0;
}

static int cmp_theme_draw_checkbox(const cmp_ui_node_t *node) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->draw_checkbox(node);
  return 0;
}

static int cmp_theme_draw_radio(const cmp_ui_node_t *node) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->draw_radio(node);
  return 0;
}

static int cmp_theme_draw_progress(const cmp_ui_node_t *node) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->draw_progress(node);
  return 0;
}

static int cmp_theme_draw_card(const cmp_ui_node_t *node) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->draw_card(node);
  return 0;
}

static int cmp_theme_draw_tooltip(const cmp_ui_node_t *node) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->draw_tooltip(node);
  return 0;
}

static int cmp_theme_draw_menu(const cmp_ui_node_t *node) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->draw_menu(node);
  return 0;
}

static int cmp_theme_draw_focus_ring(const cmp_ui_node_t *node) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->draw_focus_ring(node);
  return 0;
}

static int cmp_theme_get_spring_config(const cmp_ui_node_t *node, float *mass,
                                       float *stiffness, float *damping) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->get_spring_config(node, mass, stiffness, damping);
  return 0;
}

static int cmp_theme_get_ripple_config(const cmp_ui_node_t *node,
                                       float *duration, float *opacity) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->get_ripple_config(node, duration, opacity);
  return 0;
}

static int cmp_theme_get_state_layer_opacity(const cmp_ui_node_t *node,
                                             int state, float *opacity) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->get_state_layer_opacity(node, state, opacity);
  return 0;
}

static int cmp_theme_get_transition_duration(const cmp_ui_node_t *node,
                                             float *duration) {
  const cmp_theme_vtable_t *vt = NULL;
  int rc;
  rc = cmp_resolve_vtable(node, &vt);
  if (rc != 0)
    if (rc != 0) {
      return rc;
    }
  return rc;
  if (vt)
    return vt->get_transition_duration(node, duration);
  return 0;
}
#endif /* CMP_THEME_MODE_SINGLE_STATIC */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_THEME_DISPATCH_H */
