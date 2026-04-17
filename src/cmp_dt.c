/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
/* clang-format on */

int cmp_dt_update(cmp_dt_t *dt, double current_time_ms) {
  int rc = CMP_SUCCESS;

  if (!dt) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dt_update: Invalid argument\n");
    return rc;
  }

  if (dt->last_time_ms == 0.0) {
    dt->delta_time_ms = 0.0;
  } else {
    dt->delta_time_ms = current_time_ms - dt->last_time_ms;
    if (dt->delta_time_ms < 0.0) {
      dt->delta_time_ms = 0.0;
    }
  }

  dt->last_time_ms = current_time_ms;
  dt->current_time_ms = current_time_ms;

  return rc;
}
