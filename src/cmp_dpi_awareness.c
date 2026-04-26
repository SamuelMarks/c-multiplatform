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
  int rc = CMP_SUCCESS;

  if (g_dpi_manager) {
    return rc;
  }

  rc = cmp_dpi_create(&g_dpi_manager);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG(
        "Error in cmp_dpi_awareness_init: Failed to create DPI manager\n");
    printf("[DPI] WARNING: Failed to initialize High-DPI context.\n");
    return rc;
  }

  printf("[DPI] SUCCESS: Per-Monitor v2 High-DPI Awareness initialized.\n");
  return rc;
}

/**
 * @brief cmp_dpi_awareness_cleanup
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dpi_awareness_cleanup(void) {
  int rc = CMP_SUCCESS;

  if (g_dpi_manager) {
    rc = cmp_dpi_destroy(g_dpi_manager);
    g_dpi_manager = NULL;
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_dpi_awareness_cleanup: Failed to destroy DPI "
                "manager\n");
    }
  }
  return rc;
}
