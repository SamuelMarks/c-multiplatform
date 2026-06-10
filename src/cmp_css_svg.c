/* clang-format off */
#include "cmp_css_svg.h"
#include <stddef.h>
#include <string.h>
/* clang-format on */

int cmp_prop_svg_fill_init(cmp_prop_svg_fill_t *fill) {
  if (fill == NULL) {
    return -1;
  }
  memset(fill, 0, sizeof(cmp_prop_svg_fill_t));
  {
    int rc = cmp_prop_color_init_transparent(&fill->fill);
    if (rc != 0) return rc;
  }
  fill->rule = CMP_SVG_FILL_RULE_NONZERO;
  fill->opacity.value = 1.0f;
  return 0;
}

int cmp_prop_svg_stroke_init(cmp_prop_svg_stroke_t *stroke) {
  if (stroke == NULL) {
    return -1;
  }
  memset(stroke, 0, sizeof(cmp_prop_svg_stroke_t));
  {
    int rc = cmp_prop_color_init_transparent(&stroke->stroke);
    if (rc != 0) return rc;
  }
  {
    int rc = cmp_prop_size_init_auto(&stroke->width);
    if (rc != 0) return rc;
  }
  stroke->linecap = CMP_SVG_STROKE_LINECAP_BUTT;
  stroke->linejoin = CMP_SVG_STROKE_LINEJOIN_MITER;
  stroke->miterlimit = 4.0f;
  stroke->dasharray_count = 0;
  {
    int rc = cmp_prop_size_init_auto(&stroke->dashoffset);
    if (rc != 0) return rc;
  }
  stroke->opacity.value = 1.0f;
  return 0;
}

int cmp_prop_svg_geom_init(cmp_prop_svg_geom_t *geom) {
  if (geom == NULL) {
    return -1;
  }
  memset(geom, 0, sizeof(cmp_prop_svg_geom_t));
  if (cmp_prop_size_init_auto(&geom->cx) != 0) return -1;
  if (cmp_prop_size_init_auto(&geom->cy) != 0) return -1;
  if (cmp_prop_size_init_auto(&geom->r) != 0) return -1;
  if (cmp_prop_size_init_auto(&geom->rx) != 0) return -1;
  if (cmp_prop_size_init_auto(&geom->ry) != 0) return -1;
  if (cmp_prop_size_init_auto(&geom->x) != 0) return -1;
  if (cmp_prop_size_init_auto(&geom->y) != 0) return -1;
  return 0;
}
