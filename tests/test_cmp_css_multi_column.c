/* clang-format off */
#include "cmp_css_multi_column.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_column_def_group(void) {
  cmp_prop_column_def_group_t group;

  ASSERT_EQ(-1, cmp_prop_column_def_group_init(NULL, "auto", "3"));

  ASSERT_EQ(0, cmp_prop_column_def_group_init(&group, "120px", "3"));
  ASSERT_STR_EQ("120px", group.width);
  ASSERT_STR_EQ("3", group.count);

  ASSERT_EQ(0, cmp_prop_column_def_group_free(&group));
  ASSERT_EQ(NULL, group.width);
  ASSERT_EQ(NULL, group.count);

  /* Test partial init */
  ASSERT_EQ(0, cmp_prop_column_def_group_init(&group, "auto", NULL));
  ASSERT_STR_EQ("auto", group.width);
  ASSERT_EQ(NULL, group.count);
  ASSERT_EQ(0, cmp_prop_column_def_group_free(&group));

  ASSERT_EQ(-1, cmp_prop_column_def_group_free(NULL));

  PASS();
}

TEST test_column_rule_group(void) {
  cmp_prop_column_rule_group_t group;

  ASSERT_EQ(-1, cmp_prop_column_rule_group_init(NULL, "1px", "solid", "red"));

  ASSERT_EQ(0,
            cmp_prop_column_rule_group_init(&group, "thin", "dashed", "blue"));
  ASSERT_STR_EQ("thin", group.width);
  ASSERT_STR_EQ("dashed", group.style);
  ASSERT_STR_EQ("blue", group.color);

  ASSERT_EQ(0, cmp_prop_column_rule_group_free(&group));
  ASSERT_EQ(NULL, group.width);
  ASSERT_EQ(NULL, group.style);
  ASSERT_EQ(NULL, group.color);

  /* Test partial init */
  ASSERT_EQ(0, cmp_prop_column_rule_group_init(&group, NULL, "dotted", NULL));
  ASSERT_EQ(NULL, group.width);
  ASSERT_STR_EQ("dotted", group.style);
  ASSERT_EQ(NULL, group.color);
  ASSERT_EQ(0, cmp_prop_column_rule_group_free(&group));

  ASSERT_EQ(-1, cmp_prop_column_rule_group_free(NULL));

  PASS();
}

TEST test_column_span_fill(void) {
  cmp_prop_column_span_fill_t group;

  ASSERT_EQ(-1, cmp_prop_column_span_fill_init(NULL, CMP_COLUMN_SPAN_ALL,
                                               CMP_COLUMN_FILL_AUTO));

  ASSERT_EQ(0, cmp_prop_column_span_fill_init(&group, CMP_COLUMN_SPAN_ALL,
                                              CMP_COLUMN_FILL_BALANCE));
  ASSERT_EQ(CMP_COLUMN_SPAN_ALL, group.span);
  ASSERT_EQ(CMP_COLUMN_FILL_BALANCE, group.fill);

  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_column_def_group);
  RUN_TEST(test_column_rule_group);
  RUN_TEST(test_column_span_fill);
  GREATEST_MAIN_END();
}