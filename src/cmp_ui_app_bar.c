/* clang-format off */
#include "cmp_ui_app_bar.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Opaque internal structure for UI App Bar widget.
 */
struct cmp_ui_app_bar {
  /** @brief The root app bar node */
  cmp_ui_node_t *node_root;
  /** @brief The text node containing the app bar title */
  cmp_ui_node_t *node_title;
  /** @brief The placement of the app bar (Top or Bottom) */
  cmp_ui_app_bar_placement_t placement;
  /** @brief The raw string of the title */
  char *title;
};

/**
 * @brief cmp_ui_app_bar_create
 *
 * @param out_bar Pointer to output the newly created app bar.
 * @param placement Top or Bottom placement.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_app_bar_create(cmp_ui_app_bar_t **out_bar,
                          cmp_ui_app_bar_placement_t placement) {
  cmp_ui_app_bar_t *bar;
  int rc;

  if (!out_bar) {
    LOG_DEBUG("cmp_ui_app_bar_create: out_bar is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_app_bar_t), (void **)&bar);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_app_bar_create: OOM\n");
    return rc;
  }
  memset(bar, 0, sizeof(cmp_ui_app_bar_t));

  bar->placement = placement;

  rc = cmp_ui_box_create(&bar->node_root);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_app_bar_create: cmp_ui_box_create failed\n");
    rc = CMP_FREE(bar);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_app_bar_create: CMP_FREE failed\n");
    }
    return rc;
  }

  if (bar->node_root && bar->node_root->layout) {
    bar->node_root->layout->direction = CMP_FLEX_ROW;
    bar->node_root->bg_color = 0xFFF5F5F5;
    bar->node_root->layout->width = -1.0f;
    bar->node_root->layout->height = 64.0f;
    bar->node_root->layout->align_items = CMP_FLEX_ALIGN_CENTER;
    bar->node_root->layout->justify_content = CMP_FLEX_ALIGN_SPACE_BETWEEN;
  }

  rc = cmp_ui_text_create(&bar->node_title, "", 0);
  if (rc == CMP_SUCCESS) {
    rc = cmp_ui_node_add_child(bar->node_root, bar->node_title);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_app_bar_create: cmp_ui_node_add_child failed\n");
    }
  } else {
    LOG_DEBUG("cmp_ui_app_bar_create: cmp_ui_text_create failed\n");
    rc = cmp_ui_node_destroy(bar->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_app_bar_create: cmp_ui_node_destroy failed\n");
    }
    rc = CMP_FREE(bar);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_app_bar_create: CMP_FREE failed\n");
    }
    return CMP_ERROR_GENERAL;
  }

  *out_bar = bar;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_app_bar_destroy
 *
 * @param bar The app bar component.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_app_bar_destroy(cmp_ui_app_bar_t *bar) {
  int rc;
  if (!bar) {
    LOG_DEBUG("cmp_ui_app_bar_destroy: bar is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (bar->title) {
    rc = CMP_FREE(bar->title);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_app_bar_destroy: CMP_FREE title failed\n");
    }
  }
  if (bar->node_root) {
    rc = cmp_ui_node_destroy(bar->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_app_bar_destroy: cmp_ui_node_destroy failed\n");
    }
  }
  rc = CMP_FREE(bar);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_app_bar_destroy: CMP_FREE failed\n");
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_app_bar_get_node
 *
 * @param bar The app bar component.
 * @param out_node Pointer to store the node.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_app_bar_get_node(cmp_ui_app_bar_t *bar, cmp_ui_node_t **out_node) {
  if (!bar || !out_node) {
    LOG_DEBUG("cmp_ui_app_bar_get_node: Invalid args\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = bar->node_root;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_app_bar_set_title
 *
 * @param bar The app bar component.
 * @param title The new title.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_app_bar_set_title(cmp_ui_app_bar_t *bar, const char *title) {
  size_t len;
  int rc;

  if (!bar) {
    LOG_DEBUG("cmp_ui_app_bar_set_title: bar is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (bar->title) {
    rc = CMP_FREE(bar->title);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_app_bar_set_title: CMP_FREE title failed\n");
    }
    bar->title = NULL;
  }

  if (title) {
    len = strlen(title);
    rc = CMP_MALLOC(len + 1, (void **)&bar->title);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_app_bar_set_title: OOM\n");
      return rc;
    }
    memcpy(bar->title, title, len + 1);

    if (bar->node_title && bar->node_title->properties) {
      rc = CMP_FREE(bar->node_title->properties);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_app_bar_set_title: CMP_FREE node_title properties "
                  "failed\n");
      }
    }
    if (bar->node_title) {
      rc = CMP_MALLOC(len + 1, (void **)&bar->node_title->properties);
      if (rc == CMP_SUCCESS) {
        memcpy(bar->node_title->properties, title, len + 1);
      } else {
        LOG_DEBUG("cmp_ui_app_bar_set_title: OOM node_title properties\n");
        return CMP_ERROR_OOM;
      }
    }
  } else {
    if (bar->node_title && bar->node_title->properties) {
      rc = CMP_FREE(bar->node_title->properties);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_app_bar_set_title: CMP_FREE node_title properties "
                  "failed\n");
      }
    }
    if (bar->node_title) {
      rc = CMP_MALLOC(1, (void **)&bar->node_title->properties);
      if (rc == CMP_SUCCESS) {
        ((char *)bar->node_title->properties)[0] = '\0';
      } else {
        LOG_DEBUG("cmp_ui_app_bar_set_title: OOM node_title properties\n");
        return CMP_ERROR_OOM;
      }
    }
  }

  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_app_bar_add_action
 *
 * @param bar The app bar component.
 * @param action_node The action node to add.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_app_bar_add_action(cmp_ui_app_bar_t *bar,
                              cmp_ui_node_t *action_node) {
  int rc;
  if (!bar || !action_node) {
    LOG_DEBUG("cmp_ui_app_bar_add_action: Invalid args\n");
    return CMP_ERROR_INVALID_ARG;
  }
  rc = cmp_ui_node_add_child(bar->node_root, action_node);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_app_bar_add_action: cmp_ui_node_add_child failed\n");
  }
  return rc;
}

/**
 * @brief cmp_ui_app_bar_bind_a11y
 *
 * @param widget The component.
 * @param tree The accessibility tree.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_app_bar_bind_a11y(cmp_ui_app_bar_t *widget, cmp_a11y_tree_t *tree) {
  int rc;
  if (!widget || !tree || !widget->node_root || !widget->node_root->layout) {
    LOG_DEBUG("cmp_ui_app_bar_bind_a11y: Invalid args\n");
    return CMP_ERROR_INVALID_ARG;
  }
  rc = cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "group",
                              "App Bar");
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_app_bar_bind_a11y: cmp_a11y_tree_add_node failed\n");
  }
  return CMP_SUCCESS;
}
