/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdio.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
/* clang-format on */

static cmp_resource_manager_t *g_resource_manager = NULL;

int cmp_power_awareness_init(void) {
  int rc = CMP_SUCCESS;

  rc = cmp_resource_manager_create(&g_resource_manager);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_power_awareness_init: cmp_resource_manager_create "
              "failed\n");
    return rc;
  }
  return rc;
}

int cmp_power_awareness_poll(void) {
  int rc = CMP_SUCCESS;

  if (!g_resource_manager) {
    rc = CMP_ERROR_INVALID_STATE;
    LOG_DEBUG("Error in cmp_power_awareness_poll: Not initialized\n");
    return rc;
  }

#if defined(_WIN32)
  {
    SYSTEM_POWER_STATUS status;
    if (GetSystemPowerStatus(&status)) {
      /* If running on battery (ACLineStatus == 0) and battery is low (< 20%),
         or BatterySaver is active, we tell cmp to throttle. */
      int throttle = 0;

      /* ACLineStatus: 0 = Offline (Battery), 1 = Online (AC), 255 = Unknown */
      if (status.ACLineStatus == 0) {
        if (status.BatteryLifePercent <= 20) {
          throttle = 1;
        }
      }

      if (throttle) {
        /* Treat as "serious" thermal state to throttle GPU updates/animations
         */
        cmp_resources_set_thermal_state(g_resource_manager, 2);
      } else {
        /* Normal */
        cmp_resources_set_thermal_state(g_resource_manager, 0);
      }
    }
  }
#else
  /* On non-Windows platforms we assume normal power, or defer to other
     platform specific calls in the future. */
#endif

  return rc;
}
