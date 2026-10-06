/**
 * @file test_material2_card.c
 * @brief Unit tests for Material 2 Card component.
 */

/* clang-format off */
#include "material2/md2_card.h"
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
struct md2_card_hack {
  struct ui_card_base *base;
};

SUITE(md2_card_suite);

TEST test_md2_card_base_failures(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md2_card *card = NULL;
  struct ui_card_base *orig_base = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md2_card_create(engine, MD2_CARD_ELEVATED, &card);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  orig_base = ((struct md2_card_hack *)card)->base;
  ((struct md2_card_hack *)card)->base = NULL;

  /* Force fail set_title */
  rc = md2_card_set_title(card, "Title");
  ASSERT_NEQ(UI_ERROR_NONE, rc);

  /* Force fail set_subtitle */
  rc = md2_card_set_subtitle(card, "Subtitle");
  ASSERT_NEQ(UI_ERROR_NONE, rc);

  ((struct md2_card_hack *)card)->base = orig_base;
  rc = md2_card_destroy(card);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Force fail create by making set_attribute fail inside create (which happens
   * via OOM below) */
  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_md2_card_lifecycle(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md2_card *card = NULL;
  struct ui_card_base *base = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = md2_card_create(NULL, MD2_CARD_ELEVATED, &card);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_card_create(engine, MD2_CARD_ELEVATED, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_card_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_card_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_card_set_title(NULL, "Title");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_card_set_subtitle(NULL, "Subtitle");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid create */
  rc = md2_card_create(engine, MD2_CARD_ELEVATED, &card);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(card != NULL);

  rc = md2_card_get_base(card, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md2_card_get_base(card, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = md2_card_set_title(card, "Title");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md2_card_set_title(card, NULL); /* Test NULL title */
  ASSERT_NEQ(UI_ERROR_NONE, rc);

  rc = md2_card_set_subtitle(card, "Subtitle");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md2_card_set_subtitle(card, NULL); /* Test NULL subtitle */
  ASSERT_NEQ(UI_ERROR_NONE, rc);

  rc = md2_card_destroy(card);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md2_card_create(engine, MD2_CARD_OUTLINED, &card);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md2_card_destroy(card);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
TEST test_md2_card_oom(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md2_card *card = NULL;
  ui_error_t rc;
  int i;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (i = 0; i < 200; i++) {
    g_malloc_fail_countdown = i;
    rc = md2_card_create(engine, MD2_CARD_ELEVATED, &card);
    if (rc == UI_ERROR_NONE) {
      md2_card_destroy(card);
      break;
    }
  }

  g_malloc_fail_countdown = -1;

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}
#endif

SUITE(md2_card_suite) {
  RUN_TEST(test_md2_card_lifecycle);
  RUN_TEST(test_md2_card_base_failures);
#ifdef UI_TEST_MOCK_ALLOC
  RUN_TEST(test_md2_card_oom);
#endif
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md2_card_suite);
  GREATEST_MAIN_END();
}
