/**
 * @file test_material2_theme_card.c
 * @brief Unit tests and OOM mocks for Material Design 2 theme and card
 * components.
 */

/* clang-format off */
#include "material2/md2_theme.h"
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

SUITE(md2_theme_card_suite);

TEST test_md2_color_palette_light_dark(void) {
  struct md2_color_palette pal;
  ui_error_t rc;

  /* Null checks */
  rc = md2_color_palette_init_light(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_color_palette_init_dark(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Light theme */
  rc = md2_color_palette_init_light(&pal);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((ui_uint32)0xFF6200EE, pal.primary);
  ASSERT_EQ((ui_uint32)0xFF03DAC6, pal.secondary);
  ASSERT_EQ((ui_uint32)0xFFFFFFFF, pal.on_primary);

  /* Dark theme */
  rc = md2_color_palette_init_dark(&pal);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((ui_uint32)0xFFBB86FC, pal.primary);
  ASSERT_EQ((ui_uint32)0xFF03DAC6, pal.secondary);
  ASSERT_EQ((ui_uint32)0xFF000000, pal.on_primary);

  PASS();
}

TEST test_md2_typography_styles(void) {
  struct md2_text_style style;
  ui_error_t rc;

  /* Null check */
  rc = md2_typography_get_style(MD2_TYPOGRAPHY_H1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* H1 */
  rc = md2_typography_get_style(MD2_TYPOGRAPHY_H1, &style);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(96.0f, style.font_size_sp);
  ASSERT_EQ(0, style.all_caps);

  /* Button (all-caps) */
  rc = md2_typography_get_style(MD2_TYPOGRAPHY_BUTTON, &style);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(14.0f, style.font_size_sp);
  ASSERT_EQ(1, style.all_caps);

  /* Overline (all-caps) */
  rc = md2_typography_get_style(MD2_TYPOGRAPHY_OVERLINE, &style);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(10.0f, style.font_size_sp);
  ASSERT_EQ(1, style.all_caps);

  /* Body1 */
  rc = md2_typography_get_style(MD2_TYPOGRAPHY_BODY1, &style);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(16.0f, style.font_size_sp);

  PASS();
}

TEST test_md2_theme(void) {
  struct md2_theme theme;
  ui_error_t rc;

  /* Null checks */
  rc = md2_theme_init_light(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_theme_init_dark(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Light theme */
  rc = md2_theme_init_light(&theme);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, theme.is_dark);
  ASSERT_EQ(1.0f, theme.font_scale);
  ASSERT_EQ((ui_uint32)0xFF6200EE, theme.colors.primary);

  /* Dark theme */
  rc = md2_theme_init_dark(&theme);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, theme.is_dark);
  ASSERT_EQ(1.0f, theme.font_scale);
  ASSERT_EQ((ui_uint32)0xFFBB86FC, theme.colors.primary);

  PASS();
}

TEST test_md2_card_lifecycle(void) {
  struct ui_engine_config engine_cfg;
  struct ui_engine *engine = NULL;
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
  rc = md2_card_get_base(card, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_card_set_title(NULL, "Title");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md2_card_set_subtitle(NULL, "Subtitle");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Create elevated */
  rc = md2_card_create(engine, MD2_CARD_ELEVATED, &card);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(card != NULL);

  rc = md2_card_set_title(card, "M2 Card Title");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md2_card_set_subtitle(card, "M2 Card Subtitle");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md2_card_get_base(card, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = md2_card_destroy(card);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Create outlined */
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
  struct ui_engine_config engine_cfg;
  struct ui_engine *engine = NULL;
  struct md2_card *card = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  g_malloc_fail_countdown = 0;
  rc = md2_card_create(engine, MD2_CARD_ELEVATED, &card);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(card == NULL);
  g_malloc_fail_countdown = -1;

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}
#endif

SUITE(md2_theme_card_suite) {
  RUN_TEST(test_md2_color_palette_light_dark);
  RUN_TEST(test_md2_typography_styles);
  RUN_TEST(test_md2_theme);
  RUN_TEST(test_md2_card_lifecycle);
#ifdef UI_TEST_MOCK_ALLOC
  RUN_TEST(test_md2_card_oom);
#endif
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md2_theme_card_suite);
  GREATEST_MAIN_END();
}
