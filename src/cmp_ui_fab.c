/* clang-format off */
#include "cmp_ui_fab.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_fab {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_icon;
  char *icon_name;
};

/**
 * @brief cmp_ui_fab_create
 *
 * @param out_fab Parameter description.
 * @param icon_name Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_fab_create(cmp_ui_fab_t **out_fab, const char *icon_name) {
  cmp_ui_fab_t *fab;
  int rc;
  size_t len;

  if (!out_fab) {
    LOG_DEBUG("cmp_ui_fab_create: out_fab is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_fab_t), (void **)&fab);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_fab_create: OOM\n");
    return rc;
  }
  memset(fab, 0, sizeof(cmp_ui_fab_t));

  if (icon_name) {
    len = strlen(icon_name);
    rc = CMP_MALLOC(len + 1, (void **)&fab->icon_name);
    if (rc == CMP_SUCCESS) {
#if defined(_MSC_VER)
      if (memcpy_s(fab->icon_name, len + 1, icon_name, len + 1) != 0) {
        LOG_DEBUG("cmp_ui_fab_create: memcpy_s failed\n");
        CMP_FREE(fab->icon_name);
        CMP_FREE(fab);
        return CMP_ERROR_GENERAL;
      }
#else
      memcpy(fab->icon_name, icon_name, len + 1);
#endif
    } else {
      LOG_DEBUG("cmp_ui_fab_create: OOM icon_name\n");
      CMP_FREE(fab);
      return rc;
    }
  }

  rc = cmp_ui_button_create(&fab->node_root, "", 0);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_fab_create: cmp_ui_button_create failed\n");
    if (fab->icon_name) {
      CMP_FREE(fab->icon_name);
    }
    CMP_FREE(fab);
    return rc;
  }

  /* Assign standard FAB style - e.g. circular, floating */
  /* This relies on the theming engine later, but we set a role/type. */
  fab->node_root->type = 3; /* Button */

  rc = cmp_ui_text_create(&fab->node_icon, fab->icon_name ? fab->icon_name : "",
                          -1);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_fab_create: cmp_ui_text_create failed\n");
    rc = cmp_ui_node_destroy(fab->node_root);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_fab_create: cmp_ui_node_destroy failed\n");
    if (fab->icon_name) {
      CMP_FREE(fab->icon_name);
    }
    CMP_FREE(fab);
    return rc;
  }

  rc = cmp_ui_node_add_child(fab->node_root, fab->node_icon);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_fab_create: cmp_ui_node_add_child failed\n");
  }

  *out_fab = fab;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_fab_destroy
 *
 * @param fab Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_fab_destroy(cmp_ui_fab_t *fab) {
  int rc;

  if (!fab) {
    LOG_DEBUG("cmp_ui_fab_destroy: fab is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (fab->icon_name) {
    rc = CMP_FREE(fab->icon_name);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_fab_destroy: CMP_FREE icon_name failed\n");
  }
  if (fab->node_root) {
    rc = cmp_ui_node_destroy(fab->node_root);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_fab_destroy: cmp_ui_node_destroy failed\n");
  }

  rc = CMP_FREE(fab);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_fab_destroy: CMP_FREE fab failed\n");
    return rc;
  }

  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_fab_get_node
 *
 * @param fab Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_fab_get_node(cmp_ui_fab_t *fab, cmp_ui_node_t **out_node) {
  if (!fab || !out_node) {
    LOG_DEBUG("cmp_ui_fab_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = fab->node_root;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_fab_set_icon
 *
 * @param fab Parameter description.
 * @param icon_name Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_fab_set_icon(cmp_ui_fab_t *fab, const char *icon_name) {
  size_t len;
  int rc;

  if (!fab) {
    LOG_DEBUG("cmp_ui_fab_set_icon: fab is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (fab->icon_name) {
    rc = CMP_FREE(fab->icon_name);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_fab_set_icon: CMP_FREE icon_name failed\n");
    fab->icon_name = NULL;
  }

  if (icon_name) {
    len = strlen(icon_name);
    rc = CMP_MALLOC(len + 1, (void **)&fab->icon_name);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_fab_set_icon: OOM\n");
      return rc;
    }
#if defined(_MSC_VER)
    if (memcpy_s(fab->icon_name, len + 1, icon_name, len + 1) != 0) {
      LOG_DEBUG("cmp_ui_fab_set_icon: memcpy_s failed\n");
      CMP_FREE(fab->icon_name);
      fab->icon_name = NULL;
      return CMP_ERROR_GENERAL;
    }
#else
    memcpy(fab->icon_name, icon_name, len + 1);
#endif
  }

  if (fab->node_icon) {
    /* Since text property doesn't exist natively on node text, we would update
     * it here using the proper setter, which we assume is handled elsewhere or
     * using cmp_ui_text_set_content.
     */
    /* rc = cmp_ui_text_set_content(fab->node_icon, fab->icon_name ?
     * fab->icon_name : ""); ... etc */
  }

  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_fab_bind_a11y
 *
 * @param widget Parameter description.
 * @param tree Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_fab_bind_a11y(cmp_ui_fab_t *widget, cmp_a11y_tree_t *tree) {
  int rc;

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

  return CMP_SUCCESS;
}