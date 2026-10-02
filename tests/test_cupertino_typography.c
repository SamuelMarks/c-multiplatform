/**
 * @file test_cupertino_typography.c
 * @brief Unit tests for Apple SF Typography and Dynamic Type engine.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_typography.h"
#include "ui_arena.h"
#include <math.h>
/* clang-format on */

SUITE(cupertino_typography_suite);

TEST test_typography_metrics_and_dynamic_type(void) {
  struct cupertino_type_metrics metrics;
  ui_error_t rc;

  /* Invalid arguments */
  rc =
      cupertino_typography_get_style((enum cupertino_text_style) - 1,
                                     CUPERTINO_DYNAMIC_TYPE_LARGE, 0, &metrics);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_typography_get_style(
      CUPERTINO_TEXT_STYLE_COUNT, CUPERTINO_DYNAMIC_TYPE_LARGE, 0, &metrics);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_typography_get_style(CUPERTINO_TEXT_STYLE_BODY,
                                      (enum cupertino_dynamic_type_size) - 1, 0,
                                      &metrics);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_typography_get_style(CUPERTINO_TEXT_STYLE_BODY,
                                      CUPERTINO_DYNAMIC_TYPE_SIZE_COUNT, 0,
                                      &metrics);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_typography_get_style(CUPERTINO_TEXT_STYLE_BODY,
                                      CUPERTINO_DYNAMIC_TYPE_LARGE, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Reference scale (Large): Body is 17pt, 22pt leading, weight 400 */
  rc = cupertino_typography_get_style(
      CUPERTINO_TEXT_STYLE_BODY, CUPERTINO_DYNAMIC_TYPE_LARGE, 0, &metrics);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(metrics.point_size - 17.0f) < 1e-4f);
  ASSERT(fabs(metrics.leading - 22.0f) < 1e-4f);
  ASSERT_EQ(400, metrics.weight);
  ASSERT_EQ(0, metrics.use_display_face);

  /* Reference scale (Large): Large Title is 34pt, 41pt leading, Display face */
  rc =
      cupertino_typography_get_style(CUPERTINO_TEXT_STYLE_LARGE_TITLE,
                                     CUPERTINO_DYNAMIC_TYPE_LARGE, 0, &metrics);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(metrics.point_size - 34.0f) < 1e-4f);
  ASSERT(fabs(metrics.leading - 41.0f) < 1e-4f);
  ASSERT_EQ(1, metrics.use_display_face);

  /* Headline is Semi-Bold (weight 600) */
  rc = cupertino_typography_get_style(
      CUPERTINO_TEXT_STYLE_HEADLINE, CUPERTINO_DYNAMIC_TYPE_LARGE, 0, &metrics);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(600, metrics.weight);

  /* Bold text accessibility toggle */
  rc = cupertino_typography_get_style(
      CUPERTINO_TEXT_STYLE_BODY, CUPERTINO_DYNAMIC_TYPE_LARGE, 1, &metrics);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(600, metrics.weight); /* 400 -> 600 */

  rc = cupertino_typography_get_style(
      CUPERTINO_TEXT_STYLE_HEADLINE, CUPERTINO_DYNAMIC_TYPE_LARGE, 1, &metrics);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(700, metrics.weight); /* 600 -> 700 */

  /* Accessibility scale (AX5 massive enlargement) */
  rc = cupertino_typography_get_style(CUPERTINO_TEXT_STYLE_BODY,
                                      CUPERTINO_DYNAMIC_TYPE_AX5, 0, &metrics);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(metrics.point_size > 50.0f); /* 17 * 3.1176 ~ 53pt */
  ASSERT_EQ(1, metrics.use_display_face);

  PASS();
}

