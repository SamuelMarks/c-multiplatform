/**
 * @file test_material2_fab.c
 * @brief Unit tests for Material 2 FAB component.
 */

/* clang-format off */
#include "material2/md2_fab.h"
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

/* Hacks to access internal structure for branch coverage testing */
struct md2_fab_hack {
  struct ui_component *component;
  struct ui_fab_base *base;
  enum md2_fab_size size;
};

SUITE(md2_fab_suite);

TEST test_md2_fab_base_failures(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md2_fab *fab = NULL;
  struct ui_fab_base *orig_base = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md2_fab_create(engine, MD2_FAB_SIZE_STANDARD, "add", NULL, &fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  orig_base = ((struct md2_fab_hack *)fab)->base;
  ((struct md2_fab_hack *)fab)->base = NULL;

  rc = md2_fab_set_on_click(fab, dummy_on_click, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  ((struct md2_fab_hack *)fab)->base = orig_base;

  rc = md2_fab_destroy(fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_md2_fab_lifecycle(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md2_fab *fab = NULL;
  struct ui_fab_base *base = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = md2_fab_create(NULL, MD2_FAB_SIZE_STANDARD, "add", NULL, &fab);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_fab_create(engine, MD2_FAB_SIZE_STANDARD, "add", NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_fab_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_fab_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_fab_set_on_click(NULL, dummy_on_click, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid create standard */
  rc = md2_fab_create(engine, MD2_FAB_SIZE_STANDARD, "add", NULL, &fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fab != NULL);

  rc = md2_fab_get_base(fab, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md2_fab_get_base(fab, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Set on click */
  rc = md2_fab_set_on_click(fab, dummy_on_click, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md2_fab_destroy(fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Valid create extended */
  rc = md2_fab_create(engine, MD2_FAB_SIZE_EXTENDED, "add", "Create", &fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md2_fab_destroy(fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Valid create extended without label to hit fallback branch */
  rc = md2_fab_create(engine, MD2_FAB_SIZE_EXTENDED, "add", NULL, &fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md2_fab_destroy(fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Valid create without icon */
  rc = md2_fab_create(engine, MD2_FAB_SIZE_STANDARD, NULL, NULL, &fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md2_fab_destroy(fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
TEST test_md2_fab_oom(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md2_fab *fab = NULL;
  ui_error_t rc;
  int i;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (i = 0; i < 200; i++) {
    g_malloc_fail_countdown = i;
    rc = md2_fab_create(engine, MD2_FAB_SIZE_EXTENDED, "add", "Label", &fab);
    if (rc == UI_ERROR_NONE) {
      md2_fab_destroy(fab);
      break;
    }
  }

  g_malloc_fail_countdown = -1;

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}
#endif

SUITE(md2_fab_suite) {
  RUN_TEST(test_md2_fab_lifecycle);
  RUN_TEST(test_md2_fab_base_failures);
#ifdef UI_TEST_MOCK_ALLOC
  RUN_TEST(test_md2_fab_oom);
#endif
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md2_fab_suite);
  GREATEST_MAIN_END();
}
