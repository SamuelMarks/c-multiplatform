/**
 * @file test_material3_oom_mock.c
 * @brief Tests verifying graceful OOM handling across Material 3 components.
 */

/* clang-format off */
#include "greatest.h"
#include "ui_test_mock_mem.h"
#include "ui_focus_manager.h"
#include "ui_focus_trap.h"
#include "ui_dom_node.h"
#include "material3/md3_a11y.h"
#include "material3/md3_action_widgets.h"
#include "material3/md3_fab_menu.h"
#include "material3/md3_button.h"
#include "material3/md3_card.h"
#include "material3/md3_carousel.h"
#include "material3/md3_checkbox.h"
#include "material3/md3_chip.h"
#include "material3/md3_divider.h"
#include "material3/md3_fab.h"
#include "material3/md3_icon_button.h"
#include "material3/md3_list.h"
#include "material3/md3_navigation.h"
#include "material3/md3_overlay.h"
#include "material3/md3_overlay_menus.h"
#include "material3/md3_date_time_pickers.h"
#include "material3/md3_progress.h"
#include "material3/md3_radio_button.h"
#include "material3/md3_search.h"
#include "material3/md3_segmented_button.h"
#include "material3/md3_selection_controls.h"
#include "material3/md3_shell_layout.h"
#include "material3/md3_slider.h"
#include "material3/md3_split_button.h"
#include "material3/md3_switch.h"
#include "material3/md3_text_field.h"
#include "material3/md3_typography.h"
#include "ui_error.h"
#include "ui_font_manager.h"
#include <string.h>
/* clang-format on */

static const unsigned char tests_tiny_ttf[] = {
    0x00, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x63, 0x6d, 0x61, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c,
    0x00, 0x00, 0x00, 0x14, 0x68, 0x65, 0x61, 0x64, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x36, 0x68, 0x68, 0x65, 0x61,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0x00, 0x00, 0x00, 0x24,
    0x68, 0x6d, 0x74, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xea,
    0x00, 0x00, 0x00, 0x08, 0x67, 0x6c, 0x79, 0x66, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xf2, 0x00, 0x00, 0x00, 0x01, 0x6c, 0x6f, 0x63, 0x61,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf3, 0x00, 0x00, 0x00, 0x04,
    0x6d, 0x61, 0x78, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf7,
    0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x0c, 0x00, 0x04, 0x00, 0x20, 0x00, 0x00, 0x00, 0x04,
    0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x41, 0xff, 0xff, 0x00, 0x00,
    0x00, 0x41, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x5f, 0x0f, 0x3c, 0xf5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0xe8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x03, 0xe8, 0xff, 0x9c, 0x00, 0x00, 0x03, 0xe8,
    0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0xe8, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00};

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
extern int g_malloc_called;
#endif

