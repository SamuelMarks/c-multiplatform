/* clang-format off */
#include "cmp_ui_fab.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Opaque internal structure for UI Floating Action Button widget.
 */
struct cmp_ui_fab {
  /** @brief The root node of the fab */
  cmp_ui_node_t *node_root;
  /** @brief The node containing the fab icon */
  cmp_ui_node_t *node_icon;
  /** @brief The name of the icon */
  char *icon_name;
};

/**
 * @brief cmp_ui_fab_create
 *
 * @param out_fab Pointer to store the created FAB handle.
 * @param icon_name The name or path of the icon to display.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_fab_create(cmp_ui_fab_t **out_fab, const char *icon_name) {
  cmp_ui_fab_t *fab = NULL;
  int rc = CMP_SUCCESS;
  size_t len;

  if (!out_fab) {
    LOG_DEBUG("cmp_ui_fab_create: out_fab is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_fab_t), (void **)&fab);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_fab_create: OOM\n");
    return CMP_ERROR_OOM;
  }
  memset(fab, 0, sizeof(cmp_ui_fab_t));

  if (icon_name) {
    len = strlen(icon_name);
    rc = CMP_MALLOC(len + 1, (void **)&fab->icon_name);
    if (rc == CMP_SUCCESS) {
      memcpy(fab->icon_name, icon_name, len + 1);
    } else {
      LOG_DEBUG("cmp_ui_fab_create: OOM icon_name\n");
      rc = CMP_FREE(fab);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_fab_create: CMP_FREE failed\n");
      }
      return CMP_ERROR_OOM;
    }
  }

  rc = cmp_ui_button_create(&fab->node_root, "", 0);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_fab_create: cmp_ui_button_create failed\n");
    if (fab->icon_name) {
      if (CMP_FREE(fab->icon_name) != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_fab_create: CMP_FREE icon_name failed\n");
    }
    rc = CMP_FREE(fab);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_fab_create: CMP_FREE fab failed\n");
    return CMP_ERROR_GENERAL;
  }

  rc = CMP_MALLOC(sizeof(cmp_layout_node_t), (void **)&fab->node_root->layout);
  if (rc == CMP_SUCCESS) {
    memset(fab->node_root->layout, 0, sizeof(cmp_layout_node_t));
    fab->node_root->layout->id = 1;
  }

  if (fab->node_root) {
    fab->node_root->type = 3; /* Button */
  }

  rc = cmp_ui_text_create(&fab->node_icon, fab->icon_name ? fab->icon_name : "",
                          -1);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_fab_create: cmp_ui_text_create failed\n");
    rc = cmp_ui_node_destroy(fab->node_root);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_fab_create: cmp_ui_node_destroy failed\n");
    if (fab->icon_name) {
      if (CMP_FREE(fab->icon_name) != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_fab_create: CMP_FREE icon_name failed\n");
    }
    rc = CMP_FREE(fab);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_fab_create: CMP_FREE fab failed\n");
    return CMP_ERROR_GENERAL;
  }

  rc = cmp_ui_node_add_child(fab->node_root, fab->node_icon);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_fab_create: cmp_ui_node_add_child failed\n");
  }

  *out_fab = fab;
  return rc;
}

/**
 * @brief cmp_ui_fab_destroy
 *
 * @param fab The FAB component.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_fab_destroy(cmp_ui_fab_t *fab) {
  int rc = CMP_SUCCESS;

  if (!fab) {
    LOG_DEBUG("cmp_ui_fab_destroy: fab is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (fab->icon_name) {
    rc = CMP_FREE(fab->icon_name);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_fab_destroy: CMP_FREE icon_name failed\n");
    }
  }
  if (fab->node_root) {
    rc = cmp_ui_node_destroy(fab->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_fab_destroy: cmp_ui_node_destroy failed\n");
    }
  }

  rc = CMP_FREE(fab);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_fab_destroy: CMP_FREE fab failed\n");

    return rc;
  }

  return rc;
}

/**
 * @brief cmp_ui_fab_get_node
 *
 * @param fab The FAB component.
 * @param out_node Pointer to store the underlying UI node.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_fab_get_node(cmp_ui_fab_t *fab, cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  if (!fab || !out_node) {
    LOG_DEBUG("cmp_ui_fab_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = fab->node_root;

  return rc;
}

/**
 * @brief cmp_ui_fab_set_icon
 *
 * @param fab The FAB component.
 * @param icon_name The new icon name to display.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_fab_set_icon(cmp_ui_fab_t *fab, const char *icon_name) {
  size_t len;
  int rc = CMP_SUCCESS;

  if (!fab) {
    LOG_DEBUG("cmp_ui_fab_set_icon: fab is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (fab->icon_name) {
    rc = CMP_FREE(fab->icon_name);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_fab_set_icon: CMP_FREE icon_name failed\n");
    }
    fab->icon_name = NULL;
  }

  if (icon_name) {
    len = strlen(icon_name);
    rc = CMP_MALLOC(len + 1, (void **)&fab->icon_name);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_fab_set_icon: OOM\n");
      return CMP_ERROR_OOM;
    }
    memcpy(fab->icon_name, icon_name, len + 1);

    if (fab->node_icon && fab->node_icon->properties) {
      rc = CMP_FREE(fab->node_icon->properties);
      if (rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_fab_set_icon: CMP_FREE properties failed\n");
    }
    if (fab->node_icon) {
      rc = CMP_MALLOC(len + 1, (void **)&fab->node_icon->properties);
      if (rc == CMP_SUCCESS) {
        memcpy(fab->node_icon->properties, fab->icon_name, len + 1);
      } else {
        LOG_DEBUG("cmp_ui_fab_set_icon: OOM properties\n");
      }
    }

  } else {
    if (fab->node_icon && fab->node_icon->properties) {
      rc = CMP_FREE(fab->node_icon->properties);
      if (rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_fab_set_icon: CMP_FREE properties failed\n");
    }
    if (fab->node_icon) {
      rc = CMP_MALLOC(1, (void **)&fab->node_icon->properties);
      if (rc == CMP_SUCCESS) {
        ((char *)fab->node_icon->properties)[0] = '\0';
      } else {
        LOG_DEBUG("cmp_ui_fab_set_icon: OOM properties\n");
      }
    }
  }

  return rc;
}

/**
 * @brief cmp_ui_fab_bind_a11y
 *
 * @param widget The component.
 * @param tree The accessibility tree.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_fab_bind_a11y(cmp_ui_fab_t *widget, cmp_a11y_tree_t *tree) {
  int rc = CMP_SUCCESS;

  if (!widget || !tree) {
    LOG_DEBUG("cmp_ui_fab_bind_a11y: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (!widget->node_root || !widget->node_root->layout) {
    LOG_DEBUG("cmp_ui_fab_bind_a11y: widget missing layout\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "button",
                              "Floating Action Button");
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_fab_bind_a11y: cmp_a11y_tree_add_node failed\n");

    return rc;
  }

  rc = cmp_a11y_tree_set_node_traits(tree, widget->node_root->layout->id,
                                     CMP_A11Y_TRAIT_BUTTON);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_fab_bind_a11y: cmp_a11y_tree_set_node_traits failed\n");

    return rc;
  }

  return rc;
}
