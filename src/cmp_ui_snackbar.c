/* clang-format off */
#include "cmp_ui_snackbar.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_snackbar {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_message;
  cmp_ui_node_t *node_action;
  char *message;
  char *action_label;
};

/**
 * @brief Creates a new UI snackbar alert component.
 *
 * @param out_snackbar Pointer to store the newly created snackbar context.
 * @param message The alert text to display.
 * @param action_label Optional action button label (or NULL for no button).
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_snackbar_create(cmp_ui_snackbar_t **out_snackbar,
                           const char *message, const char *action_label) {
  cmp_ui_snackbar_t *snackbar;
  int rc = CMP_SUCCESS;
  size_t len;

  if (!out_snackbar) {
    LOG_DEBUG("cmp_ui_snackbar_create: out_snackbar is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_snackbar_t), (void **)&snackbar);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_snackbar_create: OOM\n");
    return CMP_ERROR_OOM;
  }
  memset(snackbar, 0, sizeof(cmp_ui_snackbar_t));

  if (message) {
    len = strlen(message);
    rc = CMP_MALLOC(len + 1, (void **)&snackbar->message);
    if (rc == CMP_SUCCESS) {
#if defined(_MSC_VER)
      if (memcpy_s(snackbar->message, len + 1, message, len + 1) != 0) {
        LOG_DEBUG("cmp_ui_snackbar_create: memcpy_s failed\n");
        CMP_FREE(snackbar->message);
        CMP_FREE(snackbar);
        return CMP_ERROR_GENERAL;
      }
#else
      memcpy(snackbar->message, message, len + 1);
#endif
    } else {
      LOG_DEBUG("cmp_ui_snackbar_create: OOM message\n");
      CMP_FREE(snackbar);
      return CMP_ERROR_OOM;
    }
  }

  if (action_label) {
    len = strlen(action_label);
    rc = CMP_MALLOC(len + 1, (void **)&snackbar->action_label);
    if (rc == CMP_SUCCESS) {
#if defined(_MSC_VER)
      if (memcpy_s(snackbar->action_label, len + 1, action_label, len + 1) !=
          0) {
        LOG_DEBUG("cmp_ui_snackbar_create: memcpy_s failed\n");
        CMP_FREE(snackbar->action_label);
        if (snackbar->message)
          CMP_FREE(snackbar->message);
        CMP_FREE(snackbar);
        return CMP_ERROR_GENERAL;
      }
#else
      memcpy(snackbar->action_label, action_label, len + 1);
#endif
    } else {
      LOG_DEBUG("cmp_ui_snackbar_create: OOM action_label\n");
      if (snackbar->message)
        CMP_FREE(snackbar->message);
      CMP_FREE(snackbar);
      return CMP_ERROR_OOM;
    }
  }

  rc = cmp_ui_box_create(&snackbar->node_root);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_snackbar_create: cmp_ui_box_create failed\n");
    if (snackbar->message) CMP_FREE(snackbar->message);
    if (snackbar->action_label) CMP_FREE(snackbar->action_label);
    CMP_FREE(snackbar);
    return rc;
  }

  /* Snackbars are typically a row layout */
  snackbar->node_root->layout->direction = CMP_FLEX_ROW;

  rc = cmp_ui_text_create(&snackbar->node_message,
                          snackbar->message ? snackbar->message : "", -1);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_snackbar_create: cmp_ui_text_create failed\n");
    cmp_ui_node_destroy(snackbar->node_root);
    if (snackbar->message) CMP_FREE(snackbar->message);
    if (snackbar->action_label) CMP_FREE(snackbar->action_label);
    CMP_FREE(snackbar);
    return rc;
  }

  rc = cmp_ui_node_add_child(snackbar->node_root, snackbar->node_message);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_snackbar_create: cmp_ui_node_add_child failed\n");
    cmp_ui_node_destroy(snackbar->node_message);
    cmp_ui_node_destroy(snackbar->node_root);
    if (snackbar->message) CMP_FREE(snackbar->message);
    if (snackbar->action_label) CMP_FREE(snackbar->action_label);
    CMP_FREE(snackbar);
    return rc;
  }

  if (snackbar->action_label) {
    cmp_ui_node_t *btn_text;
    rc = cmp_ui_button_create(&snackbar->node_action, "", 0);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_snackbar_create: cmp_ui_button_create failed\n");
      cmp_ui_node_destroy(snackbar->node_root);
      if (snackbar->message) CMP_FREE(snackbar->message);
      if (snackbar->action_label) CMP_FREE(snackbar->action_label);
      CMP_FREE(snackbar);
      return rc;
    }
    
    rc = cmp_ui_text_create(&btn_text, snackbar->action_label, -1);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_snackbar_create: cmp_ui_text_create btn_text failed\n");
      cmp_ui_node_destroy(snackbar->node_action);
      cmp_ui_node_destroy(snackbar->node_root);
      if (snackbar->message) CMP_FREE(snackbar->message);
      if (snackbar->action_label) CMP_FREE(snackbar->action_label);
      CMP_FREE(snackbar);
      return rc;
    }
    
    rc = cmp_ui_node_add_child(snackbar->node_action, btn_text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_snackbar_create: cmp_ui_node_add_child failed\n");
      cmp_ui_node_destroy(btn_text);
      cmp_ui_node_destroy(snackbar->node_action);
      cmp_ui_node_destroy(snackbar->node_root);
      if (snackbar->message) CMP_FREE(snackbar->message);
      if (snackbar->action_label) CMP_FREE(snackbar->action_label);
      CMP_FREE(snackbar);
      return rc;
    }
    
    rc = cmp_ui_node_add_child(snackbar->node_root, snackbar->node_action);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_snackbar_create: cmp_ui_node_add_child failed\n");
      cmp_ui_node_destroy(snackbar->node_action);
      cmp_ui_node_destroy(snackbar->node_root);
      if (snackbar->message) CMP_FREE(snackbar->message);
      if (snackbar->action_label) CMP_FREE(snackbar->action_label);
      CMP_FREE(snackbar);
      return rc;
    }
  }

  *out_snackbar = snackbar;
  return CMP_SUCCESS;
}

