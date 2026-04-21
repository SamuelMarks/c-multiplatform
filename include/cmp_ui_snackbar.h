#ifndef CMP_UI_SNACKBAR_H
#define CMP_UI_SNACKBAR_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_snackbar.h
 * \brief UI Component for rendering a Snackbar (transient message).
 */

/**
 * \brief Opaque handle to a Snackbar UI component.
 */
typedef struct cmp_ui_snackbar cmp_ui_snackbar_t;

/**
 * \brief Creates a new Snackbar component.
 *
 * \param out_snackbar Pointer to store the created Snackbar handle.
 * \param message The message text to display.
 * \param action_label Optional action label (e.g. "Undo"). Can be NULL.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_snackbar_create(cmp_ui_snackbar_t **out_snackbar,
                                   const char *message,
                                   const char *action_label);

/**
 * \brief Destroys a Snackbar component.
 *
 * \param snackbar The Snackbar component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_snackbar_destroy(cmp_ui_snackbar_t *snackbar);

/**
 * \brief Retrieves the underlying UI node for the Snackbar.
 *
 * \param snackbar The Snackbar component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_snackbar_get_node(cmp_ui_snackbar_t *snackbar,
                                     cmp_ui_node_t **out_node);

/**
 * \brief Updates the message displayed in the Snackbar.
 *
 * \param snackbar The Snackbar component.
 * \param message The new message.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_snackbar_set_message(cmp_ui_snackbar_t *snackbar,
                                        const char *message);

/**
 * \brief Updates the action label in the Snackbar.
 *
 * \param snackbar The Snackbar component.
 * \param action_label The new action label. Can be NULL to remove.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_snackbar_set_action(cmp_ui_snackbar_t *snackbar,
                                       const char *action_label);

/**
 * \brief Binds the widget to the accessibility tree.
 *
 * \param widget The component.
 * \param tree The accessibility tree.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_snackbar_bind_a11y(cmp_ui_snackbar_t *widget,
                                      cmp_a11y_tree_t *tree);

#ifdef __cplusplus

#endif /* __cplusplus */

#endif /* CMP_UI_SNACKBAR_H */