/* clang-format off */
#include "material_catalog.h"
#include <stdio.h>
/* clang-format on */

/**
 * @brief Application entry point.
 * @return 0 on success, 1 on failure.
 */
int main(void) {
  material_catalog_state_t state;

  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    printf("Failed to initialize Material Catalog app.\n");
    return 1;
  }

  if (material_catalog_create_ui(&state) != MATERIAL_CATALOG_SUCCESS) {
    printf("Failed to create UI.\n");
    material_catalog_cleanup(&state);
    return 1;
  }

  if (material_catalog_run(&state) != MATERIAL_CATALOG_SUCCESS) {
    printf("Application failed during run.\n");
    material_catalog_cleanup(&state);
    return 1;
  }

  material_catalog_cleanup(&state);
  return 0;
}
