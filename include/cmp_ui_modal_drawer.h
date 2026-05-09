#ifndef CMP_UI_MODAL_DRAWER_H
#define CMP_UI_MODAL_DRAWER_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_modal_drawer.h
 * \brief UI Component for rendering a Modal Navigation Drawer.
 */

/**
 * \brief Opaque handle to a Modal Drawer UI component.
 */
typedef struct cmp_ui_modal_drawer cmp_ui_modal_drawer_t;

/**
 * \brief Creates a new Modal Drawer component.
 *
 * \param out_drawer Pointer to store the created handle.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_modal_drawer_create(cmp_ui_modal_drawer_t **out_drawer);

/**
 * \brief Destroys a Modal Drawer component.
 *
 * \param drawer The component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_modal_drawer_destroy(cmp_ui_modal_drawer_t *drawer);

/**
 * \brief Retrieves the underlying UI node for the Modal Drawer.
 *
 * \param drawer The component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_modal_drawer_get_node(cmp_ui_modal_drawer_t *drawer,
                                         cmp_ui_node_t **out_node);

/**
 * \brief Sets whether the drawer is open or closed.
 *
 * \param drawer The component.
 * \param open 1 to open, 0 to close.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_modal_drawer_set_open(cmp_ui_modal_drawer_t *drawer,
                                         int open);

/**
 * \brief Adds a navigation item to the drawer.
 *
 * \param drawer The component.
 * \param icon_name The icon name.
 * \param label The label text.
 * \param out_index Receives the index of the newly added item.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_modal_drawer_add_item(cmp_ui_modal_drawer_t *drawer,
                                         const char *icon_name,
                                         const char *label, int *out_index);

/**
 * \brief Sets the currently selected item in the drawer.
 *
 * \param drawer The component.
 * \param index The item index to select.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_modal_drawer_set_selected(cmp_ui_modal_drawer_t *drawer,
                                             int index);

/**
 * \brief Binds the widget to the accessibility tree.
 *
 * \param widget The component.
 * \param tree The accessibility tree.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_modal_drawer_bind_a11y(cmp_ui_modal_drawer_t *widget,
                                          cmp_a11y_tree_t *tree);
#ifdef __cplusplus

#endif /* __cplusplus */

#endif /* CMP_UI_MODAL_DRAWER_H */
