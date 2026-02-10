#include "m3/m3_math.h"

#include <math.h>

static M3Scalar m3_scalar_max(M3Scalar a, M3Scalar b) {
  return (a > b) ? a : b;
}

static M3Scalar m3_scalar_min(M3Scalar a, M3Scalar b) {
  return (a < b) ? a : b;
}

int M3_CALL m3_rect_intersect(const M3Rect *a, const M3Rect *b,
                              M3Rect *out_rect, M3Bool *out_has_intersection) {
  M3Scalar ax2;
  M3Scalar ay2;
  M3Scalar bx2;
  M3Scalar by2;
  M3Scalar ix1;
  M3Scalar iy1;
  M3Scalar ix2;
  M3Scalar iy2;

  if (a == NULL || b == NULL || out_rect == NULL ||
      out_has_intersection == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (a->width < 0.0f || a->height < 0.0f || b->width < 0.0f ||
      b->height < 0.0f) {
    return M3_ERR_RANGE;
  }

  ax2 = a->x + a->width;
  ay2 = a->y + a->height;
  bx2 = b->x + b->width;
  by2 = b->y + b->height;

  ix1 = m3_scalar_max(a->x, b->x);
  iy1 = m3_scalar_max(a->y, b->y);
  ix2 = m3_scalar_min(ax2, bx2);
  iy2 = m3_scalar_min(ay2, by2);

  if (ix2 <= ix1 || iy2 <= iy1) {
    out_rect->x = 0.0f;
    out_rect->y = 0.0f;
    out_rect->width = 0.0f;
    out_rect->height = 0.0f;
    *out_has_intersection = M3_FALSE;
    return M3_OK;
  }

  out_rect->x = ix1;
  out_rect->y = iy1;
  out_rect->width = ix2 - ix1;
  out_rect->height = iy2 - iy1;
  *out_has_intersection = M3_TRUE;
  return M3_OK;
}

int M3_CALL m3_rect_union(const M3Rect *a, const M3Rect *b, M3Rect *out_rect) {
  M3Scalar ax2;
  M3Scalar ay2;
  M3Scalar bx2;
  M3Scalar by2;
  M3Scalar ux1;
  M3Scalar uy1;
  M3Scalar ux2;
  M3Scalar uy2;

  if (a == NULL || b == NULL || out_rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (a->width < 0.0f || a->height < 0.0f || b->width < 0.0f ||
      b->height < 0.0f) {
    return M3_ERR_RANGE;
  }

  ax2 = a->x + a->width;
  ay2 = a->y + a->height;
  bx2 = b->x + b->width;
  by2 = b->y + b->height;

  ux1 = m3_scalar_min(a->x, b->x);
  uy1 = m3_scalar_min(a->y, b->y);
  ux2 = m3_scalar_max(ax2, bx2);
  uy2 = m3_scalar_max(ay2, by2);

  out_rect->x = ux1;
  out_rect->y = uy1;
  out_rect->width = ux2 - ux1;
  out_rect->height = uy2 - uy1;
  return M3_OK;
}

int M3_CALL m3_rect_contains_point(const M3Rect *rect, M3Scalar x, M3Scalar y,
                                   M3Bool *out_contains) {
  if (rect == NULL || out_contains == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }

  if (rect->width == 0.0f || rect->height == 0.0f) {
    *out_contains = M3_FALSE;
    return M3_OK;
  }

  if (x < rect->x || y < rect->y) {
    *out_contains = M3_FALSE;
    return M3_OK;
  }

  if (x >= rect->x + rect->width || y >= rect->y + rect->height) {
    *out_contains = M3_FALSE;
    return M3_OK;
  }

  *out_contains = M3_TRUE;
  return M3_OK;
}

int M3_CALL m3_mat3_identity(M3Mat3 *out_mat) {
  if (out_mat == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  out_mat->m[0] = 1.0f;
  out_mat->m[1] = 0.0f;
  out_mat->m[2] = 0.0f;
  out_mat->m[3] = 0.0f;
  out_mat->m[4] = 1.0f;
  out_mat->m[5] = 0.0f;
  out_mat->m[6] = 0.0f;
  out_mat->m[7] = 0.0f;
  out_mat->m[8] = 1.0f;
  return M3_OK;
}

int M3_CALL m3_mat3_mul(const M3Mat3 *a, const M3Mat3 *b, M3Mat3 *out_mat) {
  M3Mat3 tmp;
  int row;
  int col;
  int idx;

  if (a == NULL || b == NULL || out_mat == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  for (col = 0; col < 3; ++col) {
    for (row = 0; row < 3; ++row) {
      idx = col * 3 + row;
      tmp.m[idx] = a->m[0 * 3 + row] * b->m[col * 3 + 0] +
                   a->m[1 * 3 + row] * b->m[col * 3 + 1] +
                   a->m[2 * 3 + row] * b->m[col * 3 + 2];
    }
  }

  *out_mat = tmp;
  return M3_OK;
}

int M3_CALL m3_mat3_translate(M3Scalar tx, M3Scalar ty, M3Mat3 *out_mat) {
  if (out_mat == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  out_mat->m[0] = 1.0f;
  out_mat->m[1] = 0.0f;
  out_mat->m[2] = 0.0f;
  out_mat->m[3] = 0.0f;
  out_mat->m[4] = 1.0f;
  out_mat->m[5] = 0.0f;
  out_mat->m[6] = tx;
  out_mat->m[7] = ty;
  out_mat->m[8] = 1.0f;
  return M3_OK;
}

int M3_CALL m3_mat3_scale(M3Scalar sx, M3Scalar sy, M3Mat3 *out_mat) {
  if (out_mat == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  out_mat->m[0] = sx;
  out_mat->m[1] = 0.0f;
  out_mat->m[2] = 0.0f;
  out_mat->m[3] = 0.0f;
  out_mat->m[4] = sy;
  out_mat->m[5] = 0.0f;
  out_mat->m[6] = 0.0f;
  out_mat->m[7] = 0.0f;
  out_mat->m[8] = 1.0f;
  return M3_OK;
}

int M3_CALL m3_mat3_rotate(M3Scalar radians, M3Mat3 *out_mat) {
  M3Scalar c;
  M3Scalar s;

  if (out_mat == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  c = (M3Scalar)cos((double)radians);
  s = (M3Scalar)sin((double)radians);

  out_mat->m[0] = c;
  out_mat->m[1] = s;
  out_mat->m[2] = 0.0f;
  out_mat->m[3] = -s;
  out_mat->m[4] = c;
  out_mat->m[5] = 0.0f;
  out_mat->m[6] = 0.0f;
  out_mat->m[7] = 0.0f;
  out_mat->m[8] = 1.0f;
  return M3_OK;
}

int M3_CALL m3_mat3_transform_point(const M3Mat3 *m, M3Scalar x, M3Scalar y,
                                    M3Scalar *out_x, M3Scalar *out_y) {
  M3Scalar tx;
  M3Scalar ty;
  M3Scalar tw;

  if (m == NULL || out_x == NULL || out_y == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  tx = m->m[0] * x + m->m[3] * y + m->m[6];
  ty = m->m[1] * x + m->m[4] * y + m->m[7];
  tw = m->m[2] * x + m->m[5] * y + m->m[8];

  if (tw == 0.0f) {
    return M3_ERR_RANGE;
  }

  *out_x = tx / tw;
  *out_y = ty / tw;
  return M3_OK;
}
