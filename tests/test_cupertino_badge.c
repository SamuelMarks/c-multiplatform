/**
 * @file test_cupertino_badge.c
 * @brief Unit tests for Cupertino Badge component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_badge.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_badge_suite);

TEST test_badge_invalid_arguments(void) {
  struct cupertino_badge_descriptor desc;
  struct cupertino_badge *badge = NULL;
  struct ui_badge_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *text = NULL;
  ui_color_t bg = 0;
  ui_color_t fg = 0;
  float w = 0.0f;
  float h = 0.0f;
  int count = 0;
  int hidden = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Create invalid */
  rc = cupertino_badge_create(NULL, &desc, &badge);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_badge_create(dummy_engine, NULL, &badge);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_badge_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy invalid */
  rc = cupertino_badge_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set / get count */
  rc = cupertino_badge_set_count(NULL, 5);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_badge_get_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set / get text */
  rc = cupertino_badge_set_text(NULL, "New");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_badge_get_text(NULL, &text);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set / get hidden */
  rc = cupertino_badge_set_hidden(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_badge_is_hidden(NULL, &hidden);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Dimensions / colors / base */
  rc = cupertino_badge_get_dimensions(NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_badge_get_colors(NULL, &bg, &fg);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_badge_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid badge to test NULL output pointers in getters */
  desc.count = 3;
  rc = cupertino_badge_create(dummy_engine, &desc, &badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(badge != NULL);

  rc = cupertino_badge_get_count(badge, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_badge_get_text(badge, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_badge_is_hidden(badge, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_badge_get_dimensions(badge, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_badge_get_dimensions(badge, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_badge_get_colors(badge, NULL, &fg);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_badge_get_colors(badge, &bg, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_badge_get_base(badge, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Test badge setters with badge->base == NULL */
  {
    struct ui_badge_base *saved_base = badge->base;
    badge->base = NULL;
    rc = cupertino_badge_set_count(badge, 10);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_badge_set_text(badge, "ABC");
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_badge_set_hidden(badge, 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    badge->base = saved_base;
  }

  /* Destroy with badge->base == NULL */
  {
    struct ui_badge_base *saved_base = badge->base;
    badge->base = NULL;
    rc = cupertino_badge_destroy(badge);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ui_badge_base_destroy(saved_base);
    badge = NULL;
  }

  PASS();
}

TEST test_badge_numeric_and_overflow(void) {
  struct cupertino_badge_descriptor desc;
  struct cupertino_badge *badge = NULL;
  struct ui_badge_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *text = NULL;
  float w = 0.0f;
  float h = 0.0f;
  int count = 0;
  int hidden = 0;
  ui_color_t bg = 0;
  ui_color_t fg = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_BADGE_STYLE_STANDARD;
  desc.count = 5;
  desc.max_count = 99;

  rc = cupertino_badge_create(dummy_engine, &desc, &badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(badge != NULL);

  rc = cupertino_badge_get_base(badge, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = cupertino_badge_get_count(badge, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(5, count);

  rc = cupertino_badge_get_text(badge, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("5", text);

  rc = cupertino_badge_get_dimensions(badge, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_BADGE_STANDARD_HEIGHT, h);
  ASSERT_EQ(CUPERTINO_BADGE_STANDARD_HEIGHT, w);

  rc = cupertino_badge_get_colors(badge, &bg, &fg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(bg != 0);
  ASSERT_EQ(UI_COLOR_ARGB(255, 255, 255, 255), fg);

  /* Update count to double digits */
  rc = cupertino_badge_set_count(badge, 42);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_badge_get_text(badge, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("42", text);
  rc = cupertino_badge_get_dimensions(badge, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(24.0f, w);

  /* Update count to overflow > max_count */
  rc = cupertino_badge_set_count(badge, 150);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_badge_get_text(badge, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("99+", text);
  rc = cupertino_badge_get_dimensions(badge, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(w > 24.0f);

  /* Set count to 0 -> empty text */
  rc = cupertino_badge_set_count(badge, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_badge_get_text(badge, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", text);
  rc = cupertino_badge_get_dimensions(badge, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, w);

  /* Negative count should fail */
  rc = cupertino_badge_set_count(badge, -1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Hidden toggle */
  rc = cupertino_badge_set_hidden(badge, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_badge_is_hidden(badge, &hidden);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, hidden);

  rc = cupertino_badge_set_hidden(badge, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_badge_is_hidden(badge, &hidden);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, hidden);

  rc = cupertino_badge_destroy(badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Creation with count > max_count (overflow at creation) and is_hidden = 1 */
  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_BADGE_STYLE_STANDARD;
  desc.count = 200;
  desc.max_count = 99;
  desc.is_hidden = 1;
  rc = cupertino_badge_create(dummy_engine, &desc, &badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(badge != NULL);

  rc = cupertino_badge_is_hidden(badge, &hidden);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, hidden);

  rc = cupertino_badge_get_text(badge, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("99+", text);

  rc = cupertino_badge_destroy(badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Creation with count == 0 and default max_count */
  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_BADGE_STYLE_STANDARD;
  desc.count = 0;
  desc.max_count = 0; /* Should fallback to default max count */
  rc = cupertino_badge_create(dummy_engine, &desc, &badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(badge != NULL);

  rc = cupertino_badge_get_text(badge, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", text);

  rc = cupertino_badge_destroy(badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_badge_custom_text_and_dot(void) {
  struct cupertino_badge_descriptor desc;
  struct cupertino_badge *badge = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *text = NULL;
  float w = 0.0f;
  float h = 0.0f;
  ui_error_t rc;

  /* Custom text badge */
  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_BADGE_STYLE_STANDARD;
  desc.custom_text = "New";

  rc = cupertino_badge_create(dummy_engine, &desc, &badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(badge != NULL);

  rc = cupertino_badge_get_text(badge, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("New", text);

  rc = cupertino_badge_get_dimensions(badge, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(w > 20.0f);
  ASSERT_EQ(CUPERTINO_BADGE_STANDARD_HEIGHT, h);

  /* Clear custom text with NULL */
  rc = cupertino_badge_set_text(badge, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_badge_get_text(badge, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", text);

  rc = cupertino_badge_destroy(badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Dot style badge */
  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_BADGE_STYLE_DOT;

  rc = cupertino_badge_create(dummy_engine, &desc, &badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(badge != NULL);

  rc = cupertino_badge_get_dimensions(badge, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_BADGE_DOT_SIZE, w);
  ASSERT_EQ(CUPERTINO_BADGE_DOT_SIZE, h);

  rc = cupertino_badge_destroy(badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_badge_oom_mock(void) {
  struct cupertino_badge_descriptor desc;
  struct cupertino_badge *badge = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.count = 10;

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_badge_mock_get_system_color_fail;
  extern int g_cupertino_badge_mock_create_fail;
  extern int g_cupertino_badge_mock_set_text_fail;
  extern int g_cupertino_badge_mock_set_value_fail;
  extern int g_cupertino_badge_mock_set_hidden_fail;
  extern int g_cupertino_badge_mock_destroy_fail;

  g_malloc_fail_countdown = 0;
  rc = cupertino_badge_create(dummy_engine, &desc, &badge);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, badge);

  g_malloc_fail_countdown = 1;
  rc = cupertino_badge_create(dummy_engine, &desc, &badge);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, badge);

  g_malloc_fail_countdown = -1;

  /* Mock cupertino_get_system_color fallback */
  g_cupertino_badge_mock_get_system_color_fail = 1;
  rc = cupertino_badge_create(dummy_engine, &desc, &badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(badge != NULL);
  g_cupertino_badge_mock_get_system_color_fail = 0;
  rc = cupertino_badge_destroy(badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock badge_base_create failure */
  g_cupertino_badge_mock_create_fail = 1;
  rc = cupertino_badge_create(dummy_engine, &desc, &badge);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, badge);
  g_cupertino_badge_mock_create_fail = 0;

  /* Mock set_text failure in create */
  desc.custom_text = "Hello";
  g_cupertino_badge_mock_set_text_fail = 1;
  rc = cupertino_badge_create(dummy_engine, &desc, &badge);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, badge);

  /* Mock set_text failure + destroy failure in create */
  g_cupertino_badge_mock_destroy_fail = 1;
  rc = cupertino_badge_create(dummy_engine, &desc, &badge);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  ASSERT_EQ(NULL, badge);
  g_cupertino_badge_mock_set_text_fail = 0;
  g_cupertino_badge_mock_destroy_fail = 0;
  desc.custom_text = NULL;

  /* Mock set_value failure in create */
  desc.count = 5;
  g_cupertino_badge_mock_set_value_fail = 1;
  rc = cupertino_badge_create(dummy_engine, &desc, &badge);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, badge);

  /* Mock set_value failure + destroy failure in create */
  g_cupertino_badge_mock_destroy_fail = 1;
  rc = cupertino_badge_create(dummy_engine, &desc, &badge);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  ASSERT_EQ(NULL, badge);
  g_cupertino_badge_mock_set_value_fail = 0;
  g_cupertino_badge_mock_destroy_fail = 0;

  /* Mock set_hidden failure in create */
  g_cupertino_badge_mock_set_hidden_fail = 1;
  rc = cupertino_badge_create(dummy_engine, &desc, &badge);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, badge);

  /* Mock set_hidden failure + destroy failure in create */
  g_cupertino_badge_mock_destroy_fail = 1;
  rc = cupertino_badge_create(dummy_engine, &desc, &badge);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  ASSERT_EQ(NULL, badge);
  g_cupertino_badge_mock_set_hidden_fail = 0;
  g_cupertino_badge_mock_destroy_fail = 0;

  /* Mock destroy failure in destroy */
  rc = cupertino_badge_create(dummy_engine, &desc, &badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_badge_mock_destroy_fail = 1;
  rc = cupertino_badge_destroy(badge);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_badge_mock_destroy_fail = 0;

  /* Mock set_value failure in set_count */
  rc = cupertino_badge_create(dummy_engine, &desc, &badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_badge_mock_set_value_fail = 1;
  rc = cupertino_badge_set_count(badge, 20);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_badge_mock_set_value_fail = 0;

  /* Mock set_text failure in set_text */
  g_cupertino_badge_mock_set_text_fail = 1;
  rc = cupertino_badge_set_text(badge, "World");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_badge_mock_set_text_fail = 0;

  /* Mock set_hidden failure in set_hidden */
  g_cupertino_badge_mock_set_hidden_fail = 1;
  rc = cupertino_badge_set_hidden(badge, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_badge_mock_set_hidden_fail = 0;

  rc = cupertino_badge_destroy(badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#else
  rc = cupertino_badge_create(NULL, &desc, &badge);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_badge_suite) {
  RUN_TEST(test_badge_invalid_arguments);
  RUN_TEST(test_badge_numeric_and_overflow);
  RUN_TEST(test_badge_custom_text_and_dot);
  RUN_TEST(test_badge_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_badge_suite);
  GREATEST_MAIN_END();
}
