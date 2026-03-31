/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_sys_colors_lifecycle(void) {
  cmp_sys_colors_t *ctx = NULL;
  int res = cmp_sys_colors_create(&ctx);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, ctx);

  res = cmp_sys_colors_destroy(ctx);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_sys_colors_null_args(void) {
  cmp_sys_colors_t *ctx = NULL;
  cmp_color_t color;
  int res;

  memset(&color, 0, sizeof(color));

  res = cmp_sys_colors_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_sys_colors_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_sys_colors_set(NULL, CMP_SYS_COLOR_CANVAS, &color);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_sys_colors_create(&ctx);

  res = cmp_sys_colors_set(ctx, CMP_SYS_COLOR_CANVAS, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_sys_colors_set(ctx, (cmp_sys_color_keyword_t)-1, &color);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_sys_colors_set(ctx, CMP_SYS_COLOR_MAX, &color);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_sys_colors_resolve(NULL, CMP_SYS_COLOR_CANVAS, &color);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_sys_colors_resolve(ctx, CMP_SYS_COLOR_CANVAS, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_sys_colors_resolve(ctx, (cmp_sys_color_keyword_t)-1, &color);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_sys_colors_resolve(ctx, CMP_SYS_COLOR_MAX, &color);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_sys_colors_destroy(ctx);
  PASS();
}

TEST test_sys_colors_set_resolve(void) {
  cmp_sys_colors_t *ctx = NULL;
  cmp_color_t color_in;
  cmp_color_t color_out;
  int res;

  cmp_sys_colors_create(&ctx);

  memset(&color_in, 0, sizeof(color_in));
  memset(&color_out, 0, sizeof(color_out));

  color_in.r = 1.0f;
  color_in.g = 0.5f;
  color_in.b = 0.2f;
  color_in.a = 1.0f;
  color_in.space = CMP_COLOR_SPACE_SRGB;

  /* Not set initially */
  res = cmp_sys_colors_resolve(ctx, CMP_SYS_COLOR_CANVAS, &color_out);
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  /* Set it */
  res = cmp_sys_colors_set(ctx, CMP_SYS_COLOR_CANVAS, &color_in);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Resolve it */
  res = cmp_sys_colors_resolve(ctx, CMP_SYS_COLOR_CANVAS, &color_out);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ_FMT(1.0f, color_out.r, "%f");
  ASSERT_EQ_FMT(0.5f, color_out.g, "%f");
  ASSERT_EQ_FMT(0.2f, color_out.b, "%f");
  ASSERT_EQ_FMT(1.0f, color_out.a, "%f");
  ASSERT_EQ((int)CMP_COLOR_SPACE_SRGB, (int)color_out.space);

  cmp_sys_colors_destroy(ctx);
  PASS();
}

SUITE(sys_colors_suite) {
  RUN_TEST(test_sys_colors_lifecycle);
  RUN_TEST(test_sys_colors_null_args);
  RUN_TEST(test_sys_colors_set_resolve);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  (void)argc;
  (void)argv;
  RUN_SUITE(sys_colors_suite);
  GREATEST_MAIN_END();
}
