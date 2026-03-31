/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <math.h>
/* clang-format on */

int cmp_scroll_snap_stop_evaluate(cmp_scroll_snap_stop_t mode,
                                  float current_velocity) {
  if (mode == CMP_SCROLL_SNAP_STOP_ALWAYS) {
    return 1; /* Always stop */
  }

  /* CMP_SCROLL_SNAP_STOP_NORMAL:
     Stop if velocity is low (e.g., < 1000px/s), otherwise allow skipping to
     next points */
  if (fabs((double)current_velocity) < 1000.0) {
    return 1;
  }

  return 0; /* Let it coast past */
}
