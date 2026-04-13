/* clang-format off */
#include "cmp_ui_divider.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_divider {
  cmp_ui_node_t *node_root;
};

int cmp_ui_divider_create(cmp_ui_divider_t **out_divider) {
  cmp_ui_divider_t *divider;
  int err;

  if (!out_divider) {
    return CMP_ERROR_INVALID_ARG;
  }

  err = CMP_MALLOC(sizeof(cmp_ui_divider_t), (void **)&divider);
  if (err != CMP_SUCCESS) {
    return err;
  }
  memset(divider, 0, sizeof(cmp_ui_divider_t));

  err = cmp_ui_box_create(&divider->node_root);
  if (err != CMP_SUCCESS) {
    CMP_FREE(divider);
    return err;
  }

  /* Usually a divider is 1px tall and takes 100% width or similar */
  /* Here we simply provide a background color; exact dimensions would
     be provided via styles. */
  divider->node_root->bg_color = 0xFFCCCCCC;

  *out_divider = divider;
  return CMP_SUCCESS;
}

int cmp_ui_divider_destroy(cmp_ui_divider_t *divider) {
  if (!divider) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(divider);
  return CMP_SUCCESS;
}

int cmp_ui_divider_get_node(cmp_ui_divider_t *divider,
                            cmp_ui_node_t **out_node) {
  if (!divider || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = divider->node_root;
  return CMP_SUCCESS;
}
int cmp_ui_divider_bind_a11y(cmp_ui_divider_t *widget, cmp_a11y_tree_t *tree) {
  if (!widget || !tree) {
    return CMP_ERROR_INVALID_ARG;
  }
  cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "separator",
                         "Divider");
  return CMP_SUCCESS;
}
