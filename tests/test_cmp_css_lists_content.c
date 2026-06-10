/* clang-format off */
#include "greatest.h"
#include "cmp_css_lists_content.h"
/* clang-format on */

TEST test_list_style_group_init_free(void) {
  cmp_prop_list_style_group_t group;
  ASSERT_EQ(0, cmp_prop_list_style_group_init(
                   &group, CMP_LIST_STYLE_TYPE_STRING, "->",
                   CMP_LIST_STYLE_POSITION_INSIDE, "url('bullet.png')"));
  ASSERT_EQ(CMP_LIST_STYLE_TYPE_STRING, group.type);
  ASSERT_STR_EQ("->", group.custom_string);
  ASSERT_EQ(CMP_LIST_STYLE_POSITION_INSIDE, group.position);
  ASSERT_STR_EQ("url('bullet.png')", group.image);

  ASSERT_EQ(0, cmp_prop_list_style_group_free(&group));
  ASSERT_EQ(NULL, group.custom_string);
  ASSERT_EQ(NULL, group.image);
  PASS();
}

TEST test_counter_group_init_add_free(void) {
  cmp_prop_counter_group_t group;
  ASSERT_EQ(0, cmp_prop_counter_group_init(&group));

  ASSERT_EQ(0, cmp_prop_counter_group_add_reset(&group, "section", 1));
  ASSERT_EQ(0, cmp_prop_counter_group_add_increment(&group, "chapter", 2));
  ASSERT_EQ(0, cmp_prop_counter_group_add_set(&group, "page", 5));

  ASSERT_EQ(1, group.num_resets);
  ASSERT_STR_EQ("section", group.resets[0].counter_name);
  ASSERT_EQ(1, group.resets[0].value);

  ASSERT_EQ(1, group.num_increments);
  ASSERT_STR_EQ("chapter", group.increments[0].counter_name);
  ASSERT_EQ(2, group.increments[0].value);

  ASSERT_EQ(1, group.num_sets);
  ASSERT_STR_EQ("page", group.sets[0].counter_name);
  ASSERT_EQ(5, group.sets[0].value);

  ASSERT_EQ(0, cmp_prop_counter_group_free(&group));
  ASSERT_EQ(0, group.num_resets);
  ASSERT_EQ(0, group.num_increments);
  ASSERT_EQ(0, group.num_sets);
  PASS();
}

TEST test_content_group_init_free(void) {
  cmp_prop_content_group_t group;
  ASSERT_EQ(0, cmp_prop_content_group_init(
                   &group, "\"Section \" counter(section)", "\"«\" \"»\""));
  ASSERT_STR_EQ("\"Section \" counter(section)", group.content_prop.content);
  ASSERT_STR_EQ("\"«\" \"»\"", group.quotes_prop.quotes);

  ASSERT_EQ(0, cmp_prop_content_group_free(&group));
  ASSERT_EQ(NULL, group.content_prop.content);
  ASSERT_EQ(NULL, group.quotes_prop.quotes);
  PASS();
}

SUITE(cmp_css_lists_content_suite) {
  RUN_TEST(test_list_style_group_init_free);
  RUN_TEST(test_counter_group_init_add_free);
  RUN_TEST(test_content_group_init_free);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_css_lists_content_suite);
  GREATEST_MAIN_END();
}
