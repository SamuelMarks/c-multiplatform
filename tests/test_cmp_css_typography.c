/* clang-format off */
#include "greatest.h"
#include "cmp_css_typography.h"
/* clang-format on */

TEST test_text_align_group_init(void) {
  cmp_prop_text_align_group_t group;
  ASSERT_EQ(0, cmp_prop_text_align_group_init(&group, CMP_TEXT_ALIGN_CENTER,
                                              CMP_TEXT_ALIGN_LEFT));
  ASSERT_EQ(CMP_TEXT_ALIGN_CENTER, group.text_align);
  ASSERT_EQ(CMP_TEXT_ALIGN_LEFT, group.text_align_last);
  PASS();
}

TEST test_text_justify_init(void) {
  cmp_prop_text_justify_t prop;
  ASSERT_EQ(0, cmp_prop_text_justify_init(&prop, CMP_TEXT_JUSTIFY_INTER_WORD));
  ASSERT_EQ(CMP_TEXT_JUSTIFY_INTER_WORD, prop.justify);
  PASS();
}

TEST test_text_indent_init_free(void) {
  cmp_prop_text_indent_t prop;
  ASSERT_EQ(0, cmp_prop_text_indent_init(&prop, "2em"));
  ASSERT_STR_EQ("2em", prop.indent);
  ASSERT_EQ(0, cmp_prop_text_indent_free(&prop));
  ASSERT_EQ(NULL, prop.indent);
  PASS();
}

TEST test_spacing_init_free(void) {
  cmp_prop_spacing_t prop;
  ASSERT_EQ(0, cmp_prop_spacing_init(&prop, "1px", "2px"));
  ASSERT_STR_EQ("1px", prop.letter_spacing);
  ASSERT_STR_EQ("2px", prop.word_spacing);
  ASSERT_EQ(0, cmp_prop_spacing_free(&prop));
  ASSERT_EQ(NULL, prop.letter_spacing);
  ASSERT_EQ(NULL, prop.word_spacing);
  PASS();
}

TEST test_line_height_init_free(void) {
  cmp_prop_line_height_t prop;
  ASSERT_EQ(0, cmp_prop_line_height_init(&prop, "1.5"));
  ASSERT_STR_EQ("1.5", prop.line_height);
  ASSERT_EQ(0, cmp_prop_line_height_free(&prop));
  ASSERT_EQ(NULL, prop.line_height);
  PASS();
}

TEST test_tab_size_init_free(void) {
  cmp_prop_tab_size_t prop;
  ASSERT_EQ(0, cmp_prop_tab_size_init(&prop, "4"));
  ASSERT_STR_EQ("4", prop.tab_size);
  ASSERT_EQ(0, cmp_prop_tab_size_free(&prop));
  ASSERT_EQ(NULL, prop.tab_size);
  PASS();
}

TEST test_white_space_init(void) {
  cmp_prop_white_space_t prop;
  ASSERT_EQ(0, cmp_prop_white_space_init(&prop, CMP_WHITE_SPACE_PRE_WRAP));
  ASSERT_EQ(CMP_WHITE_SPACE_PRE_WRAP, prop.white_space);
  PASS();
}

TEST test_breaking_group_init(void) {
  cmp_prop_breaking_group_t group;
  ASSERT_EQ(0, cmp_prop_breaking_group_init(
                   &group, CMP_WORD_BREAK_BREAK_ALL, CMP_LINE_BREAK_STRICT,
                   CMP_OVERFLOW_WRAP_ANYWHERE, CMP_OVERFLOW_WRAP_BREAK_WORD));
  ASSERT_EQ(CMP_WORD_BREAK_BREAK_ALL, group.word_break);
  ASSERT_EQ(CMP_LINE_BREAK_STRICT, group.line_break);
  ASSERT_EQ(CMP_OVERFLOW_WRAP_ANYWHERE, group.overflow_wrap);
  ASSERT_EQ(CMP_OVERFLOW_WRAP_BREAK_WORD, group.word_wrap);
  PASS();
}

TEST test_hyphens_init(void) {
  cmp_prop_hyphens_t prop;
  ASSERT_EQ(0, cmp_prop_hyphens_init(&prop, CMP_HYPHENS_AUTO));
  ASSERT_EQ(CMP_HYPHENS_AUTO, prop.hyphens);
  PASS();
}

TEST test_line_clamp_init_free(void) {
  cmp_prop_line_clamp_t prop;
  ASSERT_EQ(0, cmp_prop_line_clamp_init(&prop, "3"));
  ASSERT_STR_EQ("3", prop.line_clamp);
  ASSERT_EQ(0, cmp_prop_line_clamp_free(&prop));
  ASSERT_EQ(NULL, prop.line_clamp);
  PASS();
}

TEST test_hanging_punctuation_init(void) {
  cmp_prop_hanging_punctuation_t prop;
  ASSERT_EQ(0, cmp_prop_hanging_punctuation_init(&prop,
                                                 CMP_HANGING_PUNCTUATION_LAST));
  ASSERT_EQ(CMP_HANGING_PUNCTUATION_LAST, prop.hanging_punctuation);
  PASS();
}

