/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

struct cmp_motion_path {
  /* Simplified placeholder: stores a single bezier curve */
  float p0x, p0y;
  float p1x, p1y;
  float p2x, p2y;
  float p3x, p3y;
};

int cmp_motion_path_create(cmp_motion_path_t **out_path) {
  int rc = CMP_SUCCESS;
  struct cmp_motion_path *path = NULL;

  if (!out_path) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_motion_path_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_motion_path), (void **)&path);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_motion_path_create: Out of memory\n");
    return rc;
  }

  memset(path, 0, sizeof(struct cmp_motion_path));

  /* Placeholder path: simple horizontal curve */
  path->p0x = 0.0f;
  path->p0y = 0.0f;
  path->p1x = 50.0f;
  path->p1y = 100.0f;
  path->p2x = 50.0f;
  path->p2y = -100.0f;
  path->p3x = 100.0f;
  path->p3y = 0.0f;

  *out_path = (cmp_motion_path_t *)path;
  return rc;
}

int cmp_motion_path_destroy(cmp_motion_path_t *path) {
  int rc = CMP_SUCCESS;
  struct cmp_motion_path *internal_path = (struct cmp_motion_path *)path;

  if (!internal_path) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_motion_path_destroy: Invalid argument\n");
    return rc;
  }

  CMP_FREE(internal_path);
  return rc;
}

static float eval_bezier(float p0, float p1, float p2, float p3, float t) {
  float u = 1.0f - t;
  float tt = t * t;
  float uu = u * u;
  float uuu = uu * u;
  float ttt = tt * t;

  float p = uuu * p0;
  p += 3 * uu * t * p1;
  p += 3 * u * tt * p2;
  p += ttt * p3;

  return p;
}

static float eval_bezier_derivative(float p0, float p1, float p2, float p3,
                                    float t) {
  /* Derivative of cubic bezier */
  float u = 1.0f - t;
  float d = 3.0f * u * u * (p1 - p0) + 6.0f * u * t * (p2 - p1) +
            3.0f * t * t * (p3 - p2);
  return d;
}

int cmp_motion_path_evaluate(cmp_motion_path_t *path, float distance,
                             float offset_rotate, float *out_x, float *out_y,
                             float *out_angle) {
  int rc = CMP_SUCCESS;
  struct cmp_motion_path *p = (struct cmp_motion_path *)path;
  float dx;
  float dy;
  float angle_rad;

  if (!p || !out_x || !out_y || !out_angle || distance < 0.0f ||
      distance > 1.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_motion_path_evaluate: Invalid argument\n");
    return rc;
  }

  *out_x = eval_bezier(p->p0x, p->p1x, p->p2x, p->p3x, distance);
  *out_y = eval_bezier(p->p0y, p->p1y, p->p2y, p->p3y, distance);

  dx = eval_bezier_derivative(p->p0x, p->p1x, p->p2x, p->p3x, distance);
  dy = eval_bezier_derivative(p->p0y, p->p1y, p->p2y, p->p3y, distance);

  angle_rad = (float)atan2(dy, dx);
  *out_angle = angle_rad * (180.0f / 3.14159265f) + offset_rotate;

  return rc;
}
