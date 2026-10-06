/**
 * @file test_material2_bottom_navigation.c
 * @brief Unit tests for Material 2 Bottom Navigation component.
 */

/* clang-format off */
#include "material2/md2_bottom_navigation.h"
#include "ui_engine.h"
#include "greatest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

/* Hacks to access internal structure for branch coverage testing */
struct md2_bottom_navigation_hack {
  struct ui_bottom_nav_base *base;
};

struct md2_bottom_navigation_item_hack {
  struct ui_bottom_nav_item_base *base;
};

SUITE(md2_bottom_navigation_suite);

TEST test_md2_bottom_navigation_lifecycle(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md2_bottom_navigation *nav = NULL;
  struct ui_bottom_nav_base *base = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = md2_bottom_navigation_create(NULL, MD2_BOTTOM_NAVIGATION_FIXED, &nav);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_bottom_navigation_create(engine, MD2_BOTTOM_NAVIGATION_FIXED, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_bottom_navigation_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_bottom_navigation_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Create successful */
  rc = md2_bottom_navigation_create(engine, MD2_BOTTOM_NAVIGATION_FIXED, &nav);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(nav != NULL);

  rc = md2_bottom_navigation_get_base(nav, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Get base */
  rc = md2_bottom_navigation_get_base(nav, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Destroy successful */
  rc = md2_bottom_navigation_destroy(nav);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_md2_bottom_navigation_item_lifecycle(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md2_bottom_navigation *nav = NULL;
  struct md2_bottom_navigation_item *item = NULL;
  struct ui_bottom_nav_item_base *base = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = md2_bottom_navigation_item_create(NULL, "Home", "ic_home", &item);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_bottom_navigation_item_create(engine, "Home", "ic_home", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_bottom_navigation_item_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_bottom_navigation_item_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Create items with diff combinations of label/icon */
  rc = md2_bottom_navigation_item_create(engine, "Home", "ic_home", &item);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(item != NULL);

  rc = md2_bottom_navigation_item_get_base(item, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md2_bottom_navigation_item_get_base(item, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = md2_bottom_navigation_create(engine, MD2_BOTTOM_NAVIGATION_FIXED, &nav);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Append nulls */
  rc = md2_bottom_navigation_append_item(NULL, item);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_bottom_navigation_append_item(nav, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Append item */
  rc = md2_bottom_navigation_append_item(nav, item);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md2_bottom_navigation_item_destroy(item);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md2_bottom_navigation_destroy(nav);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test NULL label/icon */
  rc = md2_bottom_navigation_item_create(engine, NULL, NULL, &item);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md2_bottom_navigation_item_destroy(item);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
TEST test_md2_bottom_navigation_oom(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md2_bottom_navigation *nav = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  g_malloc_fail_countdown = 0;
  rc = md2_bottom_navigation_create(engine, MD2_BOTTOM_NAVIGATION_FIXED, &nav);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 1;
  rc = md2_bottom_navigation_create(engine, MD2_BOTTOM_NAVIGATION_FIXED, &nav);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = -1;
  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_md2_bottom_navigation_item_oom(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md2_bottom_navigation_item *item = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  g_malloc_fail_countdown = 0;
  rc = md2_bottom_navigation_item_create(engine, "Label", "Icon", &item);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 1;
  rc = md2_bottom_navigation_item_create(engine, "Label", "Icon", &item);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 2;
  rc = md2_bottom_navigation_item_create(engine, "Label", "Icon", &item);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 1; /* label alloc skipped, icon alloc fails */
  rc = md2_bottom_navigation_item_create(engine, NULL, "Icon", &item);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 3;
  rc = md2_bottom_navigation_item_create(engine, "Label", "Icon", &item);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 1; /* label alloc skipped, base alloc fails */
  rc = md2_bottom_navigation_item_create(engine, NULL, "Icon", &item);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 2; /* icon alloc skipped, base alloc fails */
  rc = md2_bottom_navigation_item_create(engine, "Label", NULL, &item);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 2; /* label alloc skipped, base alloc fails */
  rc = md2_bottom_navigation_item_create(engine, NULL, "Icon", &item);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 1; /* both skipped, base alloc fails */
  rc = md2_bottom_navigation_item_create(engine, NULL, NULL, &item);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = -1;
  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}
#endif

SUITE(md2_bottom_navigation_suite) {
  RUN_TEST(test_md2_bottom_navigation_lifecycle);
  RUN_TEST(test_md2_bottom_navigation_item_lifecycle);
#ifdef UI_TEST_MOCK_ALLOC
  RUN_TEST(test_md2_bottom_navigation_oom);
  RUN_TEST(test_md2_bottom_navigation_item_oom);
#endif
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md2_bottom_navigation_suite);
  GREATEST_MAIN_END();
}
