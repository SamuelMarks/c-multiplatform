/* clang-format off */
#include "../src/ui_form_node_internal.h"
#include "../include/ui_form_control.h"
#include "../include/ui_form_group.h"
#include "../include/ui_form_array.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

struct dummy_ui_form_control {
  struct ui_arena *arena;
  ui_signal_t *value_signal;
  ui_signal_t *status_signal;
};

static int test_form_node(void) {
  ui_form_node_t node = {0};
  enum ui_form_status status;
  struct ui_form_control *ctrl = NULL;
  struct ui_form_group *grp = NULL;
  struct ui_form_array *arr = NULL;
  struct dummy_ui_form_control dummy_ctrl = {0};

  union ui_signal_payload dummy;
  dummy.int_val = 0;
  ui_form_control_create(NULL, dummy, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                         UI_SIGNAL_MODE_SINGLE_THREADED, &ctrl);
  ui_form_group_create(NULL, UI_SIGNAL_MODE_SINGLE_THREADED, &grp);
  ui_form_array_create(NULL, UI_SIGNAL_MODE_SINGLE_THREADED, &arr);

  /* NULL status */
  _ui_form_node_get_status_internal(node, NULL, 0);

  /* Over depth */
  _ui_form_node_get_status_internal(node, &status, 100);

  /* Control */
  node.type = UI_FORM_NODE_CONTROL;
  node.node.control = ctrl;
  _ui_form_node_get_status_internal(node, &status, 0);

  node.node.control = NULL;
  _ui_form_node_get_status_internal(node, &status, 0);

  dummy_ctrl.status_signal = NULL;
  node.node.control = (struct ui_form_control *)&dummy_ctrl;
  _ui_form_node_get_status_internal(node, &status, 0);

  /* Group */
  node.type = UI_FORM_NODE_GROUP;
  node.node.group = grp;
  _ui_form_node_get_status_internal(node, &status, 0);

  /* Array */
  node.type = UI_FORM_NODE_ARRAY;
  node.node.array = arr;
  _ui_form_node_get_status_internal(node, &status, 0);

  /* Invalid type */
  node.type = 99;
  _ui_form_node_get_status_internal(node, &status, 0);

  (void)ui_form_control_destroy(ctrl);
  ui_form_group_destroy(grp);
  ui_form_array_destroy(arr);

  return 0;
}

static int run_extra_form_node(void);

int main(void) {
  if (test_form_node() || run_extra_form_node())
    return 1;
  printf("test_ui_form_node passed\n");
  return 0;
}

static int run_extra_form_node(void) {
  ui_form_node_t node = {0};
  enum ui_form_status status;
  struct ui_arena *arena;

  ui_arena_create(1024, &arena);

  node.type = UI_FORM_NODE_CONTROL;
  ui_form_control_create(arena, (union ui_signal_payload){0},
                         UI_SIGNAL_TYPE_INT32, NULL, NULL,
                         UI_SIGNAL_MODE_SINGLE_THREADED, &node.node.control);
  _ui_form_node_get_status_internal(node, &status, 0);

  /* Test null status to trigger UI_ERROR_INVALID_ARGUMENT check */
  if (_ui_form_node_get_status_internal(node, NULL, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Test depth to trigger UI_ERROR_OUT_OF_BOUNDS check */
  if (_ui_form_node_get_status_internal(node, &status, UI_FORM_MAX_DEPTH + 1) !=
      UI_ERROR_OUT_OF_BOUNDS)
    return 1;

  /* Test invalid control */
  node.node.control = NULL;
  if (_ui_form_node_get_status_internal(node, &status, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Test invalid type */
  node.type = 999;
  if (_ui_form_node_get_status_internal(node, &status, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  (void)ui_arena_destroy(arena);

  return 0;
}
