#ifndef M3_PATH_H
#define M3_PATH_H

/**
 * @file m3_path.h
 * @brief Vector path recording for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_gfx.h"

/**
 * @brief Initialize a path.
 * @param path Path to initialize.
 * @param allocator Allocator to use; NULL uses the default allocator.
 * @param initial_capacity Initial command capacity (0 selects a default).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_path_init(M3Path *path, const M3Allocator *allocator,
                                m3_usize initial_capacity);

/**
 * @brief Reset a path without freeing its storage.
 * @param path Path to reset.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_path_reset(M3Path *path);

/**
 * @brief Shut down a path and release its storage.
 * @param path Path to shut down.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_path_shutdown(M3Path *path);

/**
 * @brief Append a move-to command to the path.
 * @param path Path to update.
 * @param x Target X coordinate.
 * @param y Target Y coordinate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_path_move_to(M3Path *path, M3Scalar x, M3Scalar y);

/**
 * @brief Append a line-to command to the path.
 * @param path Path to update.
 * @param x Target X coordinate.
 * @param y Target Y coordinate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_path_line_to(M3Path *path, M3Scalar x, M3Scalar y);

/**
 * @brief Append a quadratic curve-to command to the path.
 * @param path Path to update.
 * @param cx Control point X coordinate.
 * @param cy Control point Y coordinate.
 * @param x Target X coordinate.
 * @param y Target Y coordinate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_path_quad_to(M3Path *path, M3Scalar cx, M3Scalar cy,
                                   M3Scalar x, M3Scalar y);

/**
 * @brief Append a cubic curve-to command to the path.
 * @param path Path to update.
 * @param cx1 First control point X coordinate.
 * @param cy1 First control point Y coordinate.
 * @param cx2 Second control point X coordinate.
 * @param cy2 Second control point Y coordinate.
 * @param x Target X coordinate.
 * @param y Target Y coordinate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_path_cubic_to(M3Path *path, M3Scalar cx1, M3Scalar cy1,
                                    M3Scalar cx2, M3Scalar cy2, M3Scalar x,
                                    M3Scalar y);

/**
 * @brief Append a close-path command to the path.
 * @param path Path to update.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_path_close(M3Path *path);

#ifdef M3_TESTING
/**
 * @brief Test wrapper for overflow-safe addition.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives the sum.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_path_test_add_overflow(m3_usize a, m3_usize b,
                                             m3_usize *out_value);

/**
 * @brief Test wrapper for overflow-safe multiplication.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives the product.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_path_test_mul_overflow(m3_usize a, m3_usize b,
                                             m3_usize *out_value);

/**
 * @brief Test wrapper for current-point detection.
 * @param path Path instance.
 * @param out_has_current Receives M3_TRUE if a current point exists.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_path_test_has_current(const M3Path *path,
                                            M3Bool *out_has_current);

/**
 * @brief Test control for forcing reserve growth paths.
 * @param enable M3_TRUE to force growth logic even when capacity is sufficient.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_path_test_set_force_reserve(M3Bool enable);

/**
 * @brief Test wrapper for reserving path command storage.
 * @param path Path instance.
 * @param additional Additional command slots required.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_path_test_reserve(M3Path *path, m3_usize additional);

/**
 * @brief Test wrapper for appending a raw path command.
 * @param path Path instance.
 * @param cmd Command to append.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_path_test_append(M3Path *path, const M3PathCmd *cmd);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_PATH_H */
