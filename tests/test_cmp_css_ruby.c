/* clang-format off */
#include "greatest.h"
#include "cmp_css_ruby.h"
/* clang-format on */

TEST test_layout_ruby_init(void) {
  cmp_layout_ruby_t layout;
  ASSERT_EQ(0, cmp_layout_ruby_init(&layout, CMP_LAYOUT_RUBY_RT));
  ASSERT_EQ(CMP_LAYOUT_RUBY_RT, layout.tag_type);
  PASS();
}

TEST test_ruby_align_group_init(void) {
  cmp_prop_ruby_align_group_t group;
  ASSERT_EQ(0, cmp_prop_ruby_align_group_init(&group, CMP_RUBY_POSITION_UNDER,
                                              CMP_RUBY_ALIGN_SPACE_AROUND));
  ASSERT_EQ(CMP_RUBY_POSITION_UNDER, group.position);
  ASSERT_EQ(CMP_RUBY_ALIGN_SPACE_AROUND, group.align);
  PASS();
}

TEST test_ruby_overhang_init(void) {
  cmp_prop_ruby_overhang_t prop;
  ASSERT_EQ(0, cmp_prop_ruby_overhang_init(&prop, CMP_RUBY_OVERHANG_NONE));
  ASSERT_EQ(CMP_RUBY_OVERHANG_NONE, prop.overhang);
  PASS();
}

SUITE(cmp_css_ruby_suite) {
  RUN_TEST(test_layout_ruby_init);
  RUN_TEST(test_ruby_align_group_init);
  RUN_TEST(test_ruby_overhang_init);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_css_ruby_suite);
  GREATEST_MAIN_END();
}
