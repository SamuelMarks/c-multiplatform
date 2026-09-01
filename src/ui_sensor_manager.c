/**
 * @file ui_sensor_manager.c
 * @brief ui_sensor_manager.c implementation.
 */
/* clang-format off */
#include "../include/ui_sensor_manager.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

/*
 * \file ui_sensor_manager.c
 * \brief Sensor manager implementation.
 */

/**
 * @struct ui_sensor_manager
 * \brief ui_sensor_manager structure.
 * \details Internal state for the sensor manager.
 */
struct ui_sensor_manager {
  int is_running;                           /**< is_running */
  struct ui_signal *orientation_signal;     /**< orientation_signal */
  struct ui_sensor_vector3 accel_data;      /**< accel_data */
  struct ui_sensor_vector3 gyro_data;       /**< gyro_data */
  struct ui_sensor_quaternion current_quat; /**< current_quat */
};

/**
 * \brief Creates a new sensor manager.
 * \param out_manager Pointer to store the manager.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_sensor_manager_create(struct ui_sensor_manager **out_manager) {
  struct ui_sensor_manager *mgr = NULL;

  if (!out_manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  mgr = (struct ui_sensor_manager *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_sensor_manager));
  if (!mgr) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(mgr, 0, sizeof(struct ui_sensor_manager));

  /* Initialize identity quaternion */
  mgr->current_quat.w = 1.0;
  mgr->current_quat.x = 0.0;
  mgr->current_quat.y = 0.0;
  mgr->current_quat.z = 0.0;

  *out_manager = mgr;
  return UI_ERROR_NONE;
}

/**
 * \brief Destroys a sensor manager.
 * \param manager The manager to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_sensor_manager_destroy(struct ui_sensor_manager *manager) {
  if (!manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (manager->is_running) {
    {
      ui_error_t rc_cleanup = ui_sensor_manager_stop(manager);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  /* Unbind from signals if necessary */

  C_MULTIPLATFORM_FREE(manager);
  return UI_ERROR_NONE;
}

/**
 * \brief Starts the sensor manager.
 * \param manager The manager to start.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_sensor_manager_start(struct ui_sensor_manager *manager) {
  if (!manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (manager->is_running) {
    return UI_ERROR_NONE;
  }

  manager->is_running = 1;

  /* Mock start logic. Real implementation would bind to CoreMotion / Android
   * SensorManager */
  /* Here we would typically register callbacks or start a polling timer */

  return UI_ERROR_NONE;
}

/**
 * \brief Stops the sensor manager.
 * \param manager The manager to stop.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_sensor_manager_stop(struct ui_sensor_manager *manager) {
  if (!manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!manager->is_running) {
    return UI_ERROR_NONE;
  }

  manager->is_running = 0;

  /* Mock stop logic. */

  return UI_ERROR_NONE;
}

/**
 * \brief Binds a signal for orientation updates.
 * \param manager The sensor manager.
 * \param signal The signal to bind.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_sensor_manager_bind_orientation(struct ui_sensor_manager *manager,
                                              struct ui_signal *signal) {
  if (!manager || !signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  manager->orientation_signal = signal;
  return UI_ERROR_NONE;
}

/**
 * \brief Gets the current accelerometer data.
 * \param manager The sensor manager.
 * \param out_accel Pointer to store the vector.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_sensor_manager_get_accelerometer(struct ui_sensor_manager *manager,
                                    struct ui_sensor_vector3 *out_accel) {
  if (!manager || !out_accel) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_accel = manager->accel_data;
  return UI_ERROR_NONE;
}

/**
 * \brief Gets the current gyroscope data.
 * \param manager The sensor manager.
 * \param out_gyro Pointer to store the vector.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_sensor_manager_get_gyroscope(struct ui_sensor_manager *manager,
                                           struct ui_sensor_vector3 *out_gyro) {
  if (!manager || !out_gyro) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_gyro = manager->gyro_data;
  return UI_ERROR_NONE;
}

/* Forward declarations */
/**
 * \brief Mocks a tick for the sensor manager.
 * \param manager The manager.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_sensor_manager_tick_mock(struct ui_sensor_manager *manager);

/**
 * \brief Mocks a tick for the sensor manager.
 * \param manager The manager.
 * \return UI_ERROR_NONE on success.
 */
/**
 * @brief ui_sensor_manager_tick_mock.
 * @param manager Parameter manager.
 * @return Return value.
 */
ui_error_t ui_sensor_manager_tick_mock(struct ui_sensor_manager *manager) {
  if (!manager || !manager->is_running) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Just a small mock rotation and slight acceleration noise */
  manager->accel_data.y = 9.81;
  manager->gyro_data.x = 0.01;

  /* Simple euler to quat mock rotation update over time would go here */

  if (manager->orientation_signal) {
    union ui_signal_payload payload;
    payload.ptr_val = &manager->current_quat;
    /* Send the pointer to current quat into the reactive graph */
    {
      ui_error_t rc_cleanup =
          ui_signal_set(manager->orientation_signal, payload);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  return UI_ERROR_NONE;
}
