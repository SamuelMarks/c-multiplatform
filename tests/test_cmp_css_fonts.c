/* clang-format off */
#include "greatest.h"
#include "cmp_css_fonts.h"
/* clang-format on */

TEST test_font_basic_group_init_free(void) {
  cmp_prop_font_basic_group_t group;
  const char *family[] = {"Arial", "sans-serif", NULL};

  ASSERT_EQ(0, cmp_prop_font_basic_group_init(
                   &group, family, 400, CMP_FONT_STYLE_NORMAL,
                   CMP_FONT_STRETCH_NORMAL, 0.0f, "12px"));
  ASSERT_STR_EQ("Arial", group.font_family[0]);
  ASSERT_STR_EQ("sans-serif", group.font_family[1]);
  ASSERT_EQ(NULL, group.font_family[2]);
  ASSERT_EQ(400, group.font_weight);
  ASSERT_EQ(CMP_FONT_STYLE_NORMAL, group.font_style);
  ASSERT_EQ(CMP_FONT_STRETCH_NORMAL, group.font_stretch);
  ASSERT_STR_EQ("12px", group.font_size);

  ASSERT_EQ(0, cmp_prop_font_basic_group_free(&group));
  ASSERT_EQ(NULL, group.font_family);
  ASSERT_EQ(NULL, group.font_size);
  PASS();
}

TEST test_font_size_adjust_init_free(void) {
  cmp_prop_font_size_adjust_t prop;
  ASSERT_EQ(0, cmp_prop_font_size_adjust_init(&prop, "0.5"));
  ASSERT_STR_EQ("0.5", prop.size_adjust);
  ASSERT_EQ(0, cmp_prop_font_size_adjust_free(&prop));
  ASSERT_EQ(NULL, prop.size_adjust);
  PASS();
}

TEST test_font_variant_group_init(void) {
  cmp_prop_font_variant_group_t group;
  ASSERT_EQ(0, cmp_prop_font_variant_group_init(
                   &group, CMP_FONT_VARIANT_CAPS_SMALL_CAPS,
                   CMP_FONT_VARIANT_NUMERIC_LINING_NUMS,
                   CMP_FONT_VARIANT_EAST_ASIAN_JIS78,
                   CMP_FONT_VARIANT_LIGATURES_COMMON));
  ASSERT_EQ(CMP_FONT_VARIANT_CAPS_SMALL_CAPS, group.caps);
  ASSERT_EQ(CMP_FONT_VARIANT_NUMERIC_LINING_NUMS, group.numeric);
  ASSERT_EQ(CMP_FONT_VARIANT_EAST_ASIAN_JIS78, group.east_asian);
  ASSERT_EQ(CMP_FONT_VARIANT_LIGATURES_COMMON, group.ligatures);
  PASS();
}

TEST test_font_variant_advanced_init_free(void) {
  cmp_prop_font_variant_advanced_t prop;
  ASSERT_EQ(0, cmp_prop_font_variant_advanced_init(
                   &prop, "historical-forms", CMP_FONT_VARIANT_POSITION_SUB));
  ASSERT_STR_EQ("historical-forms", prop.alternates);
  ASSERT_EQ(CMP_FONT_VARIANT_POSITION_SUB, prop.position);
  ASSERT_EQ(0, cmp_prop_font_variant_advanced_free(&prop));
  ASSERT_EQ(NULL, prop.alternates);
  PASS();
}

TEST test_font_settings_group_init_free(void) {
  cmp_prop_font_settings_group_t group;
  ASSERT_EQ(0, cmp_prop_font_settings_group_init(&group, "\"smcp\" on",
                                                 "\"wght\" 600"));
  ASSERT_STR_EQ("\"smcp\" on", group.feature_settings);
  ASSERT_STR_EQ("\"wght\" 600", group.variation_settings);
  ASSERT_EQ(0, cmp_prop_font_settings_group_free(&group));
  ASSERT_EQ(NULL, group.feature_settings);
  ASSERT_EQ(NULL, group.variation_settings);
  PASS();
}

TEST test_font_synthesis_group_init(void) {
  cmp_prop_font_synthesis_group_t group;
  ASSERT_EQ(0,
            cmp_prop_font_synthesis_group_init(
                &group, CMP_FONT_SYNTHESIS_WEIGHT | CMP_FONT_SYNTHESIS_STYLE));
  ASSERT_EQ(CMP_FONT_SYNTHESIS_WEIGHT | CMP_FONT_SYNTHESIS_STYLE,
            group.synthesis);
  PASS();
}

TEST test_font_kerning_init(void) {
  cmp_prop_font_kerning_t prop;
  ASSERT_EQ(0, cmp_prop_font_kerning_init(&prop, CMP_FONT_KERNING_NORMAL));
  ASSERT_EQ(CMP_FONT_KERNING_NORMAL, prop.kerning);
  PASS();
}

