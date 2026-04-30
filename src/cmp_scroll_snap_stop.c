/* clang-format off */
#include "cmp.h"

#include <math.h>
#include <stdlib.h>

#include "cmp_log.h"
/* clang-format on */

/**
 * @brief Evaluate scroll snap stop condition.
 *
 * @param mode Parameter description.
 * @param current_velocity Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_snap_stop_evaluate(cmp_scroll_snap_stop_t mode,
                                  float current_velocity) {
  int rc = CMP_SUCCESS;
  if (mode == CMP_SCROLL_SNAP_STOP_ALWAYS) {
    return 1; /* Always stop */
  }

  if (mode != CMP_SCROLL_SNAP_STOP_NORMAL) {
    LOG_DEBUG("Invalid argument: unrecognized mode\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* CMP_SCROLL_SNAP_STOP_NORMAL:
     Stop if velocity is low (e.g., < 1000px/s), otherwise allow skipping to
     next points */
  if (fabs((double)current_velocity) < 1000.0) {
    return 1;
  }

  return rc; /* Let it coast past */
}