/**
 * @brief Destroys a UI snackbar component and frees its resources.
 *
 * @param snackbar Pointer to the snackbar context to destroy.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_snackbar_destroy(cmp_ui_snackbar_t *snackbar) {
  int rc = CMP_SUCCESS;

  if (!snackbar) {
    LOG_DEBUG("cmp_ui_snackbar_destroy: snackbar is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (snackbar->message) {
    rc = CMP_FREE(snackbar->message);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_snackbar_destroy: CMP_FREE message failed\n");
  }
  if (snackbar->action_label) {
    rc = CMP_FREE(snackbar->action_label);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_snackbar_destroy: CMP_FREE action_label failed\n");
  }
  if (snackbar->node_root) {
    rc = cmp_ui_node_destroy(snackbar->node_root);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_snackbar_destroy: cmp_ui_node_destroy failed\n");
  }

  rc = CMP_FREE(snackbar);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_snackbar_destroy: CMP_FREE failed\n");

    return rc;
  }
  return rc;
}

/**
 * @brief Retrieves the root UI node of the snackbar component.
 *
 * @param snackbar Pointer to the snackbar context.
 * @param out_node Pointer to store the root UI node pointer.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_snackbar_get_node(cmp_ui_snackbar_t *snackbar,
                             cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  if (!snackbar || !out_node) {
    LOG_DEBUG("cmp_ui_snackbar_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = snackbar->node_root;

  return rc;
}

/**
 * @brief Updates the text message displayed by the snackbar.
 *
 * @param snackbar Pointer to the snackbar context.
 * @param message The new text message.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_snackbar_set_message(cmp_ui_snackbar_t *snackbar,
                                const char *message) {
  size_t len;
  int rc = CMP_SUCCESS;

  if (!snackbar) {
    LOG_DEBUG("cmp_ui_snackbar_set_message: snackbar is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (snackbar->message) {
    rc = CMP_FREE(snackbar->message);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_snackbar_set_message: CMP_FREE message failed\n");
    snackbar->message = NULL;
  }

  if (message) {
    len = strlen(message);
    rc = CMP_MALLOC(len + 1, (void **)&snackbar->message);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_snackbar_set_message: OOM message\n");

      return rc;
    }
#if defined(_MSC_VER)
    if (memcpy_s(snackbar->message, len + 1, message, len + 1) != 0) {
      LOG_DEBUG("cmp_ui_snackbar_set_message: memcpy_s failed\n");
      CMP_FREE(snackbar->message);
      snackbar->message = NULL;
      return CMP_ERROR_GENERAL;
    }
#else
    memcpy(snackbar->message, message, len + 1);
#endif

    if (snackbar->node_message && snackbar->node_message->properties) {
      rc = CMP_FREE(snackbar->node_message->properties);
      if (rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_snackbar_set_message: CMP_FREE properties failed\n");
      snackbar->node_message->properties = NULL;
    }
    if (snackbar->node_message) {
      rc = CMP_MALLOC(len + 1, (void **)&snackbar->node_message->properties);
      if (rc == CMP_SUCCESS) {
#if defined(_MSC_VER)
        if (memcpy_s(snackbar->node_message->properties, len + 1, message,
                     len + 1) != 0) {
          LOG_DEBUG(
              "cmp_ui_snackbar_set_message: memcpy_s properties failed\n");
          CMP_FREE(snackbar->node_message->properties);
          snackbar->node_message->properties = NULL;
        }
#else
        memcpy(snackbar->node_message->properties, message, len + 1);
#endif
      } else {
        LOG_DEBUG("cmp_ui_snackbar_set_message: OOM properties\n");
      }
    }
  } else {
    if (snackbar->node_message && snackbar->node_message->properties) {
      rc = CMP_FREE(snackbar->node_message->properties);
      if (rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_snackbar_set_message: CMP_FREE properties failed\n");
      snackbar->node_message->properties = NULL;
    }
    if (snackbar->node_message) {
      rc = CMP_MALLOC(1, (void **)&snackbar->node_message->properties);
      if (rc == CMP_SUCCESS) {
        ((char *)snackbar->node_message->properties)[0] = '\0';
      } else {
        LOG_DEBUG("cmp_ui_snackbar_set_message: OOM properties\n");
      }
    }
  }

  return rc;
}

/**
 * @brief Updates the action button label on the snackbar.
 *
 * @param snackbar Pointer to the snackbar context.
 * @param action_label The new action button label.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_snackbar_set_action(cmp_ui_snackbar_t *snackbar,
                               const char *action_label) {
  size_t len;
  int rc = CMP_SUCCESS;

  if (!snackbar) {
    LOG_DEBUG("cmp_ui_snackbar_set_action: snackbar is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (snackbar->action_label) {
    rc = CMP_FREE(snackbar->action_label);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_snackbar_set_action: CMP_FREE action_label failed\n");
    snackbar->action_label = NULL;
  }

  /* Since the layout tree modification requires replacing or updating children,
   * this is a simplified approach, only changing the internal state for the API
   * structure. Full implementation would need to add/remove the actual button
   * node from the snackbar root node.
   */
  if (action_label) {
    len = strlen(action_label);
    rc = CMP_MALLOC(len + 1, (void **)&snackbar->action_label);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_snackbar_set_action: OOM\n");

      return rc;
    }
#if defined(_MSC_VER)
    if (memcpy_s(snackbar->action_label, len + 1, action_label, len + 1) != 0) {
      LOG_DEBUG("cmp_ui_snackbar_set_action: memcpy_s failed\n");
      CMP_FREE(snackbar->action_label);
      snackbar->action_label = NULL;
      return CMP_ERROR_GENERAL;
    }
#else
    memcpy(snackbar->action_label, action_label, len + 1);
#endif
  }

  return rc;
}
/**
 * @brief Binds the snackbar to an accessibility tree as an alert.
 *
 * @param widget Pointer to the snackbar context.
 * @param tree Pointer to the accessibility tree.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_snackbar_bind_a11y(cmp_ui_snackbar_t *widget,
                              cmp_a11y_tree_t *tree) {
  int rc = CMP_SUCCESS;

  if (!widget || !tree) {
    LOG_DEBUG("cmp_ui_snackbar_bind_a11y: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (!widget->node_root || !widget->node_root->layout) {
    LOG_DEBUG("cmp_ui_snackbar_bind_a11y: widget missing layout\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "alert",
                              "Snackbar");
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_snackbar_bind_a11y: cmp_a11y_tree_add_node failed\n");

    return rc;
  }

  return rc;
}