TEST test_md3_oom_button(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_button *btn = NULL;
  ui_error_t rc;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_button_create(dummy_engine, MD3_BUTTON_FILLED, &btn);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_button_create(dummy_engine, MD3_BUTTON_FILLED, &btn);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#else
  rc = md3_button_create(NULL, MD3_BUTTON_FILLED, &btn);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

TEST test_md3_oom_list(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_list *list = NULL;
  struct md3_list_item *item = NULL;
  ui_error_t rc;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_list_create(dummy_engine, &list);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_list_create(dummy_engine, &list);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_list_item_create(dummy_engine, MD3_LIST_ITEM_ONE_LINE, &item);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_list_item_create(dummy_engine, MD3_LIST_ITEM_ONE_LINE, &item);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#else
  rc = md3_list_create(NULL, &list);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

TEST test_md3_oom_fab(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_fab *fab = NULL;
  ui_error_t rc;
#ifdef UI_TEST_MOCK_ALLOC
  int total_allocs;
  int c;
#endif

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = -1;
  g_malloc_called = 0;
  rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_REGULAR, MD3_FAB_PRIMARY,
                      "add", "Add", &fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  total_allocs = g_malloc_called;
  rc = md3_fab_destroy(fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (c = 0; c < total_allocs; c++) {
    fab = NULL;
    g_malloc_fail_countdown = c;
    rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_REGULAR, MD3_FAB_PRIMARY,
                        "add", "Add", &fab);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  g_malloc_fail_countdown = -1;
  g_malloc_called = 0;
  rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_REGULAR, MD3_FAB_PRIMARY,
                      "add", NULL, &fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  total_allocs = g_malloc_called;
  rc = md3_fab_destroy(fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (c = 0; c < total_allocs; c++) {
    fab = NULL;
    g_malloc_fail_countdown = c;
    rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_REGULAR, MD3_FAB_PRIMARY,
                        "add", NULL, &fab);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  /* Test OOM in add_speed_dial_action */
  g_malloc_fail_countdown = -1;
  rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_REGULAR, MD3_FAB_PRIMARY,
                      "add", "Add", &fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  g_malloc_called = 0;
  rc = md3_fab_add_speed_dial_action(fab, "icon", "Action", NULL, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  total_allocs = g_malloc_called;

  rc = md3_fab_destroy(fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  fab = NULL;

  for (c = 0; c < total_allocs; c++) {
    struct md3_fab *test_fab = NULL;
    g_malloc_fail_countdown = -1;
    rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_REGULAR, MD3_FAB_PRIMARY,
                        "add", "Add", &test_fab);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_malloc_fail_countdown = c;
    rc = md3_fab_add_speed_dial_action(test_fab, "icon", "Action", NULL, NULL);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    g_malloc_fail_countdown = -1;
    rc = md3_fab_destroy(test_fab);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  g_malloc_fail_countdown = -1;
  rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_REGULAR, MD3_FAB_PRIMARY,
                      "add", "Add", &fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  g_malloc_called = 0;
  rc = md3_fab_add_speed_dial_action(fab, "icon", NULL, NULL, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  total_allocs = g_malloc_called;

  rc = md3_fab_destroy(fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  fab = NULL;

  for (c = 0; c < total_allocs; c++) {
    struct md3_fab *test_fab = NULL;
    g_malloc_fail_countdown = -1;
    rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_REGULAR, MD3_FAB_PRIMARY,
                        "add", "Add", &test_fab);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_malloc_fail_countdown = c;
    rc = md3_fab_add_speed_dial_action(test_fab, "icon", NULL, NULL, NULL);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    g_malloc_fail_countdown = -1;
    rc = md3_fab_destroy(test_fab);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
#else
  rc = md3_fab_create(NULL, MD3_FAB_SIZE_REGULAR, MD3_FAB_PRIMARY, "add", "Add",
                      &fab);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

TEST test_md3_oom_search(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_search_bar *sb = NULL;
  ui_error_t rc;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_search_bar_create(dummy_engine, "Search", &sb, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = -1;
  rc = md3_search_bar_create(dummy_engine, "Search", &sb, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_malloc_fail_countdown = 0;
  rc = md3_search_bar_set_query(sb, "hello");
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
  rc = md3_search_bar_destroy(sb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#else
  rc = md3_search_bar_create(NULL, "Search", &sb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

TEST test_md3_oom_text_field(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_text_field *tf = NULL;
  ui_error_t rc;
#ifdef UI_TEST_MOCK_ALLOC
  int total_allocs;
  int c;
#endif

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = -1;
  g_malloc_called = 0;
  rc = md3_text_field_create(dummy_engine, MD3_TEXT_FIELD_FILLED, &tf, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  total_allocs = g_malloc_called;
  rc = md3_text_field_destroy(tf);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (c = 0; c < total_allocs; c++) {
    tf = NULL;
    g_malloc_fail_countdown = c;
    rc = md3_text_field_create(dummy_engine, MD3_TEXT_FIELD_FILLED, &tf, NULL);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  g_malloc_fail_countdown = -1;
  rc = md3_text_field_create(dummy_engine, MD3_TEXT_FIELD_FILLED, &tf, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_malloc_fail_countdown = 0;
  rc = md3_text_field_set_text(tf, "some text");
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
  rc = md3_text_field_destroy(tf);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#else
  rc = md3_text_field_create(NULL, MD3_TEXT_FIELD_FILLED, &tf, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

TEST test_md3_oom_slider(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_slider *s = NULL;
  struct md3_range_slider *rs = NULL;
  ui_error_t rc;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_slider_create(dummy_engine, MD3_SLIDER_DISCRETE, 0.0f, 100.0f, &s,
                         NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 1;
  rc = md3_slider_create(dummy_engine, MD3_SLIDER_DISCRETE, 0.0f, 100.0f, &s,
                         NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  /* Range slider OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_range_slider_create(dummy_engine, 0.0f, 100.0f, &rs);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 1;
  rc = md3_range_slider_create(dummy_engine, 0.0f, 100.0f, &rs);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = -1;
#else
  rc = md3_slider_create(NULL, MD3_SLIDER_DISCRETE, 0.0f, 100.0f, &s, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

TEST test_md3_oom_segmented_button(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_segmented_button *btn = NULL;
  ui_error_t rc;
#ifdef UI_TEST_MOCK_ALLOC
  int total_allocs;
  int c;

  g_malloc_fail_countdown = -1;
  g_malloc_called = 0;
  rc = md3_segmented_button_create(
      dummy_engine, UI_SEGMENTED_CONTROL_MODE_SINGLE, &btn, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  total_allocs = g_malloc_called;
  rc = md3_segmented_button_destroy(btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (c = 0; c < total_allocs; c++) {
    btn = NULL;
    g_malloc_fail_countdown = c;
    rc = md3_segmented_button_create(
        dummy_engine, UI_SEGMENTED_CONTROL_MODE_SINGLE, &btn, NULL);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  /* Test OOM in add_segment */
  g_malloc_fail_countdown = -1;
  rc = md3_segmented_button_create(
      dummy_engine, UI_SEGMENTED_CONTROL_MODE_SINGLE, &btn, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  g_malloc_called = 0;
  rc = md3_segmented_button_add_segment(btn, "Day", "sun", 1, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  total_allocs = g_malloc_called;
  rc = md3_segmented_button_destroy(btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  btn = NULL;

  for (c = 0; c < total_allocs; c++) {
    struct md3_segmented_button *tbtn = NULL;
    g_malloc_fail_countdown = -1;
    rc = md3_segmented_button_create(
        dummy_engine, UI_SEGMENTED_CONTROL_MODE_SINGLE, &tbtn, NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_malloc_fail_countdown = c;
    rc = md3_segmented_button_add_segment(tbtn, "Day", "sun", 1, NULL);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    g_malloc_fail_countdown = -1;
    rc = md3_segmented_button_destroy(tbtn);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  g_malloc_fail_countdown = -1;
#else
  rc = md3_segmented_button_create(NULL, UI_SEGMENTED_CONTROL_MODE_SINGLE, &btn,
                                   NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

TEST test_md3_oom_chip(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_chip *chip = NULL;
  ui_error_t rc;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_chip_create(dummy_engine, MD3_CHIP_FILTER, &chip, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_chip_create(dummy_engine, MD3_CHIP_FILTER, &chip, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#else
  rc = md3_chip_create(NULL, MD3_CHIP_FILTER, &chip, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

TEST test_md3_oom_navigation_bar(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_navigation_bar *bar = NULL;
  ui_error_t rc;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_navigation_bar_create(dummy_engine, &bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#else
  rc = md3_navigation_bar_create(NULL, &bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

TEST test_md3_oom_dialog(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_dialog *dialog = NULL;
  ui_error_t rc;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_dialog_create(dummy_engine, MD3_DIALOG_ALERT, &dialog);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#else
  rc = md3_dialog_create(NULL, MD3_DIALOG_ALERT, &dialog);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

TEST test_md3_oom_action_widgets(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_fab *fab = NULL;
  struct md3_fab_menu *menu = NULL;
  struct md3_button_group *group = NULL;
  struct md3_button *btn = NULL;
  struct md3_toggle_button *toggle = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct md3_toolbar *toolbar = NULL;
  ui_error_t rc;

  rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_REGULAR, MD3_FAB_PRIMARY,
                      "add", "Add", &fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  /* FAB Menu creation OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_fab_menu_create(dummy_engine, MD3_FAB_MENU_DIRECTION_UP, &menu);
  md3_fab_menu_set_primary_fab(menu, fab);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* FAB Menu add action OOM */
  rc = md3_fab_menu_create(dummy_engine, MD3_FAB_MENU_DIRECTION_UP, &menu);
  md3_fab_menu_set_primary_fab(menu, fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_malloc_fail_countdown = 0;
  rc = md3_fab_menu_add_action(menu, 1, fab, "Photo");
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
  /* Destroy menu without freeing shared fab */
  rc = md3_fab_menu_destroy(menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Button group creation OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_button_group_create(dummy_engine, MD3_BUTTON_GROUP_HORIZONTAL,
                               &group);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_button_group_create(dummy_engine, MD3_BUTTON_GROUP_HORIZONTAL,
                               &group);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Button group add button OOM */
  rc = md3_button_group_create(dummy_engine, MD3_BUTTON_GROUP_HORIZONTAL,
                               &group);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_button_create(dummy_engine, MD3_BUTTON_OUTLINED, &btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_malloc_fail_countdown = 0;
  rc = md3_button_group_add_button(group, btn);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_button_group_add_button(group, btn);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
  rc = md3_button_destroy(btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_button_group_destroy(group);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Toggle button creation OOM */
  {
    int cd;
    for (cd = 0; cd < 25; cd++) {
      g_malloc_fail_countdown = cd;
      rc = md3_toggle_button_create(dummy_engine, &toggle, &cva);
      g_malloc_fail_countdown = -1;
      if (rc == UI_ERROR_NONE) {
        if (cva) {
          C_MULTIPLATFORM_FREE(cva);
          cva = NULL;
        }
        md3_toggle_button_destroy(toggle);
        toggle = NULL;
        break;
      }
      ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    }
  }

  /* Toolbar creation OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_toolbar_create(dummy_engine, &toolbar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_toolbar_create(dummy_engine, &toolbar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Toolbar add action OOM */
  rc = md3_toolbar_create(dummy_engine, &toolbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_malloc_fail_countdown = 0;
  rc = md3_toolbar_add_action(toolbar, 1, "Action", 48.0f);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
  rc = md3_toolbar_destroy(toolbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#else
  rc = md3_fab_menu_create(NULL, NULL, MD3_FAB_MENU_DIRECTION_UP, &menu);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_button_group_create(NULL, MD3_BUTTON_GROUP_HORIZONTAL, &group);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_toggle_button_create(NULL, &toggle, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_toolbar_create(NULL, &toolbar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  rc = md3_fab_destroy(fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_oom_a11y(void) {
  struct md3_roving_tabindex *roving = NULL;
  struct md3_announcer *announcer = NULL;
  ui_error_t rc;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_roving_tabindex_create(3, &roving);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_announcer_create(&announcer);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 1;
  rc = md3_announcer_create(&announcer);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Focus trap creation OOM */
  {
    struct ui_focus_manager *mgr = NULL;
    struct ui_dom_node *root = NULL;
    struct ui_focus_trap *trap = NULL;

    rc = ui_focus_manager_create(&mgr);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    g_malloc_fail_countdown = 0;
    rc = md3_a11y_trap_focus(&trap, mgr, root);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    g_malloc_fail_countdown = -1;

    ui_dom_node_destroy(root);
    ui_focus_manager_destroy(mgr);
  }
#else
  rc = md3_roving_tabindex_create(0, &roving);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_announcer_create(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

TEST test_md3_oom_selection_controls(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_autocomplete *ac = NULL;
  struct md3_select *sel = NULL;
  struct md3_pin_input *pin = NULL;
  struct md3_rating *rating = NULL;
  struct md3_spin_button *spin = NULL;
  ui_error_t rc;

#ifdef UI_TEST_MOCK_ALLOC
  /* Autocomplete creation OOM */
  g_malloc_fail_countdown = 0;
  rc =
      md3_autocomplete_create(dummy_engine, MD3_TEXT_FIELD_OUTLINED, &ac, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc =
      md3_autocomplete_create(dummy_engine, MD3_TEXT_FIELD_OUTLINED, &ac, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Select creation OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_select_create(dummy_engine, MD3_TEXT_FIELD_FILLED, &sel, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_select_create(dummy_engine, MD3_TEXT_FIELD_FILLED, &sel, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Select add option OOM */
  rc = md3_select_create(dummy_engine, MD3_TEXT_FIELD_FILLED, &sel, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_malloc_fail_countdown = 0;
  rc = md3_select_add_option(sel, "Fail", "fail");
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
  md3_select_destroy(sel);
  sel = NULL;

  /* PIN input creation OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_pin_input_create(dummy_engine, 6, &pin, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_pin_input_create(dummy_engine, 6, &pin, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Rating creation OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_rating_create(dummy_engine, 5, &rating, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_rating_create(dummy_engine, 5, &rating, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Spin button creation OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_spin_button_create(dummy_engine, 0.0, 100.0, 1.0, &spin, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_spin_button_create(dummy_engine, 0.0, 100.0, 1.0, &spin, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#else
  rc = md3_autocomplete_create(NULL, MD3_TEXT_FIELD_OUTLINED, &ac, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_select_create(NULL, MD3_TEXT_FIELD_FILLED, &sel, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_pin_input_create(NULL, 6, &pin, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_rating_create(NULL, 5, &rating, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_spin_button_create(NULL, 0.0, 100.0, 1.0, &spin, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

TEST test_md3_oom_shell_layout(void) {
  struct ui_engine *dummy_engine;
  struct md3_loading_indicator *ind = NULL;
  struct md3_stepper *stp = NULL;
  struct md3_scaffold *scf = NULL;
  struct md3_canonical_layout *lay = NULL;
  struct md3_breadcrumbs *bc = NULL;
  struct md3_page_indicator *pi = NULL;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_loading_indicator_create(dummy_engine,
                                    MD3_LOADING_INDICATOR_CONTAINED, &ind);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_stepper_create(dummy_engine, UI_STEPPER_MODE_LINEAR,
                          MD3_STEPPER_HORIZONTAL, &stp);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_scaffold_create(dummy_engine, &scf);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_canonical_layout_create(dummy_engine,
                                   MD3_CANONICAL_LAYOUT_LIST_DETAIL, &lay);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_breadcrumbs_create(dummy_engine, &bc);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_page_indicator_create(dummy_engine, 5, &pi);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = -1;
#else
  rc =
      md3_loading_indicator_create(NULL, MD3_LOADING_INDICATOR_CONTAINED, &ind);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

TEST test_md3_oom_overlay_menus(void) {
  struct ui_engine *dummy_engine;
  struct md3_date_range_picker *drp = NULL;
  struct md3_banner *bnr = NULL;
  struct md3_inline_alert *alt = NULL;
  struct md3_menubar *mb = NULL;
  struct md3_context_menu *cm = NULL;
  struct md3_action_sheet *as = NULL;
  struct md3_color_picker *cp = NULL;
  struct md3_command_palette *cmd = NULL;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = md3_date_range_picker_create(dummy_engine, &drp);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_banner_create(dummy_engine, &bnr);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_inline_alert_create(dummy_engine, MD3_ALERT_SEVERITY_INFO, &alt);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_menubar_create(dummy_engine, &mb);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_context_menu_create(dummy_engine, &cm);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_action_sheet_create(dummy_engine, &as);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_color_picker_create(dummy_engine, &cp, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_command_palette_create(dummy_engine, &cmd);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = -1;
#else
  rc = md3_banner_create(NULL, &bnr);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

TEST test_md3_oom_components_batch(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_card *card = NULL;
  struct md3_divider *divider = NULL;
  struct md3_carousel *carousel = NULL;
  struct md3_radio_group *rg = NULL;
  struct md3_switch *sw = NULL;
  struct ui_carousel_config cfg;
  ui_error_t rc;

  memset(&cfg, 0, sizeof(cfg));

#ifdef UI_TEST_MOCK_ALLOC
  /* card */
  g_malloc_fail_countdown = 0;
  rc = md3_card_create(dummy_engine, MD3_CARD_ELEVATED, &card);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_card_create(dummy_engine, MD3_CARD_ELEVATED, &card);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  /* divider */
  g_malloc_fail_countdown = 0;
  rc = md3_divider_create(dummy_engine, UI_DIVIDER_ORIENTATION_HORIZONTAL, 0,
                          &divider);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_divider_create(dummy_engine, UI_DIVIDER_ORIENTATION_HORIZONTAL, 0,
                          &divider);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  {
    extern int g_divider_mock_fail;
    g_divider_mock_fail = 4;
    rc = md3_divider_create(dummy_engine, UI_DIVIDER_ORIENTATION_VERTICAL, 0,
                            &divider);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

    g_divider_mock_fail = 5;
    rc = md3_divider_create(dummy_engine, UI_DIVIDER_ORIENTATION_HORIZONTAL, 1,
                            &divider);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_divider_mock_fail = 0;
  }

  /* carousel */
  g_malloc_fail_countdown = 0;
  rc = md3_carousel_create(dummy_engine, MD3_CAROUSEL_LAYOUT_MULTI_BROWSE, &cfg,
                           &carousel);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_carousel_create(dummy_engine, MD3_CAROUSEL_LAYOUT_MULTI_BROWSE, &cfg,
                           &carousel);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  /* radio group */
  g_malloc_fail_countdown = 0;
  rc = md3_radio_group_create(dummy_engine, &rg, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_radio_group_create(dummy_engine, &rg, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  /* switch */
  g_malloc_fail_countdown = 0;
  rc = md3_switch_create(dummy_engine, &sw, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = md3_switch_create(dummy_engine, &sw, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  /* checkbox */
  {
    struct md3_checkbox *cb = NULL;
    g_malloc_fail_countdown = 0;
    rc = md3_checkbox_create(dummy_engine, &cb, NULL);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    g_malloc_fail_countdown = 1;
    rc = md3_checkbox_create(dummy_engine, &cb, NULL);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  /* progress */
  {
    struct md3_progress *p = NULL;
    g_malloc_fail_countdown = 0;
    rc = md3_progress_create(dummy_engine, MD3_PROGRESS_LINEAR, &p);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    g_malloc_fail_countdown = 1;
    rc = md3_progress_create(dummy_engine, MD3_PROGRESS_LINEAR, &p);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  /* split button */
  {
    struct md3_split_button *sb = NULL;
    g_malloc_fail_countdown = 0;
    rc = md3_split_button_create(dummy_engine, MD3_BUTTON_FILLED, &sb);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    g_malloc_fail_countdown = 1;
    rc = md3_split_button_create(dummy_engine, MD3_BUTTON_FILLED, &sb);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  /* icon button */
  {
    int total_allocs;
    int c;
    struct md3_icon_button *probe = NULL;
    g_malloc_fail_countdown = -1;
    g_malloc_called = 0;
    rc = md3_icon_button_create(dummy_engine, MD3_ICON_BUTTON_STANDARD, "star",
                                &probe);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    total_allocs = g_malloc_called;
    rc = md3_icon_button_destroy(probe);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    for (c = 0; c < total_allocs; c++) {
      struct md3_icon_button *ib = NULL;
      g_malloc_fail_countdown = c;
      rc = md3_icon_button_create(dummy_engine, MD3_ICON_BUTTON_STANDARD,
                                  "star", &ib);
      ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    }
    g_malloc_fail_countdown = -1;
  }

  /* typography font variations OOM */
  {
    struct ui_font_manager *mgr = NULL;
    struct ui_font *font = NULL;
    rc = ui_font_manager_create(&mgr);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_font_manager_load_font_memory(mgr, tests_tiny_ttf,
                                          sizeof(tests_tiny_ttf), &font);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_malloc_fail_countdown = 0;
    rc =
        md3_typography_apply_font_variations(font, MD3_TYPESCALE_BODY_LARGE, 0);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    g_malloc_fail_countdown = -1;
    rc = ui_font_manager_destroy(mgr);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  g_malloc_fail_countdown = -1;
#else
  rc = md3_card_create(NULL, MD3_CARD_ELEVATED, &card);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(md3_oom_mock_suite) {
  RUN_TEST(test_md3_oom_button);
  RUN_TEST(test_md3_oom_list);
  RUN_TEST(test_md3_oom_fab);
  RUN_TEST(test_md3_oom_search);
  RUN_TEST(test_md3_oom_text_field);
  RUN_TEST(test_md3_oom_slider);
  RUN_TEST(test_md3_oom_segmented_button);
  RUN_TEST(test_md3_oom_chip);
  RUN_TEST(test_md3_oom_navigation_bar);
  RUN_TEST(test_md3_oom_dialog);
  RUN_TEST(test_md3_oom_action_widgets);
  RUN_TEST(test_md3_oom_selection_controls);
  RUN_TEST(test_md3_oom_a11y);
  RUN_TEST(test_md3_oom_shell_layout);
  RUN_TEST(test_md3_oom_overlay_menus);
  RUN_TEST(test_md3_oom_components_batch);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_oom_mock_suite);
  GREATEST_MAIN_END();
}
