/* clang-format off */
#include "cmp.h"
#include "cmp.h"

#include <stdio.h>
/* clang-format on */

static cmp_dpi_t *g_dpi_manager = NULL;

int cmp_dpi_awareness_init(void) {
  int result;

  if (g_dpi_manager) {
    return CMP_SUCCESS;
  }

  result = cmp_dpi_create(&g_dpi_manager);
  if (result != CMP_SUCCESS) {
    printf("[DPI] WARNING: Failed to initialize High-DPI context.\n");
    return result;
  }

  printf("[DPI] SUCCESS: Per-Monitor v2 High-DPI Awareness initialized.\n");
  return CMP_SUCCESS;
}

int cmp_dpi_awareness_cleanup(void) {
  if (g_dpi_manager) {
    int result = cmp_dpi_destroy(g_dpi_manager);
    g_dpi_manager = NULL;
    return result;
  }
  return CMP_SUCCESS;
}
