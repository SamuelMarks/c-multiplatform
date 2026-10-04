/**
 * @file test_material3_fab_menu.c
 * @brief Unit tests for Material 3 FAB Menu component.
 */

/* clang-format off */
#include "material3/md3_fab_menu.h"
#include "ui_engine.h"
#include "greatest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

SUITE(md3_fab_menu_suite);

TEST test_md3_fab_menu_lifecycle(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md3_fab_menu *menu = NULL;
  struct ui_speed_dial_base *base = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = md3_fab_menu_create(NULL, MD3_FAB_MENU_DIRECTION_UP, &menu);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_fab_menu_create(engine, MD3_FAB_MENU_DIRECTION_UP, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_fab_menu_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_fab_menu_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid create */
  rc = md3_fab_menu_create(engine, MD3_FAB_MENU_DIRECTION_UP, &menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(menu != NULL);

  rc = md3_fab_menu_get_base(menu, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = md3_fab_menu_destroy(menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_fab_menu_actions(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md3_fab_menu *menu = NULL;
  struct md3_fab *primary = NULL;
  struct md3_fab *action = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_fab_menu_create(engine, MD3_FAB_MENU_DIRECTION_UP, &menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_fab_create(engine, MD3_FAB_SIZE_REGULAR, MD3_FAB_PRIMARY, "add",
                      NULL, &primary);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_fab_create(engine, MD3_FAB_SIZE_SMALL, MD3_FAB_SURFACE, "edit", NULL,
                      &action);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_fab_menu_set_primary_fab(NULL, primary);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_fab_menu_set_primary_fab(menu, primary);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_fab_menu_add_action(NULL, 1, action, "Label");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_fab_menu_add_action(menu, 1, NULL, "Label");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_fab_menu_add_action(menu, 1, action, "Edit");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_fab_destroy(primary);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_fab_destroy(action);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_fab_menu_destroy(menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
TEST test_md3_fab_menu_oom(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md3_fab_menu *menu = NULL;
  struct md3_fab *action = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_fab_menu_create(engine, MD3_FAB_MENU_DIRECTION_UP, &menu);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(menu == NULL);

  g_malloc_fail_countdown = -1;

  rc = md3_fab_menu_create(engine, MD3_FAB_MENU_DIRECTION_UP, &menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_fab_create(engine, MD3_FAB_SIZE_SMALL, MD3_FAB_SURFACE, "edit", NULL,
                      &action);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_fab_menu_add_action(menu, 1, action, "Edit");
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 1;
  rc = md3_fab_menu_add_action(menu, 1, action, "Edit");
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = -1;

  rc = md3_fab_destroy(action);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_fab_menu_destroy(menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}
#endif

SUITE(md3_fab_menu_suite) {
  RUN_TEST(test_md3_fab_menu_lifecycle);
  RUN_TEST(test_md3_fab_menu_actions);
#ifdef UI_TEST_MOCK_ALLOC
  RUN_TEST(test_md3_fab_menu_oom);
#endif
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_fab_menu_suite);
  GREATEST_MAIN_END();
}
