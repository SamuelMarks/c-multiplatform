/* clang-format off */
#include "cmp_css_positioning.h"
#include "greatest.h"
/* clang-format on */

TEST test_prop_z_index(void) {
  cmp_prop_z_index_t z;

  ASSERT_EQ(-1, cmp_prop_z_index_init_auto(NULL));
  ASSERT_EQ(0, cmp_prop_z_index_init_auto(&z));
  ASSERT_EQ(CMP_Z_INDEX_AUTO, z.type);

  ASSERT_EQ(-1, cmp_prop_z_index_init_integer(NULL, 10));
  ASSERT_EQ(0, cmp_prop_z_index_init_integer(&z, 10));
  ASSERT_EQ(CMP_Z_INDEX_INTEGER, z.type);
  ASSERT_EQ(10, z.value);

  PASS();
}

TEST test_prop_inset(void) {
  cmp_prop_inset_t inset;

  ASSERT_EQ(-1, cmp_prop_inset_init_auto(NULL));
  ASSERT_EQ(0, cmp_prop_inset_init_auto(&inset));
  ASSERT_EQ(CMP_PROP_INSET_AUTO, inset.type);

  ASSERT_EQ(-1, cmp_prop_inset_init_phys(NULL, 5.0f, CMP_LENGTH_PX));
  ASSERT_EQ(0, cmp_prop_inset_init_phys(&inset, 5.0f, CMP_LENGTH_PX));
  ASSERT_EQ(CMP_PROP_INSET_LENGTH_PHYS, inset.type);
  ASSERT_EQ(5.0f, inset.value.phys.value);
  ASSERT_EQ(CMP_LENGTH_PX, inset.value.phys.unit);

  ASSERT_EQ(-1, cmp_prop_inset_init_percent(NULL, 50.0f));
  ASSERT_EQ(0, cmp_prop_inset_init_percent(&inset, 50.0f));
  ASSERT_EQ(CMP_PROP_INSET_PERCENT, inset.type);
  ASSERT_EQ(50.0f, inset.value.percent.value);

  PASS();
}

TEST test_prop_inset_group(void) {
  cmp_prop_inset_group_t group;

  ASSERT_EQ(-1, cmp_prop_inset_group_init_auto(NULL));
  ASSERT_EQ(0, cmp_prop_inset_group_init_auto(&group));
  ASSERT_EQ(CMP_PROP_INSET_AUTO, group.top.type);
  ASSERT_EQ(CMP_PROP_INSET_AUTO, group.right.type);
  ASSERT_EQ(CMP_PROP_INSET_AUTO, group.bottom.type);
  ASSERT_EQ(CMP_PROP_INSET_AUTO, group.left.type);

  PASS();
}

TEST test_enums(void) {
  cmp_prop_position_t p = CMP_POSITION_STICKY;
  cmp_prop_isolation_t i = CMP_ISOLATION_ISOLATE;

  ASSERT_EQ(CMP_POSITION_STICKY, p);
  ASSERT_EQ(CMP_ISOLATION_ISOLATE, i);

  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_prop_z_index);
  RUN_TEST(test_prop_inset);
  RUN_TEST(test_prop_inset_group);
  RUN_TEST(test_enums);
  GREATEST_MAIN_END();
}