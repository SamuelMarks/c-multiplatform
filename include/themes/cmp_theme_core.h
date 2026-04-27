#ifndef CMP_THEME_CORE_H
#define CMP_THEME_CORE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp_ffi.h"
/* clang-format on */

/**
 * @brief Represents a semantic color palette for a theme.
 */
struct cmp_palette_s {
  uint32_t primary;         /**< The primary brand color. */
  uint32_t secondary;       /**< The secondary accent color. */
  uint32_t tertiary;        /**< The tertiary accent color. */
  uint32_t neutral;         /**< The neutral foreground color (e.g., text). */
  uint32_t neutral_variant; /**< A muted neutral color (e.g., borders). */
  uint32_t background;      /**< The main background color. */
  uint32_t surface;         /**< The background color of cards and dialogs. */
  uint32_t error;           /**< The semantic error color. */
};

/**
 * @brief Represents a complete UI theme configuration.
 */
struct cmp_theme_s {
  cmp_palette_t palette;    /**< The color palette for the theme. */
  float density_multiplier; /**< Multiplier for spacing and padding (1.0 =
                               standard). */
  float base_font_size;     /**< The base font size in pixels. */
  int reduce_motion;     /**< Flag indicating if motion should be reduced (0 =
                                false, 1 = true). */
  int increase_contrast; /**< Flag indicating if contrast should be
                                increased (0 = false, 1 = true). */
  int differentiate_without_color; /**< Flag to use patterns/shapes instead of
                                      just color (0 = false, 1 = true). */
};

/**
 * @brief The virtual table (VTable) defining the implementation of a specific
 * design language.
 */
struct cmp_theme_vtable_s {
  /* Measurement Phase */
  /**
   * @brief Measures the intrinsic size of a button.
   * @param node The button node to measure.
   * @param width Pointer to store the resulting width.
   * @param height Pointer to store the resulting height.
   * @return 0 on success, or an error code.
   */
  int (*measure_button)(const cmp_ui_node_t *node, float *width, float *height);

  /**
   * @brief Measures the intrinsic size of a text input.
   * @param node The text input node to measure.
   * @param width Pointer to store the resulting width.
   * @param height Pointer to store the resulting height.
   * @return 0 on success, or an error code.
   */
  int (*measure_text_input)(const cmp_ui_node_t *node, float *width,
                            float *height);

  /**
   * @brief Measures the intrinsic size of a slider.
   * @param node The slider node to measure.
   * @param width Pointer to store the resulting width.
   * @param height Pointer to store the resulting height.
   * @return 0 on success, or an error code.
   */
  int (*measure_slider)(const cmp_ui_node_t *node, float *width, float *height);

  /**
   * @brief Measures the intrinsic size of a toggle switch.
   * @param node The toggle node to measure.
   * @param width Pointer to store the resulting width.
   * @param height Pointer to store the resulting height.
   * @return 0 on success, or an error code.
   */
  int (*measure_toggle)(const cmp_ui_node_t *node, float *width, float *height);

  /**
   * @brief Measures the intrinsic size of a checkbox.
   * @param node The checkbox node to measure.
   * @param width Pointer to store the resulting width.
   * @param height Pointer to store the resulting height.
   * @return 0 on success, or an error code.
   */
  int (*measure_checkbox)(const cmp_ui_node_t *node, float *width,
                          float *height);

  /**
   * @brief Measures the intrinsic size of a radio button.
   * @param node The radio button node to measure.
   * @param width Pointer to store the resulting width.
   * @param height Pointer to store the resulting height.
   * @return 0 on success, or an error code.
   */
  int (*measure_radio)(const cmp_ui_node_t *node, float *width, float *height);

  /**
   * @brief Measures the intrinsic size of a progress bar/indicator.
   * @param node The progress node to measure.
   * @param width Pointer to store the resulting width.
   * @param height Pointer to store the resulting height.
   * @return 0 on success, or an error code.
   */
  int (*measure_progress)(const cmp_ui_node_t *node, float *width,
                          float *height);

  /**
   * @brief Measures the intrinsic size of a dropdown menu.
   * @param node The dropdown node to measure.
   * @param width Pointer to store the resulting width.
   * @param height Pointer to store the resulting height.
   * @return 0 on success, or an error code.
   */
  int (*measure_dropdown)(const cmp_ui_node_t *node, float *width,
                          float *height);

  /* Layout/Geometry Phase */
  /**
   * @brief Calculates the layout for a navigation bar.
   * @param node The navigation bar node to layout.
   * @return 0 on success, or an error code.
   */
  int (*layout_nav_bar)(cmp_ui_node_t *node);

  /**
   * @brief Calculates the layout for a tab bar.
   * @param node The tab bar node to layout.
   * @return 0 on success, or an error code.
   */
  int (*layout_tab_bar)(cmp_ui_node_t *node);

  /**
   * @brief Calculates the layout for a dialog content area.
   * @param node The dialog content node to layout.
   * @return 0 on success, or an error code.
   */
  int (*layout_dialog_content)(cmp_ui_node_t *node);

