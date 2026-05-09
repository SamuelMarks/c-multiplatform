#ifndef CMP_UI_TABS_H
#define CMP_UI_TABS_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_tabs.h
 * \brief UI Component for rendering Tabs (Primary/Secondary).
 */

/**
 * \brief Opaque handle to a Tabs UI component.
 */
typedef struct cmp_ui_tabs cmp_ui_tabs_t;

/**
 * \brief Creates a new Tabs component.
 *
 * \param out_tabs Pointer to store the created handle.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_tabs_create(cmp_ui_tabs_t **out_tabs);

/**
 * \brief Destroys a Tabs component.
 *
 * \param tabs The component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_tabs_destroy(cmp_ui_tabs_t *tabs);

/**
 * \brief Retrieves the underlying UI node for the Tabs component.
 *
 * \param tabs The component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_tabs_get_node(cmp_ui_tabs_t *tabs, cmp_ui_node_t **out_node);

/**
 * \brief Adds a tab to the component.
 *
 * \param tabs The component.
 * \param title The title text.
 * \param out_index Receives the index of the newly added tab.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_tabs_add_tab(cmp_ui_tabs_t *tabs, const char *title,
                                int *out_index);

/**
 * \brief Sets the currently active tab.
 *
 * \param tabs The component.
 * \param index The index to select.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_tabs_set_selected(cmp_ui_tabs_t *tabs, int index);

/**
 * \brief Binds the widget to the accessibility tree.
 *
 * \param widget The component.
 * \param tree The accessibility tree.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_tabs_bind_a11y(cmp_ui_tabs_t *widget, cmp_a11y_tree_t *tree);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UI_TABS_H */
