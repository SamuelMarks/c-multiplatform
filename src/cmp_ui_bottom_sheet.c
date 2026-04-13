/* clang-format off */
#include "cmp_ui_bottom_sheet.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_bottom_sheet {
  cmp_ui_node_t *node_root;
  int is_visible;
};

int cmp_ui_bottom_sheet_create(cmp_ui_bottom_sheet_t **out_sheet) {
  cmp_ui_bottom_sheet_t *sheet;
  int err;

  if (!out_sheet) {
    return CMP_ERROR_INVALID_ARG;
  }

  err = CMP_MALLOC(sizeof(cmp_ui_bottom_sheet_t), (void **)&sheet);
  if (err != CMP_SUCCESS) {
    return err;
  }
  memset(sheet, 0, sizeof(cmp_ui_bottom_sheet_t));

  err = cmp_ui_box_create(&sheet->node_root);
  if (err != CMP_SUCCESS) {
    CMP_FREE(sheet);
    return err;
  }

  sheet->node_root->layout->direction = CMP_FLEX_COLUMN;
  /* Additional layout properties like position: absolute, bottom: 0, etc.
   * would be set via the styling engine.
   */
  sheet->is_visible = 0; /* Initially hidden */

  *out_sheet = sheet;
  return CMP_SUCCESS;
}

int cmp_ui_bottom_sheet_destroy(cmp_ui_bottom_sheet_t *sheet) {
  if (!sheet) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(sheet);
  return CMP_SUCCESS;
}

int cmp_ui_bottom_sheet_get_node(cmp_ui_bottom_sheet_t *sheet,
                                 cmp_ui_node_t **out_node) {
  if (!sheet || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = sheet->node_root;
  return CMP_SUCCESS;
}

int cmp_ui_bottom_sheet_set_visible(cmp_ui_bottom_sheet_t *sheet, int visible) {
  if (!sheet) {
    return CMP_ERROR_INVALID_ARG;
  }

  sheet->is_visible = visible;
  /* Visual transition to hide or show via CSS/layout would be triggered here */

  return CMP_SUCCESS;
}
int cmp_ui_bottom_sheet_bind_a11y(cmp_ui_bottom_sheet_t *widget,
                                  cmp_a11y_tree_t *tree) {
  if (!widget || !tree) {
    return CMP_ERROR_INVALID_ARG;
  }
  cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "dialog",
                         "Bottom Sheet");
  return CMP_SUCCESS;
}
