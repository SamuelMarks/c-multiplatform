#ifndef CMP_PATH_H
#define CMP_PATH_H

/**
 * @file cmp_path.h
 * @brief Vector path recording for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_gfx.h"

/**
 * @brief Initialize a path.
 * @param path Path to initialize.
 * @param allocator Allocator to use; NULL uses the default allocator.
 * @param initial_capacity Initial command capacity (0 selects a default).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_path_init(CMPPath *path, const CMPAllocator *allocator,
                                   cmp_usize initial_capacity);

/**
 * @brief Reset a path without freeing its storage.
 * @param path Path to reset.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_path_reset(CMPPath *path);

/**
 * @brief Shut down a path and release its storage.
 * @param path Path to shut down.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_path_shutdown(CMPPath *path);

/**
 * @brief Append a move-to command to the path.
 * @param path Path to update.
 * @param x Target X coordinate.
 * @param y Target Y coordinate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_path_move_to(CMPPath *path, CMPScalar x, CMPScalar y);

/**
 * @brief Append a line-to command to the path.
 * @param path Path to update.
 * @param x Target X coordinate.
 * @param y Target Y coordinate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_path_line_to(CMPPath *path, CMPScalar x, CMPScalar y);

/**
 * @brief Append a quadratic curve-to command to the path.
 * @param path Path to update.
 * @param cx Control point X coordinate.
 * @param cy Control point Y coordinate.
 * @param x Target X coordinate.
 * @param y Target Y coordinate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_path_quad_to(CMPPath *path, CMPScalar cx, CMPScalar cy,
                                      CMPScalar x, CMPScalar y);

/**
 * @brief Append a cubic curve-to command to the path.
 * @param path Path to update.
 * @param cx1 First control point X coordinate.
 * @param cy1 First control point Y coordinate.
 * @param cx2 Second control point X coordinate.
 * @param cy2 Second control point Y coordinate.
 * @param x Target X coordinate.
 * @param y Target Y coordinate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_path_cubic_to(CMPPath *path, CMPScalar cx1,
                                       CMPScalar cy1, CMPScalar cx2,
                                       CMPScalar cy2, CMPScalar x, CMPScalar y);

/**
 * @brief Append a close-path command to the path.
 * @param path Path to update.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_path_close(CMPPath *path);

#ifdef CMP_TESTING
/**
 * @brief Test wrapper for overflow-safe addition.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives the sum.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_path_test_add_overflow(cmp_usize a, cmp_usize b,
                                                cmp_usize *out_value);

/**
 * @brief Test wrapper for overflow-safe multiplication.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives the product.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_path_test_mul_overflow(cmp_usize a, cmp_usize b,
                                                cmp_usize *out_value);

/**
 * @brief Test wrapper for current-point detection.
 * @param path Path instance.
 * @param out_has_current Receives CMP_TRUE if a current point exists.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_path_test_has_current(const CMPPath *path,
                                               CMPBool *out_has_current);

/**
 * @brief Test control for forcing reserve growth paths.
 * @param enable CMP_TRUE to force growth logic even when capacity is
 * sufficient.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_path_test_set_force_reserve(CMPBool enable);

/**
 * @brief Test wrapper for reserving path command storage.
 * @param path Path instance.
 * @param additional Additional command slots required.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_path_test_reserve(CMPPath *path, cmp_usize additional);

/**
 * @brief Test wrapper for appending a raw path command.
 * @param path Path instance.
 * @param cmd Command to append.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_path_test_append(CMPPath *path, const CMPPathCmd *cmd);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_PATH_H */
