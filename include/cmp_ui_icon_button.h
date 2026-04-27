#ifndef CMP_UI_ICON_BUTTON_H
#define CMP_UI_ICON_BUTTON_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_icon_button.h
 * \brief UI Component for rendering an Icon Button.
 */

/**
 * \brief The style of the icon button.
 */
typedef enum {
  CMP_UI_ICON_BUTTON_STYLE_STANDARD = 0,
  CMP_UI_ICON_BUTTON_STYLE_FILLED,
  CMP_UI_ICON_BUTTON_STYLE_FILLED_TONAL,
  CMP_UI_ICON_BUTTON_STYLE_OUTLINED
} cmp_ui_icon_button_style_t;

/**
 * \brief Opaque handle to an Icon Button UI component.
 */
typedef struct cmp_ui_icon_button cmp_ui_icon_button_t;

/**
 * \brief Creates a new Icon Button component.
 *
 * \param out_btn Pointer to store the created button handle.
 * \param icon_name The name or path of the icon to display.
 * \param style The specific M3 style of the icon button.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_icon_button_create(cmp_ui_icon_button_t **out_btn,
                                      const char *icon_name,
                                      cmp_ui_icon_button_style_t style);

/**
 * \brief Destroys an Icon Button component.
 *
 * \param btn The button component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_icon_button_destroy(cmp_ui_icon_button_t *btn);

/**
 * \brief Retrieves the underlying UI node for the button.
 *
 * \param btn The button component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_icon_button_get_node(cmp_ui_icon_button_t *btn,
                                        cmp_ui_node_t **out_node);

/**
 * \brief Updates the icon displayed in the button.
 *
 * \param btn The button component.
 * \param icon_name The new icon name or path to display.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_icon_button_set_icon(cmp_ui_icon_button_t *btn,
                                        const char *icon_name);

/**
 * \brief Binds the widget to the accessibility tree.
 *
 * \param widget The component.
 * \param tree The accessibility tree.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_icon_button_bind_a11y(cmp_ui_icon_button_t *widget,
                                         cmp_a11y_tree_t *tree);
#ifdef __cplusplus

#endif /* __cplusplus */

#endif /* CMP_UI_ICON_BUTTON_H */