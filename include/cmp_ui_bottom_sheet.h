#ifndef CMP_UI_BOTTOM_SHEET_H
#define CMP_UI_BOTTOM_SHEET_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_bottom_sheet.h
 * \brief UI Component for rendering a Bottom Sheet.
 */

/**
 * \brief Opaque handle to a Bottom Sheet UI component.
 */
typedef struct cmp_ui_bottom_sheet cmp_ui_bottom_sheet_t;

/**
 * \brief Creates a new Bottom Sheet component.
 *
 * \param out_sheet Pointer to store the created handle.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_bottom_sheet_create(cmp_ui_bottom_sheet_t **out_sheet);

/**
 * \brief Destroys a Bottom Sheet component.
 *
 * \param sheet The component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_bottom_sheet_destroy(cmp_ui_bottom_sheet_t *sheet);

/**
 * \brief Retrieves the underlying UI node for the Bottom Sheet.
 *
 * \param sheet The component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_bottom_sheet_get_node(cmp_ui_bottom_sheet_t *sheet,
                                         cmp_ui_node_t **out_node);

/**
 * \brief Sets whether the Bottom Sheet is expanded/visible.
 *
 * \param sheet The component.
 * \param visible 1 to show, 0 to hide.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_bottom_sheet_set_visible(cmp_ui_bottom_sheet_t *sheet,
                                            int visible);

/**
 * \brief Binds the widget to the accessibility tree.
 *
 * \param widget The component.
 * \param tree The accessibility tree.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_bottom_sheet_bind_a11y(cmp_ui_bottom_sheet_t *widget,
                                          cmp_a11y_tree_t *tree);
#ifdef __cplusplus

#endif /* __cplusplus */

#endif /* CMP_UI_BOTTOM_SHEET_H */