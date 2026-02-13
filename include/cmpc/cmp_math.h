#ifndef CMP_MATH_H
#define CMP_MATH_H

/**
 * @file cmp_math.h
 * @brief Math and geometry ABI for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"

/** @brief Scalar type used for geometry and transforms. */
typedef float CMPScalar;

/**
 * @brief 2D vector.
 */
typedef struct CMPVec2 {
  CMPScalar x; /**< X component. */
  CMPScalar y; /**< Y component. */
} CMPVec2;

/**
 * @brief 3D vector.
 */
typedef struct CMPVec3 {
  CMPScalar x; /**< X component. */
  CMPScalar y; /**< Y component. */
  CMPScalar z; /**< Z component. */
} CMPVec3;

/**
 * @brief Width/height pair.
 */
typedef struct CMPSize {
  CMPScalar width;  /**< Width component. */
  CMPScalar height; /**< Height component. */
} CMPSize;

/**
 * @brief Axis-aligned rectangle.
 */
typedef struct CMPRect {
  CMPScalar x;      /**< X coordinate of the top-left corner. */
  CMPScalar y;      /**< Y coordinate of the top-left corner. */
  CMPScalar width;  /**< Width of the rectangle. */
  CMPScalar height; /**< Height of the rectangle. */
} CMPRect;

/**
 * @brief 3x3 matrix in column-major order.
 */
typedef struct CMPMat3 {
  CMPScalar m[9]; /**< Column-major elements. */
} CMPMat3;

/**
 * @brief Compute the intersection of two rectangles.
 * @param a First rectangle.
 * @param b Second rectangle.
 * @param out_rect Receives the intersection rectangle.
 * @param out_has_intersection Receives CMP_TRUE if intersection exists.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_rect_intersect(const CMPRect *a, const CMPRect *b,
                                        CMPRect *out_rect,
                                        CMPBool *out_has_intersection);

/**
 * @brief Compute the union of two rectangles.
 * @param a First rectangle.
 * @param b Second rectangle.
 * @param out_rect Receives the union rectangle.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_rect_union(const CMPRect *a, const CMPRect *b,
                                    CMPRect *out_rect);

/**
 * @brief Test whether a point lies inside a rectangle.
 * @param rect Rectangle to test.
 * @param x Point X coordinate.
 * @param y Point Y coordinate.
 * @param out_contains Receives CMP_TRUE if the point is inside.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_rect_contains_point(const CMPRect *rect, CMPScalar x,
                                             CMPScalar y,
                                             CMPBool *out_contains);

/**
 * @brief Create an identity matrix.
 * @param out_mat Receives the identity matrix.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_mat3_identity(CMPMat3 *out_mat);

/**
 * @brief Multiply two 3x3 matrices.
 * @param a Left matrix.
 * @param b Right matrix.
 * @param out_mat Receives the product matrix.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_mat3_mul(const CMPMat3 *a, const CMPMat3 *b,
                                  CMPMat3 *out_mat);

/**
 * @brief Create a translation matrix.
 * @param tx Translation along X.
 * @param ty Translation along Y.
 * @param out_mat Receives the translation matrix.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_mat3_translate(CMPScalar tx, CMPScalar ty,
                                        CMPMat3 *out_mat);

/**
 * @brief Create a scale matrix.
 * @param sx Scale factor along X.
 * @param sy Scale factor along Y.
 * @param out_mat Receives the scale matrix.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_mat3_scale(CMPScalar sx, CMPScalar sy,
                                    CMPMat3 *out_mat);

/**
 * @brief Create a rotation matrix.
 * @param radians Rotation in radians (counter-clockwise).
 * @param out_mat Receives the rotation matrix.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_mat3_rotate(CMPScalar radians, CMPMat3 *out_mat);

/**
 * @brief Transform a 2D point by a matrix.
 * @param m Transform matrix.
 * @param x Input X coordinate.
 * @param y Input Y coordinate.
 * @param out_x Receives the transformed X coordinate.
 * @param out_y Receives the transformed Y coordinate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_mat3_transform_point(const CMPMat3 *m, CMPScalar x,
                                              CMPScalar y, CMPScalar *out_x,
                                              CMPScalar *out_y);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_MATH_H */
