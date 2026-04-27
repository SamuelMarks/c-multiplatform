#ifndef CMP_UNSTYLED_H
#define CMP_UNSTYLED_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp_theme_core.h"
/* clang-format on */

/**
 * @brief Measures an unstyled button.
 * @param node The UI node.
 * @param width Output calculated width.
 * @param height Output calculated height.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_measure_button(const cmp_ui_node_t *node, float *width,
                                float *height);
/**
 * @brief Measures an unstyled text input.
 * @param node The UI node.
 * @param width Output calculated width.
 * @param height Output calculated height.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_measure_text_input(const cmp_ui_node_t *node, float *width,
                                    float *height);
/**
 * @brief Measures an unstyled slider.
 * @param node The UI node.
 * @param width Output calculated width.
 * @param height Output calculated height.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_measure_slider(const cmp_ui_node_t *node, float *width,
                                float *height);
/**
 * @brief Measures an unstyled toggle switch.
 * @param node The UI node.
 * @param width Output calculated width.
 * @param height Output calculated height.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_measure_toggle(const cmp_ui_node_t *node, float *width,
                                float *height);
/**
 * @brief Measures an unstyled checkbox.
 * @param node The UI node.
 * @param width Output calculated width.
 * @param height Output calculated height.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_measure_checkbox(const cmp_ui_node_t *node, float *width,
                                  float *height);
/**
 * @brief Measures an unstyled radio button.
 * @param node The UI node.
 * @param width Output calculated width.
 * @param height Output calculated height.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_measure_radio(const cmp_ui_node_t *node, float *width,
                               float *height);
/**
 * @brief Measures an unstyled progress bar.
 * @param node The UI node.
 * @param width Output calculated width.
 * @param height Output calculated height.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_measure_progress(const cmp_ui_node_t *node, float *width,
                                  float *height);
/**
 * @brief Measures an unstyled dropdown menu.
 * @param node The UI node.
 * @param width Output calculated width.
 * @param height Output calculated height.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_measure_dropdown(const cmp_ui_node_t *node, float *width,
                                  float *height);
/**
 * @brief Lays out an unstyled navigation bar.
 * @param node The UI node.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_layout_nav_bar(cmp_ui_node_t *node);
/**
 * @brief Lays out an unstyled tab bar.
 * @param node The UI node.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_layout_tab_bar(cmp_ui_node_t *node);
/**
 * @brief Lays out unstyled dialog content.
 * @param node The UI node.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_layout_dialog_content(cmp_ui_node_t *node);
/**
 * @brief Lays out an unstyled sidebar.
 * @param node The UI node.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_layout_sidebar(cmp_ui_node_t *node);
/**
 * @brief Draws an unstyled button.
 * @param node The UI node.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_draw_button(const cmp_ui_node_t *node);
/**
 * @brief Draws an unstyled text input.
 * @param node The UI node.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_draw_text_input(const cmp_ui_node_t *node);
/**
 * @brief Draws an unstyled slider.
 * @param node The UI node.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_draw_slider(const cmp_ui_node_t *node);
/**
 * @brief Draws an unstyled toggle switch.
 * @param node The UI node.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_draw_toggle(const cmp_ui_node_t *node);
/**
 * @brief Draws an unstyled checkbox.
 * @param node The UI node.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_draw_checkbox(const cmp_ui_node_t *node);
/**
 * @brief Draws an unstyled radio button.
 * @param node The UI node.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_draw_radio(const cmp_ui_node_t *node);
/**
 * @brief Draws an unstyled progress bar.
 * @param node The UI node.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_draw_progress(const cmp_ui_node_t *node);
/**
 * @brief Draws an unstyled card.
 * @param node The UI node.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_draw_card(const cmp_ui_node_t *node);
/**
 * @brief Draws an unstyled tooltip.
 * @param node The UI node.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_draw_tooltip(const cmp_ui_node_t *node);
/**
 * @brief Draws an unstyled menu.
 * @param node The UI node.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_draw_menu(const cmp_ui_node_t *node);
/**
 * @brief Draws an unstyled focus ring.
 * @param node The UI node.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_draw_focus_ring(const cmp_ui_node_t *node);
/**
 * @brief Gets the spring animation configuration for unstyled elements.
 * @param node The UI node.
 * @param mass Output mass value.
 * @param stiffness Output stiffness value.
 * @param damping Output damping value.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_get_spring_config(const cmp_ui_node_t *node, float *mass,
                                   float *stiffness, float *damping);
/**
 * @brief Gets the ripple animation configuration for unstyled elements.
 * @param node The UI node.
 * @param duration Output ripple duration.
 * @param opacity Output ripple opacity.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_get_ripple_config(const cmp_ui_node_t *node, float *duration,
                                   float *opacity);
/**
 * @brief Gets the state layer opacity for unstyled elements.
 * @param node The UI node.
 * @param state The current UI state.
 * @param opacity Output opacity value.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_get_state_layer_opacity(const cmp_ui_node_t *node, int state,
                                         float *opacity);
/**
 * @brief Gets the transition duration for unstyled elements.
 * @param node The UI node.
 * @param duration Output transition duration.
 * @return CMP_SUCCESS on success, error code otherwise.
 */
int cmp_unstyled_get_transition_duration(const cmp_ui_node_t *node,
                                         float *duration);
#ifndef CMP_THEME_MODE_SINGLE_STATIC
extern const cmp_theme_vtable_t cmp_unstyled_vtable;
#endif /* CMP_THEME_MODE_SINGLE_STATIC */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UNSTYLED_H */