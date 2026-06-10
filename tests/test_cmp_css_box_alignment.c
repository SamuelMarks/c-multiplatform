/* clang-format off */
#include "cmp_css_box_alignment.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_align_content_group(void) {
  cmp_prop_align_content_group_t group;

  ASSERT_EQ(-1, cmp_prop_align_content_group_init(NULL, CMP_ALIGN_CENTER,
                                                  CMP_ALIGN_CENTER));

  ASSERT_EQ(0, cmp_prop_align_content_group_init(
                   &group, CMP_ALIGN_SPACE_BETWEEN, CMP_ALIGN_STRETCH));
  ASSERT_EQ(CMP_ALIGN_SPACE_BETWEEN, group.justify_content);
  ASSERT_EQ(CMP_ALIGN_STRETCH, group.align_content);

  PASS();
}

TEST test_align_items_group(void) {
  cmp_prop_align_items_group_t group;

  ASSERT_EQ(-1, cmp_prop_align_items_group_init(NULL, CMP_ALIGN_CENTER,
                                                CMP_ALIGN_CENTER));

  ASSERT_EQ(0, cmp_prop_align_items_group_init(&group, CMP_ALIGN_FLEX_START,
                                               CMP_ALIGN_BASELINE));
  ASSERT_EQ(CMP_ALIGN_FLEX_START, group.justify_items);
  ASSERT_EQ(CMP_ALIGN_BASELINE, group.align_items);

  PASS();
}

TEST test_align_self_group(void) {
  cmp_prop_align_self_group_t group;

  ASSERT_EQ(-1, cmp_prop_align_self_group_init(NULL, CMP_ALIGN_CENTER,
                                               CMP_ALIGN_CENTER));

  ASSERT_EQ(0, cmp_prop_align_self_group_init(&group, CMP_ALIGN_SAFE_CENTER,
                                              CMP_ALIGN_UNSAFE_CENTER));
  ASSERT_EQ(CMP_ALIGN_SAFE_CENTER, group.justify_self);
  ASSERT_EQ(CMP_ALIGN_UNSAFE_CENTER, group.align_self);

  PASS();
}

TEST test_gap(void) {
  cmp_prop_gap_t gap;

  ASSERT_EQ(-1, cmp_prop_gap_init(NULL, "10px", "10px"));

  ASSERT_EQ(0, cmp_prop_gap_init(&gap, "10px", "1em"));
  ASSERT_STR_EQ("10px", gap.row_gap);
  ASSERT_STR_EQ("1em", gap.column_gap);

  ASSERT_EQ(0, cmp_prop_gap_free(&gap));
  ASSERT_EQ(NULL, gap.row_gap);
  ASSERT_EQ(NULL, gap.column_gap);

  /* Test partial init */
  ASSERT_EQ(0, cmp_prop_gap_init(&gap, "20px", NULL));
  ASSERT_STR_EQ("20px", gap.row_gap);
  ASSERT_EQ(NULL, gap.column_gap);
  ASSERT_EQ(0, cmp_prop_gap_free(&gap));

  ASSERT_EQ(-1, cmp_prop_gap_free(NULL));

  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_align_content_group);
  RUN_TEST(test_align_items_group);
  RUN_TEST(test_align_self_group);
  RUN_TEST(test_gap);
  GREATEST_MAIN_END();
}