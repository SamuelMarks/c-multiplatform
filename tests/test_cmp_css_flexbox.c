/* clang-format off */
#include "cmp_css_flexbox.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_flex_container_group(void) {
  cmp_prop_flex_container_group_t group;

  ASSERT_EQ(-1, cmp_prop_flex_container_group_init(NULL, CMP_FLEX_DIRECTION_ROW,
                                                   CMP_FLEX_WRAP_NOWRAP));

  ASSERT_EQ(0, cmp_prop_flex_container_group_init(
                   &group, CMP_FLEX_DIRECTION_COLUMN_REVERSE,
                   CMP_FLEX_WRAP_WRAP_REVERSE));
  ASSERT_EQ(CMP_FLEX_DIRECTION_COLUMN_REVERSE, group.direction);
  ASSERT_EQ(CMP_FLEX_WRAP_WRAP_REVERSE, group.wrap);

  PASS();
}

TEST test_flex_item_group(void) {
  cmp_prop_flex_item_group_t group;

  ASSERT_EQ(-1, cmp_prop_flex_item_group_init(NULL, 1.0f, 1.0f, "auto"));

  ASSERT_EQ(0, cmp_prop_flex_item_group_init(&group, 2.0f, 0.5f, "200px"));
  ASSERT_EQ(2.0f, group.grow);
  ASSERT_EQ(0.5f, group.shrink);
  ASSERT_STR_EQ("200px", group.basis);

  ASSERT_EQ(0, cmp_prop_flex_item_group_free(&group));
  ASSERT_EQ(NULL, group.basis);

  ASSERT_EQ(0, cmp_prop_flex_item_group_init(&group, 1.0f, 1.0f, NULL));
  ASSERT_EQ(NULL, group.basis);
  ASSERT_EQ(0, cmp_prop_flex_item_group_free(&group));

  ASSERT_EQ(-1, cmp_prop_flex_item_group_free(NULL));

  PASS();
}

TEST test_order(void) {
  cmp_prop_order_t order;

  ASSERT_EQ(-1, cmp_prop_order_init(NULL, 5));

  ASSERT_EQ(0, cmp_prop_order_init(&order, -10));
  ASSERT_EQ(-10, order.order);

  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_flex_container_group);
  RUN_TEST(test_flex_item_group);
  RUN_TEST(test_order);
  GREATEST_MAIN_END();
}