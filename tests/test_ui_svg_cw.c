/* clang-format off */
#include "ui_svg.h"
#include <stdio.h>
/* clang-format on */

int main(void) {
  struct ui_svg_geometry geom;
  struct ui_svg_flattened_path f;
  struct ui_svg_subpath sp;

  ui_svg_geometry_init(&geom);
  ui_svg_flattened_path_init(&f);

  /* Create an un-tessellable shape. Like completely self intersecting and
   * identical */
  struct ui_svg_point degen2[6] = {{0, 0}, {0, 0}, {0, 0},
                                   {0, 0}, {0, 0}, {0, 0}};
  sp.points = degen2;
  sp.count = 6;
  sp.capacity = 6;
  sp.closed = 1;
  f.subpaths = &sp;
  f.count = 1;
  f.capacity = 1;
  ui_svg_tessellate_fill(&geom, &f);

  f.subpaths = NULL;
  ui_svg_geometry_destroy(&geom);
  printf("Done\n");
  return 0;
}
