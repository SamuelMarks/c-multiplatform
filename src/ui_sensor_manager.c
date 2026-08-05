/* clang-format off */
#include "../include/ui_sensor_manager.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

struct ui_sensor_manager {
  int is_running;
  struct ui_signal *orientation_signal;
  struct ui_sensor_vector3 accel_data;
  struct ui_sensor_vector3 gyro_data;
  struct ui_sensor_quaternion current_quat;
};

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

ui_error_t ui_sensor_manager_destroy(struct ui_sensor_manager *manager) {
  if (!manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (manager->is_running) {
    ui_error_t rc = ui_sensor_manager_stop(manager);
    if (rc != UI_ERROR_NONE) {
      C_MULTIPLATFORM_FREE(manager);
      return rc;
    }
  }

  /* Unbind from signals if necessary */

  C_MULTIPLATFORM_FREE(manager);
  return UI_ERROR_NONE;
}

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

/** \brief ui_error */
ui_error_t ui_sensor_manager_bind_orientation(struct ui_sensor_manager *manager,
                                              struct ui_signal *signal) {
  if (!manager || !signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  manager->orientation_signal = signal;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_sensor_manager_get_accelerometer(struct ui_sensor_manager *manager,
                                    struct ui_sensor_vector3 *out_accel) {
  if (!manager || !out_accel) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_accel = manager->accel_data;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_sensor_manager_get_gyroscope(struct ui_sensor_manager *manager,
                                           struct ui_sensor_vector3 *out_gyro) {
  if (!manager || !out_gyro) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_gyro = manager->gyro_data;
  return UI_ERROR_NONE;
}

/* Forward declarations */
ui_error_t ui_sensor_manager_tick_mock(struct ui_sensor_manager *manager);

/* Mock function to simulate a sensor tick, pushing data to signals */
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
    ui_error_t rc;
    payload.ptr_val = &manager->current_quat;
    /* Send the pointer to current quat into the reactive graph */
    rc = ui_signal_set(manager->orientation_signal, payload);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}
