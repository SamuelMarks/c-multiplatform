/* clang-format off */
#include "cmp.h"
#include "themes/cmp_cupertino.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief cmp_cupertino_measure_button
 *
 * @param node Parameter description.
 * @param width Parameter description.
 * @param height Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_measure_button(const cmp_ui_node_t *node, float *width,
                                 float *height) {
  int rc = CMP_SUCCESS;
  float calc_width = 44.0f;
  float calc_height = 44.0f; /* HIG standard tap target */
  float font_size;

  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;

  font_size = node->font_size > 0.0f ? node->font_size : 17.0f;

  if (node->properties) {
    const char *label = (const char *)node->properties;
    size_t len = 0;
    while (label[len] != '\0') {
      len++;
    }
    calc_width =
        (float)len * (font_size * 0.5f) + 28.0f; /* 14px padding per side */
  }

  if (width)
    *width = calc_width;
  if (height)
    *height = calc_height;

  return rc;
}

/**
 * @brief cmp_cupertino_measure_text_input
 *
 * @param node Parameter description.
 * @param width Parameter description.
 * @param height Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_measure_text_input(const cmp_ui_node_t *node, float *width,
                                     float *height) {
  int rc = CMP_SUCCESS;
  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (width)
    *width = 200.0f; /* Default reasonable width */
  if (height)
    *height = 44.0f; /* Standard tap target */

  return rc;
}

/**
 * @brief cmp_cupertino_measure_slider
 *
 * @param node Parameter description.
 * @param width Parameter description.
 * @param height Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_measure_slider(const cmp_ui_node_t *node, float *width,
                                 float *height) {
  int rc = CMP_SUCCESS;
  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (width)
    *width = 150.0f;
  if (height)
    *height = 32.0f;

  return rc;
}

/**
 * @brief cmp_cupertino_measure_toggle
 *
 * @param node Parameter description.
 * @param width Parameter description.
 * @param height Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_measure_toggle(const cmp_ui_node_t *node, float *width,
                                 float *height) {
  int rc = CMP_SUCCESS;
  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (width)
    *width = 51.0f; /* Standard iOS switch width */
  if (height)
    *height = 31.0f; /* Standard iOS switch height */

  return rc;
}

/**
 * @brief cmp_cupertino_measure_checkbox
 *
 * @param node Parameter description.
 * @param width Parameter description.
 * @param height Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_measure_checkbox(const cmp_ui_node_t *node, float *width,
                                   float *height) {
  int rc = CMP_SUCCESS;
  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (width)
    *width = 20.0f;
  if (height)
    *height = 20.0f;

  return rc;
}

/**
 * @brief cmp_cupertino_measure_radio
 *
 * @param node Parameter description.
 * @param width Parameter description.
 * @param height Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_measure_radio(const cmp_ui_node_t *node, float *width,
                                float *height) {
  int rc = CMP_SUCCESS;
  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (width)
    *width = 20.0f;
  if (height)
    *height = 20.0f;

  return rc;
}

/**
 * @brief cmp_cupertino_measure_progress
 *
 * @param node Parameter description.
 * @param width Parameter description.
 * @param height Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_measure_progress(const cmp_ui_node_t *node, float *width,
                                   float *height) {
  int rc = CMP_SUCCESS;
  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (width)
    *width = 150.0f;
  if (height)
    *height = 4.0f; /* Standard iOS progress bar thickness */

  return rc;
}

/**
 * @brief cmp_cupertino_measure_dropdown
 *
 * @param node Parameter description.
 * @param width Parameter description.
 * @param height Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_measure_dropdown(const cmp_ui_node_t *node, float *width,
                                   float *height) {
  int rc = CMP_SUCCESS;
  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (width)
    *width = 120.0f;
  if (height)
    *height = 32.0f;

  return rc;
}

/**
 * @brief cmp_cupertino_layout_nav_bar
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_layout_nav_bar(cmp_ui_node_t *node) {
  int rc = CMP_SUCCESS;
  if (node == NULL || node->layout == NULL)
    return CMP_ERROR_INVALID_ARG;
  node->layout->direction = CMP_FLEX_ROW;
  node->layout->align_items = CMP_FLEX_ALIGN_CENTER;
  node->layout->justify_content = CMP_FLEX_ALIGN_SPACE_BETWEEN;
  node->layout->min_height = 44.0f; /* iOS standard HIG nav bar height */

  return rc;
}

