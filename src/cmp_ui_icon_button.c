/* clang-format off */
#include "cmp_ui_icon_button.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_icon_button {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_icon;
  char *icon_name;
  cmp_ui_icon_button_style_t style;
};

/**
 * @brief cmp_ui_icon_button_create
 *
 * @param out_btn Parameter description.
 * @param icon_name Parameter description.
 * @param style Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_icon_button_create(cmp_ui_icon_button_t **out_btn,
                              const char *icon_name,
                              cmp_ui_icon_button_style_t style) {
  cmp_ui_icon_button_t *btn;
  int rc = CMP_SUCCESS;
  size_t len;

  if (!out_btn) {
    LOG_DEBUG("cmp_ui_icon_button_create: out_btn is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_icon_button_t), (void **)&btn);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_icon_button_create: OOM\n");

    return rc;
  }
  memset(btn, 0, sizeof(cmp_ui_icon_button_t));

  btn->style = style;

  if (icon_name) {
    len = strlen(icon_name);
    rc = CMP_MALLOC(len + 1, (void **)&btn->icon_name);
    if (rc == CMP_SUCCESS) {
#if defined(_MSC_VER)
      if (memcpy_s(btn->icon_name, len + 1, icon_name, len + 1) != 0) {
        LOG_DEBUG("cmp_ui_icon_button_create: memcpy_s failed\n");
        CMP_FREE(btn->icon_name);
        CMP_FREE(btn);
        return CMP_ERROR_GENERAL;
      }
#else
      memcpy(btn->icon_name, icon_name, len + 1);
#endif
    } else {
      LOG_DEBUG("cmp_ui_icon_button_create: OOM icon_name\n");
      CMP_FREE(btn);

      return rc;
    }
  }

  rc = cmp_ui_button_create(&btn->node_root, "", 0);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_icon_button_create: cmp_ui_button_create failed\n");
    if (btn->icon_name) {
      if (CMP_FREE(btn->icon_name) != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_icon_button_create: CMP_FREE icon_name failed\n");
    }
    rc = CMP_FREE(btn);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_icon_button_create: CMP_FREE btn failed\n");
    return CMP_ERROR_GENERAL;
  }

  rc = CMP_MALLOC(sizeof(cmp_layout_node_t), (void **)&btn->node_root->layout);
  if (rc == CMP_SUCCESS) {
    memset(btn->node_root->layout, 0, sizeof(cmp_layout_node_t));
    btn->node_root->layout->id = 1;
  }

  btn->node_root->type = 3; /* Button */

  rc = cmp_ui_text_create(&btn->node_icon, btn->icon_name ? btn->icon_name : "",
                          -1);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_icon_button_create: cmp_ui_text_create failed\n");
    rc = cmp_ui_node_destroy(btn->node_root);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_icon_button_create: cmp_ui_node_destroy failed\n");
    if (btn->icon_name) {
      if (CMP_FREE(btn->icon_name) != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_icon_button_create: CMP_FREE icon_name failed\n");
    }
    rc = CMP_FREE(btn);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_icon_button_create: CMP_FREE btn failed\n");
    return CMP_ERROR_GENERAL;
  }

  rc = cmp_ui_node_add_child(btn->node_root, btn->node_icon);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_icon_button_create: cmp_ui_node_add_child failed\n");
  }

  *out_btn = btn;
  return rc;
}

/**
 * @brief cmp_ui_icon_button_destroy
 *
 * @param btn Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_icon_button_destroy(cmp_ui_icon_button_t *btn) {
  int rc = CMP_SUCCESS;

  if (!btn) {
    LOG_DEBUG("cmp_ui_icon_button_destroy: btn is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (btn->icon_name) {
    rc = CMP_FREE(btn->icon_name);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_icon_button_destroy: CMP_FREE icon_name failed\n");
  }
  if (btn->node_root) {
    rc = cmp_ui_node_destroy(btn->node_root);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_icon_button_destroy: cmp_ui_node_destroy failed\n");
  }

  rc = CMP_FREE(btn);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_icon_button_destroy: CMP_FREE btn failed\n");

    return rc;
  }
  return rc;
}

/**
 * @brief cmp_ui_icon_button_get_node
 *
 * @param btn Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_icon_button_get_node(cmp_ui_icon_button_t *btn,
                                cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  if (!btn || !out_node) {
    LOG_DEBUG("cmp_ui_icon_button_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = btn->node_root;

  return rc;
}

/**
 * @brief cmp_ui_icon_button_set_icon
 *
 * @param btn Parameter description.
 * @param icon_name Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_icon_button_set_icon(cmp_ui_icon_button_t *btn,
                                const char *icon_name) {
  size_t len;
  int rc = CMP_SUCCESS;

  if (!btn) {
    LOG_DEBUG("cmp_ui_icon_button_set_icon: btn is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (btn->icon_name) {
    rc = CMP_FREE(btn->icon_name);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_icon_button_set_icon: CMP_FREE icon_name failed\n");
    btn->icon_name = NULL;
  }

  if (icon_name) {
    len = strlen(icon_name);
    rc = CMP_MALLOC(len + 1, (void **)&btn->icon_name);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_icon_button_set_icon: OOM\n");

      return rc;
    }
#if defined(_MSC_VER)
    if (memcpy_s(btn->icon_name, len + 1, icon_name, len + 1) != 0) {
      LOG_DEBUG("cmp_ui_icon_button_set_icon: memcpy_s failed\n");
      CMP_FREE(btn->icon_name);
      btn->icon_name = NULL;
      return CMP_ERROR_GENERAL;
    }
#else
    memcpy(btn->icon_name, icon_name, len + 1);
#endif
  }

  return rc;
}

/**
 * @brief cmp_ui_icon_button_bind_a11y
 *
 * @param widget Parameter description.
 * @param tree Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_icon_button_bind_a11y(cmp_ui_icon_button_t *widget,
                                 cmp_a11y_tree_t *tree) {
  int rc = CMP_SUCCESS;

  if (!widget || !tree) {
    LOG_DEBUG("cmp_ui_icon_button_bind_a11y: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (!widget->node_root || !widget->node_root->layout) {
    LOG_DEBUG("cmp_ui_icon_button_bind_a11y: widget missing layout\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "button",
                              "Icon Button");
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_icon_button_bind_a11y: cmp_a11y_tree_add_node failed\n");

    return rc;
  }

  rc = cmp_a11y_tree_set_node_traits(tree, widget->node_root->layout->id,
                                     CMP_A11Y_TRAIT_BUTTON);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG(
        "cmp_ui_icon_button_bind_a11y: cmp_a11y_tree_set_node_traits failed\n");

    return rc;
  }

  return rc;
}
