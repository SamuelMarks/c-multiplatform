#ifndef CMP_UI_FAB_H
#define CMP_UI_FAB_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_fab.h
 * \brief UI Component for rendering a Floating Action Button (FAB).
 */

/**
 * \brief Opaque handle to a FAB UI component.
 */
typedef struct cmp_ui_fab cmp_ui_fab_t;

/**
 * \brief Creates a new FAB component.
 *
 * \param out_fab Pointer to store the created FAB handle.
 * \param icon_name The name or path of the icon to display.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_fab_create(cmp_ui_fab_t **out_fab, const char *icon_name);

/**
 * \brief Destroys a FAB component.
 *
 * \param fab The FAB component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_fab_destroy(cmp_ui_fab_t *fab);

/**
 * \brief Retrieves the underlying UI node for the FAB.
 *
 * \param fab The FAB component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_fab_get_node(cmp_ui_fab_t *fab, cmp_ui_node_t **out_node);

/**
 * \brief Updates the icon displayed in the FAB.
 *
 * \param fab The FAB component.
 * \param icon_name The new icon name or path to display.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_fab_set_icon(cmp_ui_fab_t *fab, const char *icon_name);

/**
 * \brief Binds the widget to the accessibility tree.
 *
 * \param widget The component.
 * \param tree The accessibility tree.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_fab_bind_a11y(cmp_ui_fab_t *widget, cmp_a11y_tree_t *tree);
#ifdef __cplusplus

#endif /* __cplusplus */

#endif /* CMP_UI_FAB_H */