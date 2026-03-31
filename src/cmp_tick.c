/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_tick {
  double last_timestamp_ms;
};

int cmp_tick_create(cmp_tick_t **out_tick) {
  struct cmp_tick *tick;

  if (!out_tick)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_tick), (void **)&tick) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(tick, 0, sizeof(struct cmp_tick));

  *out_tick = (cmp_tick_t *)tick;
  return CMP_SUCCESS;
}

int cmp_tick_destroy(cmp_tick_t *tick) {
  struct cmp_tick *internal_tick = (struct cmp_tick *)tick;

  if (!internal_tick)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_tick);
  return CMP_SUCCESS;
}

int cmp_tick_fire(cmp_tick_t *tick, double os_timestamp_ms, double *out_dt_ms) {
  struct cmp_tick *internal_tick = (struct cmp_tick *)tick;

  if (!internal_tick || !out_dt_ms)
    return CMP_ERROR_INVALID_ARG;

  if (internal_tick->last_timestamp_ms == 0.0) {
    *out_dt_ms = 0.0;
  } else {
    *out_dt_ms = os_timestamp_ms - internal_tick->last_timestamp_ms;
    if (*out_dt_ms < 0.0) {
      *out_dt_ms = 0.0; /* Prevent negative delta time if timestamp wraps */
    }
  }

  internal_tick->last_timestamp_ms = os_timestamp_ms;
  return CMP_SUCCESS;
}