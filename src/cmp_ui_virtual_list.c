/* clang-format off */
#include "cmp_ui_virtual_list.h"
#include <stdlib.h>
#include "cmp_log.h"
/* clang-format on */

struct cmp_ui_virtual_list {
  cmp_ui_node_t *node_root;
  size_t item_count;
  float item_height;
};

/**
 * @brief cmp_ui_virtual_list_create
 *
 * @param out_list Parameter description.
 * @param item_count Parameter description.
 * @param item_height Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_virtual_list_create(cmp_ui_virtual_list_t **out_list,
                               size_t item_count, float item_height) {
  int rc = CMP_SUCCESS;
  cmp_ui_virtual_list_t *list;
  int err;

  if (!out_list) {
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_virtual_list_t), (void **)&(list));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  list->item_count = item_count;
  list->item_height = item_height;

  err = cmp_ui_box_create(&list->node_root);
  if (err != CMP_SUCCESS) {
    int free_rc = CMP_FREE(list);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_virtual_list_create: CMP_FREE failed\n");
    }
    return err;
  }

  *out_list = list;
  return 0;
}

/**
 * @brief cmp_ui_virtual_list_destroy
 *
 * @param list Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_virtual_list_destroy(cmp_ui_virtual_list_t *list) {
  int rc = CMP_SUCCESS;
  if (!list) {
    return CMP_ERROR_INVALID_ARG;
  }
  rc = CMP_FREE(list);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  return 0;
}

/**
 * @brief cmp_ui_virtual_list_get_node
 *
 * @param list Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_virtual_list_get_node(cmp_ui_virtual_list_t *list,
                                 cmp_ui_node_t **out_node) {
  if (!list || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = list->node_root;
  return 0;
}

/**
 * @brief cmp_ui_virtual_list_set_item_count
 *
 * @param list Parameter description.
 * @param item_count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_virtual_list_set_item_count(cmp_ui_virtual_list_t *list,
                                       size_t item_count) {
  if (!list) {
    return CMP_ERROR_INVALID_ARG;
  }
  list->item_count = item_count;
  return 0;
}