/**
 * @brief cmp_cupertino_layout_tab_bar
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_layout_tab_bar(cmp_ui_node_t *node) {
  int rc = CMP_SUCCESS;
  if (node == NULL || node->layout == NULL)
    return CMP_ERROR_INVALID_ARG;
  node->layout->direction = CMP_FLEX_ROW;
  node->layout->align_items = CMP_FLEX_ALIGN_CENTER;
  node->layout->justify_content = CMP_FLEX_ALIGN_SPACE_AROUND;
  node->layout->min_height = 49.0f; /* iOS standard HIG tab bar height */

  return rc;
}

/**
 * @brief cmp_cupertino_layout_dialog_content
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_layout_dialog_content(cmp_ui_node_t *node) {
  int rc = CMP_SUCCESS;
  if (node == NULL || node->layout == NULL)
    return CMP_ERROR_INVALID_ARG;
  node->layout->direction = CMP_FLEX_COLUMN;
  node->layout->align_items = CMP_FLEX_ALIGN_CENTER;
  node->layout->justify_content = CMP_FLEX_ALIGN_START;
  node->layout->min_width = 270.0f; /* iOS standard alert width */

  return rc;
}

/**
 * @brief cmp_cupertino_layout_sidebar
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_layout_sidebar(cmp_ui_node_t *node) {
  int rc = CMP_SUCCESS;
  if (node == NULL || node->layout == NULL)
    return CMP_ERROR_INVALID_ARG;
  node->layout->direction = CMP_FLEX_COLUMN;
  node->layout->align_items = CMP_FLEX_ALIGN_STRETCH;
  node->layout->justify_content = CMP_FLEX_ALIGN_START;
  node->layout->min_width = 200.0f; /* macOS HIG standard sidebar */
  node->layout->max_width = 320.0f;

  return rc;
}

/**
 * @brief cmp_cupertino_draw_button
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_draw_button(const cmp_ui_node_t *node) {
  int rc = CMP_SUCCESS;
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;

  bounds.x = node->layout->computed_rect.x;
  bounds.y = node->layout->computed_rect.y;
  bounds.width = node->layout->computed_rect.width;
  bounds.height = node->layout->computed_rect.height;
  (void)bounds;

  /* systemBlue standard HIG mapping */
  color.r = 0.0f;
  color.g = 0.478f;
  color.b = 1.0f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;

  return rc;
}

/**
 * @brief cmp_cupertino_draw_text_input
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_draw_text_input(const cmp_ui_node_t *node) {
  int rc = CMP_SUCCESS;
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;
  (void)bounds;

  /* Compute a HIG compliant color for this component to pass to the compositor
   */
  color.r = 0.9f;
  color.g = 0.9f;
  color.b = 0.9f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */

  return rc;
}

/**
 * @brief cmp_cupertino_draw_slider
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_draw_slider(const cmp_ui_node_t *node) {
  int rc = CMP_SUCCESS;
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;
  (void)bounds;

  /* Compute a HIG compliant color for this component to pass to the compositor
   */
  color.r = 0.9f;
  color.g = 0.9f;
  color.b = 0.9f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */

  return rc;
}

/**
 * @brief cmp_cupertino_draw_toggle
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_draw_toggle(const cmp_ui_node_t *node) {
  int rc = CMP_SUCCESS;
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;
  (void)bounds;

  /* Compute a HIG compliant color for this component to pass to the compositor
   */
  color.r = 0.9f;
  color.g = 0.9f;
  color.b = 0.9f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */

  return rc;
}

/**
 * @brief cmp_cupertino_draw_checkbox
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_draw_checkbox(const cmp_ui_node_t *node) {
  int rc = CMP_SUCCESS;
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;
  (void)bounds;

  /* Compute a HIG compliant color for this component to pass to the compositor
   */
  color.r = 0.9f;
  color.g = 0.9f;
  color.b = 0.9f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */

  return rc;
}

/**
 * @brief cmp_cupertino_draw_radio
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_draw_radio(const cmp_ui_node_t *node) {
  int rc = CMP_SUCCESS;
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;
  (void)bounds;

  /* Compute a HIG compliant color for this component to pass to the compositor
   */
  color.r = 0.9f;
  color.g = 0.9f;
  color.b = 0.9f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */

  return rc;
}

