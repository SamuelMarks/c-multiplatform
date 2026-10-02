/**
 * @file test_cupertino_context_menu.c
 * @brief Unit tests for Cupertino Context Menu (UIContextMenuInteraction).
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_context_menu.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_context_menu_suite);

TEST test_context_menu_invalid_arguments(void) {
  struct cupertino_context_menu_descriptor desc;
  struct cupertino_context_menu_action action;
  struct cupertino_context_menu *menu = NULL;
  struct ui_context_menu_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  size_t count;
  int flag;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  memset(&action, 0, sizeof(action));

  /* Creation invalid */
  rc = cupertino_context_menu_create(NULL, &desc, &menu);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_context_menu_create(dummy_engine, NULL, &menu);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_context_menu_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destruction invalid */
  rc = cupertino_context_menu_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Add action invalid */
  rc = cupertino_context_menu_add_action(NULL, &action);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_context_menu_add_action(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Action count / get action invalid */
  rc = cupertino_context_menu_get_action_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_context_menu_get_action(NULL, 0, &action);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Open / close / is_open invalid */
  rc = cupertino_context_menu_open_at(NULL, NULL, 100.0f, 100.0f, 800.0f,
                                      600.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_context_menu_open_at((struct cupertino_context_menu *)0x1,
                                      NULL, 100.0f, 100.0f, -1.0f, 600.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_context_menu_open_at((struct cupertino_context_menu *)0x1,
                                      NULL, 100.0f, 100.0f, 800.0f, -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_context_menu_close(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_context_menu_is_open(NULL, &flag);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Select action invalid */
  rc = cupertino_context_menu_select_action(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_context_menu_get_last_selected_index(NULL, &flag);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Base invalid */
  rc = cupertino_context_menu_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid menu to test null out pointers */
  rc = cupertino_context_menu_create(dummy_engine, &desc, &menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_context_menu_add_action(menu, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_context_menu_get_action_count(menu, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_context_menu_get_action(menu, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_context_menu_is_open(menu, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_context_menu_get_last_selected_index(menu, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_context_menu_get_base(menu, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Select out of bounds action */
  rc = cupertino_context_menu_select_action(menu, 10);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  /* Destroy with menu->base == NULL */
  struct ui_context_menu_base *saved_base = menu->base;
  menu->base = NULL;
  rc = cupertino_context_menu_destroy(menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ui_context_menu_base_destroy(saved_base);

  PASS();
}

TEST test_context_menu_actions_and_interaction(void) {
  struct cupertino_context_menu_descriptor desc;
  struct cupertino_context_menu_action action;
  struct cupertino_context_menu_action ret_action;
  struct cupertino_context_menu *menu = NULL;
  struct ui_context_menu_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  size_t count;
  int is_open, last_sel;
  size_t i;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.title = "Message Options";
  desc.is_dark = 1;

  rc = cupertino_context_menu_create(dummy_engine, &desc, &menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(menu != NULL);

  rc = cupertino_context_menu_get_base(menu, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = cupertino_context_menu_get_action_count(menu, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((size_t)0, count);

  /* Add Action 1: Reply */
  memset(&action, 0, sizeof(action));
  strcpy(action.title, "Reply");
  strcpy(action.symbol_name, "arrowshape.turn.up.left");
  action.is_destructive = 0;
  action.is_disabled = 0;
  action.is_checked = 0;

  rc = cupertino_context_menu_add_action(menu, &action);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Add Action 2: Star (Checked) */
  memset(&action, 0, sizeof(action));
  strcpy(action.title, "Star");
  strcpy(action.symbol_name, "star.fill");
  action.is_checked = 1;
  rc = cupertino_context_menu_add_action(menu, &action);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Add Action 3: Archive (Disabled) */
  memset(&action, 0, sizeof(action));
  strcpy(action.title, "Archive");
  strcpy(action.symbol_name, "archivebox");
  action.is_disabled = 1;
  rc = cupertino_context_menu_add_action(menu, &action);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Add Action 4: Delete (Destructive) */
  memset(&action, 0, sizeof(action));
  strcpy(action.title, "Delete");
  strcpy(action.symbol_name, "trash");
  action.is_destructive = 1;
  rc = cupertino_context_menu_add_action(menu, &action);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_context_menu_get_action_count(menu, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((size_t)4, count);

  /* Inspect action 1 */
  rc = cupertino_context_menu_get_action(menu, 0, &ret_action);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Reply", ret_action.title);
  ASSERT_STR_EQ("arrowshape.turn.up.left", ret_action.symbol_name);
  ASSERT_EQ(0, ret_action.is_destructive);
  ASSERT_EQ(0, ret_action.is_disabled);

  /* Inspect action 3 (destructive) */
  rc = cupertino_context_menu_get_action(menu, 3, &ret_action);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Delete", ret_action.title);
  ASSERT_EQ(1, ret_action.is_destructive);

  /* Out of bounds get */
  rc = cupertino_context_menu_get_action(menu, 99, &ret_action);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  /* Open menu at coordinates */
  rc = cupertino_context_menu_open_at(menu, NULL, 150.0f, 200.0f, 800.0f,
                                      600.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_context_menu_is_open(menu, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_open);

  /* Try selecting disabled action (index 2: Archive), should fail */
  rc = cupertino_context_menu_select_action(menu, 2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Select valid action (index 0: Reply), should succeed and close menu */
  rc = cupertino_context_menu_select_action(menu, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_context_menu_is_open(menu, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_open);

  rc = cupertino_context_menu_get_last_selected_index(menu, &last_sel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, last_sel);

  /* Fill remaining slots to reach CUPERTINO_CONTEXT_MENU_MAX_ACTIONS */
  for (i = 4; i < CUPERTINO_CONTEXT_MENU_MAX_ACTIONS; i++) {
    memset(&action, 0, sizeof(action));
    strcpy(action.title, "Item");
    rc = cupertino_context_menu_add_action(menu, &action);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* One more should overflow */
  rc = cupertino_context_menu_add_action(menu, &action);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  /* Close menu */
  rc = cupertino_context_menu_close(menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_context_menu_destroy(menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_context_menu_oom_simulation(void) {
  struct cupertino_context_menu_descriptor desc;
  struct cupertino_context_menu *menu = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.title = "Menu";

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_context_menu_mock_create_fail;
  extern int g_cupertino_context_menu_mock_destroy_fail;
  extern int g_cupertino_context_menu_mock_open_fail;
  extern int g_cupertino_context_menu_mock_close_fail;

  struct ui_overlay_director *dummy_director =
      (struct ui_overlay_director *)0x5678;

  g_malloc_fail_countdown = 0;
  rc = cupertino_context_menu_create(dummy_engine, &desc, &menu);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, menu);
  g_malloc_fail_countdown = -1;

  /* Mock base create failure */
  g_cupertino_context_menu_mock_create_fail = 1;
  rc = cupertino_context_menu_create(dummy_engine, &desc, &menu);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, menu);
  g_cupertino_context_menu_mock_create_fail = 0;

  /* Mock base destroy failure */
  rc = cupertino_context_menu_create(dummy_engine, &desc, &menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_context_menu_mock_destroy_fail = 1;
  rc = cupertino_context_menu_destroy(menu);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_context_menu_mock_destroy_fail = 0;

  /* Open with director (menu_h < 44 test with action_count == 0) */
  struct ui_dom_node *body = NULL;
  struct ui_overlay_director *director = NULL;
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &body);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_create(body, &director);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_context_menu_create(dummy_engine, &desc, &menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_context_menu_open_at(menu, director, 100.0f, 100.0f, 800.0f,
                                      600.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Open with director and 2 actions so menu_h (2 * 44 = 88) >= 44 */
  struct cupertino_context_menu_action act;
  memset(&act, 0, sizeof(act));
  strcpy(act.title, "A1");
  rc = cupertino_context_menu_add_action(menu, &act);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  strcpy(act.title, "A2");
  rc = cupertino_context_menu_add_action(menu, &act);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_context_menu_open_at(menu, director, 100.0f, 100.0f, 800.0f,
                                      600.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Open when menu->base == NULL */
  struct ui_context_menu_base *temp_base = menu->base;
  menu->base = NULL;
  rc = cupertino_context_menu_open_at(menu, director, 100.0f, 100.0f, 800.0f,
                                      600.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  menu->base = temp_base;

  /* Open with director (mock open failure) */
  g_cupertino_context_menu_mock_open_fail = 1;
  rc = cupertino_context_menu_open_at(menu, director, 100.0f, 100.0f, 800.0f,
                                      600.0f);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_context_menu_mock_open_fail = 0;

  /* Select action with mock close failure */
  struct cupertino_context_menu_action action;
  memset(&action, 0, sizeof(action));
  strcpy(action.title, "Act");
  rc = cupertino_context_menu_add_action(menu, &action);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  g_cupertino_context_menu_mock_close_fail = 1;
  rc = cupertino_context_menu_select_action(menu, 0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_context_menu_mock_close_fail = 0;

  rc = cupertino_context_menu_destroy(menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ui_overlay_director_destroy(director);
  ui_dom_node_destroy(body);
#else
  rc = cupertino_context_menu_create(NULL, &desc, &menu);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_context_menu_suite) {
  RUN_TEST(test_context_menu_invalid_arguments);
  RUN_TEST(test_context_menu_actions_and_interaction);
  RUN_TEST(test_context_menu_oom_simulation);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_context_menu_suite);
  GREATEST_MAIN_END();
}
