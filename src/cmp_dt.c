/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
/* clang-format on */

/**
 * @brief cmp_dt_update
 *
 * @param dt Parameter description.
 * @param current_time_ms Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dt_update(cmp_dt_t *dt, double current_time_ms) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (dt == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dt_update: Invalid argument (dt=NULL): %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
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

  cmp_log_debug("cmp_dt_update: Updated tick dt=%.2fms\n", dt->delta_time_ms);
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
