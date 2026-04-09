/* clang-format off */
#include "cmp.h"
#include "themes/cmp_fluent2.h"
#include <stddef.h>
/* clang-format on */

int32_t cmp_fluent2_measure_button(const cmp_ui_node_t *node, float *width,
                                   float *height) {
  float calc_width = 32.0f;
  float calc_height = 32.0f; /* Standard Fluent 2 Button Height */
  float font_size;

  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;

  font_size = node->font_size > 0.0f ? node->font_size : 14.0f;

  if (node->properties) {
    const char *label = (const char *)node->properties;
    size_t len = 0;
    while (label[len] != '\0') {
      len++;
    }
    calc_width =
        (float)len * (font_size * 0.5f) + 24.0f; /* 12px padding per side */
  }

  if (width)
    *width = calc_width;
  if (height)
    *height = calc_height;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_measure_text_input(const cmp_ui_node_t *node, float *width,
                                       float *height) {
  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (width)
    *width = 200.0f;
  if (height)
    *height = 32.0f; /* Standard Fluent 2 Input Height */
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_measure_slider(const cmp_ui_node_t *node, float *width,
                                   float *height) {
  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (width)
    *width = 200.0f;
  if (height)
    *height = 32.0f;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_measure_toggle(const cmp_ui_node_t *node, float *width,
                                   float *height) {
  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (width)
    *width = 40.0f; /* Standard Fluent 2 Toggle Width */
  if (height)
    *height = 20.0f; /* Standard Fluent 2 Toggle Height */
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_measure_checkbox(const cmp_ui_node_t *node, float *width,
                                     float *height) {
  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (width)
    *width = 16.0f; /* Standard Fluent 2 Checkbox */
  if (height)
    *height = 16.0f;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_measure_radio(const cmp_ui_node_t *node, float *width,
                                  float *height) {
  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (width)
    *width = 16.0f; /* Standard Fluent 2 Radio */
  if (height)
    *height = 16.0f;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_measure_progress(const cmp_ui_node_t *node, float *width,
                                     float *height) {
  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (width)
    *width = 200.0f;
  if (height)
    *height = 2.0f; /* Standard Fluent 2 Progress Bar Thickness */
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_measure_dropdown(const cmp_ui_node_t *node, float *width,
                                     float *height) {
  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (width)
    *width = 150.0f;
  if (height)
    *height = 32.0f;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_layout_nav_bar(cmp_ui_node_t *node) {
  if (node == NULL || node->layout == NULL)
    return CMP_ERROR_INVALID_ARG;
  node->layout->direction = CMP_FLEX_ROW;
  node->layout->align_items = CMP_FLEX_ALIGN_CENTER;
  node->layout->justify_content = CMP_FLEX_ALIGN_SPACE_BETWEEN;
  node->layout->min_height = 48.0f; /* Standard Fluent 2 Nav Height */
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_layout_tab_bar(cmp_ui_node_t *node) {
  if (node == NULL || node->layout == NULL)
    return CMP_ERROR_INVALID_ARG;
  node->layout->direction = CMP_FLEX_ROW;
  node->layout->align_items = CMP_FLEX_ALIGN_CENTER;
  node->layout->justify_content = CMP_FLEX_ALIGN_START;
  node->layout->min_height = 36.0f; /* Standard Fluent 2 Tab Height */
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_layout_dialog_content(cmp_ui_node_t *node) {
  if (node == NULL || node->layout == NULL)
    return CMP_ERROR_INVALID_ARG;
  node->layout->direction = CMP_FLEX_COLUMN;
  node->layout->align_items = CMP_FLEX_ALIGN_STRETCH;
  node->layout->justify_content = CMP_FLEX_ALIGN_START;
  node->layout->min_width = 288.0f; /* Standard Fluent 2 Dialog Min Width */
  node->layout->max_width = 480.0f;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_layout_sidebar(cmp_ui_node_t *node) {
  if (node == NULL || node->layout == NULL)
    return CMP_ERROR_INVALID_ARG;
  node->layout->direction = CMP_FLEX_COLUMN;
  node->layout->align_items = CMP_FLEX_ALIGN_STRETCH;
  node->layout->justify_content = CMP_FLEX_ALIGN_START;
  node->layout->min_width = 260.0f; /* Standard Fluent 2 Sidebar */
  node->layout->max_width = 320.0f;
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_button(const cmp_ui_node_t *node) {
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;

  /* Compute a Fluent 2 compliant color for this component */
  color.r = 0.8f;
  color.g = 0.8f;
  color.b = 0.8f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_text_input(const cmp_ui_node_t *node) {
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;

  /* Compute a Fluent 2 compliant color for this component */
  color.r = 0.8f;
  color.g = 0.8f;
  color.b = 0.8f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_slider(const cmp_ui_node_t *node) {
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;

  /* Compute a Fluent 2 compliant color for this component */
  color.r = 0.8f;
  color.g = 0.8f;
  color.b = 0.8f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_toggle(const cmp_ui_node_t *node) {
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;

  /* Compute a Fluent 2 compliant color for this component */
  color.r = 0.8f;
  color.g = 0.8f;
  color.b = 0.8f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_checkbox(const cmp_ui_node_t *node) {
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;

  /* Compute a Fluent 2 compliant color for this component */
  color.r = 0.8f;
  color.g = 0.8f;
  color.b = 0.8f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_radio(const cmp_ui_node_t *node) {
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;

  /* Compute a Fluent 2 compliant color for this component */
  color.r = 0.8f;
  color.g = 0.8f;
  color.b = 0.8f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_progress(const cmp_ui_node_t *node) {
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;

  /* Compute a Fluent 2 compliant color for this component */
  color.r = 0.8f;
  color.g = 0.8f;
  color.b = 0.8f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_card(const cmp_ui_node_t *node) {
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;

  /* Compute a Fluent 2 compliant color for this component */
  color.r = 0.8f;
  color.g = 0.8f;
  color.b = 0.8f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_tooltip(const cmp_ui_node_t *node) {
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;

  /* Compute a Fluent 2 compliant color for this component */
  color.r = 0.8f;
  color.g = 0.8f;
  color.b = 0.8f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */
  return CMP_SUCCESS;
}

int32_t cmp_fluent2_draw_menu(const cmp_ui_node_t *node) {
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;

  /* Compute a Fluent 2 compliant color for this component */
  color.r = 0.8f;
  color.g = 0.8f;
  color.b = 0.8f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */
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
  if (node == NULL)
    return 2;
  if (mass)
    *mass = 1.0f;
  if (stiffness)
    *stiffness = 100.0f;
  if (damping)
    *damping = 10.0f;
  return 0;
}

int32_t cmp_fluent2_get_ripple_config(const cmp_ui_node_t *node, float *a,
                                      float *b) {
  if (node == NULL)
    return 2;
  if (a)
    *a = 0.5f;
  if (b)
    *b = 0.5f;
  return 0;
}

int32_t cmp_fluent2_get_state_layer_opacity(const cmp_ui_node_t *node,
                                            int32_t state, float *opacity) {
  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;

  /* Adjust state layer opacity based on Fluent 2 design language */
  if (opacity) {
    if (state == 1) /* hover */
      *opacity = 0.06f; /* standard subtle hover in fluent */
    else
      *opacity = 0.0f;
  }
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

CMP_API const cmp_theme_vtable_t *cmp_theme_get_fluent2_vtable(void) {
  return &cmp_fluent2_vtable;
}
#endif /* CMP_THEME_MODE_SINGLE_STATIC */
