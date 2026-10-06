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
extern int g_md3_top_app_bar_mock_fail;
extern int g_md3_top_app_bar_destroy_mock_fail;
extern ui_error_t md3_top_app_bar_test_duplicate_string(struct ui_arena *arena,
                                                        const char *src,
                                                        char **out_str);
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

  rc = md3_top_app_bar_get_base(bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_top_app_bar_get_base(bar, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = md3_top_app_bar_handle_scroll(NULL, 10.0f, 5.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_top_app_bar_handle_scroll(bar, 10.0f, 5.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Handle scroll when bar->base is NULL */
  {
    struct md3_top_app_bar *bar_no_base = NULL;
    rc = md3_top_app_bar_create(engine, &config, &bar_no_base);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    /* Zero out base field in struct */
    *(struct ui_top_app_bar_base **)bar_no_base = NULL;
    rc = md3_top_app_bar_handle_scroll(bar_no_base, 10.0f, 5.0f);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_top_app_bar_destroy(bar_no_base);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Destroy when bar->arena is NULL */
  {
    struct md3_top_app_bar *bar_no_arena = NULL;
    struct ui_arena *saved_arena = NULL;
    rc = md3_top_app_bar_create(engine, &config, &bar_no_arena);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    saved_arena = ((struct ui_arena **)bar_no_arena)[1];
    ((struct ui_arena **)bar_no_arena)[1] = NULL;
    rc = md3_top_app_bar_destroy(bar_no_arena);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_arena_destroy(saved_arena);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

#ifdef UI_TEST_MOCK_ALLOC
  /* Mock destroy failures */
  {
    struct md3_top_app_bar *bar_mock = NULL;
    rc = md3_top_app_bar_create(engine, &config, &bar_mock);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_md3_top_app_bar_destroy_mock_fail = 1;
    rc = md3_top_app_bar_destroy(bar_mock);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_md3_top_app_bar_destroy_mock_fail = 0;
  }

  {
    struct md3_top_app_bar *bar_mock = NULL;
    rc = md3_top_app_bar_create(engine, &config, &bar_mock);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_md3_top_app_bar_destroy_mock_fail = 2;
    rc = md3_top_app_bar_destroy(bar_mock);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_md3_top_app_bar_destroy_mock_fail = 0;
  }
#endif

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

  for (i = 0; i < 5; i++) {
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
    case 4:
      config.variant = (enum md3_top_app_bar_variant)99; /* default branch */
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

  /* The creation does allocations: struct, arena, title, subtitle. */
  g_malloc_fail_countdown = 0;
  rc = md3_top_app_bar_create(engine, &config, &bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(bar == NULL);

  g_malloc_fail_countdown = 1;
  rc = md3_top_app_bar_create(engine, &config, &bar);
  ASSERT(rc != UI_ERROR_NONE);

  g_malloc_fail_countdown = 2;
  rc = md3_top_app_bar_create(engine, &config, &bar);
  ASSERT(rc != UI_ERROR_NONE);

  g_malloc_fail_countdown = -1;

  /* Mock subtitle duplicate_string failure */
  g_md3_top_app_bar_mock_fail = 3;
  rc = md3_top_app_bar_create(engine, &config, &bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(bar == NULL);
  g_md3_top_app_bar_mock_fail = 0;

  /* Mock ui_top_app_bar_base_create failure */
  g_md3_top_app_bar_mock_fail = 1;
  rc = md3_top_app_bar_create(engine, &config, &bar);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  ASSERT(bar == NULL);
  g_md3_top_app_bar_mock_fail = 0;

  /* Mock arena destroy failure in cleanup */
  g_md3_top_app_bar_mock_fail = 1;
  g_md3_top_app_bar_destroy_mock_fail = 2;
  rc = md3_top_app_bar_create(engine, &config, &bar);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  ASSERT(bar == NULL);
  g_md3_top_app_bar_mock_fail = 0;
  g_md3_top_app_bar_destroy_mock_fail = 0;

  /* Test duplicate_string null out_str check */
  rc = md3_top_app_bar_test_duplicate_string(NULL, "Test", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

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

  /* Add action items to trigger initial allocation and reallocation */
  for (i = 0; i < 8; i++) {
    rc = md3_top_app_bar_add_action_item(bar, dummy_action);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

#ifdef UI_TEST_MOCK_ALLOC
  /* Test arena allocation failure in add_action_item */
  g_md3_top_app_bar_mock_fail = 2;
  rc = md3_top_app_bar_add_action_item(bar, dummy_action);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_md3_top_app_bar_mock_fail = 0;
#endif

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
