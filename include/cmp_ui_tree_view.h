#ifndef CMP_UI_TREE_VIEW_H
#define CMP_UI_TREE_VIEW_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_tree_view.h
 * \brief UI Component for rendering a hierarchical tree view.
 */

/**
 * \brief Opaque handle to a tree view UI component.
 */
typedef struct cmp_ui_tree_view cmp_ui_tree_view_t;

/**
 * \brief Creates a new tree view component.
 *
 * \param out_tree_view Pointer to store the created tree view handle.
 * \param bg_color Background color of the tree view (ARGB).
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_tree_view_create(cmp_ui_tree_view_t **out_tree_view,
                                    uint32_t bg_color);

/**
 * \brief Destroys a tree view component.
 *
 * \param tree_view The tree view component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_tree_view_destroy(cmp_ui_tree_view_t *tree_view);

/**
 * \brief Retrieves the underlying UI node for the tree view.
 *
 * \param tree_view The tree view component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_tree_view_get_node(cmp_ui_tree_view_t *tree_view,
                                      cmp_ui_node_t **out_node);

/**
 * \brief Appends an item to the tree view.
 *
 * \param tree_view The tree view component.
 * \param label The item label text.
 * \param depth The hierarchical depth (0 for root).
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_tree_view_add_item(cmp_ui_tree_view_t *tree_view,
                                      const char *label, int depth);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UI_TREE_VIEW_H */
