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

/**
 * @brief Create a motion path.
 *
 * @param out_path Pointer to store the created path.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_motion_path_create(cmp_motion_path_t **out_path) {
  int rc = CMP_SUCCESS;
  struct cmp_motion_path *path;

  rc = CMP_SUCCESS;
  path = NULL;

  if (out_path == NULL) {
    LOG_DEBUG("Error in cmp_motion_path_create: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_motion_path), (void **)&path);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_motion_path_create: CMP_MALLOC failed (OOM)\n");
    return CMP_ERROR_OOM;
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

/**
 * @brief Destroy a motion path.
 *
 * @param path The path to destroy.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_motion_path_destroy(cmp_motion_path_t *path) {
  int rc = CMP_SUCCESS;
  struct cmp_motion_path *internal_path;

  rc = CMP_SUCCESS;

  if (path == NULL) {
    LOG_DEBUG("Error in cmp_motion_path_destroy: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  internal_path = (struct cmp_motion_path *)path;

  rc = CMP_FREE(internal_path);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_motion_path_destroy: CMP_FREE failed\n");
    return rc;
  }
  return rc;
}

int CMP_API cmp_motion_path_set_curve(cmp_motion_path_t *path, float p0x,
                                      float p0y, float p1x, float p1y,
                                      float p2x, float p2y, float p3x,
                                      float p3y) {
  int rc = 0;
  struct cmp_motion_path *p = (struct cmp_motion_path *)path;
  if (!p)
    return CMP_ERROR_INVALID_ARG;
  p->p0x = p0x;
  p->p0y = p0y;
  p->p1x = p1x;
  p->p1y = p1y;
  p->p2x = p2x;
  p->p2y = p2y;
  p->p3x = p3x;
  p->p3y = p3y;
  return rc;
}

/**
 * @brief Evaluate cubic bezier at parameter t.
 *
 * @param p0 Point 0.
 * @param p1 Point 1.
 * @param p2 Point 2.
 * @param p3 Point 3.
 * @param t Parameter t in [0.0, 1.0].
 * @return Returns the evaluated value.
 */
static float cmp_math_eval_bezier(float p0, float p1, float p2, float p3,
                                  float t) {
  float u = 1.0f - t;
  float tt = t * t;
  float uu = u * u;
  float uuu = uu * u;
  float ttt = tt * t;
  float p;

  p = uuu * p0;
  p += 3 * uu * t * p1;
  p += 3 * u * tt * p2;
  p += ttt * p3;

  return p;
}

/**
 * @brief Evaluate derivative of cubic bezier at parameter t.
 *
 * @param p0 Point 0.
 * @param p1 Point 1.
 * @param p2 Point 2.
 * @param p3 Point 3.
 * @param t Parameter t in [0.0, 1.0].
 * @return Returns the evaluated derivative.
 */
static float cmp_math_eval_bezier_derivative(float p0, float p1, float p2,
                                             float p3, float t) {
  /* Derivative of cubic bezier */
  float u = 1.0f - t;
  float d = 3.0f * u * u * (p1 - p0) + 6.0f * u * t * (p2 - p1) +
            3.0f * t * t * (p3 - p2);
  return d;
}

/**
 * @brief Evaluate the position and rotation along the path.
 *
 * @param path The path to evaluate.
 * @param distance Normalized distance [0.0, 1.0].
 * @param offset_rotate Rotation offset in degrees.
 * @param out_x Pointer to store X coordinate.
 * @param out_y Pointer to store Y coordinate.
 * @param out_angle Pointer to store angle in degrees.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_motion_path_evaluate(cmp_motion_path_t *path, float distance,
                             float offset_rotate, float *out_x, float *out_y,
                             float *out_angle) {
  int rc = CMP_SUCCESS;
  struct cmp_motion_path *p;
  float dx;
  float dy;
  float angle_rad;

  if (path == NULL || out_x == NULL || out_y == NULL || out_angle == NULL ||
      distance < 0.0f || distance > 1.0f) {
    LOG_DEBUG("Error in cmp_motion_path_evaluate: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  p = (struct cmp_motion_path *)path;

  *out_x = cmp_math_eval_bezier(p->p0x, p->p1x, p->p2x, p->p3x, distance);
  *out_y = cmp_math_eval_bezier(p->p0y, p->p1y, p->p2y, p->p3y, distance);

  dx =
      cmp_math_eval_bezier_derivative(p->p0x, p->p1x, p->p2x, p->p3x, distance);
  dy =
      cmp_math_eval_bezier_derivative(p->p0y, p->p1y, p->p2y, p->p3y, distance);
  angle_rad = (float)atan2(dy, dx);
  *out_angle = angle_rad * (180.0f / 3.14159265f) + offset_rotate;
  return rc;
}
