/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <math.h>
/* clang-format on */

TEST test_scrollbar_visibility(void) {
  cmp_scroll_view_t *ctx = NULL;
  float opacity;

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_view_create(&ctx));

  /* Active scroll */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_scroll_view_evaluate_scrollbar(ctx, 1, 0.0f, &opacity));
  ASSERT_EQ(1.0f, opacity);

  /* Stopped, 100ms ago */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_scroll_view_evaluate_scrollbar(ctx, 0, 100.0f, &opacity));
  ASSERT_EQ(1.0f, opacity);

  /* Stopped, 650ms ago (middle of fade) */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_scroll_view_evaluate_scrollbar(ctx, 0, 650.0f, &opacity));
  ASSERT_EQ(0.5f, opacity);

  /* Stopped > 1s */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_scroll_view_evaluate_scrollbar(ctx, 0, 1500.0f, &opacity));
  ASSERT_EQ(0.0f, opacity);

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_view_destroy(ctx));
  PASS();
}

TEST test_rubber_banding(void) {
  cmp_scroll_view_t *ctx = NULL;
  float translation;

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_view_create(&ctx));

  /* Pulling down 100px on a 1000px screen */
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_view_calculate_rubber_band(
                             ctx, 100.0f, 1000.0f, &translation));
  /* 1 - (1 / ((100 * .55 / 1000) + 1)) = ~0.052, * 1000 = ~52px */
  ASSERT_GT(100.0f, translation); /* Visual translation should be heavily damped
                                     vs physical */

  /* Extreme pull */
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_view_calculate_rubber_band(
                             ctx, 2000.0f, 1000.0f, &translation));
  ASSERT_GT(2000.0f, translation); /* Huge damping */

  /* Negative pull */
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_view_calculate_rubber_band(
                             ctx, -100.0f, 1000.0f, &translation));
  ASSERT_GT(translation, -100.0f);

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_view_destroy(ctx));
  PASS();
}

TEST test_scroll_configurations(void) {
  cmp_scroll_view_t *ctx = NULL;
  int is_interactive;

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_view_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_view_set_deceleration_rate(
                             ctx, CMP_SCROLL_DECELERATION_FAST));
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_view_set_paging_enabled(ctx, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_view_set_keyboard_dismiss_mode(
                             ctx, CMP_SCROLL_KEYBOARD_DISMISS_INTERACTIVE));

  /* Hit Test Scrollbar */
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_view_hit_test_scrollbar(
                             ctx, 390.0f, 100.0f, 400.0f, &is_interactive));
  ASSERT_EQ(1, is_interactive); /* Inside 16pt trailing region */

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_view_hit_test_scrollbar(
                             ctx, 300.0f, 100.0f, 400.0f, &is_interactive));
  ASSERT_EQ(0, is_interactive);

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_view_destroy(ctx));
  PASS();
}

TEST test_null_args(void) {
  cmp_scroll_view_t *sv = NULL;
  float f;
  int i;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_scroll_view_create(NULL));
  cmp_scroll_view_create(&sv);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_scroll_view_set_deceleration_rate(
                                       NULL, CMP_SCROLL_DECELERATION_FAST));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_scroll_view_set_paging_enabled(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_scroll_view_set_keyboard_dismiss_mode(
                                       NULL, CMP_SCROLL_KEYBOARD_DISMISS_NONE));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_scroll_view_evaluate_scrollbar(NULL, 1, 0.0f, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_scroll_view_evaluate_scrollbar(sv, 1, 0.0f, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_scroll_view_hit_test_scrollbar(NULL, 10.0f, 10.0f, 100.0f, &i));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_scroll_view_hit_test_scrollbar(sv, 10.0f, 10.0f, 100.0f, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_scroll_view_calculate_rubber_band(NULL, 100.0f, 1000.0f, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_scroll_view_calculate_rubber_band(sv, 100.0f, 1000.0f, NULL));

  cmp_scroll_view_destroy(sv);
  PASS();
}

SUITE(scroll_view_suite) {
  RUN_TEST(test_scrollbar_visibility);
  RUN_TEST(test_rubber_banding);
  RUN_TEST(test_scroll_configurations);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(scroll_view_suite);
  GREATEST_MAIN_END();
}
