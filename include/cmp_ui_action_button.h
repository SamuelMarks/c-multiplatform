#ifndef CMP_UI_ACTION_BUTTON_H
#define CMP_UI_ACTION_BUTTON_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_action_button.h
 * \brief UI Component for Material 3 and equivalent Action Buttons.
 */

/**
 * \brief The style of the action button.
 */
typedef enum {
  CMP_UI_ACTION_BUTTON_STYLE_ELEVATED = 0,
  CMP_UI_ACTION_BUTTON_STYLE_FILLED,
  CMP_UI_ACTION_BUTTON_STYLE_TONAL,
  CMP_UI_ACTION_BUTTON_STYLE_OUTLINED,
  CMP_UI_ACTION_BUTTON_STYLE_TEXT
} cmp_ui_action_button_style_t;

/**
 * \brief Opaque handle to an action button component.
 */
typedef struct cmp_ui_action_button cmp_ui_action_button_t;

/**
 * \brief Creates a new action button component.
 *
 * \param out_btn Pointer to store the created button handle.
 * \param label The text label of the button.
 * \param style The specific M3 style (elevated, filled, etc).
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_action_button_create(cmp_ui_action_button_t **out_btn,
                                        const char *label,
                                        cmp_ui_action_button_style_t style);

/**
 * \brief Destroys an action button component.
 *
 * \param btn The button component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_action_button_destroy(cmp_ui_action_button_t *btn);

/**
 * \brief Retrieves the underlying UI node for the action button.
 *
 * \param btn The button component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_action_button_get_node(cmp_ui_action_button_t *btn,
                                          cmp_ui_node_t **out_node);

/**
 * \brief Updates the label displayed in the action button.
 *
 * \param btn The button component.
 * \param label The new label.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_action_button_set_label(cmp_ui_action_button_t *btn,
                                           const char *label);

/**
 * \brief Binds the widget to the accessibility tree.
 *
 * \param widget The component.
 * \param tree The accessibility tree.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_action_button_bind_a11y(cmp_ui_action_button_t *widget,
                                           cmp_a11y_tree_t *tree);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UI_ACTION_BUTTON_H */