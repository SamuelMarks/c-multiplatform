/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief cmp_ui_box_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_box_create(cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_box_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_box_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 1; /* Box */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    rc = CMP_FREE(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_text_create
 *
 * @param out_node Parameter description.
 * @param text Parameter description.
 * @param text_len Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_text_create(cmp_ui_node_t **out_node, const char *text,
                       int text_len) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;
  char *text_copy;
  size_t len;

  if (out_node == NULL || text == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_text_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_text_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 2; /* Text */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    rc = CMP_FREE(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  len = (text_len < 0) ? strlen(text) : (size_t)text_len;
  if (CMP_MALLOC(len + 1, (void **)&text_copy) != CMP_SUCCESS) {
    cmp_layout_node_destroy(node->layout);
    rc = CMP_FREE(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    return CMP_ERROR_OOM;
  }

  memcpy(text_copy, text, len);
  text_copy[len] = '\0';

  node->properties = text_copy;
  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_button_create
 *
 * @param out_node Parameter description.
 * @param label Parameter description.
 * @param label_len Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_button_create(cmp_ui_node_t **out_node, const char *label,
                         int label_len) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;
  char *label_copy;
  size_t len;

  if (out_node == NULL || label == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_button_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_button_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 3; /* Button */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    rc = CMP_FREE(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  len = (label_len < 0) ? strlen(label) : (size_t)label_len;
  if (CMP_MALLOC(len + 1, (void **)&label_copy) != CMP_SUCCESS) {
    cmp_layout_node_destroy(node->layout);
    rc = CMP_FREE(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    return CMP_ERROR_OOM;
  }

  memcpy(label_copy, label, len);
  label_copy[len] = '\0';

  node->properties = label_copy;
  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_text_input_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_text_input_create(cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_text_input_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_text_input_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 4; /* Text Input */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    rc = CMP_FREE(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_checkbox_create
 *
 * @param out_node Parameter description.
 * @param label Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_checkbox_create(cmp_ui_node_t **out_node, const char *label) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;
  char *label_copy = NULL;

  if (out_node == NULL || label == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_checkbox_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_checkbox_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 5; /* Checkbox */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    rc = CMP_FREE(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  if (label != NULL) {
    size_t len = strlen(label);
    if (CMP_MALLOC(len + 1, (void **)&label_copy) != CMP_SUCCESS) {
      cmp_layout_node_destroy(node->layout);
      rc = CMP_FREE(node);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
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
  return rc;
}

/**
 * @brief cmp_ui_radio_create
 *
 * @param out_node Parameter description.
 * @param group_id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_radio_create(cmp_ui_node_t **out_node, int group_id) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;
  int *group_prop;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_radio_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_radio_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 6; /* Radio */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    rc = CMP_FREE(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  if (CMP_MALLOC(sizeof(int), (void **)&group_prop) != CMP_SUCCESS) {
    cmp_layout_node_destroy(node->layout);
    rc = CMP_FREE(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    return CMP_ERROR_OOM;
  }

  *group_prop = group_id;
  node->properties = group_prop;

  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_image_view_create
 *
 * @param out_node Parameter description.
 * @param image_path Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_image_view_create(cmp_ui_node_t **out_node, const char *image_path) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;
  char *path_copy = NULL;
  cmp_svg_renderer_t *renderer = NULL;
  void **props = NULL;
  size_t len;

  if (out_node == NULL || image_path == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_image_view_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_image_view_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 7; /* ImageView */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    rc = CMP_FREE(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  len = strlen(image_path);
  if (CMP_MALLOC(len + 1, (void **)&path_copy) == CMP_SUCCESS) {
#if defined(_MSC_VER)
    strcpy_s(path_copy, len + 1, image_path);
#else
    strcpy(path_copy, image_path);
#endif
  }

  if (strstr(image_path, ".svg") != NULL) {
    void *buffer = NULL;
    size_t size = 0;
    if (cmp_svg_renderer_create(&renderer, 0.5f) == CMP_SUCCESS) {
      if (cmp_vfs_read_file_sync(image_path, &buffer, &size) == CMP_SUCCESS &&
          buffer != NULL) {
        char *svg_str;
        if (CMP_MALLOC(size + 1, (void **)&svg_str) == CMP_SUCCESS) {
          char *p;
          memcpy(svg_str, buffer, size);
          svg_str[size] = '\0';

          p = strstr(svg_str, " d=\"");
          if (p) {
            char *start = p + 4;
            char *end = strchr(start, '"');
            if (end) {
              *end = '\0';
              cmp_svg_parse_path_str(start, renderer);
            }
          }
          rc = CMP_FREE(svg_str);
          if (rc != CMP_SUCCESS) {
            LOG_DEBUG("CMP_FREE: %d\n", rc);
            return rc;
          }
        }
        rc = CMP_FREE(buffer);
        if (rc != CMP_SUCCESS) {
          LOG_DEBUG("CMP_FREE: %d\n", rc);
          return rc;
        }
      }
    }
  }

  if (CMP_MALLOC(sizeof(void *) * 2, (void **)&props) == CMP_SUCCESS) {
    props[0] = path_copy;
    props[1] = renderer;
    node->properties = props;
  } else {
    if (path_copy)
      rc = CMP_FREE(path_copy);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    if (renderer)
      cmp_svg_renderer_destroy(renderer);
  }

  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_slider_create
 *
 * @param out_node Parameter description.
 * @param min Parameter description.
 * @param max Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_slider_create(cmp_ui_node_t **out_node, float min, float max) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;
  float *bounds;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_slider_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_slider_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 8; /* Slider */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    rc = CMP_FREE(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  if (CMP_MALLOC(sizeof(float) * 2, (void **)&bounds) != CMP_SUCCESS) {
    cmp_layout_node_destroy(node->layout);
    rc = CMP_FREE(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    return CMP_ERROR_OOM;
  }

  bounds[0] = min;
  bounds[1] = max;
  node->properties = bounds;

  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_list_view_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_list_view_create(cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_list_view_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_list_view_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 9; /* ListView */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    rc = CMP_FREE(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_grid_view_create
 *
 * @param out_node Parameter description.
 * @param columns Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_grid_view_create(cmp_ui_node_t **out_node, int columns) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;
  int *cols_prop;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_grid_view_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_grid_view_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 10; /* GridView */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    rc = CMP_FREE(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  if (CMP_MALLOC(sizeof(int), (void **)&cols_prop) != CMP_SUCCESS) {
    cmp_layout_node_destroy(node->layout);
    rc = CMP_FREE(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    return CMP_ERROR_OOM;
  }

  *cols_prop = columns;
  node->properties = cols_prop;

  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_dropdown_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_dropdown_create(cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_dropdown_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_dropdown_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 11; /* Dropdown */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    rc = CMP_FREE(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_modal_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_modal_create(cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_modal_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_modal_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 12; /* Modal */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    rc = CMP_FREE(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_canvas_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_canvas_create(cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_canvas_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_canvas_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 13; /* Canvas */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    rc = CMP_FREE(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_rich_text_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_rich_text_create(cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_rich_text_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_rich_text_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 14; /* Rich Text */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    rc = CMP_FREE(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_node_add_child
 *
 * @param parent Parameter description.
 * @param child Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_node_add_child(cmp_ui_node_t *parent, cmp_ui_node_t *child) {
  int rc = CMP_SUCCESS;
  if (parent == NULL || child == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_node_add_child: %d\n", rc);

    return rc;
  }

  if (parent->child_count >= parent->child_capacity) {
    size_t new_cap =
        parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;
    cmp_ui_node_t **new_children;

    rc = CMP_MALLOC(sizeof(cmp_ui_node_t *) * new_cap, (void **)&new_children);
    if (rc != CMP_SUCCESS) {

      LOG_DEBUG("cmp_ui_node_add_child CMP_MALLOC: %d\n", rc);

      return rc;
    }

    if (parent->children != NULL) {
      memcpy(new_children, parent->children,
             sizeof(cmp_ui_node_t *) * parent->child_count);
      rc = CMP_FREE(parent->children);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }

    parent->children = new_children;
    parent->child_capacity = new_cap;
  }

  /* Link UI Hierarchy */
  parent->children[parent->child_count++] = child;
  child->parent = parent;

  /* Link Layout Hierarchy */
  cmp_layout_node_add_child(parent->layout, child->layout);

  return rc;
}

/**
 * @brief disabled_cmp_event_dispatch_run
 *
 * @param tree Parameter description.
 * @param target_node Parameter description.
 * @param event Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
CMP_EXEMPT(int disabled_cmp_event_dispatch_run(cmp_ui_node_t *tree,
                                               cmp_ui_node_t *target_node,
                                               cmp_event_t *event)) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *path[64];
  int path_len = 0;
  int i;
  cmp_ui_node_t *curr = target_node;

  if (tree == NULL || target_node == NULL || event == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  /* Build path from target up to root */
  while (curr != NULL && path_len < 64) {
    path[path_len++] = curr;
    if (curr == tree)
      break;
    curr = curr->parent;
  }

  /* Phase 1: Capturing (Root down to Target) */
  for (i = path_len - 1; i >= 0; i--) {
    cmp_event_listener_node_t *listener = path[i]->event_listeners;
    while (listener) {
      if (listener->event_type == event->type && listener->capture) {
        listener->callback(event, path[i], listener->user_data);
      }
      listener = listener->next;
    }
  }

  /* Phase 2: Bubbling (Target up to Root) */
  for (i = 0; i < path_len; i++) {
    cmp_event_listener_node_t *listener = path[i]->event_listeners;
    while (listener) {
      if (listener->event_type == event->type && !listener->capture) {
        listener->callback(event, path[i], listener->user_data);
      }
      listener = listener->next;
    }
  }

  return rc;
}

/**
 * @brief disabled_cmp_ui_node_add_event_listener
 * @param node
 * @param event_type
 * @param capture
 * @param callback
 * @param user_data
 * @return
 */
CMP_EXEMPT(int disabled_cmp_ui_node_add_event_listener(
    cmp_ui_node_t *node, uint32_t event_type, int capture,
    void (*callback)(cmp_event_t *, cmp_ui_node_t *, void *),
    void *user_data)) {
  int rc = CMP_SUCCESS;
  cmp_event_listener_node_t *listener;

  if (node == NULL || callback == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("disabled_cmp_ui_node_add_event_listener: %d\n", rc);
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_event_listener_node_t), (void **)&listener);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("CMP_MALLOC: %d\n", rc);
    return rc;
  }

  listener->event_type = event_type;
  listener->capture = capture;
  listener->callback = callback;
  listener->user_data = user_data;
  listener->next = node->event_listeners;
  node->event_listeners = listener;

  return rc;
}

/**
 * @brief cmp_ui_node_destroy
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_node_destroy(cmp_ui_node_t *node) {
  int rc = CMP_SUCCESS;
  size_t i;

  cmp_event_listener_node_t *listener;
  cmp_event_listener_node_t *next_listener;

  if (node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_node_destroy: %d\n", rc);

    return rc;
  }

  listener = node->event_listeners;
  while (listener) {
    next_listener = listener->next;
    rc = CMP_FREE(listener);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    listener = next_listener;
  }

  for (i = 0; i < node->child_count; i++) {
    cmp_ui_node_destroy(node->children[i]);
  }

  if (node->children != NULL) {
    rc = CMP_FREE(node->children);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
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
    rc = CMP_FREE(node->properties);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
  } else if (node->type == 7 && node->properties != NULL) {
    void **props = (void **)node->properties;
    if (props[0])
      rc = CMP_FREE(props[0]);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    if (props[1])
      cmp_svg_renderer_destroy((cmp_svg_renderer_t *)props[1]);
    rc = CMP_FREE(node->properties);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
  }

  rc = CMP_FREE(node);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("CMP_FREE: %d\n", rc);
    return rc;
  }
  return rc;
}
