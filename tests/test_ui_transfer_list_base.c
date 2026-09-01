/* clang-format off */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "ui_transfer_list_base.h"
#include "ui_error.h"
/* clang-format on */

struct ui_component {
  int id;
};

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

static ui_error_t dummy_on_change(union ui_signal_payload new_value,
                                  void *user_data) {
  if (new_value.ptr_val) {
    free(new_value.ptr_val);
  }
  return UI_ERROR_NONE;
}

static ui_error_t dummy_on_touched(void *user_data) { return UI_ERROR_NONE; }

static int test_transfer_list_init(void) {
  struct ui_transfer_list_base list;
  struct ui_component comp;
  struct ui_control_value_accessor cva;
  ui_error_t err;
  int failed = 0;

  err = ui_transfer_list_base_init(NULL, &comp, NULL);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_transfer_list_base_init(&list, NULL, NULL);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_transfer_list_base_init(&list, &comp, &cva);
  failed |= (err != UI_ERROR_NONE);
  failed |= (list.component != &comp);
  failed |= (list.left_list != NULL);
  failed |= (list.right_list != NULL);

  /* CVA methods */
  failed |= (cva.write_value(NULL, (union ui_signal_payload){0}) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (cva.register_on_change(NULL, NULL, NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (cva.register_on_touched(NULL, NULL, NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (cva.set_disabled_state(NULL, 1) != UI_ERROR_INVALID_ARGUMENT);

  failed |=
      (cva.register_on_change(&list, dummy_on_change, NULL) != UI_ERROR_NONE);
  failed |=
      (cva.register_on_touched(&list, dummy_on_touched, NULL) != UI_ERROR_NONE);
  failed |= (cva.set_disabled_state(&list, 1) != UI_ERROR_NONE);
  failed |= (list.is_disabled != 1);

  /* test early returns when disabled */
  failed |= (ui_transfer_list_base_set_selected(&list, 99, 1) != UI_ERROR_NONE);
  failed |= (ui_transfer_list_base_move_selected(&list, 1) != UI_ERROR_NONE);
  failed |= (ui_transfer_list_base_move_all(&list, 1) != UI_ERROR_NONE);

  /* re-enable to allow operations */
  failed |= (cva.set_disabled_state(&list, 0) != UI_ERROR_NONE);

  /* write_value */
  struct ui_transfer_list_payload *pl =
      (struct ui_transfer_list_payload *)malloc(
          sizeof(struct ui_transfer_list_payload));
  union ui_signal_payload sp = {0};
  if (pl) {
    pl->left_list = NULL;
    pl->right_list = NULL;
    sp.ptr_val = pl;
    failed |= (cva.write_value(&list, sp) != UI_ERROR_NONE);
  }

  sp.ptr_val = NULL;
  failed |= (cva.write_value(&list, sp) != UI_ERROR_NONE);

  /* Trigger change logic while CVA is registered */
  ui_transfer_list_base_add_item(&list, 0, 99, NULL);
  ui_transfer_list_base_set_selected(&list, 99, 1);
  ui_transfer_list_base_set_selected(&list, 99, 1);
  ui_transfer_list_base_move_selected(&list, 1);

#ifdef UI_TEST_MOCK_ALLOC
  /* trigger malloc fail in notify_change */
  g_malloc_fail_countdown = 0;
  ui_transfer_list_base_move_all(&list, 0);
  g_malloc_fail_countdown = -1;
#endif

  ui_transfer_list_base_cleanup(&list);

  return failed;
}

static int test_transfer_list_operations(void) {
  struct ui_transfer_list_base list;
  struct ui_component comp;
  ui_error_t err;
  int failed = 0;

  ui_transfer_list_base_init(&list, &comp, NULL);

  /* Add items to left */
  failed |= (ui_transfer_list_base_add_item(NULL, 0, 1, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  failed |= (ui_transfer_list_base_add_item(&list, 0, 1, NULL) !=
             UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
#endif

  failed |=
      (ui_transfer_list_base_add_item(&list, 0, 1, NULL) != UI_ERROR_NONE);
  failed |=
      (ui_transfer_list_base_add_item(&list, 0, 2, NULL) != UI_ERROR_NONE);
  failed |=
      (ui_transfer_list_base_add_item(&list, 0, 3, NULL) != UI_ERROR_NONE);

  /* Add items to right */
  failed |=
      (ui_transfer_list_base_add_item(&list, 1, 4, NULL) != UI_ERROR_NONE);

  failed |= (list.left_list == NULL);
  failed |= (list.right_list == NULL);
  failed |= (list.right_list->id != 4);

  /* Select item 2 on left and move to right */
  err = ui_transfer_list_base_set_selected(NULL, 2, 1);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_transfer_list_base_set_selected(&list, 2, 1);
  failed |= (err != UI_ERROR_NONE);

  err = ui_transfer_list_base_set_selected(&list, 4, 1);
  failed |= (err != UI_ERROR_NONE);

  err = ui_transfer_list_base_set_selected(&list, 999, 1);
  failed |= (err != UI_ERROR_NOT_FOUND);

  /* invalid move direction */
  err = ui_transfer_list_base_move_selected(NULL, 1);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_transfer_list_base_move_selected(&list, 1);
  failed |= (err != UI_ERROR_NONE);

  err = ui_transfer_list_base_move_selected(&list, 0);
  failed |= (err != UI_ERROR_NONE);

  err = ui_transfer_list_base_move_selected(&list, 0);
  failed |= (err != UI_ERROR_NONE);

  err = ui_transfer_list_base_move_selected(&list, 1);
  failed |= (err != UI_ERROR_NONE);

  /* Move all to left */
  err = ui_transfer_list_base_move_all(NULL, 0);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_transfer_list_base_move_all(&list, 0);
  failed |= (err != UI_ERROR_NONE);
  failed |= (list.right_list != NULL);

  err = ui_transfer_list_base_move_all(&list, 0);
  failed |= (err != UI_ERROR_NONE);

  err = ui_transfer_list_base_move_all(&list, 1);
  failed |= (err != UI_ERROR_NONE);

  err = ui_transfer_list_base_move_all(&list, 1);
  failed |= (err != UI_ERROR_NONE);

  ui_transfer_list_base_cleanup(NULL);
  ui_transfer_list_base_cleanup(&list);
  return failed;
}

int main(void) {
  int failed = 0;
  failed |= test_transfer_list_init();
  failed |= test_transfer_list_operations();
  if (!failed) {
    printf("test_ui_transfer_list_base passed\n");
  }
  return failed;
}
