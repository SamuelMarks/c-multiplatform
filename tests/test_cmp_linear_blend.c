/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
#include <math.h>
/* clang-format on */

TEST test_linear_blend_create_destroy(void) {
  cmp_linear_blend_t *blend = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_linear_blend_create(2.2f, &blend));
  ASSERT_NEQ(NULL, blend);

  ASSERT_EQ(CMP_SUCCESS, cmp_linear_blend_destroy(blend));
  PASS();
}

TEST test_linear_blend_edge_cases(void) {
  cmp_linear_blend_t *blend = NULL;
  cmp_color_t color;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_linear_blend_create(0.0f, &blend));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_linear_blend_create(-1.0f, &blend));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_linear_blend_create(2.2f, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_linear_blend_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_linear_blend_create(2.2f, &blend));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_linear_blend_srgb_to_linear(NULL, &color, &color));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_linear_blend_srgb_to_linear(blend, NULL, &color));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_linear_blend_srgb_to_linear(blend, &color, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_linear_blend_linear_to_srgb(NULL, &color, &color));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_linear_blend_linear_to_srgb(blend, NULL, &color));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_linear_blend_linear_to_srgb(blend, &color, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_linear_blend_mix(NULL, &color, &color, 1.0f, &color));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_linear_blend_mix(blend, NULL, &color, 1.0f, &color));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_linear_blend_mix(blend, &color, NULL, 1.0f, &color));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_linear_blend_mix(blend, &color, &color, 1.0f, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_linear_blend_destroy(blend));
  PASS();
}

TEST test_linear_blend_srgb_to_linear(void) {
  cmp_linear_blend_t *blend = NULL;
  cmp_color_t srgb = {0.5f, 0.5f, 0.5f, 1.0f, CMP_COLOR_SPACE_SRGB};
  cmp_color_t linear;

  ASSERT_EQ(CMP_SUCCESS, cmp_linear_blend_create(2.2f, &blend));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_linear_blend_srgb_to_linear(blend, &srgb, &linear));

  /* 0.5 ^ 2.2 is roughly 0.2176376 */
  ASSERT(linear.r > 0.21f && linear.r < 0.22f);
  ASSERT(linear.g > 0.21f && linear.g < 0.22f);
  ASSERT(linear.b > 0.21f && linear.b < 0.22f);
  ASSERT_EQ(1.0f, linear.a);

  ASSERT_EQ(CMP_SUCCESS, cmp_linear_blend_destroy(blend));
  PASS();
}

TEST test_linear_blend_linear_to_srgb(void) {
  cmp_linear_blend_t *blend = NULL;
  cmp_color_t linear = {0.2176376f, 0.2176376f, 0.2176376f, 1.0f,
                        CMP_COLOR_SPACE_SRGB};
  cmp_color_t srgb;

  ASSERT_EQ(CMP_SUCCESS, cmp_linear_blend_create(2.2f, &blend));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_linear_blend_linear_to_srgb(blend, &linear, &srgb));

  /* Should be roughly 0.5 */
  ASSERT(srgb.r > 0.49f && srgb.r < 0.51f);
  ASSERT(srgb.g > 0.49f && srgb.g < 0.51f);
  ASSERT(srgb.b > 0.49f && srgb.b < 0.51f);
  ASSERT_EQ(1.0f, srgb.a);

  ASSERT_EQ(CMP_SUCCESS, cmp_linear_blend_destroy(blend));
  PASS();
}

TEST test_linear_blend_mix(void) {
  cmp_linear_blend_t *blend = NULL;
  cmp_color_t bg = {1.0f, 0.0f, 0.0f, 1.0f,
                    CMP_COLOR_SPACE_SRGB}; /* Opaque red */
  cmp_color_t fg = {0.0f, 0.0f, 1.0f, 0.5f,
                    CMP_COLOR_SPACE_SRGB}; /* 50% opaque blue */
  cmp_color_t blended;
  cmp_color_t trans_bg = {0.0f, 0.0f, 0.0f, 0.0f, CMP_COLOR_SPACE_SRGB};
  cmp_color_t trans_fg = {0.0f, 0.0f, 0.0f, 0.0f, CMP_COLOR_SPACE_SRGB};

  ASSERT_EQ(CMP_SUCCESS, cmp_linear_blend_create(2.2f, &blend));

  ASSERT_EQ(CMP_SUCCESS, cmp_linear_blend_mix(blend, &bg, &fg, 1.0f, &blended));

  /* Alpha should be 1.0 */
  ASSERT_EQ(1.0f, blended.a);

  /* Red shouldn't be completely 0.5 because of linear interpolation */
  ASSERT(blended.r > 0.7f && blended.r < 0.8f);
  ASSERT_EQ(0.0f, blended.g);
  ASSERT(blended.b > 0.7f && blended.b < 0.8f);

  /* Testing alpha clamping */
  ASSERT_EQ(CMP_SUCCESS, cmp_linear_blend_mix(blend, &bg, &fg, 1.5f, &blended));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_linear_blend_mix(blend, &bg, &fg, -0.5f, &blended));

  /* Testing zero alpha mix */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_linear_blend_mix(blend, &trans_bg, &trans_fg, 1.0f, &blended));
  ASSERT_EQ(0.0f, blended.a);
  ASSERT_EQ(0.0f, blended.r);

  ASSERT_EQ(CMP_SUCCESS, cmp_linear_blend_destroy(blend));
  PASS();
}

SUITE(cmp_linear_blend_suite) {
  RUN_TEST(test_linear_blend_create_destroy);
  RUN_TEST(test_linear_blend_edge_cases);
  RUN_TEST(test_linear_blend_srgb_to_linear);
  RUN_TEST(test_linear_blend_linear_to_srgb);
  RUN_TEST(test_linear_blend_mix);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_linear_blend_suite);
  GREATEST_MAIN_END();
}
