#ifndef UI_REACTIVE_GRAPH_H
#define UI_REACTIVE_GRAPH_H

/* clang-format off */
#include "../include/ui_types.h"
#include "../include/ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Notification callback for reactive graph changes.
 */
typedef ui_error_t (*ui_notify_fn)(void *user_data);

/**
 * @brief Represents a listener in the reactive graph.
 */
struct ui_reactive_node {
  ui_notify_fn notify_fn;
  void *user_data;
};

/**
 * @brief Gets the currently evaluating reactive node for the current thread.
 *
 * @param out_node Pointer to store the result.
 * @return ui_error_t
 */
ui_error_t
ui_reactive_graph_get_current_node(struct ui_reactive_node **out_node);

/**
 * @brief Sets the currently evaluating reactive node for the current thread.
 *
 * @param node The node to set as current.
 * @param out_prev_node Pointer to store the previously active node.
 * @return ui_error_t
 */
ui_error_t
ui_reactive_graph_set_current_node(struct ui_reactive_node *node,
                                   struct ui_reactive_node **out_prev_node);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_REACTIVE_GRAPH_H */
