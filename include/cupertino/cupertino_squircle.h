/**
 * @file cupertino_squircle.h
 * @brief Cupertino Squircle geometry and continuous corner curvature APIs.
 */

#ifndef CUPERTINO_CUPERTINO_SQUIRCLE_H
#define CUPERTINO_CUPERTINO_SQUIRCLE_H

/* clang-format off */
#include "ui_error.h"
#include "ui_types.h"
#include "ui_renderer.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Apple continuous curve superellipse parameter (n ~ 4.7).
 */
#define CUPERTINO_SQUIRCLE_DEFAULT_N 4.7f

/**
 * @brief Calculates a 2D Cartesian point on a Lamé superellipse at a given
 * angle.
 *
 * Evaluates the equation (|x/a|^n + |y/b|^n = 1).
 *
 * @param a Semi-major axis length (must be > 0).
 * @param b Semi-minor axis length (must be > 0).
 * @param n Superellipse exponent parameter (must be > 0).
 * @param theta Polar angle in radians.
 * @param out_x Pointer to receive the calculated x coordinate.
 * @param out_y Pointer to receive the calculated y coordinate.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_squircle_point_at(
    float a, float b, float n, float theta, float *out_x, float *out_y);

/**
 * @brief Computes dynamic inner border radius for concentric continuous
 * borders.
 *
 * Implements R_inner = max(0, R_outer - stroke_width).
 *
 * @param outer_radius Outer corner radius in points.
 * @param stroke_width Border stroke thickness in points.
 * @param out_inner_radius Pointer to receive calculated inner radius.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_squircle_inset(
    float outer_radius, float stroke_width, float *out_inner_radius);

/**
 * @brief Resolves hairline border stroke width based on display density scale.
 *
 * 1x displays: 1.0pt.
 * 2x Retina displays: 0.5pt.
 * 3x Super Retina displays: 0.33333334pt.
 *
 * @param scale Device display scale factor (e.g., 1.0, 2.0, 3.0).
 * @param out_width Pointer to receive resolved hairline width in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_hairline_width(float scale, float *out_width);

/**
 * @brief Snaps a layout coordinate to physical device pixel boundaries.
 *
 * For odd stroke widths, coordinates are aligned to half-pixel offsets to
 * avoid anti-aliasing blur.
 *
 * @param pt Coordinate value in points.
 * @param scale Device display scale factor (must be > 0).
 * @param stroke_width Border stroke thickness in points (must be >= 0).
 * @param out_snapped Pointer to receive pixel-snapped point coordinate.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_snap_to_device_pixel(
    float pt, float scale, float stroke_width, float *out_snapped);

/**
 * @brief Initializes a vector path structure with pre-allocated command
 * capacity.
 *
 * @param path Pointer to struct ui_path to initialize.
 * @param initial_capacity Initial capacity for path commands (must be > 0).
 * @return UI_ERROR_NONE on success, or UI_ERROR_OUT_OF_MEMORY /
 * UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_path_init(struct ui_path *path, int initial_capacity);

/**
 * @brief Frees resources allocated for a vector path structure.
 *
 * @param path Pointer to struct ui_path to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_path_destroy(struct ui_path *path);

/**
 * @brief Appends a move-to command to a vector path.
 *
 * @param path Target path.
 * @param x Target X coordinate.
 * @param y Target Y coordinate.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_path_move_to(struct ui_path *path, float x, float y);

/**
 * @brief Appends a line-to command to a vector path.
 *
 * @param path Target path.
 * @param x Target X coordinate.
 * @param y Target Y coordinate.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_path_line_to(struct ui_path *path, float x, float y);

/**
 * @brief Appends a cubic Bézier curve command to a vector path.
 *
 * @param path Target path.
 * @param x1 First control point X.
 * @param y1 First control point Y.
 * @param x2 Second control point X.
 * @param y2 Second control point Y.
 * @param x3 End point X.
 * @param y3 End point Y.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_path_bezier_to(struct ui_path *path, float x1, float y1, float x2,
                         float y2, float x3, float y3);

/**
 * @brief Appends a close-path command to a vector path.
 *
 * @param path Target path.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_path_close(struct ui_path *path);

/**
 * @brief Generates a closed G2 curvature-continuous squircle path.
 *
 * Creates a rectangle with Apple continuous corners using cubic Bézier
 * segments.
 *
 * @param x Rectangle origin X.
 * @param y Rectangle origin Y.
 * @param width Rectangle width (must be > 0).
 * @param height Rectangle height (must be > 0).
 * @param corner_radius Corner radius in points (must be >= 0).
 * @param smoothness Smoothness factor in [0.0, 1.0] (0 = circular arc, 1 = full
 * continuous squircle).
 * @param out_path Destination path to append commands to.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_squircle_generate_path(
    float x, float y, float width, float height, float corner_radius,
    float smoothness, struct ui_path *out_path);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_SQUIRCLE_H */