  /**
   * @brief Calculates the layout for a sidebar.
   * @param node The sidebar node to layout.
   * @return 0 on success, or an error code.
   */
  int (*layout_sidebar)(cmp_ui_node_t *node);

  /* Rendering Phase */
  /**
   * @brief Draws a button node.
   * @param node The button node to draw.
   * @return 0 on success, or an error code.
   */
  int (*draw_button)(const cmp_ui_node_t *node);

  /**
   * @brief Draws a text input node.
   * @param node The text input node to draw.
   * @return 0 on success, or an error code.
   */
  int (*draw_text_input)(const cmp_ui_node_t *node);

  /**
   * @brief Draws a slider node.
   * @param node The slider node to draw.
   * @return 0 on success, or an error code.
   */
  int (*draw_slider)(const cmp_ui_node_t *node);

  /**
   * @brief Draws a toggle node.
   * @param node The toggle node to draw.
   * @return 0 on success, or an error code.
   */
  int (*draw_toggle)(const cmp_ui_node_t *node);

  /**
   * @brief Draws a checkbox node.
   * @param node The checkbox node to draw.
   * @return 0 on success, or an error code.
   */
  int (*draw_checkbox)(const cmp_ui_node_t *node);

  /**
   * @brief Draws a radio button node.
   * @param node The radio button node to draw.
   * @return 0 on success, or an error code.
   */
  int (*draw_radio)(const cmp_ui_node_t *node);

  /**
   * @brief Draws a progress indicator node.
   * @param node The progress node to draw.
   * @return 0 on success, or an error code.
   */
  int (*draw_progress)(const cmp_ui_node_t *node);

  /**
   * @brief Draws a card container node.
   * @param node The card node to draw.
   * @return 0 on success, or an error code.
   */
  int (*draw_card)(const cmp_ui_node_t *node);

  /**
   * @brief Draws a tooltip node.
   * @param node The tooltip node to draw.
   * @return 0 on success, or an error code.
   */
  int (*draw_tooltip)(const cmp_ui_node_t *node);

  /**
   * @brief Draws a menu node.
   * @param node The menu node to draw.
   * @return 0 on success, or an error code.
   */
  int (*draw_menu)(const cmp_ui_node_t *node);

  /**
   * @brief Draws a focus ring around a node.
   * @param node The node to draw a focus ring around.
   * @return 0 on success, or an error code.
   */
  int (*draw_focus_ring)(const cmp_ui_node_t *node);

  /* Animation/Physics Phase */
  /**
   * @brief Gets the spring configuration for a node.
   * @param node The node to query.
   * @param mass Pointer to store the spring mass.
   * @param stiffness Pointer to store the spring stiffness.
   * @param damping Pointer to store the spring damping.
   * @return 0 on success, or an error code.
   */
  int (*get_spring_config)(const cmp_ui_node_t *node, float *mass,
                           float *stiffness, float *damping);

  /**
   * @brief Gets the ripple configuration for a node.
   * @param node The node to query.
   * @param duration Pointer to store the ripple duration.
   * @param opacity Pointer to store the ripple opacity.
   * @return 0 on success, or an error code.
   */
  int (*get_ripple_config)(const cmp_ui_node_t *node, float *duration,
                           float *opacity);

  /**
   * @brief Gets the state layer opacity for a node.
   * @param node The node to query.
   * @param state The state to query (e.g., hover, pressed).
   * @param opacity Pointer to store the state layer opacity.
   * @return 0 on success, or an error code.
   */
  int (*get_state_layer_opacity)(const cmp_ui_node_t *node, int state,
                                 float *opacity);

  /**
   * @brief Gets the transition duration for a node.
   * @param node The node to query.
   * @param duration Pointer to store the transition duration.
   * @return 0 on success, or an error code.
   */
  int (*get_transition_duration)(const cmp_ui_node_t *node, float *duration);
};
#ifndef CMP_THEME_MODE_SINGLE_STATIC
int CMP_API
cmp_theme_get_material3_vtable(const cmp_theme_vtable_t **out_vtable);
int CMP_API cmp_theme_get_fluent2_vtable(const cmp_theme_vtable_t **out_vtable);
int CMP_API
cmp_theme_get_cupertino_vtable(const cmp_theme_vtable_t **out_vtable);
int CMP_API
cmp_theme_get_unstyled_vtable(const cmp_theme_vtable_t **out_vtable);

/* Cross-Language FFI boundaries */
int CMP_API cmp_ffi_get_material3_vtable(const cmp_theme_vtable_t **out_vtable);
int CMP_API cmp_ffi_get_fluent2_vtable(const cmp_theme_vtable_t **out_vtable);
int CMP_API cmp_ffi_get_cupertino_vtable(const cmp_theme_vtable_t **out_vtable);
int CMP_API cmp_ffi_get_unstyled_vtable(const cmp_theme_vtable_t **out_vtable);
#endif /* CMP_THEME_MODE_SINGLE_STATIC */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_THEME_CORE_H */
