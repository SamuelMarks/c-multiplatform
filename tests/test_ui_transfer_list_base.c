/* clang-format off */
#include "greatest.h"
#include "ui_transfer_list_base.h"
#include "ui_error.h"
#include "ui_component.h"
#include "ui_control_value_accessor.h"
#include "ui_test_mock_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
extern int g_transfer_mock_fail;

static int g_cb_touched_fail = 0;
static int g_cb_change_fail = 0;

static ui_error_t mock_on_change(union ui_signal_payload new_value,
                                 void *user_data) {
  void *unused_u = user_data;
  user_data = unused_u;
  if (new_value.ptr_val) {
    free(new_value.ptr_val);
  }
  if (g_cb_change_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return UI_ERROR_NONE;
}

static ui_error_t mock_on_touched(void *user_data) {
  void *unused_u = user_data;
  user_data = unused_u;
  if (g_cb_touched_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return UI_ERROR_NONE;
}

TEST test_transfer_invalid_args(void) {
  struct ui_transfer_list_base list;
  struct ui_component comp;
  struct ui_control_value_accessor cva;
  union ui_signal_payload empty_payload;

  memset(&empty_payload, 0, sizeof(empty_payload));
  memset(&list, 0, sizeof(list));
  memset(&comp, 0, sizeof(comp));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_transfer_list_base_init(NULL, &comp, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_transfer_list_base_init(&list, NULL, NULL));
  ASSERT_EQ(UI_ERROR_NONE, ui_transfer_list_base_cleanup(NULL));

  ASSERT_EQ(UI_ERROR_NONE, ui_transfer_list_base_init(&list, &comp, &cva));

  /* CVA invalid args */
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, cva.write_value(NULL, empty_payload));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            cva.register_on_change(NULL, NULL, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            cva.register_on_touched(NULL, NULL, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, cva.set_disabled_state(NULL, 1));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_transfer_list_base_add_item(NULL, 0, 1, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_transfer_list_base_set_selected(NULL, 1, 1));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_transfer_list_base_move_selected(NULL, 1));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_transfer_list_base_move_all(NULL, 1));

  ASSERT_EQ(UI_ERROR_NONE, ui_transfer_list_base_cleanup(&list));

  PASS();
}

