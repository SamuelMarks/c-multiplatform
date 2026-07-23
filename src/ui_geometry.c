/* clang-format off */
#include "ui_geometry.h"
/* clang-format on */

enum ui_error ui_dom_point_init(struct ui_dom_point *point, double x, double y,
                                double z, double w) {
  if (!point) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  point->x = x;
  point->y = y;
  point->z = z;
  point->w = w;
  return UI_ERROR_NONE;
}

enum ui_error ui_dom_rect_init(struct ui_dom_rect *rect, double x, double y,
                               double width, double height) {
  if (!rect) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  rect->x = x;
  rect->y = y;
  rect->width = width;
  rect->height = height;

  /* Calculate derived properties */
  if (width < 0.0) {
    rect->left = x + width;
    rect->right = x;
  } else {
    rect->left = x;
    rect->right = x + width;
  }

  if (height < 0.0) {
    rect->top = y + height;
    rect->bottom = y;
  } else {
    rect->top = y;
    rect->bottom = y + height;
  }
  return UI_ERROR_NONE;
}

static enum ui_error get_min(double a, double b, double c, double d,
                             double *out_min) {
  double min;
  min = a;
  if (b < min)
    min = b;
  if (c < min)
    min = c;
  if (d < min)
    min = d;
  *out_min = min;
  return UI_ERROR_NONE;
}

static enum ui_error get_max(double a, double b, double c, double d,
                             double *out_max) {
  double max;
  max = a;
  if (b > max)
    max = b;
  if (c > max)
    max = c;
  if (d > max)
    max = d;
  *out_max = max;
  return UI_ERROR_NONE;
}

enum ui_error ui_dom_quad_init(struct ui_dom_quad *quad,
                               const struct ui_dom_point *p1,
                               const struct ui_dom_point *p2,
                               const struct ui_dom_point *p3,
                               const struct ui_dom_point *p4) {
  double min_x, max_x, min_y, max_y;

  if (!quad || !p1 || !p2 || !p3 || !p4) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  quad->p1 = *p1;
  quad->p2 = *p2;
  quad->p3 = *p3;
  quad->p4 = *p4;

  get_min(p1->x, p2->x, p3->x, p4->x, &min_x);
  get_max(p1->x, p2->x, p3->x, p4->x, &max_x);
  get_min(p1->y, p2->y, p3->y, p4->y, &min_y);
  get_max(p1->y, p2->y, p3->y, p4->y, &max_y);

  ui_dom_rect_init(&quad->bounds, min_x, min_y, max_x - min_x, max_y - min_y);
  return UI_ERROR_NONE;
}

enum ui_error ui_dom_matrix_init_identity(struct ui_dom_matrix *matrix) {
  if (!matrix) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  matrix->m11 = 1.0;
  matrix->m12 = 0.0;
  matrix->m13 = 0.0;
  matrix->m14 = 0.0;
  matrix->m21 = 0.0;
  matrix->m22 = 1.0;
  matrix->m23 = 0.0;
  matrix->m24 = 0.0;
  matrix->m31 = 0.0;
  matrix->m32 = 0.0;
  matrix->m33 = 1.0;
  matrix->m34 = 0.0;
  matrix->m41 = 0.0;
  matrix->m42 = 0.0;
  matrix->m43 = 0.0;
  matrix->m44 = 1.0;
  return UI_ERROR_NONE;
}
