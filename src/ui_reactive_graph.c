/* clang-format off */
#include "ui_reactive_graph.h"
#include <stddef.h>
#include "../include/ui_types.h"
#include "../include/ui_error.h"
/* clang-format on */

static UI_THREAD_LOCAL struct ui_reactive_node *g_current_node = NULL;

/** \brief ui_error */
enum ui_error
ui_reactive_graph_get_current_node(struct ui_reactive_node **out_node) {
  if (!out_node)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_node = g_current_node;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_reactive_graph_set_current_node(struct ui_reactive_node *node,
                                   struct ui_reactive_node **out_prev_node) {
  if (out_prev_node)
    *out_prev_node = g_current_node;
  g_current_node = node;
  return UI_ERROR_NONE;
}
