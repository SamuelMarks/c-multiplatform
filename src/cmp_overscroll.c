/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

/**
 * @brief Evaluate overscroll mode logic.
 *
 * @param mode The overscroll mode to evaluate.
 * @param boundary_hit 1 if a scroll boundary was hit, 0 otherwise.
 * @param out_chains Pointer to store 1 if scroll chaining is allowed (event
 * should bubble), 0 if trapped.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_overscroll_evaluate(cmp_overscroll_t mode, int boundary_hit,
                            int *out_chains) {
  if (out_chains == NULL) {
    LOG_DEBUG("Error in cmp_overscroll_evaluate: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  *out_chains = 1; /* Default to bubbling (true) */

  if (!boundary_hit) {
    /* Not at boundary, standard evaluation applies (bubbling usually) */
    return CMP_SUCCESS;
  }

  /* We hit the boundary. Check chaining rules */
  if (mode == CMP_OVERSCROLL_AUTO) {
    *out_chains = 1; /* Allow chaining to parent */
  } else if (mode == CMP_OVERSCROLL_CONTAIN || mode == CMP_OVERSCROLL_NONE) {
    *out_chains = 0; /* Trap scroll here, do not bubble to parent */
  }

  return CMP_SUCCESS;
}
