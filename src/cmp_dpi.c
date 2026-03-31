/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

#define CMP_MAX_MONITORS 16

struct cmp_dpi {
  struct {
    int monitor_id;
    float scale;
  } monitors[CMP_MAX_MONITORS];
  int monitor_count;
};

int cmp_dpi_create(cmp_dpi_t **out_dpi) {
  cmp_dpi_t *dpi;

  if (out_dpi == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_dpi_t), (void **)&dpi) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  memset(dpi, 0, sizeof(cmp_dpi_t));

  *out_dpi = dpi;
  return CMP_SUCCESS;
}

int cmp_dpi_destroy(cmp_dpi_t *dpi) {
  if (dpi == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  CMP_FREE(dpi);
  return CMP_SUCCESS;
}

int cmp_dpi_set_monitor_scale(cmp_dpi_t *dpi, int monitor_id, float scale) {
  int i;

  if (dpi == NULL || scale <= 0.0f) {
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < dpi->monitor_count; ++i) {
    if (dpi->monitors[i].monitor_id == monitor_id) {
      dpi->monitors[i].scale = scale;
      return CMP_SUCCESS;
    }
  }

  if (dpi->monitor_count >= CMP_MAX_MONITORS) {
    return CMP_ERROR_BOUNDS;
  }

  dpi->monitors[dpi->monitor_count].monitor_id = monitor_id;
  dpi->monitors[dpi->monitor_count].scale = scale;
  dpi->monitor_count++;

  return CMP_SUCCESS;
}

int cmp_dpi_get_monitor_scale(const cmp_dpi_t *dpi, int monitor_id,
                              float *out_scale) {
  int i;

  if (dpi == NULL || out_scale == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < dpi->monitor_count; ++i) {
    if (dpi->monitors[i].monitor_id == monitor_id) {
      *out_scale = dpi->monitors[i].scale;
      return CMP_SUCCESS;
    }
  }

  /* Default scale if monitor not found */
  *out_scale = 1.0f;
  return CMP_ERROR_NOT_FOUND;
}

int cmp_dpi_update_window_scale(cmp_dpi_t *dpi, cmp_window_t *window,
                                int monitor_id) {
  float scale;
  int err;

  if (dpi == NULL || window == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  err = cmp_dpi_get_monitor_scale(dpi, monitor_id, &scale);
  if (err != CMP_SUCCESS) {
    scale = 1.0f; /* default to 1x */
  }

  /* In a real implementation, this would trigger a relayout/rescale of the
   * window's UI tree. */
  /* For now, just a stub that successfully "applied" it */
  return CMP_SUCCESS;
}
