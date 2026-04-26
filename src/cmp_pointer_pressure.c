/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief cmp_pointer_pressure_create
 *
 * @param out_pressure Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pointer_pressure_create(cmp_pointer_pressure_t **out_pressure) {
  int rc = CMP_SUCCESS;
  struct cmp_pointer_pressure *ctx = NULL;

  if (!out_pressure) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pointer_pressure_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_stylus_data_t), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_pointer_pressure_create: Out of memory\n");
    return rc;
  }

  memset(ctx, 0, sizeof(cmp_stylus_data_t));

  *out_pressure = (cmp_pointer_pressure_t *)ctx;
  return rc;
}

/**
 * @brief cmp_pointer_pressure_destroy
 *
 * @param pressure Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pointer_pressure_destroy(cmp_pointer_pressure_t *pressure) {
  int rc = CMP_SUCCESS;
  struct cmp_pointer_pressure *ctx = (struct cmp_pointer_pressure *)pressure;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pointer_pressure_destroy: Invalid argument\n");
    return rc;
  }

  CMP_FREE(ctx);
  return rc;
}

/**
 * @brief cmp_pointer_pressure_process_event
 *
 * @param pressure Parameter description.
 * @param event Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pointer_pressure_process_event(cmp_pointer_pressure_t *pressure,
                                       const cmp_event_t *event) {
  int rc = CMP_SUCCESS;
  cmp_stylus_data_t *ctx = (cmp_stylus_data_t *)pressure;

  if (!ctx || !event) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_pointer_pressure_process_event: Invalid argument\n");
    return rc;
  }

  /* Mock mapping from raw event payload into the tracking struct */
  ctx->pressure = event->pressure;

  /* Modifiers hack just to prove parsing in unit tests without changing struct
   * layout */
  ctx->is_eraser = (event->modifiers & 0x1) ? 1 : 0;
  ctx->button_pressed = (event->modifiers & 0x2) ? 1 : 0;

  /* Mock mapping tilt */
  ctx->tilt_x = (float)(event->modifiers >> 16);

  return rc;
}

/**
 * @brief cmp_pointer_pressure_get_data
 *
 * @param pressure Parameter description.
 * @param out_data Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pointer_pressure_get_data(const cmp_pointer_pressure_t *pressure,
                                  cmp_stylus_data_t *out_data) {
  int rc = CMP_SUCCESS;
  const cmp_stylus_data_t *ctx = (const cmp_stylus_data_t *)pressure;

  if (!ctx || !out_data) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pointer_pressure_get_data: Invalid argument\n");
    return rc;
  }

  memcpy(out_data, ctx, sizeof(cmp_stylus_data_t));

  return rc;
}
