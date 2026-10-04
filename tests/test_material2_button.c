/**
 * @file test_material2_button.c
 * @brief Unit tests for Material 2 Button component.
 */

/* clang-format off */
#include "material2/md2_button.h"
#include "ui_engine.h"
#include "greatest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

static ui_error_t dummy_on_click(struct ui_button_base *btn, void *user_data) {
  int *clicked = (int *)user_data;
  if (clicked)
    *clicked = 1;
  return UI_ERROR_NONE;
}

SUITE(md2_button_suite);

TEST test_md2_button_lifecycle(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md2_button *btn = NULL;
  struct ui_button_base *base = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = md2_button_create(NULL, MD2_BUTTON_CONTAINED, "text", &btn);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_button_create(engine, MD2_BUTTON_CONTAINED, "text", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_button_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_button_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_button_set_text(NULL, "text");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_button_set_text(btn, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_button_set_on_click(NULL, dummy_on_click, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid create without text */
  rc = md2_button_create(engine, MD2_BUTTON_CONTAINED, NULL, &btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(btn != NULL);

  rc = md2_button_get_base(btn, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Set text */
  rc = md2_button_set_text(btn, "uppercase me");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Set on click */
  rc = md2_button_set_on_click(btn, dummy_on_click, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md2_button_destroy(btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Valid create with text */
  rc = md2_button_create(engine, MD2_BUTTON_OUTLINED, "hello", &btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md2_button_destroy(btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
TEST test_md2_button_oom(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md2_button *btn = NULL;
  ui_error_t rc;
  int i;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (i = 0; i < 4; i++) {
    g_malloc_fail_countdown = i;
    rc = md2_button_create(engine, MD2_BUTTON_CONTAINED, "text", &btn);
    if (rc == UI_ERROR_NONE) {
      md2_button_destroy(btn);
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  g_malloc_fail_countdown = -1;

  rc = md2_button_create(engine, MD2_BUTTON_CONTAINED, "text", &btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  g_malloc_fail_countdown = 0;
  rc = md2_button_set_text(btn, "new text");
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  g_malloc_fail_countdown = -1;
  rc = md2_button_destroy(btn);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}
#endif

SUITE(md2_button_suite) {
  RUN_TEST(test_md2_button_lifecycle);
#ifdef UI_TEST_MOCK_ALLOC
  RUN_TEST(test_md2_button_oom);
#endif
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md2_button_suite);
  GREATEST_MAIN_END();
}
