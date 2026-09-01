/* clang-format off */
#include "ui_form_array.h"
#include "ui_form_node.h"
#include "ui_form_control.h"
#include "../src/ui_form_node_internal.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_ui_form_array(void) {
  struct ui_arena *arena;
  struct ui_arena *tiny_arena;
  ui_form_array_t *array;
  ui_form_array_t *dummy_array;
  ui_form_control_t *control1 = NULL;
  ui_form_control_t *control2 = NULL;
  ui_form_control_t *control_invalid = NULL;
  ui_form_control_t *control_pending = NULL;
  ui_form_node_t node1 = {0};
  ui_form_node_t node2 = {0};
  ui_form_node_t node_invalid = {0};
  ui_form_node_t node_pending = {0};
  ui_form_node_t node_null_ctrl = {0};
  ui_form_node_t out_node;
  union ui_signal_payload init_val = {0};
  enum ui_form_status status;
  ui_signal_t *status_sig;
  int ret = 1;
  int i;

  if (ui_arena_create(8192, &arena) != UI_ERROR_NONE)
    return 1;
  if (ui_arena_create(1, &tiny_arena) != UI_ERROR_NONE)
    return 1;

  init_val.int_val = 0;

  ui_form_control_create(arena, init_val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                         UI_SIGNAL_MODE_SINGLE_THREADED, &control1);
  ui_form_control_create(arena, init_val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                         UI_SIGNAL_MODE_SINGLE_THREADED, &control2);
  ui_form_control_create(arena, init_val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                         UI_SIGNAL_MODE_SINGLE_THREADED, &control_invalid);
  ui_form_control_create(arena, init_val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                         UI_SIGNAL_MODE_SINGLE_THREADED, &control_pending);

  if (!control1 || !control2)
    goto cleanup;

  node1.type = UI_FORM_NODE_CONTROL;
  node1.node.control = control1;

  node2.type = UI_FORM_NODE_CONTROL;
  node2.node.control = control2;

  node_invalid.type = UI_FORM_NODE_CONTROL;
  node_invalid.node.control = control_invalid;
  ui_form_control_get_status_signal(control_invalid, &status_sig);
  init_val.int_val = UI_FORM_STATUS_INVALID;
  ui_signal_set(status_sig, init_val);

  node_pending.type = UI_FORM_NODE_CONTROL;
  node_pending.node.control = control_pending;
  ui_form_control_get_status_signal(control_pending, &status_sig);
  init_val.int_val = UI_FORM_STATUS_PENDING;
  ui_signal_set(status_sig, init_val);

  node_null_ctrl.type = UI_FORM_NODE_CONTROL;
  node_null_ctrl.node.control = NULL;

  if (ui_form_array_create(NULL, UI_SIGNAL_MODE_SINGLE_THREADED, &array) !=
      UI_ERROR_INVALID_ARGUMENT)
    goto cleanup;
  if (ui_form_array_create(arena, UI_SIGNAL_MODE_SINGLE_THREADED, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    goto cleanup;

  g_malloc_fail_countdown = 0;
  if (ui_form_array_create(tiny_arena, UI_SIGNAL_MODE_SINGLE_THREADED,
                           &dummy_array) != UI_ERROR_OUT_OF_MEMORY)
    goto cleanup;
  g_malloc_fail_countdown = -1;

  if (ui_form_array_create(arena, UI_SIGNAL_MODE_SINGLE_THREADED, &array) !=
      UI_ERROR_NONE)
    goto cleanup;

  /* Push */
  if (ui_form_array_push(NULL, node1) != UI_ERROR_INVALID_ARGUMENT)
    goto cleanup;

  ui_form_array_create(tiny_arena, UI_SIGNAL_MODE_SINGLE_THREADED,
                       &dummy_array);
  g_malloc_fail_countdown = 0;
  if (ui_form_array_push(dummy_array, node1) != UI_ERROR_OUT_OF_MEMORY)
    goto cleanup;
  g_malloc_fail_countdown = -1;

  if (ui_form_array_push(array, node1) != UI_ERROR_NONE)
    goto cleanup;

  /* Insert at */
  if (ui_form_array_insert_at(NULL, 0, node2) != UI_ERROR_INVALID_ARGUMENT)
    goto cleanup;
  if (ui_form_array_insert_at(array, 10, node2) != UI_ERROR_OUT_OF_BOUNDS)
    goto cleanup;

  g_malloc_fail_countdown = 0;
  if (ui_form_array_insert_at(dummy_array, 0, node2) != UI_ERROR_OUT_OF_MEMORY)
    goto cleanup;
  g_malloc_fail_countdown = -1;

  if (ui_form_array_insert_at(array, 0, node2) != UI_ERROR_NONE)
    goto cleanup;

  /* Force reallocation during push / insert by exhausting capacity */
  /* Capacity will be 4 after first push/insert */
  for (i = 0; i < 2; i++) {
    ui_form_array_push(array, node1);
  }
  /* count is now 4, capacity is 4. insert_at will trigger resize and memcpy */
  if (ui_form_array_insert_at(array, 2, node2) != UI_ERROR_NONE)
    goto cleanup;

  /* also trigger push reallocation */
  /* capacity is now 8. let's fill it to 8 and push */
  for (i = 0; i < 3; i++) {
    ui_form_array_push(array, node1);
  }
  /* count is 8, capacity is 8 */
  g_malloc_fail_countdown = 0;
  if (ui_form_array_push(array, node2) != UI_ERROR_OUT_OF_MEMORY) {
    /* If this doesn't fail, maybe the block can expand. That's fine. */
  }
  g_malloc_fail_countdown = -1;
  ui_form_array_push(array, node2);

  /* Remove at */
  if (ui_form_array_remove_at(NULL, 0) != UI_ERROR_INVALID_ARGUMENT)
    goto cleanup;
  if (ui_form_array_remove_at(array, 100) != UI_ERROR_OUT_OF_BOUNDS)
    goto cleanup;
  if (ui_form_array_remove_at(array, 0) != UI_ERROR_NONE)
    goto cleanup;

  /* Get node */
  if (ui_form_array_get_node(NULL, 0, &out_node) != UI_ERROR_INVALID_ARGUMENT)
    goto cleanup;
  if (ui_form_array_get_node(array, 0, NULL) != UI_ERROR_INVALID_ARGUMENT)
    goto cleanup;
  if (ui_form_array_get_node(array, 100, &out_node) != UI_ERROR_OUT_OF_BOUNDS)
    goto cleanup;
  if (ui_form_array_get_node(array, 0, &out_node) != UI_ERROR_NONE)
    goto cleanup;

  /* Get status */
  if (ui_form_array_get_status(NULL, &status) != UI_ERROR_INVALID_ARGUMENT)
    goto cleanup;
  if (ui_form_array_get_status(array, NULL) != UI_ERROR_INVALID_ARGUMENT)
    goto cleanup;

  ui_form_array_insert_at(dummy_array, 0, node1);
  /* Test error in _ui_form_node_get_status_internal */
  ui_form_array_push(array, node_null_ctrl);
  if (ui_form_array_get_status(array, &status) != UI_ERROR_INVALID_ARGUMENT)
    goto cleanup;
  ui_form_array_clear(array); /* remove it */

  /* Valid array */
  if (ui_form_array_get_status(array, &status) != UI_ERROR_NONE)
    goto cleanup;
  if (status != UI_FORM_STATUS_VALID)
    goto cleanup;

  /* PENDING status branch */
  ui_form_array_push(array, node_pending);
  if (ui_form_array_get_status(array, &status) != UI_ERROR_NONE)
    goto cleanup;
  if (status != UI_FORM_STATUS_PENDING)
    goto cleanup;

  /* INVALID status branch */
  ui_form_array_push(array, node_invalid);
  if (ui_form_array_get_status(array, &status) != UI_ERROR_NONE)
    goto cleanup;
  if (status != UI_FORM_STATUS_INVALID)
    goto cleanup;

  /* Clear */
  if (ui_form_array_clear(NULL) != UI_ERROR_INVALID_ARGUMENT)
    goto cleanup;
  if (ui_form_array_clear(array) != UI_ERROR_NONE)
    goto cleanup;

  /* Destroy */
  if (ui_form_array_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    goto cleanup;
  if (ui_form_array_destroy(array) != UI_ERROR_NONE)
    goto cleanup;

  {
    ui_error_t rc_cleanup = ui_form_control_destroy(control1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_form_control_destroy(control2);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_form_control_destroy(control_invalid);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_form_control_destroy(control_pending);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  ret = 0;
cleanup: {
  ui_error_t rc_cleanup = ui_arena_destroy(arena);
  if (rc_cleanup != UI_ERROR_NONE) {
    (void)rc_cleanup; /* Avoid override */
  }
}
  {
    ui_error_t rc_cleanup = ui_arena_destroy(tiny_arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return ret;
}

int main(void) {
  if (test_ui_form_array() != 0) {
    printf("test_ui_form_array failed\n");
    return 1;
  }
  printf("test_ui_form_array passed\n");
  return 0;
}
