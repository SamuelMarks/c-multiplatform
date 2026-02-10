#ifndef M3_CAMERA_H
#define M3_CAMERA_H

/**
 * @file m3_camera.h
 * @brief Camera plugin helpers for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_env.h"

/**
 * @brief Camera session configuration.
 */
typedef struct M3CameraSessionConfig {
    M3Env *env; /**< Environment backend used to acquire the camera interface. */
    M3CameraConfig config; /**< Backend camera configuration. */
} M3CameraSessionConfig;

/**
 * @brief Camera session state.
 */
typedef struct M3CameraSession {
    M3Camera camera; /**< Camera interface handle. */
    m3_u32 camera_id; /**< Backend camera identifier. */
    m3_u32 facing; /**< Requested camera facing (M3_CAMERA_FACING_*). */
    m3_u32 width; /**< Requested frame width in pixels. */
    m3_u32 height; /**< Requested frame height in pixels. */
    m3_u32 format; /**< Requested pixel format (M3_CAMERA_FORMAT_*). */
    M3Bool opened; /**< M3_TRUE when the camera is opened. */
    M3Bool streaming; /**< M3_TRUE when streaming is active. */
} M3CameraSession;

/**
 * @brief Initialize camera configuration defaults.
 * @param config Configuration to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_camera_config_init(M3CameraSessionConfig *config);

/**
 * @brief Open a camera session using the supplied configuration.
 * @param session Session to initialize.
 * @param config Camera configuration.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_camera_init(M3CameraSession *session, const M3CameraSessionConfig *config);

/**
 * @brief Stop streaming (if needed) and close the camera session.
 * @param session Session to shut down.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_camera_shutdown(M3CameraSession *session);

/**
 * @brief Start streaming frames from an opened camera session.
 * @param session Camera session.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_camera_start(M3CameraSession *session);

/**
 * @brief Stop streaming frames from an opened camera session.
 * @param session Camera session.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_camera_stop(M3CameraSession *session);

/**
 * @brief Read the latest frame from a streaming camera.
 * @param session Camera session.
 * @param out_frame Receives the frame description.
 * @param out_has_frame Receives M3_TRUE if a frame was read.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_camera_read_frame(M3CameraSession *session, M3CameraFrame *out_frame, M3Bool *out_has_frame);

/**
 * @brief Copy a camera frame into a caller-provided buffer.
 * @param frame Frame to copy.
 * @param dst Destination buffer (may be NULL when frame->size is 0).
 * @param dst_capacity Size of the destination buffer in bytes.
 * @param out_size Receives the number of bytes copied.
 * @return M3_OK on success, M3_ERR_RANGE if the buffer is too small, or a failure code.
 */
M3_API int M3_CALL m3_camera_copy_frame(const M3CameraFrame *frame, void *dst, m3_usize dst_capacity, m3_usize *out_size);

#ifdef M3_TESTING
/**
 * @brief Test hook to bypass camera vtable completeness checks.
 * @param enable M3_TRUE to bypass, M3_FALSE to enforce normal checks.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_camera_test_set_skip_vtable_check(M3Bool enable);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_CAMERA_H */
