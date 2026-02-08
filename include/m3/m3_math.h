#ifndef M3_MATH_H
#define M3_MATH_H

/**
 * @file m3_math.h
 * @brief Math and geometry ABI for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_core.h"

/** @brief Scalar type used for geometry and transforms. */
typedef float M3Scalar;

/**
 * @brief 2D vector.
 */
typedef struct M3Vec2 {
    M3Scalar x; /**< X component. */
    M3Scalar y; /**< Y component. */
} M3Vec2;

/**
 * @brief 3D vector.
 */
typedef struct M3Vec3 {
    M3Scalar x; /**< X component. */
    M3Scalar y; /**< Y component. */
    M3Scalar z; /**< Z component. */
} M3Vec3;

/**
 * @brief Width/height pair.
 */
typedef struct M3Size {
    M3Scalar width; /**< Width component. */
    M3Scalar height; /**< Height component. */
} M3Size;

/**
 * @brief Axis-aligned rectangle.
 */
typedef struct M3Rect {
    M3Scalar x; /**< X coordinate of the top-left corner. */
    M3Scalar y; /**< Y coordinate of the top-left corner. */
    M3Scalar width; /**< Width of the rectangle. */
    M3Scalar height; /**< Height of the rectangle. */
} M3Rect;

/**
 * @brief 3x3 matrix in column-major order.
 */
typedef struct M3Mat3 {
    M3Scalar m[9]; /**< Column-major elements. */
} M3Mat3;

/**
 * @brief Compute the intersection of two rectangles.
 * @param a First rectangle.
 * @param b Second rectangle.
 * @param out_rect Receives the intersection rectangle.
 * @param out_has_intersection Receives M3_TRUE if intersection exists.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_rect_intersect(const M3Rect *a, const M3Rect *b, M3Rect *out_rect, M3Bool *out_has_intersection);

/**
 * @brief Compute the union of two rectangles.
 * @param a First rectangle.
 * @param b Second rectangle.
 * @param out_rect Receives the union rectangle.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_rect_union(const M3Rect *a, const M3Rect *b, M3Rect *out_rect);

/**
 * @brief Test whether a point lies inside a rectangle.
 * @param rect Rectangle to test.
 * @param x Point X coordinate.
 * @param y Point Y coordinate.
 * @param out_contains Receives M3_TRUE if the point is inside.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_rect_contains_point(const M3Rect *rect, M3Scalar x, M3Scalar y, M3Bool *out_contains);

/**
 * @brief Create an identity matrix.
 * @param out_mat Receives the identity matrix.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_mat3_identity(M3Mat3 *out_mat);

/**
 * @brief Multiply two 3x3 matrices.
 * @param a Left matrix.
 * @param b Right matrix.
 * @param out_mat Receives the product matrix.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_mat3_mul(const M3Mat3 *a, const M3Mat3 *b, M3Mat3 *out_mat);

/**
 * @brief Create a translation matrix.
 * @param tx Translation along X.
 * @param ty Translation along Y.
 * @param out_mat Receives the translation matrix.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_mat3_translate(M3Scalar tx, M3Scalar ty, M3Mat3 *out_mat);

/**
 * @brief Create a scale matrix.
 * @param sx Scale factor along X.
 * @param sy Scale factor along Y.
 * @param out_mat Receives the scale matrix.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_mat3_scale(M3Scalar sx, M3Scalar sy, M3Mat3 *out_mat);

/**
 * @brief Create a rotation matrix.
 * @param radians Rotation in radians (counter-clockwise).
 * @param out_mat Receives the rotation matrix.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_mat3_rotate(M3Scalar radians, M3Mat3 *out_mat);

/**
 * @brief Transform a 2D point by a matrix.
 * @param m Transform matrix.
 * @param x Input X coordinate.
 * @param y Input Y coordinate.
 * @param out_x Receives the transformed X coordinate.
 * @param out_y Receives the transformed Y coordinate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_mat3_transform_point(const M3Mat3 *m, M3Scalar x, M3Scalar y, M3Scalar *out_x, M3Scalar *out_y);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_MATH_H */