TEST test_font_language_override_init_free(void) {
  cmp_prop_font_language_override_t prop;
  ASSERT_EQ(0, cmp_prop_font_language_override_init(&prop, "ENG"));
  ASSERT_STR_EQ("ENG", prop.language_override);
  ASSERT_EQ(0, cmp_prop_font_language_override_free(&prop));
  ASSERT_EQ(NULL, prop.language_override);
  PASS();
}

TEST test_font_palette_init_free(void) {
  cmp_prop_font_palette_t prop;
  ASSERT_EQ(0, cmp_prop_font_palette_init(&prop, "--custom-palette"));
  ASSERT_STR_EQ("--custom-palette", prop.palette);
  ASSERT_EQ(0, cmp_prop_font_palette_free(&prop));
  ASSERT_EQ(NULL, prop.palette);
  PASS();
}

TEST test_at_rule_font_face_init_free(void) {
  cmp_at_rule_font_face_t rule;
  const char *src[] = {"url('font.woff2') format('woff2')", NULL};
  ASSERT_EQ(0, cmp_at_rule_font_face_init(&rule, "MyFont", src, "bold",
                                          "italic", "swap", "U+0000-00FF"));
  ASSERT_STR_EQ("MyFont", rule.font_family);
  ASSERT_STR_EQ("url('font.woff2') format('woff2')", rule.src[0]);
  ASSERT_EQ(NULL, rule.src[1]);
  ASSERT_STR_EQ("bold", rule.font_weight);
  ASSERT_STR_EQ("italic", rule.font_style);
  ASSERT_STR_EQ("swap", rule.font_display);
  ASSERT_STR_EQ("U+0000-00FF", rule.unicode_range);

  ASSERT_EQ(0, cmp_at_rule_font_face_free(&rule));
  ASSERT_EQ(NULL, rule.font_family);
  ASSERT_EQ(NULL, rule.src);
  ASSERT_EQ(NULL, rule.font_weight);
  ASSERT_EQ(NULL, rule.font_style);
  ASSERT_EQ(NULL, rule.font_display);
  ASSERT_EQ(NULL, rule.unicode_range);
  PASS();
}

TEST test_at_rule_font_advanced_init_free(void) {
  cmp_at_rule_font_advanced_t rule;
  ASSERT_EQ(0, cmp_at_rule_font_advanced_init(&rule, "MyFont",
                                              "@swash { swishy: 1; }"));
  ASSERT_STR_EQ("MyFont", rule.font_family);
  ASSERT_STR_EQ("@swash { swishy: 1; }", rule.values_blob);

  ASSERT_EQ(0, cmp_at_rule_font_advanced_free(&rule));
  ASSERT_EQ(NULL, rule.font_family);
  ASSERT_EQ(NULL, rule.values_blob);
  PASS();
}

TEST test_initial_letter_init_free(void) {
  cmp_prop_initial_letter_t prop;
  ASSERT_EQ(0, cmp_prop_initial_letter_init(&prop, "3 2"));
  ASSERT_STR_EQ("3 2", prop.initial_letter);
  ASSERT_EQ(0, cmp_prop_initial_letter_free(&prop));
  ASSERT_EQ(NULL, prop.initial_letter);
  PASS();
}

TEST test_initial_letter_align_init(void) {
  cmp_prop_initial_letter_align_t prop;
  ASSERT_EQ(0, cmp_prop_initial_letter_align_init(
                   &prop, CMP_INITIAL_LETTER_ALIGN_HANGING));
  ASSERT_EQ(CMP_INITIAL_LETTER_ALIGN_HANGING, prop.align);
  PASS();
}

TEST test_font_smoothing_init(void) {
  cmp_prop_font_smoothing_t prop;
  ASSERT_EQ(
      0, cmp_prop_font_smoothing_init(&prop, CMP_FONT_SMOOTHING_ANTIALIASED));
  ASSERT_EQ(CMP_FONT_SMOOTHING_ANTIALIASED, prop.smoothing);
  PASS();
}

SUITE(cmp_css_fonts_suite) {
  RUN_TEST(test_font_basic_group_init_free);
  RUN_TEST(test_font_size_adjust_init_free);
  RUN_TEST(test_font_variant_group_init);
  RUN_TEST(test_font_variant_advanced_init_free);
  RUN_TEST(test_font_settings_group_init_free);
  RUN_TEST(test_font_synthesis_group_init);
  RUN_TEST(test_font_kerning_init);
  RUN_TEST(test_font_language_override_init_free);
  RUN_TEST(test_font_palette_init_free);
  RUN_TEST(test_at_rule_font_face_init_free);
  RUN_TEST(test_at_rule_font_advanced_init_free);
  RUN_TEST(test_initial_letter_init_free);
  RUN_TEST(test_initial_letter_align_init);
  RUN_TEST(test_font_smoothing_init);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_css_fonts_suite);
  GREATEST_MAIN_END();
}
