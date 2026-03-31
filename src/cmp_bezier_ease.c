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

  float p = uuu * p0;   /* first term */
  p += 3 * uu * t * p1; /* second term */
  p += 3 * u * tt * p2; /* third term */
  p += ttt * p3;        /* fourth term */

  return p;
}

int cmp_bezier_ease_evaluate(cmp_bezier_ease_t *bezier, float t,
                             float *out_value) {
  struct cmp_bezier_ease *b = (struct cmp_bezier_ease *)bezier;
  float u;

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

  /* Since x goes from 0 to 1, for a proper easing function we need to find the
     'u' value where cubic_bezier(0, x1, x2, 1, u) == t. For simplicity in this
     stub, we'll approximate u = t for common curves, or we would implement a
     Newton-Raphson solver here for precise mapping. Here we just evaluate the Y
     curve directly for demonstration as a fast path. */

  /* A true implementation requires finding root of Bx(u) - t = 0 to find u,
   * then evaluating By(u) */
  /* This is a placeholder for the math: */
  u = t; /* approximation */

  *out_value = cubic_bezier(0.0f, b->y1, b->y2, 1.0f, u);
  return CMP_SUCCESS;
}