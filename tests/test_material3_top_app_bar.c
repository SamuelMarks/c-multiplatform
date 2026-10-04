/**
 * @file test_material3_top_app_bar.c
 * @brief Unit tests for Material 3 Top App Bar component.
 */

/* clang-format off */
#include "material3/md3_top_app_bar.h"
#include "material3/md3_icon_button.h"
#include "ui_engine.h"
#include "greatest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

SUITE(md3_top_app_bar_suite);

TEST test_md3_top_app_bar_lifecycle(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md3_top_app_bar *bar = NULL;
  struct ui_top_app_bar_base *base = NULL;
  struct md3_top_app_bar_config config;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  config.variant = MD3_TOP_APP_BAR_MEDIUM;
  config.scroll_behavior = MD3_TOP_APP_BAR_SCROLL_PINNED;
  config.title = "Hello";
  config.subtitle = "World";

  /* Null checks */
  rc = md3_top_app_bar_create(NULL, &config, &bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_top_app_bar_create(engine, NULL, &bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_top_app_bar_create(engine, &config, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_top_app_bar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_top_app_bar_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid create */
  rc = md3_top_app_bar_create(engine, &config, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(bar != NULL);

  rc = md3_top_app_bar_get_base(bar, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = md3_top_app_bar_handle_scroll(NULL, 10.0f, 5.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_top_app_bar_handle_scroll(bar, 10.0f, 5.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_top_app_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_top_app_bar_variants(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md3_top_app_bar *bar = NULL;
  struct md3_top_app_bar_config config;
  ui_error_t rc;
  int i;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  config.scroll_behavior = MD3_TOP_APP_BAR_SCROLL_ENTER_ALWAYS;
  config.title = NULL;
  config.subtitle = NULL;

  for (i = 0; i < 4; i++) {
    switch (i) {
    case 0:
      config.variant = MD3_TOP_APP_BAR_SMALL;
      break;
    case 1:
      config.variant = MD3_TOP_APP_BAR_CENTER_ALIGNED;
      break;
    case 2:
      config.variant = MD3_TOP_APP_BAR_MEDIUM;
      break;
    case 3:
      config.variant = MD3_TOP_APP_BAR_LARGE;
      break;
    }

    rc = md3_top_app_bar_create(engine, &config, &bar);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = md3_top_app_bar_destroy(bar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
TEST test_md3_top_app_bar_oom(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md3_top_app_bar *bar = NULL;
  struct md3_top_app_bar_config config;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  config.variant = MD3_TOP_APP_BAR_SMALL;
  config.scroll_behavior = MD3_TOP_APP_BAR_SCROLL_PINNED;
  config.title = "Test";
  config.subtitle = "Subtitle";

  /* The creation does 3 allocations: struct, title, subtitle. */
  g_malloc_fail_countdown = 0;
  rc = md3_top_app_bar_create(engine, &config, &bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(bar == NULL);

  g_malloc_fail_countdown = 1;
  rc = md3_top_app_bar_create(engine, &config, &bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = 2;
  rc = md3_top_app_bar_create(engine, &config, &bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = -1;

  rc = md3_top_app_bar_create(engine, &config, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_top_app_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}
#endif

TEST test_md3_top_app_bar_slots(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md3_top_app_bar *bar = NULL;
  struct md3_top_app_bar_config config;
  ui_error_t rc;
  /* Since we do not have md3_icon_button fully mocked or created here, we can
   * pass dummy pointers */
  struct md3_icon_button *dummy_icon = (struct md3_icon_button *)0x1234;
  struct md3_icon_button *dummy_action = (struct md3_icon_button *)0x5678;
  int i;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  config.variant = MD3_TOP_APP_BAR_SMALL;
  config.scroll_behavior = MD3_TOP_APP_BAR_SCROLL_PINNED;
  config.title = "Title";
  config.subtitle = NULL;

  rc = md3_top_app_bar_create(engine, &config, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = md3_top_app_bar_set_navigation_icon(NULL, dummy_icon);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_top_app_bar_add_action_item(NULL, dummy_action);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_top_app_bar_add_action_item(bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_top_app_bar_set_navigation_icon(bar, dummy_icon);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_top_app_bar_set_navigation_icon(bar, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (i = 0; i < 5; i++) {
    rc = md3_top_app_bar_add_action_item(bar, dummy_action);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_top_app_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

SUITE(md3_top_app_bar_suite) {
  RUN_TEST(test_md3_top_app_bar_lifecycle);
  RUN_TEST(test_md3_top_app_bar_variants);
  RUN_TEST(test_md3_top_app_bar_slots);
#ifdef UI_TEST_MOCK_ALLOC
  RUN_TEST(test_md3_top_app_bar_oom);
#endif
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_top_app_bar_suite);
  GREATEST_MAIN_END();
}
