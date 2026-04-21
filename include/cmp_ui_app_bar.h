#ifndef CMP_UI_APP_BAR_H
#define CMP_UI_APP_BAR_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_app_bar.h
 * \brief UI Component for rendering a Top or Bottom App Bar.
 */

/**
 * \brief The placement of the App Bar.
 */
typedef enum {
  CMP_UI_APP_BAR_PLACEMENT_TOP = 0,
  CMP_UI_APP_BAR_PLACEMENT_BOTTOM
} cmp_ui_app_bar_placement_t;

/**
 * \brief Opaque handle to an App Bar UI component.
 */
typedef struct cmp_ui_app_bar cmp_ui_app_bar_t;

/**
 * \brief Creates a new App Bar component.
 *
 * \param out_bar Pointer to store the created handle.
 * \param placement Top or Bottom placement.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_app_bar_create(cmp_ui_app_bar_t **out_bar,
                                  cmp_ui_app_bar_placement_t placement);

/**
 * \brief Destroys an App Bar component.
 *
 * \param bar The component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_app_bar_destroy(cmp_ui_app_bar_t *bar);

/**
 * \brief Retrieves the underlying UI node for the App Bar.
 *
 * \param bar The component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_app_bar_get_node(cmp_ui_app_bar_t *bar,
                                    cmp_ui_node_t **out_node);

/**
 * \brief Sets the title of the App Bar.
 *
 * \param bar The component.
 * \param title The title text.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_app_bar_set_title(cmp_ui_app_bar_t *bar, const char *title);

/**
 * \brief Adds an action node (e.g. icon button) to the App Bar.
 *
 * \param bar The component.
 * \param action_node The UI node representing the action.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_app_bar_add_action(cmp_ui_app_bar_t *bar,
                                      cmp_ui_node_t *action_node);

/**
 * \brief Binds the widget to the accessibility tree.
 *
 * \param widget The component.
 * \param tree The accessibility tree.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_app_bar_bind_a11y(cmp_ui_app_bar_t *widget,
                                     cmp_a11y_tree_t *tree);

#ifdef __cplusplus

#endif /* __cplusplus */

#endif /* CMP_UI_APP_BAR_H */