/* clang-format off */
#include "cmp_ui_segmented_button.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

typedef struct cmp_ui_segment {
  cmp_ui_node_t *node;
  int is_selected;
} cmp_ui_segment_t;

struct cmp_ui_segmented_button {
  cmp_ui_node_t *node_root;
  cmp_ui_segment_t *segments;
  int segment_count;
  int segment_capacity;
  int multi_select;
};

/**
 * @brief cmp_ui_segmented_button_create
 *
 * @param out_btn Parameter description.
 * @param multi_select Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_segmented_button_create(cmp_ui_segmented_button_t **out_btn,
                                   int multi_select) {
  cmp_ui_segmented_button_t *btn;
  int err;

  if (!out_btn) {
    return CMP_ERROR_INVALID_ARG;
  }

  err = CMP_MALLOC(sizeof(cmp_ui_segmented_button_t), (void **)&btn);
  if (err != CMP_SUCCESS) {
    return err;
  }
  memset(btn, 0, sizeof(cmp_ui_segmented_button_t));

  btn->multi_select = multi_select;
  btn->segment_capacity = 4;
  err = CMP_MALLOC(sizeof(cmp_ui_segment_t) * btn->segment_capacity,
                   (void **)&btn->segments);
  if (err != CMP_SUCCESS) {
    CMP_FREE(btn);
    return err;
  }

  /* Create a container box for the segments */
  err = cmp_ui_box_create(&btn->node_root);
  if (err != CMP_SUCCESS) {
    int free_rc = CMP_FREE(btn->segments);
    if (free_rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_segmented_button_create: CMP_FREE failed\n");
    free_rc = CMP_FREE(btn);
    if (free_rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_segmented_button_create: CMP_FREE failed\n");
    return err;
  }

  err = CMP_MALLOC(sizeof(cmp_layout_node_t), (void **)&btn->node_root->layout);
  if (err == CMP_SUCCESS) {
    memset(btn->node_root->layout, 0, sizeof(cmp_layout_node_t));
    btn->node_root->layout->id = 1;
    btn->node_root->layout->direction = CMP_FLEX_ROW;
  }

  *out_btn = btn;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_segmented_button_destroy
 *
 * @param btn Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_segmented_button_destroy(cmp_ui_segmented_button_t *btn) {
  if (!btn) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (btn->segments) {
    int free_rc = CMP_FREE(btn->segments);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_segmented_button_destroy: CMP_FREE segments failed\n");
    }
  }
  int free_rc = CMP_FREE(btn);
  if (free_rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_segmented_button_destroy: CMP_FREE btn failed\n");
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_segmented_button_get_node
 *
 * @param btn Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_segmented_button_get_node(cmp_ui_segmented_button_t *btn,
                                     cmp_ui_node_t **out_node) {
  if (!btn || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = btn->node_root;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_segmented_button_add_segment
 *
 * @param btn Parameter description.
 * @param label Parameter description.
 * @param icon_name Parameter description.
 * @param out_segment_index Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_segmented_button_add_segment(cmp_ui_segmented_button_t *btn,
                                        const char *label,
                                        const char *icon_name,
                                        int *out_segment_index) {
  cmp_ui_node_t *seg_node;
  int err;

  if (!btn) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (btn->segment_count >= btn->segment_capacity) {
    int new_cap = btn->segment_capacity * 2;
    cmp_ui_segment_t *new_segs;
    err = CMP_MALLOC(sizeof(cmp_ui_segment_t) * new_cap, (void **)&new_segs);
    if (err != CMP_SUCCESS) {
      return err;
    }
    memcpy(new_segs, btn->segments,
           sizeof(cmp_ui_segment_t) * btn->segment_count);
    int free_rc = CMP_FREE(btn->segments);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_segmented_button_add_segment: CMP_FREE failed\n");
    }
    btn->segments = new_segs;
    btn->segment_capacity = new_cap;
  }

  err = cmp_ui_button_create(&seg_node, label ? label : "", -1);
  if (err != CMP_SUCCESS) {
    return err;
  }

  seg_node->type = 3; /* Button */

  /* If icon exists, it would be added here as a child or property based on the
   * button implementation */
  (void)icon_name;

  cmp_ui_node_add_child(btn->node_root, seg_node);

  btn->segments[btn->segment_count].node = seg_node;
  btn->segments[btn->segment_count].is_selected = 0;

  if (out_segment_index) {
    *out_segment_index = btn->segment_count;
  }

  btn->segment_count++;

  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_segmented_button_set_selected
 *
 * @param btn Parameter description.
 * @param index Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_segmented_button_set_selected(cmp_ui_segmented_button_t *btn,
                                         int index) {
  if (!btn || index < 0 || index >= btn->segment_count) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (!btn->multi_select) {
    int i;
    for (i = 0; i < btn->segment_count; ++i) {
      btn->segments[i].is_selected = 0;
      /* Here we would update visual state to unselected */
    }
  }

  btn->segments[index].is_selected = 1;
  /* Here we would update visual state to selected */

  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_segmented_button_set_deselected
 *
 * @param btn Parameter description.
 * @param index Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_segmented_button_set_deselected(cmp_ui_segmented_button_t *btn,
                                           int index) {
  if (!btn || index < 0 || index >= btn->segment_count) {
    return CMP_ERROR_INVALID_ARG;
  }

  btn->segments[index].is_selected = 0;
  /* Here we would update visual state to deselected */

  return CMP_SUCCESS;
}
/**
 * @brief cmp_ui_segmented_button_bind_a11y
 *
 * @param widget Parameter description.
 * @param tree Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_segmented_button_bind_a11y(cmp_ui_segmented_button_t *widget,
                                      cmp_a11y_tree_t *tree) {
  if (!widget || !tree) {
    return CMP_ERROR_INVALID_ARG;
  }
  cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "group",
                         "Segmented Button");
  return CMP_SUCCESS;
}
