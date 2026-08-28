/**
 * @file ui_form_node_internal.h
 * @brief Internal declarations for form nodes.
 */
#ifndef UI_FORM_NODE_INTERNAL_H
#define UI_FORM_NODE_INTERNAL_H

/* clang-format off */
#include "ui_form_node.h"
#include "ui_form_control.h"
/* clang-format on */

/** @def UI_FORM_MAX_DEPTH
 * @brief Maximum depth for form node
 */
#define UI_FORM_MAX_DEPTH 64

/** @brief Get status of form node (internal)
 * @param node Node
 * @param depth Recursion depth
 * @param out_status Out status
 * @return Error code
 */
ui_error_t _ui_form_node_get_status_internal(ui_form_node_t node,
                                             enum ui_form_status *out_status,
                                             size_t depth);

/**
 * @brief Internal function to get the status of a form group.
 * @param group The form group.
 * @param out_status Pointer to store the status.
 * @param depth The current recursion depth.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t _ui_form_group_get_status_internal(struct ui_form_group *group,
                                              enum ui_form_status *out_status,
                                              size_t depth);

/**
 * @brief Internal function to get the status of a form array.
 * @param array The form array.
 * @param out_status Pointer to store the status.
 * @param depth The current recursion depth.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t _ui_form_array_get_status_internal(struct ui_form_array *array,
                                              enum ui_form_status *out_status,
                                              size_t depth);

#endif /* UI_FORM_NODE_INTERNAL_H */
