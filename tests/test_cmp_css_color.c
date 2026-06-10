/* clang-format off */
#include "cmp_css_color.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_color_currentcolor(void) {
  cmp_prop_color_t color;

  ASSERT_EQ(-1, cmp_prop_color_init_currentcolor(NULL));
  ASSERT_EQ(0, cmp_prop_color_init_currentcolor(&color));
  ASSERT_EQ(CMP_PROP_COLOR_CURRENTCOLOR, color.type);

  ASSERT_EQ(0, cmp_prop_color_free(&color));
  PASS();
}

TEST test_color_transparent(void) {
  cmp_prop_color_t color;

  ASSERT_EQ(-1, cmp_prop_color_init_transparent(NULL));
  ASSERT_EQ(0, cmp_prop_color_init_transparent(&color));
  ASSERT_EQ(CMP_PROP_COLOR_TRANSPARENT, color.type);

  ASSERT_EQ(0, cmp_prop_color_free(&color));
  PASS();
}

TEST test_color_rgba(void) {
  cmp_prop_color_t color;

  ASSERT_EQ(-1, cmp_prop_color_init_rgba(NULL, 255.0f, 0.0f, 0.0f, 1.0f));
  ASSERT_EQ(0, cmp_prop_color_init_rgba(&color, 255.0f, 0.0f, 0.0f, 1.0f));
  ASSERT_EQ(CMP_PROP_COLOR_LEGACY, color.type);
  ASSERT_EQ(255.0f, color.value.legacy.r);
  ASSERT_EQ(0.0f, color.value.legacy.g);
  ASSERT_EQ(0.0f, color.value.legacy.b);
  ASSERT_EQ(1.0f, color.value.legacy.a);
  ASSERT_EQ(0, color.value.legacy.is_hsl);

  ASSERT_EQ(0, cmp_prop_color_free(&color));
  PASS();
}

TEST test_color_hsla(void) {
  cmp_prop_color_t color;

  ASSERT_EQ(-1, cmp_prop_color_init_hsla(NULL, 120.0f, 1.0f, 0.5f, 1.0f));
  ASSERT_EQ(0, cmp_prop_color_init_hsla(&color, 120.0f, 1.0f, 0.5f, 1.0f));
  ASSERT_EQ(CMP_PROP_COLOR_LEGACY, color.type);
  ASSERT_EQ(120.0f, color.value.legacy.r);
  ASSERT_EQ(1.0f, color.value.legacy.g);
  ASSERT_EQ(0.5f, color.value.legacy.b);
  ASSERT_EQ(1.0f, color.value.legacy.a);
  ASSERT_EQ(1, color.value.legacy.is_hsl);

  ASSERT_EQ(0, cmp_prop_color_free(&color));
  PASS();
}

TEST test_color_modern(void) {
  cmp_prop_color_t color;

  ASSERT_EQ(-1, cmp_prop_color_init_modern(NULL, CMP_COLOR_MODERN_OKLCH, 0.6f,
                                           0.1f, 180.0f, 1.0f));
  ASSERT_EQ(0, cmp_prop_color_init_modern(&color, CMP_COLOR_MODERN_OKLCH, 0.6f,
                                          0.1f, 180.0f, 1.0f));
  ASSERT_EQ(CMP_PROP_COLOR_MODERN, color.type);
  ASSERT_EQ(CMP_COLOR_MODERN_OKLCH, color.value.modern.type);
  ASSERT_EQ(0.6f, color.value.modern.c1);
  ASSERT_EQ(0.1f, color.value.modern.c2);
  ASSERT_EQ(180.0f, color.value.modern.c3);
  ASSERT_EQ(1.0f, color.value.modern.alpha);

  ASSERT_EQ(0, cmp_prop_color_free(&color));
  PASS();
}

