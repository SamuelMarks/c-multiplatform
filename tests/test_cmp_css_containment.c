/* clang-format off */
#include "greatest.h"
#include "cmp_css_containment.h"
/* clang-format on */

TEST test_contain_init(void) {
  cmp_prop_contain_t prop;
  ASSERT_EQ(
      0, cmp_prop_contain_init(&prop, CMP_CONTAIN_STRICT | CMP_CONTAIN_PAINT));
  ASSERT_EQ(CMP_CONTAIN_STRICT | CMP_CONTAIN_PAINT, prop.flags);
  PASS();
}

TEST test_content_visibility_init(void) {
  cmp_prop_content_visibility_t prop;
  ASSERT_EQ(0, cmp_prop_content_visibility_init(&prop,
                                                CMP_CONTENT_VISIBILITY_HIDDEN));
  ASSERT_EQ(CMP_CONTENT_VISIBILITY_HIDDEN, prop.visibility);
  PASS();
}

TEST test_contain_intrinsic_init_free(void) {
  cmp_prop_contain_intrinsic_t prop;
  ASSERT_EQ(0, cmp_prop_contain_intrinsic_init(&prop, "100px", "50px", "none"));
  ASSERT_STR_EQ("100px", prop.size);
  ASSERT_STR_EQ("50px", prop.block_size);
  ASSERT_STR_EQ("none", prop.inline_size);

  ASSERT_EQ(0, cmp_prop_contain_intrinsic_free(&prop));
  ASSERT_EQ(NULL, prop.size);
  ASSERT_EQ(NULL, prop.block_size);
  ASSERT_EQ(NULL, prop.inline_size);
  PASS();
}

TEST test_will_change_init_free(void) {
  cmp_prop_will_change_t prop;
  ASSERT_EQ(0, cmp_prop_will_change_init(&prop, "transform, opacity"));
  ASSERT_STR_EQ("transform, opacity", prop.will_change);

  ASSERT_EQ(0, cmp_prop_will_change_free(&prop));
  ASSERT_EQ(NULL, prop.will_change);
  PASS();
}

TEST test_container_group(void) {
  cmp_prop_container_group_t group;
  int rc;

  rc = cmp_prop_container_group_init(&group, CMP_CONTAINER_TYPE_SIZE,
                                     "my-container");
  ASSERT_EQ(0, rc);
  ASSERT_EQ(CMP_CONTAINER_TYPE_SIZE, group.type);
  ASSERT_STR_EQ("my-container", group.name);

  rc = cmp_prop_container_group_free(&group);
  ASSERT_EQ(0, rc);

  rc = cmp_prop_container_group_init(NULL, CMP_CONTAINER_TYPE_NORMAL, NULL);
  ASSERT_EQ(-1, rc);

  rc = cmp_prop_container_group_free(NULL);
  ASSERT_EQ(-1, rc);

  PASS();
}

SUITE(cmp_css_containment_suite) {
  RUN_TEST(test_contain_init);
  RUN_TEST(test_content_visibility_init);
  RUN_TEST(test_contain_intrinsic_init_free);
  RUN_TEST(test_will_change_init_free);
  RUN_TEST(test_container_group);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_css_containment_suite);
  GREATEST_MAIN_END();
}
