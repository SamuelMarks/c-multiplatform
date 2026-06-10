/* clang-format off */
#include "greatest.h"
#include "cmp_css_writing_modes.h"
/* clang-format on */

TEST test_bidi_group_init(void) {
  cmp_prop_bidi_group_t group;
  ASSERT_EQ(0, cmp_prop_bidi_group_init(&group, CMP_DIRECTION_RTL,
                                        CMP_UNICODE_BIDI_ISOLATE));
  ASSERT_EQ(CMP_DIRECTION_RTL, group.direction);
  ASSERT_EQ(CMP_UNICODE_BIDI_ISOLATE, group.unicode_bidi);
  PASS();
}

TEST test_writing_mode_group_init(void) {
  cmp_prop_writing_mode_group_t group;
  ASSERT_EQ(
      0, cmp_prop_writing_mode_group_init(&group, CMP_WRITING_MODE_VERTICAL_RL,
                                          CMP_TEXT_ORIENTATION_UPRIGHT,
                                          CMP_TEXT_COMBINE_UPRIGHT_DIGITS, 2));
  ASSERT_EQ(CMP_WRITING_MODE_VERTICAL_RL, group.writing_mode);
  ASSERT_EQ(CMP_TEXT_ORIENTATION_UPRIGHT, group.text_orientation);
  ASSERT_EQ(CMP_TEXT_COMBINE_UPRIGHT_DIGITS, group.text_combine_upright_type);
  ASSERT_EQ(2, group.text_combine_upright_digits);
  PASS();
}

SUITE(cmp_css_writing_modes_suite) {
  RUN_TEST(test_bidi_group_init);
  RUN_TEST(test_writing_mode_group_init);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_css_writing_modes_suite);
  GREATEST_MAIN_END();
}
