#include "ui_property_grid_base.h"

void test_extra_property_grid(void) {
  struct ui_property_grid_base *grid = NULL;
  ui_property_grid_base_get_group_by_id(grid, NULL, NULL);
  ui_property_grid_base_get_group_by_id(grid, "id", NULL);
  /* The out_group parameter gets written, so pass a valid one */
  struct ui_property_group *grp = NULL;
  /* wait, if we pass grid=NULL, it shouldn't crash, but let's look at
   * get_group_by_id */
}
