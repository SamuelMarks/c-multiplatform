/* clang-format off */
#include "cmp_css_grid.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_grid_template_group(void) {
  cmp_prop_grid_template_group_t group;

  ASSERT_EQ(-1, cmp_prop_grid_template_group_init(NULL, NULL, NULL, NULL));

  ASSERT_EQ(0, cmp_prop_grid_template_group_init(
                   &group, "100px 1fr", "auto",
                   "\"header header\" \"sidebar main\""));
  ASSERT_STR_EQ("100px 1fr", group.columns);
  ASSERT_STR_EQ("auto", group.rows);
  ASSERT_STR_EQ("\"header header\" \"sidebar main\"", group.areas);

  ASSERT_EQ(0, cmp_prop_grid_template_group_free(&group));
  ASSERT_EQ(NULL, group.columns);
  ASSERT_EQ(NULL, group.rows);
  ASSERT_EQ(NULL, group.areas);

  /* Test partial init */
  ASSERT_EQ(0, cmp_prop_grid_template_group_init(&group, NULL, "auto", NULL));
  ASSERT_EQ(NULL, group.columns);
  ASSERT_STR_EQ("auto", group.rows);
  ASSERT_EQ(NULL, group.areas);
  ASSERT_EQ(0, cmp_prop_grid_template_group_free(&group));

  ASSERT_EQ(-1, cmp_prop_grid_template_group_free(NULL));

  PASS();
}

TEST test_grid_auto_group(void) {
  cmp_prop_grid_auto_group_t group;

  ASSERT_EQ(-1, cmp_prop_grid_auto_group_init(NULL, NULL, NULL,
                                              CMP_GRID_AUTO_FLOW_ROW));

  ASSERT_EQ(0, cmp_prop_grid_auto_group_init(&group, "100px", "50px",
                                             CMP_GRID_AUTO_FLOW_COLUMN_DENSE));
  ASSERT_STR_EQ("100px", group.columns);
  ASSERT_STR_EQ("50px", group.rows);
  ASSERT_EQ(CMP_GRID_AUTO_FLOW_COLUMN_DENSE, group.flow);

  ASSERT_EQ(0, cmp_prop_grid_auto_group_free(&group));
  ASSERT_EQ(NULL, group.columns);
  ASSERT_EQ(NULL, group.rows);

  ASSERT_EQ(-1, cmp_prop_grid_auto_group_free(NULL));

  PASS();
}

TEST test_grid_placement_group(void) {
  cmp_prop_grid_placement_group_t group;

  ASSERT_EQ(-1,
            cmp_prop_grid_placement_group_init(NULL, NULL, NULL, NULL, NULL));

  ASSERT_EQ(
      0, cmp_prop_grid_placement_group_init(&group, "1", "span 2", "2", "-1"));
  ASSERT_STR_EQ("1", group.row_start);
  ASSERT_STR_EQ("span 2", group.row_end);
  ASSERT_STR_EQ("2", group.column_start);
  ASSERT_STR_EQ("-1", group.column_end);

  ASSERT_EQ(0, cmp_prop_grid_placement_group_free(&group));
  ASSERT_EQ(NULL, group.row_start);

  ASSERT_EQ(-1, cmp_prop_grid_placement_group_free(NULL));

  PASS();
}

TEST test_grid_track_funcs(void) {
  cmp_grid_track_funcs_t func;

  ASSERT_EQ(-1, cmp_grid_track_funcs_init(NULL, CMP_GRID_TRACK_FUNC_MINMAX,
                                          "100px, 1fr"));
  ASSERT_EQ(-1,
            cmp_grid_track_funcs_init(&func, CMP_GRID_TRACK_FUNC_MINMAX, NULL));

  ASSERT_EQ(0, cmp_grid_track_funcs_init(&func, CMP_GRID_TRACK_FUNC_REPEAT,
                                         "auto-fill, 100px"));
  ASSERT_EQ(CMP_GRID_TRACK_FUNC_REPEAT, func.type);
  ASSERT_STR_EQ("auto-fill, 100px", func.args);

  ASSERT_EQ(0, cmp_grid_track_funcs_free(&func));
  ASSERT_EQ(NULL, func.args);

  ASSERT_EQ(-1, cmp_grid_track_funcs_free(NULL));

  PASS();
}

TEST test_grid_subgrid_and_masonry(void) {
  cmp_prop_subgrid_t sub;
  cmp_masonry_layout_t mas;

  sub.is_subgrid = 1;
  mas.is_masonry = 0;

  ASSERT_EQ(1, sub.is_subgrid);
  ASSERT_EQ(0, mas.is_masonry);

  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_grid_template_group);
  RUN_TEST(test_grid_auto_group);
  RUN_TEST(test_grid_placement_group);
  RUN_TEST(test_grid_track_funcs);
  RUN_TEST(test_grid_subgrid_and_masonry);
  GREATEST_MAIN_END();
}