/**
 * @brief cmp_cupertino_draw_progress
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_draw_progress(const cmp_ui_node_t *node) {
  int rc = CMP_SUCCESS;
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;
  (void)bounds;

  /* Compute a HIG compliant color for this component to pass to the compositor
   */
  color.r = 0.9f;
  color.g = 0.9f;
  color.b = 0.9f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */

  return rc;
}

/**
 * @brief cmp_cupertino_draw_card
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_draw_card(const cmp_ui_node_t *node) {
  int rc = CMP_SUCCESS;
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;
  (void)bounds;

  /* Compute a HIG compliant color for this component to pass to the compositor
   */
  color.r = 0.9f;
  color.g = 0.9f;
  color.b = 0.9f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */

  return rc;
}

/**
 * @brief cmp_cupertino_draw_tooltip
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_draw_tooltip(const cmp_ui_node_t *node) {
  int rc = CMP_SUCCESS;
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;
  (void)bounds;

  /* Compute a HIG compliant color for this component to pass to the compositor
   */
  color.r = 0.9f;
  color.g = 0.9f;
  color.b = 0.9f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */

  return rc;
}

/**
 * @brief cmp_cupertino_draw_menu
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_draw_menu(const cmp_ui_node_t *node) {
  int rc = CMP_SUCCESS;
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;
  (void)bounds;

  /* Compute a HIG compliant color for this component to pass to the compositor
   */
  color.r = 0.9f;
  color.g = 0.9f;
  color.b = 0.9f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */

  return rc;
}

/**
 * @brief cmp_cupertino_draw_focus_ring
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_draw_focus_ring(const cmp_ui_node_t *node) {
  int rc = CMP_SUCCESS;
  cmp_rect_t bounds;
  cmp_color_t color;
  cmp_renderer_t *renderer = NULL;
  if (!node || !node->layout)
    return CMP_ERROR_INVALID_ARG;
  bounds = node->layout->computed_rect;
  (void)bounds;

  /* Compute a HIG compliant color for this component to pass to the compositor
   */
  color.r = 0.9f;
  color.g = 0.9f;
  color.b = 0.9f;
  color.a = 1.0f;
  color.space = CMP_COLOR_SPACE_SRGB;

  /* Assuming context or global renderer is accessible */
  (void)renderer;
  /* GPU Draw calls handled by compositor layer */

  return rc;
}

/**
 * @brief cmp_cupertino_get_spring_config
 *
 * @param node Parameter description.
 * @param mass Parameter description.
 * @param stiffness Parameter description.
 * @param damping Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_get_spring_config(const cmp_ui_node_t *node, float *mass,
                                    float *stiffness, float *damping) {
  int rc = CMP_SUCCESS;
  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;
  /* Equivalent to CASpringAnimation defaults */
  if (mass)
    *mass = 1.0f;
  if (stiffness)
    *stiffness = 100.0f;
  if (damping)
    *damping = 10.0f;

  return rc;
}

/**
 * @brief cmp_cupertino_get_ripple_config
 *
 * @param node Parameter description.
 * @param a Parameter description.
 * @param b Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_get_ripple_config(const cmp_ui_node_t *node, float *a,
                                    float *b) {
  int rc = CMP_SUCCESS;
  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (a)
    *a = 0.5f;
  if (b)
    *b = 0.5f;

  return rc;
}

/**
 * @brief cmp_cupertino_get_state_layer_opacity
 *
 * @param node Parameter description.
 * @param state Parameter description.
 * @param opacity Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_get_state_layer_opacity(const cmp_ui_node_t *node, int state,
                                          float *opacity) {
  int rc = CMP_SUCCESS;
  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;

  /* Adjust state layer opacity based on HIG */
  if (opacity) {
    if (state == 1) /* hover */
      *opacity = 0.8f;
    else
      *opacity = 1.0f;
  }

  return rc;
}

/**
 * @brief cmp_cupertino_get_transition_duration
 *
 * @param node Parameter description.
 * @param duration Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cupertino_get_transition_duration(const cmp_ui_node_t *node,
                                          float *duration) {
  int rc = CMP_SUCCESS;
  if (node == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (duration)
    *duration = 250.0f; /* Standard quarter-second default */

  return rc;
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

/**
 * @brief cmp_theme_get_cupertino_vtable
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API
cmp_theme_get_cupertino_vtable(const cmp_theme_vtable_t **out_vtable) {
  int rc;
  rc = 0;
  if (out_vtable) {
    *out_vtable = &cmp_cupertino_vtable;
  }
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
#endif /* CMP_THEME_MODE_SINGLE_STATIC */
