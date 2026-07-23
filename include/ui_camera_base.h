#ifndef UI_CAMERA_BASE_H
#define UI_CAMERA_BASE_H

/* clang-format off */
#include "ui_component.h"
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_camera_base;

/**
 * @brief Represents the current state of the camera.
 */
enum ui_camera_state {
  UI_CAMERA_STATE_UNINITIALIZED,
  UI_CAMERA_STATE_REQUESTING_PERMISSION,
  UI_CAMERA_STATE_PERMISSION_DENIED,
  UI_CAMERA_STATE_READY,
  UI_CAMERA_STATE_STREAMING,
  UI_CAMERA_STATE_ERROR
};

/**
 * @brief Frame callback for zero-copy texture streaming or CPU processing.
 */
typedef enum ui_error (*ui_camera_frame_callback)(struct ui_camera_base *camera,
                                                  const void *frame_data,
                                                  size_t size, int width,
                                                  int height, void *user_data);

/**
 * @brief Creates a new unstyled camera base component.
 *
 * @param out_camera Pointer to output the initialized camera component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_camera_base_create(struct ui_camera_base **out_camera);

/**
 * @brief Destroys a camera base component.
 *
 * @param camera The camera component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_camera_base_destroy(struct ui_camera_base *camera);

/**
 * @brief Retrieves the base component.
 *
 * @param camera The camera component.
 * @return The base component.
 */
enum ui_error ui_camera_base_get_component(struct ui_camera_base *camera,
                                           struct ui_component **out_component);

/**
 * @brief Requests camera permissions asynchronously.
 *
 * @param camera The camera component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_camera_base_request_permission(struct ui_camera_base *camera);

/**
 * @brief Starts the camera stream.
 *
 * @param camera The camera component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_camera_base_start_stream(struct ui_camera_base *camera);

/**
 * @brief Stops the camera stream.
 *
 * @param camera The camera component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_camera_base_stop_stream(struct ui_camera_base *camera);

/**
 * @brief Gets the current state of the camera.
 *
 * @param camera The camera component.
 * @param out_state Pointer to receive the state.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_camera_base_get_state(struct ui_camera_base *camera,
                                       enum ui_camera_state *out_state);

/**
 * @brief Registers a callback for receiving frame data.
 *
 * @param camera The camera component.
 * @param callback The callback function.
 * @param user_data User data to pass to the callback.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error
ui_camera_base_set_frame_callback(struct ui_camera_base *camera,
                                  ui_camera_frame_callback callback,
                                  void *user_data);

/**
 * @brief Mocks an incoming frame (for testing).
 *
 * @param camera The camera component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_camera_base_mock_frame(struct ui_camera_base *camera);

/**
 * @brief Mocks a permission response (for testing).
 *
 * @param camera The camera component.
 * @param granted Non-zero to simulate permission granted, zero for denied.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error
ui_camera_base_mock_permission_response(struct ui_camera_base *camera,
                                        int granted);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CAMERA_BASE_H */