TEST test_text_decoration_group_init_free(void) {
  cmp_prop_text_decoration_group_t group;
  cmp_prop_color_t color;
  color.type = CMP_PROP_COLOR_LEGACY;
  color.value.legacy.r = 255;
  color.value.legacy.g = 0;
  color.value.legacy.b = 0;
  color.value.legacy.a = 1.0f;

  ASSERT_EQ(0, cmp_prop_text_decoration_group_init(
                   &group,
                   CMP_TEXT_DECORATION_LINE_UNDERLINE |
                       CMP_TEXT_DECORATION_LINE_OVERLINE,
                   &color, CMP_TEXT_DECORATION_STYLE_WAVY, "2px"));
  ASSERT_EQ(CMP_TEXT_DECORATION_LINE_UNDERLINE |
                CMP_TEXT_DECORATION_LINE_OVERLINE,
            group.text_decoration_line);
  ASSERT_EQ(CMP_PROP_COLOR_LEGACY, group.text_decoration_color.type);
  ASSERT_EQ(255, group.text_decoration_color.value.legacy.r);
  ASSERT_EQ(CMP_TEXT_DECORATION_STYLE_WAVY, group.text_decoration_style);
  ASSERT_STR_EQ("2px", group.text_decoration_thickness);

  ASSERT_EQ(0, cmp_prop_text_decoration_group_free(&group));
  ASSERT_EQ(NULL, group.text_decoration_thickness);
  PASS();
}

TEST test_text_underline_group_init_free(void) {
  cmp_prop_text_underline_group_t group;
  ASSERT_EQ(0, cmp_prop_text_underline_group_init(
                   &group, CMP_TEXT_UNDERLINE_POSITION_UNDER, "1px",
                   CMP_TEXT_DECORATION_SKIP_INK_NONE));
  ASSERT_EQ(CMP_TEXT_UNDERLINE_POSITION_UNDER, group.text_underline_position);
  ASSERT_STR_EQ("1px", group.text_underline_offset);
  ASSERT_EQ(CMP_TEXT_DECORATION_SKIP_INK_NONE, group.text_decoration_skip_ink);

  ASSERT_EQ(0, cmp_prop_text_underline_group_free(&group));
  ASSERT_EQ(NULL, group.text_underline_offset);
  PASS();
}

TEST test_text_shadow_init_free(void) {
  cmp_prop_text_shadow_t prop;
  ASSERT_EQ(0, cmp_prop_text_shadow_init(&prop, "1px 1px 2px black"));
  ASSERT_STR_EQ("1px 1px 2px black", prop.text_shadow);
  ASSERT_EQ(0, cmp_prop_text_shadow_free(&prop));
  ASSERT_EQ(NULL, prop.text_shadow);
  PASS();
}

TEST test_text_transform_init(void) {
  cmp_prop_text_transform_t prop;
  ASSERT_EQ(0,
            cmp_prop_text_transform_init(&prop, CMP_TEXT_TRANSFORM_UPPERCASE));
  ASSERT_EQ(CMP_TEXT_TRANSFORM_UPPERCASE, prop.transform);
  PASS();
}

TEST test_text_emphasis_group_init_free(void) {
  cmp_prop_text_emphasis_group_t group;
  cmp_prop_color_t color;
  color.type = CMP_PROP_COLOR_LEGACY;
  color.value.legacy.r = 0;
  color.value.legacy.g = 255;
  color.value.legacy.b = 0;
  color.value.legacy.a = 1.0f;

  ASSERT_EQ(0, cmp_prop_text_emphasis_group_init(
                   &group, &color, CMP_TEXT_EMPHASIS_STYLE_STRING, "*",
                   CMP_TEXT_EMPHASIS_POSITION_UNDER_LEFT));
  ASSERT_EQ(CMP_TEXT_EMPHASIS_STYLE_STRING, group.text_emphasis_style);
  ASSERT_STR_EQ("*", group.text_emphasis_string);
  ASSERT_EQ(CMP_TEXT_EMPHASIS_POSITION_UNDER_LEFT,
            group.text_emphasis_position);

  ASSERT_EQ(0, cmp_prop_text_emphasis_group_free(&group));
  ASSERT_EQ(NULL, group.text_emphasis_string);
  PASS();
}

SUITE(cmp_css_typography_suite) {
  RUN_TEST(test_text_align_group_init);
  RUN_TEST(test_text_justify_init);
  RUN_TEST(test_text_indent_init_free);
  RUN_TEST(test_spacing_init_free);
  RUN_TEST(test_line_height_init_free);
  RUN_TEST(test_tab_size_init_free);
  RUN_TEST(test_white_space_init);
  RUN_TEST(test_breaking_group_init);
  RUN_TEST(test_hyphens_init);
  RUN_TEST(test_line_clamp_init_free);
  RUN_TEST(test_hanging_punctuation_init);
  RUN_TEST(test_text_decoration_group_init_free);
  RUN_TEST(test_text_underline_group_init_free);
  RUN_TEST(test_text_shadow_init_free);
  RUN_TEST(test_text_transform_init);
  RUN_TEST(test_text_emphasis_group_init_free);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_css_typography_suite);
  GREATEST_MAIN_END();
}
