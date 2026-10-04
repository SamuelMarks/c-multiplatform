/**
 * @file test_material3_floating_toolbar.c
 * @brief Unit tests for Material 3 Floating Toolbar component.
 */

/* clang-format off */
#include "material3/md3_floating_toolbar.h"
#include "material3/md3_fab.h"
#include "ui_engine.h"
#include "greatest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

SUITE(md3_floating_toolbar_suite);

TEST test_md3_floating_toolbar_lifecycle(void) {
  struct ui_engine_config engine_cfg;
  struct ui_engine *engine = NULL;
  struct md3_floating_toolbar *tb = NULL;
  struct ui_floating_toolbar_base *base = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = md3_floating_toolbar_create(NULL, UI_FLOATING_TOOLBAR_HORIZONTAL,
                                   MD3_FLOATING_TOOLBAR_STANDARD, &tb);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_floating_toolbar_create(engine, UI_FLOATING_TOOLBAR_HORIZONTAL,
                                   MD3_FLOATING_TOOLBAR_STANDARD, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_floating_toolbar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_floating_toolbar_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Create standard */
  rc = md3_floating_toolbar_create(engine, UI_FLOATING_TOOLBAR_HORIZONTAL,
                                   MD3_FLOATING_TOOLBAR_STANDARD, &tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tb != NULL);

  rc = md3_floating_toolbar_get_base(tb, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = md3_floating_toolbar_destroy(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_floating_toolbar_expansion(void) {
  struct ui_engine_config engine_cfg;
  struct ui_engine *engine = NULL;
  struct md3_floating_toolbar *tb = NULL;
  int expanded = 0;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Create expandable */
  rc = md3_floating_toolbar_create(engine, UI_FLOATING_TOOLBAR_VERTICAL,
                                   MD3_FLOATING_TOOLBAR_EXPANDABLE, &tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tb != NULL);

  rc = md3_floating_toolbar_is_expanded(NULL, &expanded);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_floating_toolbar_is_expanded(tb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_floating_toolbar_toggle_expansion(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Initial state for expandable is collapsed (0) */
  rc = md3_floating_toolbar_is_expanded(tb, &expanded);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, expanded);

  /* Toggle to expanded */
  rc = md3_floating_toolbar_toggle_expansion(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_floating_toolbar_is_expanded(tb, &expanded);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, expanded);

  /* Toggle back to collapsed */
  rc = md3_floating_toolbar_toggle_expansion(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_floating_toolbar_is_expanded(tb, &expanded);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, expanded);

  rc = md3_floating_toolbar_destroy(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_floating_toolbar_fab(void) {
  struct ui_engine_config engine_cfg;
  struct ui_engine *engine = NULL;
  struct md3_floating_toolbar *tb = NULL;
  struct md3_fab *fab = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_floating_toolbar_create(engine, UI_FLOATING_TOOLBAR_HORIZONTAL,
                                   MD3_FLOATING_TOOLBAR_STANDARD, &tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_fab_create(engine, MD3_FAB_SIZE_REGULAR, MD3_FAB_PRIMARY, "add",
                      "Add", &fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fab != NULL);

  rc = md3_floating_toolbar_set_fab(NULL, fab);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Mount FAB */
  rc = md3_floating_toolbar_set_fab(tb, fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Clear FAB */
  rc = md3_floating_toolbar_set_fab(tb, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_fab_destroy(fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_floating_toolbar_destroy(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
TEST test_md3_floating_toolbar_oom(void) {
  struct ui_engine_config engine_cfg;
  struct ui_engine *engine = NULL;
  struct md3_floating_toolbar *tb = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_floating_toolbar_create(engine, UI_FLOATING_TOOLBAR_HORIZONTAL,
                                   MD3_FLOATING_TOOLBAR_STANDARD, &tb);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(tb == NULL);
  g_malloc_fail_countdown = -1;

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}
#endif

SUITE(md3_floating_toolbar_suite) {
  RUN_TEST(test_md3_floating_toolbar_lifecycle);
  RUN_TEST(test_md3_floating_toolbar_expansion);
  RUN_TEST(test_md3_floating_toolbar_fab);
#ifdef UI_TEST_MOCK_ALLOC
  RUN_TEST(test_md3_floating_toolbar_oom);
#endif
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_floating_toolbar_suite);
  GREATEST_MAIN_END();
}
