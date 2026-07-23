/* clang-format off */
#include "ui_svg.h"
#include <stdio.h>
/* clang-format on */

int main(void) {
  struct ui_svg_geometry geom;
  struct ui_svg_flattened_path f;
  struct ui_svg_subpath sp;
  struct ui_svg_point pts[2] = {{0, 0}, {10, 10}};

  ui_svg_geometry_init(&geom);
  ui_svg_flattened_path_init(&f);
  sp.points = pts;
  sp.count = 2;
  sp.capacity = 2;
  sp.closed = 0;
  f.subpaths = &sp;
  f.count = 1;
  f.capacity = 1;

  ui_svg_tessellate_stroke(&geom, &f, 2.0f);

  /* Trigger degenerate stroke */
  struct ui_svg_point dpts[2] = {{0, 0}, {0, 0}};
  sp.points = dpts;
  ui_svg_tessellate_stroke(&geom, &f, 2.0f);

  f.subpaths = NULL;
  ui_svg_geometry_destroy(&geom);

  return 0;
}
