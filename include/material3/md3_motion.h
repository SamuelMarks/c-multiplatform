/**
 * @file md3_motion.h
 * @brief Material 3 Baseline Motion Curves and Easing functions.
 */

#ifndef MATERIAL3_MD3_MOTION_H
#define MATERIAL3_MD3_MOTION_H

/* clang-format off */
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum md3_motion_curve
 * @brief Material 3 Baseline Cubic-Bezier Easing Curves.
 */
enum md3_motion_curve {
  MD3_MOTION_CURVE_EMPHASIZED = 0,
  MD3_MOTION_CURVE_EMPHASIZED_ACCELERATE,
  MD3_MOTION_CURVE_EMPHASIZED_DECELERATE,
  MD3_MOTION_CURVE_STANDARD,
  MD3_MOTION_CURVE_STANDARD_ACCELERATE,
  MD3_MOTION_CURVE_STANDARD_DECELERATE,
  MD3_MOTION_CURVE_COUNT
};

/**
 * @struct md3_cubic_bezier
 * @brief Control points for a cubic Bezier curve (x1, y1, x2, y2).
 */
struct md3_cubic_bezier {
  float x1;
  float y1;
  float x2;
  float y2;
};

/**
 * @brief Retrieves the cubic Bezier control points for a designated Material 3
 * motion curve.
 *
 * @param curve The motion curve type.
 * @param out_bezier Pointer to receive the control point coordinates.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_motion_get_curve(
    enum md3_motion_curve curve, struct md3_cubic_bezier *out_bezier);

/**
 * @brief Evaluates an easing curve value at normalized time progress t in
 * [0.0, 1.0].
 *
 * @param curve The motion curve type.
 * @param progress Time progress normalized in [0.0, 1.0].
 * @param out_value Pointer to receive the eased value.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_motion_cubic_bezier_evaluate(
    enum md3_motion_curve curve, float progress, float *out_value);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL3_MD3_MOTION_H */
