/* clang-format off */
#include "cmp.h"
/* clang-format on */

int cmp_dt_update(cmp_dt_t *dt, double current_time_ms) {
  if (!dt)
    return CMP_ERROR_INVALID_ARG;

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

  return CMP_SUCCESS;
}
