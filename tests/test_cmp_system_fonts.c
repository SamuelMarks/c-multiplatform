/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_system_fonts(void) {
  cmp_system_fonts_t *ctx = NULL;
  cmp_font_t *font = NULL;
  int is_display;
  float tracking;
  float lh, leading, baseline;

  ASSERT_EQ(CMP_SUCCESS, cmp_system_fonts_create(&ctx));

  /* Test Font Request ecosystem */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_system_fonts_request(ctx, CMP_SYSTEM_FONT_SF_PRO,
                                     CMP_TEXT_STYLE_BODY, 400, &font));
  ASSERT_EQ(17.0f, font->default_size); /* Mocked memory pointer logic */

  ASSERT_EQ(CMP_SUCCESS,
            cmp_system_fonts_request(ctx, CMP_SYSTEM_FONT_SF_COMPACT,
                                     CMP_TEXT_STYLE_BODY, 400, &font));
  ASSERT_EQ(16.0f, font->default_size);

  /* Test Tracking / Optical Sizing constraints */
  ASSERT_EQ(CMP_SUCCESS, cmp_system_fonts_get_tracking_and_optical(
                             ctx, 32.0f, &is_display, &tracking));
  ASSERT_EQ(1, is_display); /* Swapped to Display Variant */

  ASSERT_EQ(CMP_SUCCESS, cmp_system_fonts_get_tracking_and_optical(
                             ctx, 14.0f, &is_display, &tracking));
  ASSERT_EQ(0, is_display); /* Stays on Text Variant */

  /* Test Semantic Line Heights */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_system_fonts_get_metrics(ctx, CMP_TEXT_STYLE_LARGE_TITLE, &lh,
                                         &leading, &baseline));
  ASSERT_EQ(41.0f, lh);
  ASSERT_EQ(34.0f, baseline);

  /* OpenType & Variable axis bounds check */
  ASSERT_EQ(CMP_SUCCESS, cmp_font_set_opentype_features(font, 1, 1, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_font_set_variable_axes(font, 700.0f, 100.0f));

  ASSERT_EQ(CMP_SUCCESS, cmp_system_fonts_destroy(ctx));
  PASS();
}

TEST test_null_args(void) {
  cmp_system_fonts_t *ctx = NULL;
  cmp_font_t *font = NULL;
  int bool_out;
  float float_out;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_system_fonts_create(NULL));
  cmp_system_fonts_create(&ctx);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_system_fonts_request(NULL, CMP_SYSTEM_FONT_SF_PRO,
                                     CMP_TEXT_STYLE_BODY, 400, &font));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_system_fonts_request(ctx, CMP_SYSTEM_FONT_SF_PRO,
                                     CMP_TEXT_STYLE_BODY, 400, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_system_fonts_get_tracking_and_optical(
                                       ctx, 16.0f, NULL, &float_out));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_system_fonts_get_tracking_and_optical(
                                       ctx, 16.0f, &bool_out, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_font_set_opentype_features(NULL, 1, 1, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_font_set_variable_axes(NULL, 1.0f, 1.0f));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_system_fonts_get_metrics(ctx, CMP_TEXT_STYLE_BODY, NULL,
                                         &float_out, &float_out));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_system_fonts_get_metrics(ctx, CMP_TEXT_STYLE_BODY, &float_out,
                                         NULL, &float_out));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_system_fonts_get_metrics(ctx, CMP_TEXT_STYLE_BODY, &float_out,
                                         &float_out, NULL));

  cmp_system_fonts_destroy(ctx);
  PASS();
}

SUITE(system_fonts_suite) {
  RUN_TEST(test_system_fonts);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(system_fonts_suite);
  GREATEST_MAIN_END();
}
