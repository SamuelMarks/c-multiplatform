/* clang-format off */
#include "ui_reactive_graph.h"
#include "ui_error.h"
#include <stdio.h>
/* clang-format on */

static int test_reactive_graph(void) {
  struct ui_reactive_node node1;
  struct ui_reactive_node *out_node = NULL;
  struct ui_reactive_node *prev_node = NULL;

  if (ui_reactive_graph_get_current_node(&out_node) != UI_ERROR_NONE)
    return 1;
  if (out_node != NULL)
    return 1;

  if (ui_reactive_graph_set_current_node(&node1, &prev_node) != UI_ERROR_NONE)
    return 1;
  if (prev_node != NULL)
    return 1;

  if (ui_reactive_graph_get_current_node(&out_node) != UI_ERROR_NONE)
    return 1;
  if (out_node != &node1)
    return 1;

  if (ui_reactive_graph_set_current_node(NULL, &prev_node) != UI_ERROR_NONE)
    return 1;
  if (prev_node != &node1)
    return 1;

  /* Null checks */
  ui_reactive_graph_get_current_node(NULL);

  return 0;
}

int main(void) {
  if (test_reactive_graph())
    return 1;
  return 0;
}
