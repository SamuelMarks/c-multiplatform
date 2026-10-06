/**
 * @file test_material2_top_app_bar.c
 * @brief Unit tests for Material Design 2 Top App Bar component.
 */

/* clang-format off */
#include "material2/md2_top_app_bar.h"
#include "ui_error.h"
#include "ui_engine.h"
#include "ui_top_app_bar_base.h"

#include "ui_test_mock_mem.h"
#include "ui_component.h"
#include "ui_arena.h"

struct md2_top_app_bar {
  struct ui_component *component;
  struct ui_top_app_bar_base *base;
  struct ui_arena *arena;
  enum md2_top_app_bar_variant variant;
  char *title;
};

#include <greatest.h>
/* clang-format on */

TEST test_md2_top_app_bar_create_destroy(void) {
  struct md2_top_app_bar *bar = NULL;
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  /* Test with regular variant */
  rc = md2_top_app_bar_create(engine, MD2_TOP_APP_BAR_REGULAR, "Test Title",
                              &bar);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, bar);

  rc = md2_top_app_bar_destroy(bar);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  /* Test with prominent variant and NULL title */
  rc = md2_top_app_bar_create(engine, MD2_TOP_APP_BAR_PROMINENT, NULL, &bar);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, bar);

  rc = md2_top_app_bar_destroy(bar);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  ui_engine_destroy(engine);
  PASS();
}

TEST test_md2_top_app_bar_invalid_args(void) {
  struct md2_top_app_bar *bar = NULL;
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  struct ui_top_app_bar_base *base = NULL;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  /* Null checks */
  rc = md2_top_app_bar_create(NULL, MD2_TOP_APP_BAR_REGULAR, "Title", &bar);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_top_app_bar_create(engine, MD2_TOP_APP_BAR_REGULAR, "Title", NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_top_app_bar_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_top_app_bar_get_base(NULL, &base);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_top_app_bar_set_title(NULL, "Title");
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_top_app_bar_create(engine, MD2_TOP_APP_BAR_REGULAR, "Title", &bar);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_top_app_bar_get_base(bar, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_top_app_bar_set_title(bar, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  md2_top_app_bar_destroy(bar);
  ui_engine_destroy(engine);

  PASS();
}

TEST test_md2_top_app_bar_getters_setters(void) {
  struct md2_top_app_bar *bar = NULL;
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  struct ui_top_app_bar_base *base = NULL;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_top_app_bar_create(engine, MD2_TOP_APP_BAR_REGULAR, "Title", &bar);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_top_app_bar_get_base(bar, &base);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, base);

  rc = md2_top_app_bar_set_title(bar, "New Title");
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  md2_top_app_bar_destroy(bar);
  ui_engine_destroy(engine);

  PASS();
}

TEST test_md2_top_app_bar_oom(void) {
  struct md2_top_app_bar *bar = NULL;
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  ui_error_t rc;
  int i;

  memset(&config, 0, sizeof(config));
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  for (i = 0; i < 500; ++i) {
    g_malloc_fail_countdown = i;
    rc = md2_top_app_bar_create(engine, MD2_TOP_APP_BAR_PROMINENT, "OOM Title",
                                &bar);
    if (rc == UI_ERROR_NONE) {
      md2_top_app_bar_destroy(bar);
      break;
    }
  }
  g_malloc_fail_countdown = -1;

  rc = md2_top_app_bar_create(engine, MD2_TOP_APP_BAR_REGULAR, "Initial", &bar);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  for (i = 0; i < 50; ++i) {
    g_malloc_fail_countdown = i;
    rc = md2_top_app_bar_set_title(bar, "New OOM Title");
    if (rc == UI_ERROR_NONE) {
      break;
    }
  }
  g_malloc_fail_countdown = -1;
  md2_top_app_bar_destroy(bar);
  ui_engine_destroy(engine);

  PASS();
}

extern int g_md2_top_app_bar_mock_destroy_fail;
int g_md2_top_app_bar_mock_destroy_fail = 0;

TEST test_md2_top_app_bar_mock_fail(void) {
  struct md2_top_app_bar *bar = NULL;
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_top_app_bar_create(engine, MD2_TOP_APP_BAR_REGULAR, "Test Title",
                              &bar);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  g_md2_top_app_bar_mock_destroy_fail = 1;
  rc = md2_top_app_bar_destroy(bar);
  ASSERT_EQ_FMT(UI_ERROR_UNKNOWN, rc, "%d");
  g_md2_top_app_bar_mock_destroy_fail = 0;

  ui_engine_destroy(engine);
  PASS();
}

TEST test_md2_top_app_bar_partial_destroy(void) {
  struct md2_top_app_bar *p_bar;
  ui_error_t rc;

  p_bar = ui_mock_malloc(sizeof(*p_bar));
  memset(p_bar, 0, sizeof(*p_bar));

  rc = md2_top_app_bar_destroy(p_bar);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  PASS();
}

TEST test_md2_top_app_bar_strcpy_fail(void) {
  struct md2_top_app_bar *bar = NULL;
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  rc = ui_engine_create(&config, &engine);

  g_mock_strcpy_fail = 1;
  rc = md2_top_app_bar_create(engine, MD2_TOP_APP_BAR_REGULAR, "Test Title",
                              &bar);
  md2_top_app_bar_destroy(bar);

  rc = md2_top_app_bar_create(engine, MD2_TOP_APP_BAR_REGULAR, "Test Title",
                              &bar);
  g_mock_strcpy_fail = 1;
  rc = md2_top_app_bar_set_title(bar, "Another title");

  md2_top_app_bar_destroy(bar);
  ui_engine_destroy(engine);
  PASS();
}

SUITE(material2_top_app_bar_suite) {
  RUN_TEST(test_md2_top_app_bar_create_destroy);
  RUN_TEST(test_md2_top_app_bar_invalid_args);
  RUN_TEST(test_md2_top_app_bar_getters_setters);
  RUN_TEST(test_md2_top_app_bar_oom);
  RUN_TEST(test_md2_top_app_bar_mock_fail);
  RUN_TEST(test_md2_top_app_bar_partial_destroy);
  RUN_TEST(test_md2_top_app_bar_strcpy_fail);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(material2_top_app_bar_suite);
  GREATEST_MAIN_END();
}
