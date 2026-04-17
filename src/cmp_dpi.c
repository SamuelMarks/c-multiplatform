/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
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
  int rc = CMP_SUCCESS;
  cmp_dpi_t *dpi = NULL;

  if (out_dpi == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dpi_create: Invalid argument (out_dpi=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_dpi_t), (void **)&dpi);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_dpi_create: Out of memory\n");
    return rc;
  }

  memset(dpi, 0, sizeof(cmp_dpi_t));

  *out_dpi = dpi;
  return rc;
}

int cmp_dpi_destroy(cmp_dpi_t *dpi) {
  int rc = CMP_SUCCESS;

  if (dpi == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dpi_destroy: Invalid argument (dpi=NULL)\n");
    return rc;
  }

  CMP_FREE(dpi);
  return rc;
}

int cmp_dpi_set_monitor_scale(cmp_dpi_t *dpi, int monitor_id, float scale) {
  int rc = CMP_SUCCESS;
  int i;

  if (dpi == NULL || scale <= 0.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dpi_set_monitor_scale: Invalid argument\n");
    return rc;
  }

  for (i = 0; i < dpi->monitor_count; ++i) {
    if (dpi->monitors[i].monitor_id == monitor_id) {
      dpi->monitors[i].scale = scale;
      return rc;
    }
  }

  if (dpi->monitor_count >= CMP_MAX_MONITORS) {
    rc = CMP_ERROR_BOUNDS;
    LOG_DEBUG(
        "Error in cmp_dpi_set_monitor_scale: Maximum monitor count reached\n");
    return rc;
  }

  dpi->monitors[dpi->monitor_count].monitor_id = monitor_id;
  dpi->monitors[dpi->monitor_count].scale = scale;
  dpi->monitor_count++;

  return rc;
}

int cmp_dpi_get_monitor_scale(const cmp_dpi_t *dpi, int monitor_id,
                              float *out_scale) {
  int rc = CMP_SUCCESS;
  int i;

  if (dpi == NULL || out_scale == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dpi_get_monitor_scale: Invalid argument\n");
    return rc;
  }

  for (i = 0; i < dpi->monitor_count; ++i) {
    if (dpi->monitors[i].monitor_id == monitor_id) {
      *out_scale = dpi->monitors[i].scale;
      return rc;
    }
  }

  /* Default scale if monitor not found */
  *out_scale = 1.0f;
  rc = CMP_ERROR_NOT_FOUND;
  LOG_DEBUG("Error in cmp_dpi_get_monitor_scale: Monitor not found\n");
  return rc;
}

int cmp_dpi_update_window_scale(cmp_dpi_t *dpi, cmp_window_t *window,
                                int monitor_id) {
  int rc = CMP_SUCCESS;
  float scale;
  int err;

  if (dpi == NULL || window == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dpi_update_window_scale: Invalid argument\n");
    return rc;
  }

  err = cmp_dpi_get_monitor_scale(dpi, monitor_id, &scale);
  if (err != CMP_SUCCESS) {
    scale = 1.0f; /* default to 1x */
  }

  /* In a fully implemented renderer, trigger a relayout/rescale of the
   * window's UI tree here using the resolved scale. */
  return rc;
}
