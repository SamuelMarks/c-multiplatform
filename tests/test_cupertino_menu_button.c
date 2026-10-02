/**
 * @file test_cupertino_menu_button.c
 * @brief Unit tests for Cupertino Pop-up and Pull-down Menus.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_menu_button.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

static int g_selected_call_count = 0;
static void dummy_on_select(void *user_data) {
  int *p = (int *)user_data;
  if (p) {
    (*p)++;
  }
  g_selected_call_count++;
}

TEST test_cupertino_menu_button_lifecycle(void) {
  struct cupertino_menu_button *menu = NULL;
  struct cupertino_menu_button_descriptor desc;
  struct cupertino_button *btn = NULL;
  ui_error_t rc;
  int is_open = 0;
  const char *title = NULL;

  memset(&desc, 0, sizeof(desc));
  desc.mode = CUPERTINO_MENU_BUTTON_POP_UP;
  desc.style = CUPERTINO_BUTTON_FILLED;
  strncpy(desc.initial_title, "Options", sizeof(desc.initial_title) - 1);

  /* Null arg validations */
  rc = cupertino_menu_button_create(NULL, &desc, &menu);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_button_create((struct ui_engine *)1, NULL, &menu);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_button_create((struct ui_engine *)1, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_menu_button_create((struct ui_engine *)1, &desc, &menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, menu);

  rc = cupertino_menu_button_get_button(menu, &btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, btn);

  rc = cupertino_menu_button_get_current_title(menu, &title);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Options", title);

  rc = cupertino_menu_button_is_open(menu, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_open);

  rc = cupertino_menu_button_set_open(menu, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_menu_button_is_open(menu, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_open);

  rc = cupertino_menu_button_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_menu_button_destroy(menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_cupertino_menu_button_pop_up_mode(void) {
  struct cupertino_menu_button *menu = NULL;
  struct cupertino_menu_button_descriptor desc;
  struct cupertino_menu_button_item item1, item2, fetched_item;
  const char *title = NULL;
  size_t count = 0;
  int selected_idx = -1;
  int cb_counter = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.mode = CUPERTINO_MENU_BUTTON_POP_UP;
  desc.style = CUPERTINO_BUTTON_GRAY;
  strncpy(desc.initial_title, "Sort By", sizeof(desc.initial_title) - 1);

  rc = cupertino_menu_button_create((struct ui_engine *)1, &desc, &menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&item1, 0, sizeof(item1));
  strncpy(item1.title, "Date Added", sizeof(item1.title) - 1);
  item1.on_select = dummy_on_select;
  item1.user_data = &cb_counter;

  memset(&item2, 0, sizeof(item2));
  strncpy(item2.title, "Name A-Z", sizeof(item2.title) - 1);
  item2.is_destructive = 0;

  rc = cupertino_menu_button_add_item(menu, &item1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_menu_button_add_item(menu, &item2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_menu_button_get_item_count(menu, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, (int)count);

  rc = cupertino_menu_button_get_item(menu, 0, &fetched_item);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Date Added", fetched_item.title);

  /* Select item 0 -> should update current title in POP_UP mode */
  rc = cupertino_menu_button_select_item(menu, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, cb_counter);

  rc = cupertino_menu_button_get_selected_index(menu, &selected_idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, selected_idx);

  rc = cupertino_menu_button_get_current_title(menu, &title);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Date Added", title);

  /* Select item 1 -> update title to Name A-Z */
  rc = cupertino_menu_button_select_item(menu, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_menu_button_get_current_title(menu, &title);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Name A-Z", title);

  rc = cupertino_menu_button_destroy(menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_cupertino_menu_button_pull_down_mode(void) {
  struct cupertino_menu_button *menu = NULL;
  struct cupertino_menu_button_descriptor desc;
  struct cupertino_menu_button_item item1, item_disabled;
  const char *title = NULL;
  ui_error_t rc;
  int selected_idx = -1;

  memset(&desc, 0, sizeof(desc));
  desc.mode = CUPERTINO_MENU_BUTTON_PULL_DOWN;
  desc.style = CUPERTINO_BUTTON_PLAIN;
  strncpy(desc.initial_title, "Actions", sizeof(desc.initial_title) - 1);

  rc = cupertino_menu_button_create((struct ui_engine *)1, &desc, &menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&item1, 0, sizeof(item1));
  strncpy(item1.title, "Duplicate", sizeof(item1.title) - 1);
  rc = cupertino_menu_button_add_item(menu, &item1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&item_disabled, 0, sizeof(item_disabled));
  strncpy(item_disabled.title, "Delete", sizeof(item_disabled.title) - 1);
  item_disabled.is_destructive = 1;
  item_disabled.is_disabled = 1;
  rc = cupertino_menu_button_add_item(menu, &item_disabled);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Select item 0 in PULL_DOWN mode -> title must NOT change */
  rc = cupertino_menu_button_select_item(menu, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_menu_button_get_current_title(menu, &title);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Actions", title);

  rc = cupertino_menu_button_get_selected_index(menu, &selected_idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, selected_idx);

  /* Disabled item selection */
  rc = cupertino_menu_button_select_item(menu, 1);
  ASSERT_EQ(UI_ERROR_UNSUPPORTED, rc);

  /* Out of range selection */
  rc = cupertino_menu_button_select_item(menu, 10);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_menu_button_destroy(menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_cupertino_menu_button_invalid_args_and_capacity(void) {
  struct cupertino_menu_button *menu = NULL;
  struct cupertino_menu_button_descriptor desc;
  struct cupertino_menu_button_item item;
  ui_error_t rc;
  size_t i;

  memset(&desc, 0, sizeof(desc));
  strncpy(desc.initial_title, "Menu", sizeof(desc.initial_title) - 1);

  rc = cupertino_menu_button_create((struct ui_engine *)1, &desc, &menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = cupertino_menu_button_add_item(NULL, &item);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_button_add_item(menu, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_menu_button_get_item_count(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_button_get_item(NULL, 0, &item);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_button_get_item(menu, 50, &item);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_menu_button_select_item(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_button_get_selected_index(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_button_set_open(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_button_is_open(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_button_get_current_title(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_button_get_button(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Max capacity check */
  memset(&item, 0, sizeof(item));
  strncpy(item.title, "Entry", sizeof(item.title) - 1);
  for (i = 0; i < CUPERTINO_MENU_BUTTON_MAX_ITEMS; i++) {
    rc = cupertino_menu_button_add_item(menu, &item);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = cupertino_menu_button_add_item(menu, &item);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  /* Pointers null checks */
  rc = cupertino_menu_button_get_item_count(menu, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_button_get_item(menu, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_button_get_selected_index(menu, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_button_is_open(menu, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_button_get_current_title(menu, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_menu_button_get_button(menu, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_menu_button_destroy(menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_cupertino_menu_button_oom_mock(void) {
  struct cupertino_menu_button *menu = NULL;
  struct cupertino_menu_button_descriptor desc;
  struct cupertino_menu_button_item item;
  struct cupertino_button *saved_btn = NULL;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  strncpy(desc.initial_title, "OOM Menu", sizeof(desc.initial_title) - 1);

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_menu_button_mock_button_create_fail;
  extern int g_cupertino_menu_button_mock_button_destroy_fail;
  extern int g_cupertino_menu_button_mock_set_text_fail;

  g_malloc_fail_countdown = 0;
  rc = cupertino_menu_button_create((struct ui_engine *)1, &desc, &menu);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Mock button_create failure */
  g_cupertino_menu_button_mock_button_create_fail = 1;
  rc = cupertino_menu_button_create((struct ui_engine *)1, &desc, &menu);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  ASSERT_EQ(NULL, menu);
  g_cupertino_menu_button_mock_button_create_fail = 0;

  /* Mock button_destroy failure */
  rc = cupertino_menu_button_create((struct ui_engine *)1, &desc, &menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_menu_button_mock_button_destroy_fail = 1;
  rc = cupertino_menu_button_destroy(menu);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_menu_button_mock_button_destroy_fail = 0;

  /* Destroy with menu->button == NULL */
  rc = cupertino_menu_button_create((struct ui_engine *)1, &desc, &menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  saved_btn = menu->button;
  menu->button = NULL;
  rc = cupertino_menu_button_destroy(menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  cupertino_button_destroy(saved_btn);

  /* Mock set_text failure in select_item */
  desc.mode = CUPERTINO_MENU_BUTTON_POP_UP;
  rc = cupertino_menu_button_create((struct ui_engine *)1, &desc, &menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  memset(&item, 0, sizeof(item));
  strncpy(item.title, "Choice", sizeof(item.title) - 1);
  rc = cupertino_menu_button_add_item(menu, &item);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  extern int g_cupertino_menu_button_mock_get_base_fail;
  g_cupertino_menu_button_mock_get_base_fail = 1;
  rc = cupertino_menu_button_select_item(menu, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_menu_button_mock_get_base_fail = 0;

  g_cupertino_menu_button_mock_set_text_fail = 1;
  rc = cupertino_menu_button_select_item(menu, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_menu_button_mock_set_text_fail = 0;

  /* Select item with menu->button == NULL */
  desc.mode = CUPERTINO_MENU_BUTTON_POP_UP;
  rc = cupertino_menu_button_create((struct ui_engine *)1, &desc, &menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_menu_button_add_item(menu, &item);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  saved_btn = menu->button;
  menu->button = NULL;
  rc = cupertino_menu_button_select_item(menu, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  menu->button = saved_btn;
  rc = cupertino_menu_button_destroy(menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Select item with menu->button != NULL but menu->button->base == NULL */
  desc.mode = CUPERTINO_MENU_BUTTON_POP_UP;
  rc = cupertino_menu_button_create((struct ui_engine *)1, &desc, &menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_menu_button_add_item(menu, &item);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  struct ui_button_base *saved_bb = menu->button->base;
  menu->button->base = NULL;
  rc = cupertino_menu_button_select_item(menu, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  menu->button->base = saved_bb;
  rc = cupertino_menu_button_destroy(menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#endif

  PASS();
}

SUITE(cupertino_menu_button_suite) {
  RUN_TEST(test_cupertino_menu_button_lifecycle);
  RUN_TEST(test_cupertino_menu_button_pop_up_mode);
  RUN_TEST(test_cupertino_menu_button_pull_down_mode);
  RUN_TEST(test_cupertino_menu_button_invalid_args_and_capacity);
  RUN_TEST(test_cupertino_menu_button_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_menu_button_suite);
  GREATEST_MAIN_END();
}
