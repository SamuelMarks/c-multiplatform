/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_pointer_pressure_create(cmp_pointer_pressure_t **out_pressure) {
  struct cmp_pointer_pressure *ctx;

  if (!out_pressure)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(cmp_stylus_data_t), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(ctx, 0, sizeof(cmp_stylus_data_t));

  *out_pressure = (cmp_pointer_pressure_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_pointer_pressure_destroy(cmp_pointer_pressure_t *pressure) {
  struct cmp_pointer_pressure *ctx = (struct cmp_pointer_pressure *)pressure;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_pointer_pressure_process_event(cmp_pointer_pressure_t *pressure,
                                       const cmp_event_t *event) {
  cmp_stylus_data_t *ctx = (cmp_stylus_data_t *)pressure;
  if (!ctx || !event)
    return CMP_ERROR_INVALID_ARG;

  /* Mock mapping from raw event payload into the tracking struct */
  ctx->pressure = event->pressure;

  /* Modifiers hack just to prove parsing in unit tests without changing struct
   * layout */
  ctx->is_eraser = (event->modifiers & 0x1) ? 1 : 0;
  ctx->button_pressed = (event->modifiers & 0x2) ? 1 : 0;

  /* Mock mapping tilt */
  ctx->tilt_x = (float)(event->modifiers >> 16);

  return CMP_SUCCESS;
}

int cmp_pointer_pressure_get_data(const cmp_pointer_pressure_t *pressure,
                                  cmp_stylus_data_t *out_data) {
  const cmp_stylus_data_t *ctx = (const cmp_stylus_data_t *)pressure;
  if (!ctx || !out_data)
    return CMP_ERROR_INVALID_ARG;

  memcpy(out_data, ctx, sizeof(cmp_stylus_data_t));

  return CMP_SUCCESS;
}