TEST test_transfer_lifecycle_and_moves(void) {
  struct ui_transfer_list_base list;
  struct ui_component comp;
  struct ui_control_value_accessor cva;
  union ui_signal_payload sp;
  struct ui_transfer_list_payload *pl;
  ui_error_t rc;

  memset(&list, 0, sizeof(list));
  memset(&comp, 0, sizeof(comp));

  rc = ui_transfer_list_base_init(&list, &comp, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cva.register_on_change(&list, mock_on_change, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cva.register_on_touched(&list, mock_on_touched, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Add items to left and right */
  rc = ui_transfer_list_base_add_item(&list, 0, 1, NULL); /* Left */
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_transfer_list_base_add_item(&list, 0, 2, NULL); /* Left */
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_transfer_list_base_add_item(&list, 1, 3, NULL); /* Right */
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Select non-existent item */
  ASSERT_EQ(UI_ERROR_NOT_FOUND,
            ui_transfer_list_base_set_selected(&list, 99, 1));

  /* Select and deselect items */
  rc = ui_transfer_list_base_set_selected(&list, 1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Setting same selection is no-op */
  rc = ui_transfer_list_base_set_selected(&list, 1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_transfer_list_base_set_selected(&list, 3, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Move selected left to right */
  rc = ui_transfer_list_base_move_selected(&list, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Move selected right to left */
  rc = ui_transfer_list_base_set_selected(&list, 3, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_transfer_list_base_move_selected(&list, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Move all left to right */
  rc = ui_transfer_list_base_move_all(&list, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Move all right to left */
  rc = ui_transfer_list_base_move_all(&list, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Move when list has no selected items (moved == 0) */
  rc = ui_transfer_list_base_move_selected(&list, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Disable and verify operations early return */
  rc = cva.set_disabled_state(&list, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, list.is_disabled);

  rc = ui_transfer_list_base_set_selected(&list, 1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_transfer_list_base_move_selected(&list, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_transfer_list_base_move_all(&list, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cva.set_disabled_state(&list, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* write_value */
  pl = (struct ui_transfer_list_payload *)malloc(
      sizeof(struct ui_transfer_list_payload));
  ASSERT(pl != NULL);
  pl->left_list = NULL;
  pl->right_list = NULL;
  sp.ptr_val = pl;
  rc = cva.write_value(&list, sp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  sp.ptr_val = NULL;
  rc = cva.write_value(&list, sp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_transfer_list_base_cleanup(&list);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test operations without CVA callbacks and move_all when empty */
  {
    struct ui_transfer_list_base list2;
    memset(&list2, 0, sizeof(list2));
    rc = ui_transfer_list_base_init(&list2, &comp, NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* Move all when empty (moved == 0) */
    rc = ui_transfer_list_base_move_all(&list2, 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* Add item without cva_on_change */
    rc = ui_transfer_list_base_add_item(&list2, 0, 10, NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* Set selected without cva_on_touched */
    rc = ui_transfer_list_base_set_selected(&list2, 10, 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = ui_transfer_list_base_cleanup(&list2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  PASS();
}

TEST test_transfer_error_branches(void) {
  struct ui_transfer_list_base list;
  struct ui_component comp;
  struct ui_control_value_accessor cva;
  ui_error_t rc;

  memset(&list, 0, sizeof(list));
  memset(&comp, 0, sizeof(comp));

  rc = ui_transfer_list_base_init(&list, &comp, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cva.register_on_change(&list, mock_on_change, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cva.register_on_touched(&list, mock_on_touched, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_transfer_list_base_add_item(&list, 0, 1, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_transfer_list_base_add_item(&list, 1, 2, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 1. cva_on_touched failure */
  g_cb_touched_fail = 1;
  rc = ui_transfer_list_base_set_selected(&list, 1, 1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  rc = ui_transfer_list_base_move_selected(&list, 1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  rc = ui_transfer_list_base_move_all(&list, 1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cb_touched_fail = 0;

  /* 2. cva_on_change failure */
  g_cb_change_fail = 1;
  rc = ui_transfer_list_base_set_selected(&list, 1, 0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  rc = ui_transfer_list_base_move_all(&list, 1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cb_change_fail = 0;

  rc = ui_transfer_list_base_set_selected(&list, 2, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  g_cb_change_fail = 1;
  rc = ui_transfer_list_base_move_selected(&list, 0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cb_change_fail = 0;

  /* 3. move_items mock failure (line 317, 356) */
  g_transfer_mock_fail = 2;
  rc = ui_transfer_list_base_move_selected(&list, 1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  rc = ui_transfer_list_base_move_all(&list, 1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_transfer_mock_fail = 0;

  /* 4. free_list mock failure in cleanup (lines 395-396, 401-402) */
  g_transfer_mock_fail = 1;
  rc = ui_transfer_list_base_cleanup(&list);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_transfer_mock_fail = 0;

  PASS();
}

TEST test_transfer_oom(void) {
  struct ui_transfer_list_base list;
  struct ui_component comp;
  struct ui_control_value_accessor cva;
  ui_error_t rc;

  memset(&list, 0, sizeof(list));
  memset(&comp, 0, sizeof(comp));

  rc = ui_transfer_list_base_init(&list, &comp, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cva.register_on_change(&list, mock_on_change, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* add_item item malloc OOM */
  g_malloc_fail_countdown = 0;
  rc = ui_transfer_list_base_add_item(&list, 0, 1, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* trigger_cva_change payload malloc OOM */
  g_malloc_fail_countdown = 1;
  rc = ui_transfer_list_base_add_item(&list, 0, 1, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_malloc_fail_countdown = -1;

  rc = ui_transfer_list_base_cleanup(&list);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(ui_transfer_list_base_suite) {
  RUN_TEST(test_transfer_invalid_args);
  RUN_TEST(test_transfer_lifecycle_and_moves);
  RUN_TEST(test_transfer_error_branches);
  RUN_TEST(test_transfer_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_transfer_list_base_suite);
  GREATEST_MAIN_END();
}
