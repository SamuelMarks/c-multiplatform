/* clang-format off */
#include "greatest.h"
#include "cmp_css_tables.h"
/* clang-format on */

TEST test_table_layout_enums(void) {
  cmp_prop_table_layout_t layout = CMP_TABLE_LAYOUT_FIXED;
  ASSERT_EQ(CMP_TABLE_LAYOUT_FIXED, layout);
  PASS();
}

TEST test_table_border_collapse_enums(void) {
  cmp_prop_table_border_collapse_t collapse =
      CMP_TABLE_BORDER_COLLAPSE_COLLAPSE;
  ASSERT_EQ(CMP_TABLE_BORDER_COLLAPSE_COLLAPSE, collapse);
  PASS();
}

TEST test_caption_side_enums(void) {
  cmp_prop_caption_side_t side = CMP_CAPTION_SIDE_BOTTOM;
  ASSERT_EQ(CMP_CAPTION_SIDE_BOTTOM, side);
  PASS();
}

TEST test_empty_cells_enums(void) {
  cmp_prop_empty_cells_t empty = CMP_EMPTY_CELLS_HIDE;
  ASSERT_EQ(CMP_EMPTY_CELLS_HIDE, empty);
  PASS();
}

TEST test_border_spacing_init(void) {
  cmp_prop_table_border_spacing_val_t val;
  int rc;

  rc = cmp_prop_table_border_spacing_val_init_phys(&val, 10.0f, CMP_LENGTH_PX);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(CMP_TABLE_BORDER_SPACING_LENGTH_PHYS, val.type);
  ASSERT_EQ(10.0f, val.value.phys.value);
  ASSERT_EQ(CMP_LENGTH_PX, val.value.phys.unit);

  rc = cmp_prop_table_border_spacing_val_init_rel(&val, 2.5f, CMP_LENGTH_EM);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(CMP_TABLE_BORDER_SPACING_LENGTH_REL, val.type);
  ASSERT_EQ(2.5f, val.value.rel.value);
  ASSERT_EQ(CMP_LENGTH_EM, val.value.rel.unit);

  rc = cmp_prop_table_border_spacing_val_init_phys(NULL, 10.0f, CMP_LENGTH_PX);
  ASSERT_EQ(-1, rc);

  rc = cmp_prop_table_border_spacing_val_init_rel(NULL, 2.5f, CMP_LENGTH_EM);
  ASSERT_EQ(-1, rc);

  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_table_layout_enums);
  RUN_TEST(test_table_border_collapse_enums);
  RUN_TEST(test_caption_side_enums);
  RUN_TEST(test_empty_cells_enums);
  RUN_TEST(test_border_spacing_init);
  GREATEST_MAIN_END();
}
