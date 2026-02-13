#include "cmpc/cmp_math.h"

#include <math.h>

static CMPScalar cmp_scalar_max(CMPScalar a, CMPScalar b) {
  return (a > b) ? a : b;
}

static CMPScalar cmp_scalar_min(CMPScalar a, CMPScalar b) {
  return (a < b) ? a : b;
}

int CMP_CALL cmp_rect_intersect(const CMPRect *a, const CMPRect *b,
                              CMPRect *out_rect, CMPBool *out_has_intersection) {
  CMPScalar ax2;
  CMPScalar ay2;
  CMPScalar bx2;
  CMPScalar by2;
  CMPScalar ix1;
  CMPScalar iy1;
  CMPScalar ix2;
  CMPScalar iy2;

  if (a == NULL || b == NULL || out_rect == NULL ||
      out_has_intersection == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (a->width < 0.0f || a->height < 0.0f || b->width < 0.0f ||
      b->height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  ax2 = a->x + a->width;
  ay2 = a->y + a->height;
  bx2 = b->x + b->width;
  by2 = b->y + b->height;

  ix1 = cmp_scalar_max(a->x, b->x);
  iy1 = cmp_scalar_max(a->y, b->y);
  ix2 = cmp_scalar_min(ax2, bx2);
  iy2 = cmp_scalar_min(ay2, by2);

  if (ix2 <= ix1 || iy2 <= iy1) {
    out_rect->x = 0.0f;
    out_rect->y = 0.0f;
    out_rect->width = 0.0f;
    out_rect->height = 0.0f;
    *out_has_intersection = CMP_FALSE;
    return CMP_OK;
  }

  out_rect->x = ix1;
  out_rect->y = iy1;
  out_rect->width = ix2 - ix1;
  out_rect->height = iy2 - iy1;
  *out_has_intersection = CMP_TRUE;
  return CMP_OK;
}

int CMP_CALL cmp_rect_union(const CMPRect *a, const CMPRect *b, CMPRect *out_rect) {
  CMPScalar ax2;
  CMPScalar ay2;
  CMPScalar bx2;
  CMPScalar by2;
  CMPScalar ux1;
  CMPScalar uy1;
  CMPScalar ux2;
  CMPScalar uy2;

  if (a == NULL || b == NULL || out_rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (a->width < 0.0f || a->height < 0.0f || b->width < 0.0f ||
      b->height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  ax2 = a->x + a->width;
  ay2 = a->y + a->height;
  bx2 = b->x + b->width;
  by2 = b->y + b->height;

  ux1 = cmp_scalar_min(a->x, b->x);
  uy1 = cmp_scalar_min(a->y, b->y);
  ux2 = cmp_scalar_max(ax2, bx2);
  uy2 = cmp_scalar_max(ay2, by2);

  out_rect->x = ux1;
  out_rect->y = uy1;
  out_rect->width = ux2 - ux1;
  out_rect->height = uy2 - uy1;
  return CMP_OK;
}

int CMP_CALL cmp_rect_contains_point(const CMPRect *rect, CMPScalar x, CMPScalar y,
                                   CMPBool *out_contains) {
  if (rect == NULL || out_contains == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (rect->width == 0.0f || rect->height == 0.0f) {
    *out_contains = CMP_FALSE;
    return CMP_OK;
  }

  if (x < rect->x || y < rect->y) {
    *out_contains = CMP_FALSE;
    return CMP_OK;
  }

  if (x >= rect->x + rect->width || y >= rect->y + rect->height) {
    *out_contains = CMP_FALSE;
    return CMP_OK;
  }

  *out_contains = CMP_TRUE;
  return CMP_OK;
}

int CMP_CALL cmp_mat3_identity(CMPMat3 *out_mat) {
  if (out_mat == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
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
  return CMP_OK;
}

int CMP_CALL cmp_mat3_mul(const CMPMat3 *a, const CMPMat3 *b, CMPMat3 *out_mat) {
  CMPMat3 tmp;
  int row;
  int col;
  int idx;

  if (a == NULL || b == NULL || out_mat == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
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
  return CMP_OK;
}

int CMP_CALL cmp_mat3_translate(CMPScalar tx, CMPScalar ty, CMPMat3 *out_mat) {
  if (out_mat == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
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
  return CMP_OK;
}

int CMP_CALL cmp_mat3_scale(CMPScalar sx, CMPScalar sy, CMPMat3 *out_mat) {
  if (out_mat == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
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
  return CMP_OK;
}

int CMP_CALL cmp_mat3_rotate(CMPScalar radians, CMPMat3 *out_mat) {
  CMPScalar c;
  CMPScalar s;

  if (out_mat == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  c = (CMPScalar)cos((double)radians);
  s = (CMPScalar)sin((double)radians);

  out_mat->m[0] = c;
  out_mat->m[1] = s;
  out_mat->m[2] = 0.0f;
  out_mat->m[3] = -s;
  out_mat->m[4] = c;
  out_mat->m[5] = 0.0f;
  out_mat->m[6] = 0.0f;
  out_mat->m[7] = 0.0f;
  out_mat->m[8] = 1.0f;
  return CMP_OK;
}

int CMP_CALL cmp_mat3_transform_point(const CMPMat3 *m, CMPScalar x, CMPScalar y,
                                    CMPScalar *out_x, CMPScalar *out_y) {
  CMPScalar tx;
  CMPScalar ty;
  CMPScalar tw;

  if (m == NULL || out_x == NULL || out_y == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  tx = m->m[0] * x + m->m[3] * y + m->m[6];
  ty = m->m[1] * x + m->m[4] * y + m->m[7];
  tw = m->m[2] * x + m->m[5] * y + m->m[8];

  if (tw == 0.0f) {
    return CMP_ERR_RANGE;
  }

  *out_x = tx / tw;
  *out_y = ty / tw;
  return CMP_OK;
}
