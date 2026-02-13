#ifndef CMP_CAMERA_H
#define CMP_CAMERA_H

/**
 * @file cmp_camera.h
 * @brief Camera plugin helpers for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_env.h"

/**
 * @brief Camera session configuration.
 */
typedef struct CMPCameraSessionConfig {
  CMPEnv *env; /**< Environment backend used to acquire the camera interface. */
  CMPCameraConfig config; /**< Backend camera configuration. */
} CMPCameraSessionConfig;

/**
 * @brief Camera session state.
 */
typedef struct CMPCameraSession {
  CMPCamera camera;  /**< Camera interface handle. */
  cmp_u32 camera_id; /**< Backend camera identifier. */
  cmp_u32 facing;    /**< Requested camera facing (CMP_CAMERA_FACING_*). */
  cmp_u32 width;     /**< Requested frame width in pixels. */
  cmp_u32 height;    /**< Requested frame height in pixels. */
  cmp_u32 format;    /**< Requested pixel format (CMP_CAMERA_FORMAT_*). */
  CMPBool opened;    /**< CMP_TRUE when the camera is opened. */
  CMPBool streaming; /**< CMP_TRUE when streaming is active. */
} CMPCameraSession;

/**
 * @brief Initialize camera configuration defaults.
 * @param config Configuration to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_camera_config_init(CMPCameraSessionConfig *config);

/**
 * @brief Open a camera session using the supplied configuration.
 * @param session Session to initialize.
 * @param config Camera configuration.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_camera_init(CMPCameraSession *session,
                                  const CMPCameraSessionConfig *config);

/**
 * @brief Stop streaming (if needed) and close the camera session.
 * @param session Session to shut down.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_camera_shutdown(CMPCameraSession *session);

/**
 * @brief Start streaming frames from an opened camera session.
 * @param session Camera session.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_camera_start(CMPCameraSession *session);

/**
 * @brief Stop streaming frames from an opened camera session.
 * @param session Camera session.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_camera_stop(CMPCameraSession *session);

/**
 * @brief Read the latest frame from a streaming camera.
 * @param session Camera session.
 * @param out_frame Receives the frame description.
 * @param out_has_frame Receives CMP_TRUE if a frame was read.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_camera_read_frame(CMPCameraSession *session,
                                        CMPCameraFrame *out_frame,
                                        CMPBool *out_has_frame);

/**
 * @brief Copy a camera frame into a caller-provided buffer.
 * @param frame Frame to copy.
 * @param dst Destination buffer (may be NULL when frame->size is 0).
 * @param dst_capacity Size of the destination buffer in bytes.
 * @param out_size Receives the number of bytes copied.
 * @return CMP_OK on success, CMP_ERR_RANGE if the buffer is too small, or a
 * failure code.
 */
CMP_API int CMP_CALL cmp_camera_copy_frame(const CMPCameraFrame *frame, void *dst,
                                        cmp_usize dst_capacity,
                                        cmp_usize *out_size);

#ifdef CMP_TESTING
/**
 * @brief Test hook to bypass camera vtable completeness checks.
 * @param enable CMP_TRUE to bypass, CMP_FALSE to enforce normal checks.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_camera_test_set_skip_vtable_check(CMPBool enable);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_CAMERA_H */
