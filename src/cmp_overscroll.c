/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

int cmp_overscroll_evaluate(cmp_overscroll_t mode, int boundary_hit) {
  if (!boundary_hit) {
    /* Not at boundary, standard evaluation applies (bubbling usually) */
    return 1;
  }

  /* We hit the boundary. Check chaining rules */
  if (mode == CMP_OVERSCROLL_AUTO) {
    return 1; /* Allow chaining to parent */
  } else if (mode == CMP_OVERSCROLL_CONTAIN || mode == CMP_OVERSCROLL_NONE) {
    return 0; /* Trap scroll here, do not bubble to parent */
  }

  return 1; /* Default to bubbling */
}
