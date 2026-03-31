/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_forced_colors_lifecycle(void) {
  cmp_forced_colors_t *ctx = NULL;
  int res = cmp_forced_colors_create(&ctx);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, ctx);

  res = cmp_forced_colors_destroy(ctx);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_forced_colors_null_args(void) {
  cmp_forced_colors_t *ctx = NULL;
  int strip = 0;
  int res = cmp_forced_colors_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_forced_colors_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_forced_colors_set(NULL, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_forced_colors_create(&ctx);

  res = cmp_forced_colors_strip_background(NULL, &strip);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_forced_colors_strip_background(ctx, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_forced_colors_strip_box_shadow(NULL, &strip);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_forced_colors_strip_box_shadow(ctx, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_forced_colors_destroy(ctx);
  PASS();
}

TEST test_forced_colors_apply(void) {
  cmp_forced_colors_t *ctx = NULL;
  int strip_bg = -1;
  int strip_shadow = -1;
  int res;

  cmp_forced_colors_create(&ctx);

  /* By default, not enabled */
  res = cmp_forced_colors_strip_background(ctx, &strip_bg);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, strip_bg);

  res = cmp_forced_colors_strip_box_shadow(ctx, &strip_shadow);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, strip_shadow);

  /* Enable forced colors */
  res = cmp_forced_colors_set(ctx, 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_forced_colors_strip_background(ctx, &strip_bg);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, strip_bg);

  res = cmp_forced_colors_strip_box_shadow(ctx, &strip_shadow);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, strip_shadow);

  cmp_forced_colors_destroy(ctx);
  PASS();
}

SUITE(forced_colors_suite) {
  RUN_TEST(test_forced_colors_lifecycle);
  RUN_TEST(test_forced_colors_null_args);
  RUN_TEST(test_forced_colors_apply);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  (void)argc;
  (void)argv;
  RUN_SUITE(forced_colors_suite);
  GREATEST_MAIN_END();
}