TEST test_tracking_and_min_tap_target(void) {
  float tracking = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  ui_error_t rc;

  /* Tracking invalid args */
  rc = cupertino_typography_get_tracking(0.0f, &tracking);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_typography_get_tracking(-5.0f, &tracking);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_typography_get_tracking(16.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Small size (< 20pt) tracking is positive */
  rc = cupertino_typography_get_tracking(12.0f, &tracking);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tracking > 0.0f);

  /* Display size (>= 20pt) tracking is negative (tighter) */
  rc = cupertino_typography_get_tracking(34.0f, &tracking);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tracking < 0.0f);

  /* Min tap target invalid args */
  rc = cupertino_typography_ensure_min_tap_target(10.0f, 10.0f, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_typography_ensure_min_tap_target(10.0f, 10.0f, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Clamping small dimensions up to 44pt */
  rc = cupertino_typography_ensure_min_tap_target(20.0f, 15.0f, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(w - 44.0f) < 1e-5f);
  ASSERT(fabs(h - 44.0f) < 1e-5f);

  /* Dimensions larger than 44pt are preserved */
  rc = cupertino_typography_ensure_min_tap_target(60.0f, 80.0f, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(w - 60.0f) < 1e-5f);
  ASSERT(fabs(h - 80.0f) < 1e-5f);

  PASS();
}

TEST test_typography_tokens_injection(void) {
  struct ui_arena *arena = NULL;
  struct ui_design_token_dict dict;
  float num = 0.0f;
  ui_error_t rc;

  /* Invalid args */
  rc = cupertino_typography_apply_tokens(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid injection */
  rc = ui_arena_create(4096, &arena);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(arena != NULL);

  rc = ui_design_token_dict_init(arena, &dict);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_typography_apply_tokens(&dict);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Verify tokens present */
  rc = ui_design_token_get_number(&dict, "--apple-typescale-body-size", &num);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(num - 17.0f) < 1e-4f);

  rc = ui_design_token_get_number(
      &dict, "--apple-typescale-large-title-leading", &num);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(num - 41.0f) < 1e-4f);

  rc = ui_arena_destroy(arena);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
TEST test_typography_mock_and_branch_coverage(void) {
  extern int g_cupertino_typography_mock_get_tracking_fail;
  extern int g_cupertino_typography_mock_token_fail;
  extern int g_cupertino_typography_mock_token_fail_step;
  extern int g_cupertino_typography_mock_weight_override;
  extern void cupertino_typography_mock_reset_token_count(void);

  struct cupertino_type_metrics metrics;
  struct ui_arena *arena = NULL;
  struct ui_design_token_dict dict;
  ui_error_t rc;
  int step;

  /* Additional boundary / arg checks */
  rc =
      cupertino_typography_get_style(CUPERTINO_TEXT_STYLE_LARGE_TITLE,
                                     CUPERTINO_DYNAMIC_TYPE_LARGE, 0, &metrics);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Bold weight branches */
  g_cupertino_typography_mock_weight_override = 650; /* >600 and <=700 -> 800 */
  rc = cupertino_typography_get_style(
      CUPERTINO_TEXT_STYLE_BODY, CUPERTINO_DYNAMIC_TYPE_LARGE, 1, &metrics);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(800, metrics.weight);

  g_cupertino_typography_mock_weight_override = 800; /* >700 -> 900 */
  rc = cupertino_typography_get_style(
      CUPERTINO_TEXT_STYLE_BODY, CUPERTINO_DYNAMIC_TYPE_LARGE, 1, &metrics);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(900, metrics.weight);

  g_cupertino_typography_mock_weight_override = 0;

  /* Mock tracking failure */
  g_cupertino_typography_mock_get_tracking_fail = 1;
  rc = cupertino_typography_get_style(
      CUPERTINO_TEXT_STYLE_BODY, CUPERTINO_DYNAMIC_TYPE_LARGE, 0, &metrics);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_typography_mock_get_tracking_fail = 0;

  /* Token injection failures */
  rc = ui_arena_create(4096, &arena);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_design_token_dict_init(arena, &dict);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  extern int g_cupertino_typography_mock_get_style_fail;
  g_cupertino_typography_mock_get_style_fail = 1;
  rc = cupertino_typography_apply_tokens(&dict);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_typography_mock_get_style_fail = 0;

  g_cupertino_typography_mock_token_fail = 1;
  rc = cupertino_typography_apply_tokens(&dict);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_cupertino_typography_mock_token_fail = 0;

  /* Step through token failure steps (size, leading, tracking, weight) */
  for (step = 0; step < 4; step++) {
    cupertino_typography_mock_reset_token_count();
    g_cupertino_typography_mock_token_fail_step = step;
    rc = cupertino_typography_apply_tokens(&dict);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  g_cupertino_typography_mock_token_fail_step = -1;
  cupertino_typography_mock_reset_token_count();

  rc = ui_arena_destroy(arena);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}
#endif

SUITE(cupertino_typography_suite) {
  RUN_TEST(test_typography_metrics_and_dynamic_type);
  RUN_TEST(test_tracking_and_min_tap_target);
  RUN_TEST(test_typography_tokens_injection);
#ifdef UI_TEST_MOCK_ALLOC
  RUN_TEST(test_typography_mock_and_branch_coverage);
#endif
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_typography_suite);
  GREATEST_MAIN_END();
}
