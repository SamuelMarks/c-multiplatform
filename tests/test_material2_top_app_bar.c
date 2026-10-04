/**
 * @file test_material2_top_app_bar.c
 * @brief Unit tests for Material 2 Top App Bar component.
 */

/* clang-format off */
#include "material2/md2_top_app_bar.h"
#include "ui_engine.h"
#include "greatest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

SUITE(md2_top_app_bar_suite);

TEST test_md2_top_app_bar_lifecycle(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md2_top_app_bar *bar = NULL;
  struct ui_top_app_bar_base *base = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = md2_top_app_bar_create(NULL, MD2_TOP_APP_BAR_REGULAR, "Title", &bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_top_app_bar_create(engine, MD2_TOP_APP_BAR_REGULAR, "Title", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_top_app_bar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_top_app_bar_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_top_app_bar_set_title(NULL, "Title");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_top_app_bar_set_title(bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid create regular */
  rc = md2_top_app_bar_create(engine, MD2_TOP_APP_BAR_REGULAR, "Regular", &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(bar != NULL);

  rc = md2_top_app_bar_get_base(bar, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Set title */
  rc = md2_top_app_bar_set_title(bar, "New Title");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md2_top_app_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Valid create prominent */
  rc = md2_top_app_bar_create(engine, MD2_TOP_APP_BAR_PROMINENT, NULL, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md2_top_app_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
TEST test_md2_top_app_bar_oom(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md2_top_app_bar *bar = NULL;
  ui_error_t rc;
  int i;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (i = 0; i < 4; i++) {
    g_malloc_fail_countdown = i;
    rc = md2_top_app_bar_create(engine, MD2_TOP_APP_BAR_REGULAR, "Title", &bar);
    if (rc == UI_ERROR_NONE) {
      md2_top_app_bar_destroy(bar);
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  g_malloc_fail_countdown = -1;

  rc = md2_top_app_bar_create(engine, MD2_TOP_APP_BAR_REGULAR, "Title", &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  g_malloc_fail_countdown = 0;
  rc = md2_top_app_bar_set_title(bar, "New Title");
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = -1;
  rc = md2_top_app_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}
#endif

SUITE(md2_top_app_bar_suite) {
  RUN_TEST(test_md2_top_app_bar_lifecycle);
#ifdef UI_TEST_MOCK_ALLOC
  RUN_TEST(test_md2_top_app_bar_oom);
#endif
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md2_top_app_bar_suite);
  GREATEST_MAIN_END();
}
