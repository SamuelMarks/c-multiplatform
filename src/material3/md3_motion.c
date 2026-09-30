/**
 * @file md3_motion.c
 * @brief Material 3 Baseline Motion Curves and Cubic Bezier evaluation
 * implementation.
 */

/* clang-format off */
#include "material3/md3_motion.h"
#include <math.h>
/* clang-format on */

/**
 * @brief Table of Material 3 Baseline Cubic-Bezier control points.
 */
static const struct md3_cubic_bezier g_md3_curves[MD3_MOTION_CURVE_COUNT] = {
    /* MD3_MOTION_CURVE_EMPHASIZED: cubic-bezier(0.2, 0.0, 0.0, 1.0) */
    {0.2f, 0.0f, 0.0f, 1.0f},
    /* MD3_MOTION_CURVE_EMPHASIZED_ACCELERATE: cubic-bezier(0.3, 0.0, 0.8, 0.15)
     */
    {0.3f, 0.0f, 0.8f, 0.15f},
    /* MD3_MOTION_CURVE_EMPHASIZED_DECELERATE: cubic-bezier(0.05, 0.7, 0.1, 1.0)
     */
    {0.05f, 0.7f, 0.1f, 1.0f},
    /* MD3_MOTION_CURVE_STANDARD: cubic-bezier(0.2, 0.0, 0.0, 1.0) */
    {0.2f, 0.0f, 0.0f, 1.0f},
    /* MD3_MOTION_CURVE_STANDARD_ACCELERATE: cubic-bezier(0.3, 0.0, 1.0, 1.0) */
    {0.3f, 0.0f, 1.0f, 1.0f},
    /* MD3_MOTION_CURVE_STANDARD_DECELERATE: cubic-bezier(0.0, 0.0, 0.0, 1.0) */
    {0.0f, 0.0f, 0.0f, 1.0f}};

/**
 * @brief Evaluates 1D cubic Bezier polynomial.
 */
static float sample_curve(float p1, float p2, float t) {
  /* B(t) = 3*(1-t)^2 * t * p1 + 3*(1-t) * t^2 * p2 + t^3 */
  float one_minus_t;
  one_minus_t = 1.0f - t;
  return 3.0f * one_minus_t * one_minus_t * t * p1 +
         3.0f * one_minus_t * t * t * p2 + t * t * t;
}

/**
 * @brief Solves for t given x on a cubic Bezier curve using binary bisection.
 */
static float solve_curve_t(float x1, float x2, float target_x) {
  float t_low;
  float t_high;
  float t;
  int iter;

  t_low = 0.0f;
  t_high = 1.0f;
  t = target_x;

  for (iter = 0; iter < 14; iter++) {
    float current_x;
    current_x = sample_curve(x1, x2, t);
    if (target_x > current_x) {
      t_low = t;
    } else {
      t_high = t;
    }
    t = (t_high + t_low) * 0.5f;
  }

  return t;
}

/**
 * @brief Retrieves the cubic Bezier control points for a designated Material 3
 * motion curve.
 *
 * @param curve The motion curve type.
 * @param out_bezier Pointer to receive the control point coordinates.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t md3_motion_get_curve(enum md3_motion_curve curve,
                                struct md3_cubic_bezier *out_bezier) {
  if (!out_bezier || (unsigned)curve >= MD3_MOTION_CURVE_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_bezier = g_md3_curves[curve];
  return UI_ERROR_NONE;
}

/**
 * @brief Evaluates an easing curve value at normalized time progress t in
 * [0.0, 1.0].
 *
 * @param curve The motion curve type.
 * @param progress Time progress normalized in [0.0, 1.0].
 * @param out_value Pointer to receive the eased value.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t md3_motion_cubic_bezier_evaluate(enum md3_motion_curve curve,
                                            float progress, float *out_value) {
  struct md3_cubic_bezier bez;
  float t;

  if (!out_value || (unsigned)curve >= MD3_MOTION_CURVE_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (progress <= 0.0f) {
    *out_value = 0.0f;
    return UI_ERROR_NONE;
  }
  if (progress >= 1.0f) {
    *out_value = 1.0f;
    return UI_ERROR_NONE;
  }

  bez = g_md3_curves[curve];
  t = solve_curve_t(bez.x1, bez.x2, progress);
  *out_value = sample_curve(bez.y1, bez.y2, t);
  return UI_ERROR_NONE;
}
