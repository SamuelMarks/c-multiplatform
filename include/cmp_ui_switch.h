#ifndef CMP_UI_SWITCH_H
#define CMP_UI_SWITCH_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_switch.h
 * \brief UI Component for rendering a Switch (toggle).
 */

/**
 * \brief Opaque handle to a Switch UI component.
 */
typedef struct cmp_ui_switch cmp_ui_switch_t;

/**
 * \brief Creates a new Switch component.
 *
 * \param out_switch Pointer to store the created handle.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_switch_create(cmp_ui_switch_t **out_switch);

/**
 * \brief Destroys a Switch component.
 *
 * \param sw The component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_switch_destroy(cmp_ui_switch_t *sw);

/**
 * \brief Retrieves the underlying UI node for the Switch.
 *
 * \param sw The component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_switch_get_node(cmp_ui_switch_t *sw,
                                   cmp_ui_node_t **out_node);

/**
 * \brief Toggles the state of the switch.
 *
 * \param sw The component.
 * \param is_on 1 for ON, 0 for OFF.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_switch_set_on(cmp_ui_switch_t *sw, int is_on);

/**
 * \brief Binds the widget to the accessibility tree.
 *
 * \param widget The component.
 * \param tree The accessibility tree.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_switch_bind_a11y(cmp_ui_switch_t *widget,
                                    cmp_a11y_tree_t *tree);
#ifdef __cplusplus

#endif /* __cplusplus */

#endif /* CMP_UI_SWITCH_H */