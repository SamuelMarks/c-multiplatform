/* clang-format off */
#include "cmp_ui_virtual_list.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_ui_virtual_list {
  cmp_ui_node_t *node_root;
  size_t item_count;
  float item_height;
};

int cmp_ui_virtual_list_create(cmp_ui_virtual_list_t **out_list,
                               size_t item_count, float item_height) {
  cmp_ui_virtual_list_t *list;
  int err;

  if (!out_list) {
    return CMP_ERROR_INVALID_ARG;
  }

  list = (cmp_ui_virtual_list_t *)malloc(sizeof(cmp_ui_virtual_list_t));
  if (!list) {
    return CMP_ERROR_OOM;
  }

  list->item_count = item_count;
  list->item_height = item_height;

  err = cmp_ui_box_create(&list->node_root);
  if (err != 0) {
    free(list);
    return err;
  }

  *out_list = list;
  return 0;
}

int cmp_ui_virtual_list_destroy(cmp_ui_virtual_list_t *list) {
  if (!list) {
    return CMP_ERROR_INVALID_ARG;
  }
  free(list);
  return 0;
}

int cmp_ui_virtual_list_get_node(cmp_ui_virtual_list_t *list,
                                 cmp_ui_node_t **out_node) {
  if (!list || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = list->node_root;
  return 0;
}

int cmp_ui_virtual_list_set_item_count(cmp_ui_virtual_list_t *list,
                                       size_t item_count) {
  if (!list) {
    return CMP_ERROR_INVALID_ARG;
  }
  list->item_count = item_count;
  return 0;
}
