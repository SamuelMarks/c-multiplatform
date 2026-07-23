/* clang-format off */
#include "ui_section_index_base.h"
#include "ui_component.h"
#include <stdio.h>
/* clang-format on */

int main(void) {
  struct ui_section_index_base *index = NULL;
  struct ui_component *comp = NULL;
  enum ui_error rc;
  const char *sections[] = {"A", "B", "C", "D"};

  rc = ui_section_index_base_create(&index);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create section index\n");
    return 1;
  }

  if (ui_section_index_base_get_component(index, &comp) != UI_ERROR_NONE ||
      !comp) {
    printf("Failed to get component\n");
    return 1;
  }

  rc = ui_section_index_base_set_sections(index, sections, 4);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to set sections\n");
    return 1;
  }

  rc = ui_section_index_base_set_active_section(index, 2); /* Highlight 'C' */
  if (rc != UI_ERROR_NONE) {
    printf("Failed to set active section\n");
    return 1;
  }

  /* Test out of bounds */
  rc = ui_section_index_base_set_active_section(index, 5);
  if (rc != UI_ERROR_OUT_OF_BOUNDS) {
    printf("Expected UI_ERROR_OUT_OF_BOUNDS for active section out of range\n");
    return 1;
  }

  ui_section_index_base_destroy(index);

  printf("test_ui_section_index_base passed\n");
  return 0;
}
