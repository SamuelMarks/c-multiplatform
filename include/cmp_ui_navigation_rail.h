#ifndef CMP_UI_NAVIGATION_RAIL_H
#define CMP_UI_NAVIGATION_RAIL_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_navigation_rail.h
 * \brief UI Component for rendering a Navigation Rail (side navigation).
 */

/**
 * \brief Opaque handle to a Navigation Rail UI component.
 */
typedef struct cmp_ui_navigation_rail cmp_ui_navigation_rail_t;

/**
 * \brief Creates a new Navigation Rail component.
 *
 * \param out_rail Pointer to store the created handle.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_navigation_rail_create(cmp_ui_navigation_rail_t **out_rail);

/**
 * \brief Destroys a Navigation Rail component.
 *
 * \param rail The component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_navigation_rail_destroy(cmp_ui_navigation_rail_t *rail);

/**
 * \brief Retrieves the underlying UI node for the Navigation Rail.
 *
 * \param rail The component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_navigation_rail_get_node(cmp_ui_navigation_rail_t *rail,
                                            cmp_ui_node_t **out_node);

/**
 * \brief Adds a destination (icon + optional label) to the Navigation Rail.
 *
 * \param rail The component.
 * \param icon_name The icon.
 * \param label The optional label text.
 * \param out_index Receives the index of the newly added destination.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int
cmp_ui_navigation_rail_add_destination(cmp_ui_navigation_rail_t *rail,
                                       const char *icon_name, const char *label,
                                       int *out_index);

/**
 * \brief Sets the currently active destination.
 *
 * \param rail The component.
 * \param index The index to select.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_navigation_rail_set_selected(cmp_ui_navigation_rail_t *rail,
                                                int index);

/**
 * \brief Binds the widget to the accessibility tree.
 *
 * \param widget The component.
 * \param tree The accessibility tree.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_navigation_rail_bind_a11y(cmp_ui_navigation_rail_t *widget,
                                             cmp_a11y_tree_t *tree);

#ifdef __cplusplus

#endif /* __cplusplus */

#endif /* CMP_UI_NAVIGATION_RAIL_H */