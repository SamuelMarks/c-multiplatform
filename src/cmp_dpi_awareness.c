/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdio.h>
/* clang-format on */

static cmp_dpi_t *g_dpi_manager = NULL;

/**
 * @brief cmp_dpi_awareness_init
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dpi_awareness_init(void) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (g_dpi_manager != NULL) {
    cmp_log_debug("cmp_dpi_awareness_init: Already initialized\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = cmp_dpi_create(&g_dpi_manager);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dpi_awareness_init: Failed to create DPI manager: %s\n",
                  err_str);
    /* Safe CRT not needed for simple literal logging to standard output. */
    printf("[DPI] WARNING: Failed to initialize High-DPI context.\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  cmp_log_debug("cmp_dpi_awareness_init: Per-Monitor v2 High-DPI Awareness "
                "initialized.\n");
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_dpi_awareness_cleanup
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dpi_awareness_cleanup(void) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (g_dpi_manager != NULL) {
    rc = cmp_dpi_destroy(g_dpi_manager);
    g_dpi_manager = NULL;
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_dpi_awareness_cleanup: Failed to destroy DPI "
                    "manager: %s\n",
                    err_str);
    }
  }

  cmp_log_debug("cmp_dpi_awareness_cleanup: Cleaned up context\n");
  return CMP_SUCCESS;
}
