#ifndef CMP_UI_DIALOG_H
#define CMP_UI_DIALOG_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_dialog.h
 * \brief UI Component for rendering an in-app Custom Dialog.
 */

/**
 * \brief Opaque handle to a Dialog UI component.
 */
typedef struct cmp_ui_dialog cmp_ui_dialog_t;

/**
 * \brief Creates a new Dialog component.
 *
 * \param out_dialog Pointer to store the created handle.
 * \param title Optional title of the dialog. Can be NULL.
 * \param content Optional text content. Can be NULL.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_dialog_create(cmp_ui_dialog_t **out_dialog,
                                 const char *title, const char *content);

/**
 * \brief Destroys a Dialog component.
 *
 * \param dialog The component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_dialog_destroy(cmp_ui_dialog_t *dialog);

/**
 * \brief Retrieves the underlying UI node for the Dialog.
 *
 * \param dialog The component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_dialog_get_node(cmp_ui_dialog_t *dialog,
                                   cmp_ui_node_t **out_node);

/**
 * \brief Sets whether the Dialog is open/visible.
 *
 * \param dialog The component.
 * \param visible 1 to show, 0 to hide.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_dialog_set_visible(cmp_ui_dialog_t *dialog, int visible);

/**
 * \brief Binds the widget to the accessibility tree.
 *
 * \param widget The component.
 * \param tree The accessibility tree.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_dialog_bind_a11y(cmp_ui_dialog_t *widget,
                                    cmp_a11y_tree_t *tree);

#ifdef __cplusplus

#endif /* __cplusplus */

#endif /* CMP_UI_DIALOG_H */