/**
 * \file ui_reactive_graph.c
 * \brief Implementation of the UI Reactive Graph component.
 */

/* clang-format off */
#include "ui_reactive_graph.h"
#include <stddef.h>
#include "ui_types.h"
#include "ui_error.h"
/* clang-format on */

/** \brief Thread-local pointer to the currently evaluating reactive node */
static UI_THREAD_LOCAL struct ui_reactive_node *g_current_node = NULL;

/**
 * \brief Gets the currently evaluating reactive node for the current thread.
 *
 * \param out_node Pointer to store the result.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_reactive_graph_get_current_node(struct ui_reactive_node **out_node) {
  if (!out_node)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_node = g_current_node;
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the currently evaluating reactive node for the current thread.
 *
 * \param node The node to set as current.
 * \param out_prev_node Pointer to store the previously active node.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_reactive_graph_set_current_node(struct ui_reactive_node *node,
                                   struct ui_reactive_node **out_prev_node) {
  if (out_prev_node)
    *out_prev_node = g_current_node;
  g_current_node = node;
  return UI_ERROR_NONE;
}
