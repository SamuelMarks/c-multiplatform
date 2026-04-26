/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_bezier_ease {
  float x1;
  float y1;
  float x2;
  float y2;
};

/**
 * @brief cmp_bezier_ease_create
 *
 * @param x1 Parameter description.
 * @param y1 Parameter description.
 * @param x2 Parameter description.
 * @param y2 Parameter description.
 * @param out_bezier Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_bezier_ease_create(float x1, float y1, float x2, float y2,
                           cmp_bezier_ease_t **out_bezier) {
  struct cmp_bezier_ease *bezier;

  if (!out_bezier || x1 < 0.0f || x1 > 1.0f || x2 < 0.0f || x2 > 1.0f)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_bezier_ease), (void **)&bezier) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  bezier->x1 = x1;
  bezier->y1 = y1;
  bezier->x2 = x2;
  bezier->y2 = y2;

  *out_bezier = (cmp_bezier_ease_t *)bezier;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_bezier_ease_destroy
 *
 * @param bezier Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_bezier_ease_destroy(cmp_bezier_ease_t *bezier) {
  struct cmp_bezier_ease *internal_bezier = (struct cmp_bezier_ease *)bezier;
  if (!internal_bezier)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_bezier);
  return CMP_SUCCESS;
}

/* Helper to evaluate cubic bezier polynomial */
static float cubic_bezier(float p0, float p1, float p2, float p3, float t) {
  float u = 1.0f - t;
  float tt = t * t;
  float uu = u * u;
  float uuu = uu * u;
  float ttt = tt * t;

  float p = uuu * p0;      /* first term */
  p += 3.0f * uu * t * p1; /* second term */
  p += 3.0f * u * tt * p2; /* third term */
  p += ttt * p3;           /* fourth term */

  return p;
}

/* Helper to evaluate cubic bezier derivative polynomial */
static float cubic_bezier_derivative(float p0, float p1, float p2, float p3,
                                     float t) {
  float u = 1.0f - t;
  float dt_p0 = 3.0f * u * u * (p1 - p0);
  float dt_p1 = 6.0f * u * t * (p2 - p1);
  float dt_p2 = 3.0f * t * t * (p3 - p2);
  return dt_p0 + dt_p1 + dt_p2;
}

/**
 * @brief cmp_bezier_ease_evaluate
 *
 * @param bezier Parameter description.
 * @param t Parameter description.
 * @param out_value Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_bezier_ease_evaluate(cmp_bezier_ease_t *bezier, float t,
                             float *out_value) {
  struct cmp_bezier_ease *b = (struct cmp_bezier_ease *)bezier;
  float guess_t;
  float current_x;
  float current_slope;
  int i;

  if (!b || !out_value || t < 0.0f || t > 1.0f)
    return CMP_ERROR_INVALID_ARG;

  if (t == 0.0f) {
    *out_value = 0.0f;
    return CMP_SUCCESS;
  }
  if (t == 1.0f) {
    *out_value = 1.0f;
    return CMP_SUCCESS;
  }

  /* Newton-Raphson solver to find u where Bx(u) - t = 0 */
  guess_t = t;
  for (i = 0; i < 8; ++i) {
    current_x = cubic_bezier(0.0f, b->x1, b->x2, 1.0f, guess_t) - t;
    current_slope = cubic_bezier_derivative(0.0f, b->x1, b->x2, 1.0f, guess_t);
    if (current_slope == 0.0f) {
      break;
    }
    guess_t -= current_x / current_slope;
  }

  if (guess_t < 0.0f) {
    guess_t = 0.0f;
  } else if (guess_t > 1.0f) {
    guess_t = 1.0f;
  }

  *out_value = cubic_bezier(0.0f, b->y1, b->y2, 1.0f, guess_t);
  return CMP_SUCCESS;
}
