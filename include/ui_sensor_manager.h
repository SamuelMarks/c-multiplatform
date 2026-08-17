#ifndef UI_SENSOR_MANAGER_H
#define UI_SENSOR_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_geometry.h"
#include "ui_signal.h"
/* clang-format on */

/**
 * @struct ui_sensor_vector3
 * @brief Represents raw 3-axis sensor data (e.g., from accelerometer or
 * magnetometer).
 */
struct ui_sensor_vector3 {
  /** @brief X-axis value. */
  double x;
  /** @brief Y-axis value. */
  double y;
  /** @brief Z-axis value. */
  double z;
};

/**
 * @struct ui_sensor_quaternion
 * @brief Represents orientation as a smoothed quaternion.
 */
struct ui_sensor_quaternion {
  /** @brief X component of the quaternion. */
  double x;
  /** @brief Y component of the quaternion. */
  double y;
  /** @brief Z component of the quaternion. */
  double z;
  /** @brief W (real) component of the quaternion. */
  double w;
};

/**
 * @struct ui_sensor_manager
 * @brief Opaque handle to the sensor manager subsystem.
 */
struct ui_sensor_manager;

/**
 * @brief Creates a new hardware sensor manager instance.
 *
 * @param out_manager Pointer to receive the manager instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_sensor_manager_create(struct ui_sensor_manager **out_manager);

/**
 * @brief Destroys the sensor manager and releases hardware handles.
 *
 * @param manager The manager instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_sensor_manager_destroy(struct ui_sensor_manager *manager);

/**
 * @brief Starts polling hardware sensors and computing smoothed orientation
 * logic.
 *
 * @param manager The manager instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_sensor_manager_start(struct ui_sensor_manager *manager);

/**
 * @brief Stops polling hardware sensors to conserve battery.
 *
 * @param manager The manager instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_sensor_manager_stop(struct ui_sensor_manager *manager);

/**
 * @brief Binds a reactive signal to the smoothed quaternion output.
 *        The signal's value will be a pointer to a struct ui_sensor_quaternion.
 *
 * @param manager The manager instance.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_sensor_manager_bind_orientation(struct ui_sensor_manager *manager,
                                              struct ui_signal *signal);

/**
 * @brief Retrieves the latest raw accelerometer vector data.
 *
 * @param manager The manager instance.
 * @param out_accel Pointer to receive the vector data.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_sensor_manager_get_accelerometer(struct ui_sensor_manager *manager,
                                    struct ui_sensor_vector3 *out_accel);

/**
 * @brief Retrieves the latest raw gyroscope vector data.
 *
 * @param manager The manager instance.
 * @param out_gyro Pointer to receive the vector data.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_sensor_manager_get_gyroscope(struct ui_sensor_manager *manager,
                                           struct ui_sensor_vector3 *out_gyro);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SENSOR_MANAGER_H */
