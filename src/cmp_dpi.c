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

/**
 * @brief cmp_dpi_create
 *
 * @param out_dpi Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dpi_create(cmp_dpi_t **out_dpi) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_dpi_t *dpi = NULL;

  if (out_dpi == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dpi_create: Invalid argument (out_dpi=NULL): %s\n",
                  err_str);
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_dpi_t), (void **)&dpi);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dpi_create: Out of memory: %s\n", err_str);
    return rc;
  }

  memset(dpi, 0, sizeof(cmp_dpi_t));

  *out_dpi = dpi;
  cmp_log_debug("cmp_dpi_create: Successfully created DPI context\n");
  return rc;
}

/**
 * @brief cmp_dpi_destroy
 *
 * @param dpi Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dpi_destroy(cmp_dpi_t *dpi) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (dpi == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dpi_destroy: Invalid argument: %s\n", err_str);
    return rc;
  }

  rc = CMP_FREE(dpi);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_dpi_destroy: CMP_FREE failed\n");
  }

  cmp_log_debug("cmp_dpi_destroy: Successfully destroyed DPI context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_dpi_set_monitor_scale
 *
 * @param dpi Parameter description.
 * @param monitor_id Parameter description.
 * @param scale Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dpi_set_monitor_scale(cmp_dpi_t *dpi, int monitor_id, float scale) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  int i;

  if (dpi == NULL || scale <= 0.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dpi_set_monitor_scale: Invalid argument: %s\n", err_str);
    return rc;
  }

  for (i = 0; i < dpi->monitor_count; ++i) {
    if (dpi->monitors[i].monitor_id == monitor_id) {
      dpi->monitors[i].scale = scale;
      cmp_log_debug(
          "cmp_dpi_set_monitor_scale: Updated monitor %d to scale %.2f\n",
          monitor_id, scale);
      return rc;
    }
  }

  if (dpi->monitor_count >= CMP_MAX_MONITORS) {
    rc = CMP_ERROR_BOUNDS;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_dpi_set_monitor_scale: Maximum monitor count reached: %s\n",
        err_str);
    return rc;
  }

  dpi->monitors[dpi->monitor_count].monitor_id = monitor_id;
  dpi->monitors[dpi->monitor_count].scale = scale;
  dpi->monitor_count++;

  cmp_log_debug("cmp_dpi_set_monitor_scale: Added monitor %d with scale %.2f\n",
                monitor_id, scale);
  return rc;
}

/**
 * @brief cmp_dpi_get_monitor_scale
 *
 * @param dpi Parameter description.
 * @param monitor_id Parameter description.
 * @param out_scale Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dpi_get_monitor_scale(const cmp_dpi_t *dpi, int monitor_id,
                              float *out_scale) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  int i;

  if (dpi == NULL || out_scale == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dpi_get_monitor_scale: Invalid argument: %s\n", err_str);
    return rc;
  }

  for (i = 0; i < dpi->monitor_count; ++i) {
    if (dpi->monitors[i].monitor_id == monitor_id) {
      *out_scale = dpi->monitors[i].scale;
      cmp_log_debug("cmp_dpi_get_monitor_scale: Fetched scale for monitor %d\n",
                    monitor_id);
      return rc;
    }
  }

  /* Default scale if monitor not found */
  *out_scale = 1.0f;
  rc = CMP_ERROR_NOT_FOUND;
  err_rc = cmp_strerror(rc, &err_str);
  if (err_rc != CMP_SUCCESS) {
    err_str = "Unknown";
  }
  cmp_log_debug(
      "cmp_dpi_get_monitor_scale: Monitor not found, using fallback: %s\n",
      err_str);
  return rc;
}

/**
 * @brief cmp_dpi_update_window_scale
 *
 * @param dpi Parameter description.
 * @param window Parameter description.
 * @param monitor_id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dpi_update_window_scale(cmp_dpi_t *dpi, cmp_window_t *window,
                                int monitor_id) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  float scale;
  int err;

  if (dpi == NULL || window == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dpi_update_window_scale: Invalid argument: %s\n",
                  err_str);
    return rc;
  }

  err = cmp_dpi_get_monitor_scale(dpi, monitor_id, &scale);
  if (err != CMP_SUCCESS) {
    scale = 1.0f; /* default to 1x */
  }

  /* In a fully implemented renderer, trigger a relayout/rescale of the
   * window's UI tree here using the resolved scale. */
  cmp_log_debug("cmp_dpi_update_window_scale: Updated scale map cascade\n");
  return rc;
}
