/* clang-format off */
#include "greatest.h"
#include "cmp_css_logical.h"
/* clang-format on */

TEST test_logical_size_group_init_free(void) {
  cmp_prop_logical_size_group_t group;
  ASSERT_EQ(0, cmp_prop_logical_size_group_init(&group, "100px", "50%",
                                                "min-content", "max-content",
                                                "200px", "100vw"));
  ASSERT_STR_EQ("100px", group.block_size);
  ASSERT_STR_EQ("50%", group.inline_size);
  ASSERT_STR_EQ("min-content", group.min_block_size);
  ASSERT_STR_EQ("max-content", group.min_inline_size);
  ASSERT_STR_EQ("200px", group.max_block_size);
  ASSERT_STR_EQ("100vw", group.max_inline_size);

  ASSERT_EQ(0, cmp_prop_logical_size_group_free(&group));
  ASSERT_EQ(NULL, group.block_size);
  PASS();
}

TEST test_logical_margin_group_init_free(void) {
  cmp_prop_logical_margin_group_t group;
  ASSERT_EQ(0, cmp_prop_logical_margin_group_init(&group, "10px", "20px",
                                                  "auto", "5px"));
  ASSERT_STR_EQ("10px", group.margin_block_start);
  ASSERT_STR_EQ("20px", group.margin_block_end);
  ASSERT_STR_EQ("auto", group.margin_inline_start);
  ASSERT_STR_EQ("5px", group.margin_inline_end);

  ASSERT_EQ(0, cmp_prop_logical_margin_group_free(&group));
  ASSERT_EQ(NULL, group.margin_block_start);
  PASS();
}

TEST test_logical_padding_group_init_free(void) {
  cmp_prop_logical_padding_group_t group;
  ASSERT_EQ(0, cmp_prop_logical_padding_group_init(&group, "1px", "2px", "3px",
                                                   "4px"));
  ASSERT_STR_EQ("1px", group.padding_block_start);
  ASSERT_STR_EQ("2px", group.padding_block_end);
  ASSERT_STR_EQ("3px", group.padding_inline_start);
  ASSERT_STR_EQ("4px", group.padding_inline_end);

  ASSERT_EQ(0, cmp_prop_logical_padding_group_free(&group));
  ASSERT_EQ(NULL, group.padding_block_start);
  PASS();
}

TEST test_logical_border_group_init_free(void) {
  cmp_prop_logical_border_group_t group;
  cmp_prop_color_t color;
  color.type = CMP_PROP_COLOR_LEGACY;
  color.value.legacy.r = 255;
  color.value.legacy.g = 0;
  color.value.legacy.b = 0;
  color.value.legacy.a = 1.0f;

  ASSERT_EQ(0, cmp_prop_logical_border_group_init(&group));

  ASSERT_EQ(0, cmp_logical_border_part_init(&group.block_start, "1px",
                                            CMP_BORDER_STYLE_SOLID, &color));
  ASSERT_EQ(0, cmp_logical_border_part_init(&group.block_end, "2px",
                                            CMP_BORDER_STYLE_DASHED, NULL));
  ASSERT_EQ(0, cmp_logical_border_part_init(&group.inline_start, "3px",
                                            CMP_BORDER_STYLE_DOTTED, &color));
  ASSERT_EQ(0, cmp_logical_border_part_init(&group.inline_end, "4px",
                                            CMP_BORDER_STYLE_DOUBLE, NULL));

  ASSERT_STR_EQ("1px", group.block_start.width);
  ASSERT_EQ(CMP_BORDER_STYLE_SOLID, group.block_start.style);
  ASSERT_EQ(CMP_PROP_COLOR_LEGACY, group.block_start.color.type);

  ASSERT_STR_EQ("2px", group.block_end.width);
  ASSERT_EQ(CMP_BORDER_STYLE_DASHED, group.block_end.style);
  ASSERT_EQ(CMP_PROP_COLOR_CURRENTCOLOR, group.block_end.color.type);

  ASSERT_EQ(0, cmp_prop_logical_border_group_free(&group));
  ASSERT_EQ(NULL, group.block_start.width);
  ASSERT_EQ(NULL, group.block_end.width);
  PASS();
}

TEST test_logical_inset_group_init_free(void) {
  cmp_prop_logical_inset_group_t group;
  ASSERT_EQ(0, cmp_prop_logical_inset_group_init(&group, "10px", "20px", "auto",
                                                 "5px"));
  ASSERT_STR_EQ("10px", group.inset_block_start);
  ASSERT_STR_EQ("20px", group.inset_block_end);
  ASSERT_STR_EQ("auto", group.inset_inline_start);
  ASSERT_STR_EQ("5px", group.inset_inline_end);

  ASSERT_EQ(0, cmp_prop_logical_inset_group_free(&group));
  ASSERT_EQ(NULL, group.inset_block_start);
  PASS();
}

TEST test_logical_radius_group_init_free(void) {
  cmp_prop_logical_radius_group_t group;
  ASSERT_EQ(0, cmp_prop_logical_radius_group_init(&group, "5px", "10px", "15px",
                                                  "20px"));
  ASSERT_STR_EQ("5px", group.start_start);
  ASSERT_STR_EQ("10px", group.start_end);
  ASSERT_STR_EQ("15px", group.end_start);
  ASSERT_STR_EQ("20px", group.end_end);

  ASSERT_EQ(0, cmp_prop_logical_radius_group_free(&group));
  ASSERT_EQ(NULL, group.start_start);
  PASS();
}

SUITE(cmp_css_logical_suite) {
  RUN_TEST(test_logical_size_group_init_free);
  RUN_TEST(test_logical_margin_group_init_free);
  RUN_TEST(test_logical_padding_group_init_free);
  RUN_TEST(test_logical_border_group_init_free);
  RUN_TEST(test_logical_inset_group_init_free);
  RUN_TEST(test_logical_radius_group_init_free);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_css_logical_suite);
  GREATEST_MAIN_END();
}
