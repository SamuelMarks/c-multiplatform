/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_visuals_suite);

TEST test_gradient_create_destroy(void) {
  cmp_gradient_t *grad = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_gradient_create(&grad, CMP_GRADIENT_LINEAR));
  ASSERT_NEQ(NULL, grad);
  ASSERT_EQ(CMP_GRADIENT_LINEAR, grad->type);

  ASSERT_EQ(CMP_SUCCESS, cmp_gradient_destroy(grad));
  PASS();
}

TEST test_gradient_stops(void) {
  cmp_gradient_t *grad = NULL;
  cmp_color_t c1 = {1.0f, 0.0f, 0.0f, 1.0f, CMP_COLOR_SPACE_SRGB};
  cmp_color_t c2 = {0.0f, 1.0f, 0.0f, 1.0f, CMP_COLOR_SPACE_SRGB};

  cmp_gradient_create(&grad, CMP_GRADIENT_RADIAL);

  ASSERT_EQ(CMP_SUCCESS, cmp_gradient_add_stop(grad, c1, 0.0f));
  ASSERT_EQ(CMP_SUCCESS, cmp_gradient_add_stop(grad, c2, 1.0f));

  ASSERT_EQ(2, grad->stop_count);
  ASSERT_EQ(1.0f, grad->stops[0].color.r);
  ASSERT_EQ(0.0f, grad->stops[0].position);
  ASSERT_EQ(1.0f, grad->stops[1].color.g);
  ASSERT_EQ(1.0f, grad->stops[1].position);

  cmp_gradient_destroy(grad);
  PASS();
}

TEST test_color_parsing(void) {
  cmp_color_t c;
  ASSERT_EQ(CMP_SUCCESS,
            cmp_color_parse_p3("color(display-p3 1.0 0.5 0.0)", &c));
  ASSERT_EQ(CMP_COLOR_SPACE_DISPLAY_P3, c.space);
  ASSERT_EQ(1.0f, c.a);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_color_parse_p3("rgb(255,0,0)", &c));
  PASS();
}

TEST test_color_mixing(void) {
  cmp_color_t c1 = {1.0f, 0.0f, 0.0f, 1.0f, CMP_COLOR_SPACE_SRGB};
  cmp_color_t c2 = {0.0f, 0.0f, 1.0f, 1.0f, CMP_COLOR_SPACE_SRGB};
  cmp_color_t out;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_color_mix(&c1, &c2, 0.5f, CMP_COLOR_SPACE_SRGB, &out));
  ASSERT_EQ(CMP_COLOR_SPACE_SRGB, out.space);
  ASSERT_EQ(0.5f, out.r);
  ASSERT_EQ(0.0f, out.g);
  ASSERT_EQ(0.5f, out.b);
  ASSERT_EQ(1.0f, out.a);

  PASS();
}

TEST test_color_luminance_contrast(void) {
  cmp_color_t black = {0.0f, 0.0f, 0.0f, 1.0f, CMP_COLOR_SPACE_SRGB};
  cmp_color_t white = {1.0f, 1.0f, 1.0f, 1.0f, CMP_COLOR_SPACE_SRGB};
  float lum_b, lum_w, ratio;

  ASSERT_EQ(CMP_SUCCESS, cmp_color_luminance(&black, &lum_b));
  ASSERT_EQ(CMP_SUCCESS, cmp_color_luminance(&white, &lum_w));

  /* Black is ~0, White is ~1 (0.2126+0.7152+0.0722) */
  ASSERT(lum_b < 0.01f);
  ASSERT(lum_w > 0.99f);

  ASSERT_EQ(CMP_SUCCESS, cmp_color_contrast_ratio(&white, &black, &ratio));
  /* Approx 21:1 for perfect white on black */
  ASSERT(ratio > 20.0f);

  PASS();
}

TEST test_icc_profile(void) {
  unsigned char dummy_image[64] = {0};
  void *profile = NULL;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_icc_profile_parse(dummy_image, sizeof(dummy_image), &profile));
  ASSERT_EQ(NULL, profile); /* Stubbed to null */
  PASS();
}

SUITE(cmp_visuals_suite) {
  RUN_TEST(test_gradient_create_destroy);
  RUN_TEST(test_gradient_stops);
  RUN_TEST(test_color_parsing);
  RUN_TEST(test_color_mixing);
  RUN_TEST(test_color_luminance_contrast);
  RUN_TEST(test_icc_profile);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_visuals_suite);
  GREATEST_MAIN_END();
}
