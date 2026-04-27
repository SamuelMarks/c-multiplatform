#ifndef CMP_UI_DIVIDER_H
#define CMP_UI_DIVIDER_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_divider.h
 * \brief UI Component for rendering a Divider line.
 */

/**
 * \brief Opaque handle to a Divider UI component.
 */
typedef struct cmp_ui_divider cmp_ui_divider_t;

/**
 * \brief Creates a new Divider component.
 *
 * \param out_divider Pointer to store the created handle.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_divider_create(cmp_ui_divider_t **out_divider);

/**
 * \brief Destroys a Divider component.
 *
 * \param divider The component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_divider_destroy(cmp_ui_divider_t *divider);

/**
 * \brief Retrieves the underlying UI node for the Divider.
 *
 * \param divider The component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_divider_get_node(cmp_ui_divider_t *divider,
                                    cmp_ui_node_t **out_node);

/**
 * \brief Binds the widget to the accessibility tree.
 *
 * \param widget The component.
 * \param tree The accessibility tree.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_divider_bind_a11y(cmp_ui_divider_t *widget,
                                     cmp_a11y_tree_t *tree);
#ifdef __cplusplus

#endif /* __cplusplus */

#endif /* CMP_UI_DIVIDER_H */