/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
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
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_bezier_ease *bezier = NULL;

  if (out_bezier == NULL || x1 < 0.0f || x1 > 1.0f || x2 < 0.0f || x2 > 1.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_bezier_ease_create: Invalid argument: %s\n", err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_bezier_ease), (void **)&bezier);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_bezier_ease_create: Out of memory: %s\n", err_str);

    return rc;
  }

  bezier->x1 = x1;
  bezier->y1 = y1;
  bezier->x2 = x2;
  bezier->y2 = y2;

  *out_bezier = (cmp_bezier_ease_t *)bezier;
  cmp_log_debug(
      "cmp_bezier_ease_create: Successfully created bezier ease context\n");

  return rc;
}

/**
 * @brief cmp_bezier_ease_destroy
 *
 * @param bezier Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_bezier_ease_destroy(cmp_bezier_ease_t *bezier) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_bezier_ease *internal_bezier = (struct cmp_bezier_ease *)bezier;

  if (internal_bezier == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_bezier_ease_destroy: Invalid argument (bezier=NULL): %s\n",
        err_str);

    return rc;
  }

  CMP_FREE(internal_bezier);
  cmp_log_debug(
      "cmp_bezier_ease_destroy: Successfully destroyed bezier ease context\n");

  return rc;
}

/* Helper to evaluate cubic bezier polynomial */
static float cmp_math_cubic_bezier(float p0, float p1, float p2, float p3,
                                   float t) {
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
static float cmp_math_cubic_bezier_derivative(float p0, float p1, float p2,
                                              float p3, float t) {
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
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_bezier_ease *b = (struct cmp_bezier_ease *)bezier;
  float guess_t;
  float current_x;
  float current_slope;
  int i;

  if (b == NULL || out_value == NULL || t < 0.0f || t > 1.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_bezier_ease_evaluate: Invalid argument: %s\n", err_str);

    return rc;
  }

  if (t == 0.0f) {
    *out_value = 0.0f;
    return rc;
  }
  if (t == 1.0f) {
    *out_value = 1.0f;
    return rc;
  }

  /* Newton-Raphson solver to find u where Bx(u) - t = 0 */
  guess_t = t;
  for (i = 0; i < 8; ++i) {
    current_x = cmp_math_cubic_bezier(0.0f, b->x1, b->x2, 1.0f, guess_t) - t;
    current_slope =
        cmp_math_cubic_bezier_derivative(0.0f, b->x1, b->x2, 1.0f, guess_t);
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

  *out_value = cmp_math_cubic_bezier(0.0f, b->y1, b->y2, 1.0f, guess_t);
  cmp_log_debug(
      "cmp_bezier_ease_evaluate: Evaluated bezier ease at t=%.2f -> %.2f\n", t,
      *out_value);

  return rc;
}
