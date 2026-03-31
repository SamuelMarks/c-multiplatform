/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ptr {
  cmp_ptr_state_t state;
};

int cmp_ptr_create(cmp_ptr_t **out_ptr) {
  struct cmp_ptr *ptr;

  if (!out_ptr)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_ptr), (void **)&ptr) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(ptr, 0, sizeof(struct cmp_ptr));
  ptr->state = CMP_PTR_STATE_IDLE;

  *out_ptr = (cmp_ptr_t *)ptr;
  return CMP_SUCCESS;
}

int cmp_ptr_destroy(cmp_ptr_t *ptr) {
  struct cmp_ptr *internal_ptr = (struct cmp_ptr *)ptr;

  if (!internal_ptr)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_ptr);
  return CMP_SUCCESS;
}

int cmp_ptr_update(cmp_ptr_t *ptr, float overscroll_y, float threshold_y,
                   cmp_ptr_state_t *out_state, float *out_progress) {
  struct cmp_ptr *internal_ptr = (struct cmp_ptr *)ptr;

  if (!internal_ptr || !out_state || !out_progress)
    return CMP_ERROR_INVALID_ARG;

  if (internal_ptr->state == CMP_PTR_STATE_REFRESHING) {
    *out_state = CMP_PTR_STATE_REFRESHING;
    *out_progress = 1.0f;
    return CMP_SUCCESS;
  }

  if (overscroll_y <= 0.0f) {
    internal_ptr->state = CMP_PTR_STATE_IDLE;
    *out_state = CMP_PTR_STATE_IDLE;
    *out_progress = 0.0f;
    return CMP_SUCCESS;
  }

  *out_progress = overscroll_y / threshold_y;
  if (*out_progress > 1.0f) {
    *out_progress = 1.0f;
  }

  if (overscroll_y >= threshold_y) {
    internal_ptr->state = CMP_PTR_STATE_READY_TO_REFRESH;
  } else {
    internal_ptr->state = CMP_PTR_STATE_PULLING;
  }

  *out_state = internal_ptr->state;
  return CMP_SUCCESS;
}

int cmp_ptr_set_refreshing(cmp_ptr_t *ptr, int is_refreshing) {
  struct cmp_ptr *internal_ptr = (struct cmp_ptr *)ptr;

  if (!internal_ptr)
    return CMP_ERROR_INVALID_ARG;

  if (is_refreshing) {
    internal_ptr->state = CMP_PTR_STATE_REFRESHING;
  } else {
    internal_ptr->state = CMP_PTR_STATE_IDLE;
  }

  return CMP_SUCCESS;
}