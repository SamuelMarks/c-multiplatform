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

TEST test_cmp_font_fallback_chain(void) {
  unsigned char dummy_ttf[256] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00};
  cmp_font_t *font_main = NULL;
  cmp_font_t *font_fallback_1 = NULL;
  cmp_font_t *font_fallback_2 = NULL;

  int rc =
      cmp_font_load_memory(dummy_ttf, sizeof(dummy_ttf), 16.0f, &font_main);
  if (rc == CMP_SUCCESS) {
    if (cmp_font_load_memory(dummy_ttf, sizeof(dummy_ttf), 16.0f,
                             &font_fallback_1) == CMP_SUCCESS) {
      if (cmp_font_load_memory(dummy_ttf, sizeof(dummy_ttf), 16.0f,
                               &font_fallback_2) == CMP_SUCCESS) {
        ASSERT_EQ(CMP_SUCCESS,
                  cmp_font_add_fallback(font_main, font_fallback_1));
        ASSERT_EQ(CMP_SUCCESS,
                  cmp_font_add_fallback(font_main, font_fallback_2));
        ASSERT_EQ(2, (int)font_main->fallback_count);
        ASSERT(font_main->fallback_capacity >= 2);
        ASSERT_EQ(font_fallback_1, font_main->fallbacks[0]);
        ASSERT_EQ(font_fallback_2, font_main->fallbacks[1]);

        cmp_font_destroy(font_fallback_2);
      }
      cmp_font_destroy(font_fallback_1);
    }
    cmp_font_destroy(font_main);
  }

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
  RUN_TEST(test_cmp_font_fallback_chain);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(system_fonts_suite);
  GREATEST_MAIN_END();
}
