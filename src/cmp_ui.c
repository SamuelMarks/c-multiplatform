/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_ui_box_create(cmp_ui_node_t **out_node) {
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 1; /* Box */

  if (cmp_layout_node_create(&node->layout) != CMP_SUCCESS) {
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

  *out_node = node;
  return CMP_SUCCESS;
}

int cmp_ui_text_create(cmp_ui_node_t **out_node, const char *text,
                       int text_len) {
  cmp_ui_node_t *node;
  char *text_copy;
  size_t len;

  if (out_node == NULL || text == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 2; /* Text */

  if (cmp_layout_node_create(&node->layout) != CMP_SUCCESS) {
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

  len = (text_len < 0) ? strlen(text) : (size_t)text_len;
  if (CMP_MALLOC(len + 1, (void **)&text_copy) != CMP_SUCCESS) {
    cmp_layout_node_destroy(node->layout);
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

  memcpy(text_copy, text, len);
  text_copy[len] = '\0';

  node->properties = text_copy;
  *out_node = node;
  return CMP_SUCCESS;
}

int cmp_ui_button_create(cmp_ui_node_t **out_node, const char *label,
                         int label_len) {
  cmp_ui_node_t *node;
  char *label_copy;
  size_t len;

  if (out_node == NULL || label == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 3; /* Button */

  if (cmp_layout_node_create(&node->layout) != CMP_SUCCESS) {
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

  len = (label_len < 0) ? strlen(label) : (size_t)label_len;
  if (CMP_MALLOC(len + 1, (void **)&label_copy) != CMP_SUCCESS) {
    cmp_layout_node_destroy(node->layout);
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

  memcpy(label_copy, label, len);
  label_copy[len] = '\0';

  node->properties = label_copy;
  *out_node = node;
  return CMP_SUCCESS;
}

int cmp_ui_text_input_create(cmp_ui_node_t **out_node) {
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 4; /* Text Input */

  if (cmp_layout_node_create(&node->layout) != CMP_SUCCESS) {
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

  *out_node = node;
  return CMP_SUCCESS;
}

int cmp_ui_checkbox_create(cmp_ui_node_t **out_node, const char *label) {
  cmp_ui_node_t *node;
  char *label_copy = NULL;

  if (out_node == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 5; /* Checkbox */

  if (cmp_layout_node_create(&node->layout) != CMP_SUCCESS) {
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

  if (label != NULL) {
    size_t len = strlen(label);
    if (CMP_MALLOC(len + 1, (void **)&label_copy) != CMP_SUCCESS) {
      cmp_layout_node_destroy(node->layout);
      CMP_FREE(node);
      return CMP_ERROR_OOM;
    }

#if defined(_MSC_VER)
    strcpy_s(label_copy, len + 1, label);
#else
    strcpy(label_copy, label);
#endif
  }

  node->properties = label_copy;
  *out_node = node;
  return CMP_SUCCESS;
}

int cmp_ui_radio_create(cmp_ui_node_t **out_node, int group_id) {
  cmp_ui_node_t *node;
  int *group_prop;

  if (out_node == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 6; /* Radio */

  if (cmp_layout_node_create(&node->layout) != CMP_SUCCESS) {
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

  if (CMP_MALLOC(sizeof(int), (void **)&group_prop) != CMP_SUCCESS) {
    cmp_layout_node_destroy(node->layout);
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

  *group_prop = group_id;
  node->properties = group_prop;

  *out_node = node;
  return CMP_SUCCESS;
}

int cmp_ui_image_view_create(cmp_ui_node_t **out_node, const char *image_path) {
  cmp_ui_node_t *node;
  char *path_copy;
  size_t len;

  if (out_node == NULL || image_path == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 7; /* ImageView */

  if (cmp_layout_node_create(&node->layout) != CMP_SUCCESS) {
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

  len = strlen(image_path);
  if (CMP_MALLOC(len + 1, (void **)&path_copy) != CMP_SUCCESS) {
    cmp_layout_node_destroy(node->layout);
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

#if defined(_MSC_VER)
  strcpy_s(path_copy, len + 1, image_path);
#else
  strcpy(path_copy, image_path);
#endif

  node->properties = path_copy;
  *out_node = node;
  return CMP_SUCCESS;
}

int cmp_ui_slider_create(cmp_ui_node_t **out_node, float min, float max) {
  cmp_ui_node_t *node;
  float *bounds;

  if (out_node == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 8; /* Slider */

  if (cmp_layout_node_create(&node->layout) != CMP_SUCCESS) {
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

  if (CMP_MALLOC(sizeof(float) * 2, (void **)&bounds) != CMP_SUCCESS) {
    cmp_layout_node_destroy(node->layout);
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

  bounds[0] = min;
  bounds[1] = max;
  node->properties = bounds;

  *out_node = node;
  return CMP_SUCCESS;
}

int cmp_ui_list_view_create(cmp_ui_node_t **out_node) {
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 9; /* ListView */

  if (cmp_layout_node_create(&node->layout) != CMP_SUCCESS) {
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

  *out_node = node;
  return CMP_SUCCESS;
}

int cmp_ui_grid_view_create(cmp_ui_node_t **out_node, int columns) {
  cmp_ui_node_t *node;
  int *cols_prop;

  if (out_node == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 10; /* GridView */

  if (cmp_layout_node_create(&node->layout) != CMP_SUCCESS) {
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

  if (CMP_MALLOC(sizeof(int), (void **)&cols_prop) != CMP_SUCCESS) {
    cmp_layout_node_destroy(node->layout);
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

  *cols_prop = columns;
  node->properties = cols_prop;

  *out_node = node;
  return CMP_SUCCESS;
}

int cmp_ui_dropdown_create(cmp_ui_node_t **out_node) {
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 11; /* Dropdown */

  if (cmp_layout_node_create(&node->layout) != CMP_SUCCESS) {
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

  *out_node = node;
  return CMP_SUCCESS;
}

int cmp_ui_modal_create(cmp_ui_node_t **out_node) {
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 12; /* Modal */

  if (cmp_layout_node_create(&node->layout) != CMP_SUCCESS) {
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

  *out_node = node;
  return CMP_SUCCESS;
}

int cmp_ui_canvas_create(cmp_ui_node_t **out_node) {
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 13; /* Canvas */

  if (cmp_layout_node_create(&node->layout) != CMP_SUCCESS) {
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

  *out_node = node;
  return CMP_SUCCESS;
}

int cmp_ui_rich_text_create(cmp_ui_node_t **out_node) {
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 14; /* Rich Text */

  if (cmp_layout_node_create(&node->layout) != CMP_SUCCESS) {
    CMP_FREE(node);
    return CMP_ERROR_OOM;
  }

  *out_node = node;
  return CMP_SUCCESS;
}

int cmp_ui_node_add_child(cmp_ui_node_t *parent, cmp_ui_node_t *child) {
  if (parent == NULL || child == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (parent->child_count >= parent->child_capacity) {
    size_t new_cap =
        parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;
    cmp_ui_node_t **new_children;

    if (CMP_MALLOC(sizeof(cmp_ui_node_t *) * new_cap, (void **)&new_children) !=
        CMP_SUCCESS) {
      return CMP_ERROR_OOM;
    }

    if (parent->children != NULL) {
      memcpy(new_children, parent->children,
             sizeof(cmp_ui_node_t *) * parent->child_count);
      CMP_FREE(parent->children);
    }

    parent->children = new_children;
    parent->child_capacity = new_cap;
  }

  /* Link UI Hierarchy */
  parent->children[parent->child_count++] = child;
  child->parent = parent;

  /* Link Layout Hierarchy */
  cmp_layout_node_add_child(parent->layout, child->layout);

  return CMP_SUCCESS;
}

int cmp_ui_node_destroy(cmp_ui_node_t *node) {
  size_t i;

  cmp_event_listener_node_t *listener;
  cmp_event_listener_node_t *next_listener;

  if (node == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  listener = node->event_listeners;
  while (listener) {
    next_listener = listener->next;
    CMP_FREE(listener);
    listener = next_listener;
  }

  for (i = 0; i < node->child_count; i++) {
    cmp_ui_node_destroy(node->children[i]);
  }

  if (node->children != NULL) {
    CMP_FREE(node->children);
  }

  if (node->layout != NULL) {
    /* Layout tree is not destroyed here recursively because children's layouts
     * are destroyed when the child UI node is destroyed. We just destroy the
     * single layout node itself by detaching it first to prevent double-free if
     * someone calls cmp_layout_node_destroy manually */
    node->layout->child_count = 0;
    cmp_layout_node_destroy(node->layout);
  }

  if (node->type == 2 && node->properties != NULL) {
    /* Free text string */
    CMP_FREE(node->properties);
  }

  CMP_FREE(node);
  return CMP_SUCCESS;
}
