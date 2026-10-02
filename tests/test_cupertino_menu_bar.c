/**
 * @file test_cupertino_menu_bar.c
 * @brief Unit tests for macOS Global Application Menu Bar engine.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_menu_bar.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_menu_bar_suite);

TEST test_menu_bar_invalid_arguments(void) {
  struct cupertino_menu_bar_descriptor desc;
  struct cupertino_menu_bar *bar = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  int m_idx = 0;
  int i_idx = 0;
  float w = 0.0f;
  float h = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Creation invalid */
  rc = cupertino_menu_bar_create(NULL, &desc, &bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_create(dummy_engine, NULL, &bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destruction invalid */
  rc = cupertino_menu_bar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Creation with default NULL app_name */
  rc = cupertino_menu_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(bar != NULL);
  ASSERT_STR_EQ("App", bar->app_name);

  /* Add menu / item invalid combinations */
  rc = cupertino_menu_bar_add_menu(NULL, "File", &m_idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_add_menu(bar, NULL, &m_idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_add_menu(bar, "File", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_menu_bar_add_item(NULL, 0, "New", "Cmd+N", 0, &i_idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_add_item(bar, -1, "New", "Cmd+N", 0, &i_idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_add_item(bar, 99, "New", "Cmd+N", 0, &i_idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_add_item(bar, 0, "New", "Cmd+N", 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* States / open / close invalid */
  rc = cupertino_menu_bar_set_item_enabled(NULL, 0, 0, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_set_item_enabled(bar, -1, 0, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_set_item_enabled(bar, 99, 0, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_set_item_enabled(bar, 0, -1, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_set_item_enabled(bar, 0, 99, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Add an item to menu 0 so valid item index exists for bounds check */
  rc = cupertino_menu_bar_add_item(bar, 0, "Item0", NULL, 0, &i_idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_menu_bar_set_item_state(NULL, 0, 0,
                                         CUPERTINO_MENU_ITEM_STATE_ON);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_set_item_state(bar, -1, 0,
                                         CUPERTINO_MENU_ITEM_STATE_ON);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_set_item_state(bar, 99, 0,
                                         CUPERTINO_MENU_ITEM_STATE_ON);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_set_item_state(bar, 0, -1,
                                         CUPERTINO_MENU_ITEM_STATE_ON);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_set_item_state(bar, 0, 99,
                                         CUPERTINO_MENU_ITEM_STATE_ON);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_set_item_state(bar, 0, 0,
                                         (enum cupertino_menu_item_state) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_set_item_state(bar, 0, 0,
                                         (enum cupertino_menu_item_state)3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_menu_bar_open_menu(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_open_menu(bar, -1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_open_menu(bar, 99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_menu_bar_close(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_menu_bar_get_active_menu(NULL, &m_idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_get_active_menu(bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_menu_bar_get_dimensions(NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_get_dimensions(bar, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_bar_get_dimensions(bar, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Fill up menus to capacity CUPERTINO_MENU_BAR_MAX_MENUS */
  while (bar->menu_count < CUPERTINO_MENU_BAR_MAX_MENUS) {
    rc = cupertino_menu_bar_add_menu(bar, "Extra", &m_idx);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  /* Exceed capacity */
  rc = cupertino_menu_bar_add_menu(bar, "Overflow", &m_idx);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  /* Fill up items in menu 0 to capacity CUPERTINO_MENU_BAR_MAX_ITEMS */
  while (bar->menus[0].item_count < CUPERTINO_MENU_BAR_MAX_ITEMS) {
    rc = cupertino_menu_bar_add_item(bar, 0, "Item", "Cmd+I", 0, &i_idx);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  /* Exceed item capacity */
  rc = cupertino_menu_bar_add_item(bar, 0, "ItemOverflow", "Cmd+I", 0, &i_idx);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  rc = cupertino_menu_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_menu_bar_hierarchy_and_states(void) {
  struct cupertino_menu_bar_descriptor desc;
  struct cupertino_menu_bar *bar = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  int file_menu_idx = -1;
  int new_item_idx = -1;
  int sep_item_idx = -1;
  int save_item_idx = -1;
  int active_menu = -1;
  float w = 0.0f;
  float h = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.app_name = "TextEdit";

  rc = cupertino_menu_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(bar != NULL);

  /* Default Apple & App menus should be present */
  ASSERT_EQ(2, bar->menu_count);
  ASSERT_STR_EQ("Apple", bar->menus[0].title);
  ASSERT_STR_EQ("TextEdit", bar->menus[1].title);

  /* Dimensions */
  rc = cupertino_menu_bar_get_dimensions(bar, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_MENU_BAR_HEIGHT, h);

  /* Add 'File' menu */
  rc = cupertino_menu_bar_add_menu(bar, "File", &file_menu_idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, file_menu_idx);

  /* Add 'New' with shortcut Cmd+N */
  rc = cupertino_menu_bar_add_item(bar, file_menu_idx, "New", "Cmd+N", 0,
                                   &new_item_idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, new_item_idx);

  /* Add separator */
  rc = cupertino_menu_bar_add_item(bar, file_menu_idx, NULL, NULL, 1,
                                   &sep_item_idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, sep_item_idx);

  /* Add 'Save' with Cmd+S */
  rc = cupertino_menu_bar_add_item(bar, file_menu_idx, "Save", "Cmd+S", 0,
                                   &save_item_idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, save_item_idx);

  /* Modify item states */
  rc =
      cupertino_menu_bar_set_item_enabled(bar, file_menu_idx, save_item_idx, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, bar->menus[file_menu_idx].items[save_item_idx].is_enabled);

  rc = cupertino_menu_bar_set_item_state(bar, file_menu_idx, new_item_idx,
                                         CUPERTINO_MENU_ITEM_STATE_ON);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_MENU_ITEM_STATE_ON,
            bar->menus[file_menu_idx].items[new_item_idx].state);

  /* Test opening / closing menus */
  rc = cupertino_menu_bar_open_menu(bar, file_menu_idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_menu_bar_get_active_menu(bar, &active_menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(file_menu_idx, active_menu);

  rc = cupertino_menu_bar_close(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_menu_bar_get_active_menu(bar, &active_menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(-1, active_menu);

  rc = cupertino_menu_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_menu_bar_oom_mock(void) {
  struct cupertino_menu_bar_descriptor desc;
  struct cupertino_menu_bar *bar = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_menu_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, bar);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_menu_bar_create(NULL, &desc, &bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_menu_bar_suite) {
  RUN_TEST(test_menu_bar_invalid_arguments);
  RUN_TEST(test_menu_bar_hierarchy_and_states);
  RUN_TEST(test_menu_bar_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_menu_bar_suite);
  GREATEST_MAIN_END();
}
