#ifndef UI_REACTIVE_GRAPH_H
#define UI_REACTIVE_GRAPH_H

/**
 * \file ui_reactive_graph.h
 * \brief UI Reactive Graph component.
 *
 * This file contains definitions for tracking dependencies within
 * the reactive signal graph. It provides thread-local storage access
 * for identifying the currently evaluating computed signal/effect.
 */

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Notification callback for reactive graph changes.
 *
 * @param user_data Opaque pointer to user data.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_notify_fn)(void *user_data);

/**
 * @brief Represents a listener in the reactive graph.
 */
struct ui_reactive_node {
  ui_notify_fn notify_fn; /**< Callback invoked when dependency changes */
  void *user_data;        /**< Opaque user data for the callback */
};

/**
 * @brief Gets the currently evaluating reactive node for the current thread.
 *
 * @param out_node Pointer to store the result.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_reactive_graph_get_current_node(struct ui_reactive_node **out_node);

/**
 * @brief Sets the currently evaluating reactive node for the current thread.
 *
 * @param node The node to set as current.
 * @param out_prev_node Pointer to store the previously active node.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_reactive_graph_set_current_node(struct ui_reactive_node *node,
                                   struct ui_reactive_node **out_prev_node);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_REACTIVE_GRAPH_H */