TEST test_color_space(void) {
  cmp_prop_color_t color;

  ASSERT_EQ(-1, cmp_prop_color_init_space(NULL, CMP_COLOR_SPACE_DISPLAY_P3,
                                          1.0f, 0.5f, 0.0f, 1.0f));
  ASSERT_EQ(0, cmp_prop_color_init_space(&color, CMP_COLOR_SPACE_DISPLAY_P3,
                                         1.0f, 0.5f, 0.0f, 1.0f));
  ASSERT_EQ(CMP_PROP_COLOR_SPACE, color.type);
  ASSERT_EQ(CMP_COLOR_SPACE_DISPLAY_P3, color.value.space.space);
  ASSERT_EQ(1.0f, color.value.space.p1);
  ASSERT_EQ(0.5f, color.value.space.p2);
  ASSERT_EQ(0.0f, color.value.space.p3);
  ASSERT_EQ(1.0f, color.value.space.alpha);

  ASSERT_EQ(0, cmp_prop_color_free(&color));
  PASS();
}

TEST test_color_system(void) {
  cmp_prop_color_t color;

  ASSERT_EQ(-1, cmp_prop_color_init_system(NULL, "WindowText"));
  ASSERT_EQ(-1, cmp_prop_color_init_system(&color, NULL));
  ASSERT_EQ(0, cmp_prop_color_init_system(&color, "WindowText"));
  ASSERT_EQ(CMP_PROP_COLOR_SYSTEM, color.type);
  ASSERT_STR_EQ("WindowText", color.value.system_color);

  ASSERT_EQ(0, cmp_prop_color_free(&color));
  ASSERT_EQ(NULL, color.value.system_color);
  PASS();
}

TEST test_color_complex(void) {
  cmp_prop_color_t color;

  ASSERT_EQ(-1, cmp_prop_color_init_complex(NULL, CMP_PROP_COLOR_MIX,
                                            "color-mix(...)"));
  ASSERT_EQ(-1, cmp_prop_color_init_complex(&color, CMP_PROP_COLOR_MIX, NULL));
  ASSERT_EQ(-1, cmp_prop_color_init_complex(
                    &color, CMP_PROP_COLOR_LEGACY,
                    "color-mix(...)")); /* Invalid type for complex */

  ASSERT_EQ(0, cmp_prop_color_init_complex(&color, CMP_PROP_COLOR_MIX,
                                           "color-mix(in srgb, red, blue)"));
  ASSERT_EQ(CMP_PROP_COLOR_MIX, color.type);
  ASSERT_STR_EQ("color-mix(in srgb, red, blue)", color.value.complex_func);

  ASSERT_EQ(0, cmp_prop_color_free(&color));
  ASSERT_EQ(NULL, color.value.complex_func);
  PASS();
}

TEST test_color_cmyk(void) {
  cmp_prop_color_t color;

  ASSERT_EQ(-1,
            cmp_prop_color_init_cmyk(NULL, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f, NULL));
  ASSERT_EQ(
      0, cmp_prop_color_init_cmyk(&color, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f, "red"));
  ASSERT_EQ(CMP_PROP_COLOR_CMYK, color.type);
  ASSERT_EQ(0.0f, color.value.cmyk.c);
  ASSERT_EQ(0.5f, color.value.cmyk.m);
  ASSERT_EQ(0.5f, color.value.cmyk.y);
  ASSERT_EQ(0.0f, color.value.cmyk.k);
  ASSERT_EQ(1.0f, color.value.cmyk.alpha);
  ASSERT_STR_EQ("red", color.value.cmyk.fallback_color);

  ASSERT_EQ(0, cmp_prop_color_free(&color));
  ASSERT_EQ(NULL, color.value.cmyk.fallback_color);

  /* Test without fallback */
  ASSERT_EQ(
      0, cmp_prop_color_init_cmyk(&color, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f, NULL));
  ASSERT_EQ(NULL, color.value.cmyk.fallback_color);
  ASSERT_EQ(0, cmp_prop_color_free(&color));

  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_color_currentcolor);
  RUN_TEST(test_color_transparent);
  RUN_TEST(test_color_rgba);
  RUN_TEST(test_color_system);
  RUN_TEST(test_color_complex);
  RUN_TEST(test_color_cmyk);
  GREATEST_MAIN_END();
}