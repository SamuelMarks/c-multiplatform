/* clang-format off */
#include "ui_svg.h"
#include <stdio.h>
/* clang-format on */

int main(void) {
  struct ui_svg_path path;
  ui_svg_path_init(&path);
  ui_svg_path_parse(
      &path, "M 0 0 C 100 0 100 100 0 100 Z M 10 10 Q 20 20 30 30 T 40 40");

  /* Trigger error parse */
  ui_svg_path_parse(&path, "X");
  ui_svg_path_parse(&path, "M");
  ui_svg_path_parse(&path, "10");
  ui_svg_path_parse(&path, "M 10 A 1");

  struct ui_svg_flattened_path flat;
  ui_svg_flattened_path_init(&flat);
  ui_svg_path_flatten(&flat, &path, 1.0f);

  struct ui_svg_geometry geom;
  ui_svg_geometry_init(&geom);
  ui_svg_tessellate_fill(&geom, &flat);
  ui_svg_tessellate_stroke(&geom, &flat, 2.0f);

  ui_svg_geometry_destroy(&geom);
  ui_svg_flattened_path_destroy(&flat);
  ui_svg_path_destroy(&path);

  printf("Done coverage\n");
  return 0;
}
