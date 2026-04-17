/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

int cmp_overscroll_evaluate(cmp_overscroll_t mode, int boundary_hit) {
  int rc = 1; /* Default to bubbling (true) */

  if (!boundary_hit) {
    /* Not at boundary, standard evaluation applies (bubbling usually) */
    rc = 1;
    return rc;
  }

  /* We hit the boundary. Check chaining rules */
  if (mode == CMP_OVERSCROLL_AUTO) {
    rc = 1; /* Allow chaining to parent */
  } else if (mode == CMP_OVERSCROLL_CONTAIN || mode == CMP_OVERSCROLL_NONE) {
    rc = 0; /* Trap scroll here, do not bubble to parent */
  }

  return rc;
}
