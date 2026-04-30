/* clang-format off */
#include "cmp_ui_badge.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Opaque internal structure for UI Badge widget.
 */
struct cmp_ui_badge {
  /** @brief The root box node of the badge */
  cmp_ui_node_t *node_root;
  /** @brief The text node containing the badge text */
  cmp_ui_node_t *node_text;
  /** @brief The raw string of the badge text */
  char *text;
};

/**
 * @brief cmp_ui_badge_create
 *
 * @param out_badge Pointer to output the newly created badge.
 * @param text The badge text.
 * @param bg_color The background color.
 * @param text_color The text color.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_badge_create(cmp_ui_badge_t **out_badge, const char *text,
                        uint32_t bg_color, uint32_t text_color) {
  int rc = CMP_SUCCESS;
  cmp_ui_badge_t *badge = NULL;
  size_t len = 0;
  cmp_string_t translated = {NULL, 0, 0};
  const char *final_text = text;

  if (!out_badge) {
    LOG_DEBUG("cmp_ui_badge_create: Invalid argument out_badge\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_badge_t), (void **)&(badge));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_badge_create: OOM\n");
    return CMP_ERROR_OOM;
  }

  badge->node_root = NULL;
  badge->node_text = NULL;
  badge->text = NULL;

  if (text) {
    rc = cmp_i18n_translate(text, &translated);
    if (rc == CMP_SUCCESS && translated.data) {
      final_text = translated.data;
    } else if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_badge_create: cmp_i18n_translate failed\n");
    }
  }

  if (final_text) {
    len = strlen(final_text);
    rc = CMP_MALLOC(len + 1, (void **)&(badge->text));
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_badge_create: OOM text\n");
      if (translated.data) {
        rc = cmp_string_destroy(&translated);
        if (rc != CMP_SUCCESS) {
          LOG_DEBUG("cmp_ui_badge_create: cmp_string_destroy failed\n");
        }
      }
      rc = CMP_FREE(badge);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_badge_create: CMP_FREE badge failed\n");
      }
      return CMP_ERROR_OOM;
    }

    memcpy(badge->text, final_text, len + 1);
  }

  if (translated.data) {
    rc = cmp_string_destroy(&translated);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_badge_create: cmp_string_destroy failed\n");
    }
  }

  rc = cmp_ui_box_create(&badge->node_root);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_badge_create: cmp_ui_box_create failed\n");
    if (badge->text) {
      rc = CMP_FREE(badge->text);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_badge_create: CMP_FREE text failed\n");
      }
    }
    rc = CMP_FREE(badge);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_badge_create: CMP_FREE badge failed\n");
    }

    return rc;
  }

  badge->node_root->bg_color = bg_color;

  rc =
      cmp_ui_text_create(&badge->node_text, badge->text ? badge->text : "", -1);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_badge_create: cmp_ui_text_create failed\n");
    if (badge->text) {
      rc = CMP_FREE(badge->text);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_badge_create: CMP_FREE text failed\n");
      }
    }
    rc = cmp_ui_node_destroy(badge->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_badge_create: cmp_ui_node_destroy failed\n");
    }
    rc = CMP_FREE(badge);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_badge_create: CMP_FREE badge failed\n");
    }

    return rc;
  }

  badge->node_text->text_color = text_color;

  rc = cmp_ui_node_add_child(badge->node_root, badge->node_text);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_badge_create: cmp_ui_node_add_child failed\n");
  }

  *out_badge = badge;
  return rc;
}

/**
 * @brief cmp_ui_badge_destroy
 *
 * @param badge The badge component.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_badge_destroy(cmp_ui_badge_t *badge) {
  int rc = CMP_SUCCESS;
  if (!badge) {
    LOG_DEBUG("cmp_ui_badge_destroy: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (badge->text) {
    rc = CMP_FREE(badge->text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_badge_destroy: CMP_FREE text failed\n");
    }
  }
  if (badge->node_root) {
    rc = cmp_ui_node_destroy(badge->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_badge_destroy: cmp_ui_node_destroy failed\n");
    }
  }
  rc = CMP_FREE(badge);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_badge_destroy: CMP_FREE badge failed\n");

    return rc;
  }
  return rc;
}

/**
 * @brief cmp_ui_badge_get_node
 *
 * @param badge The badge component.
 * @param out_node Pointer to store the node.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_badge_get_node(cmp_ui_badge_t *badge, cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  if (!badge || !out_node) {
    LOG_DEBUG("cmp_ui_badge_get_node: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = badge->node_root;

  return rc;
}

/**
 * @brief cmp_ui_badge_set_text
 *
 * @param badge The badge component.
 * @param text The new text for the badge.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_badge_set_text(cmp_ui_badge_t *badge, const char *text) {
  int rc = CMP_SUCCESS;
  size_t len = 0;
  cmp_string_t translated = {NULL, 0, 0};
  const char *final_text = text;

  if (!badge) {
    LOG_DEBUG("cmp_ui_badge_set_text: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (badge->text) {
    rc = CMP_FREE(badge->text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_badge_set_text: CMP_FREE text failed\n");
    }
    badge->text = NULL;
  }

  if (text) {
    rc = cmp_i18n_translate(text, &translated);
    if (rc == CMP_SUCCESS && translated.data) {
      final_text = translated.data;
    } else if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_badge_set_text: cmp_i18n_translate failed\n");
    }
  }

  if (final_text) {
    len = strlen(final_text);
    rc = CMP_MALLOC(len + 1, (void **)&(badge->text));
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_badge_set_text: OOM\n");
      if (translated.data) {
        rc = cmp_string_destroy(&translated);
        if (rc != CMP_SUCCESS) {
          LOG_DEBUG("cmp_ui_badge_set_text: cmp_string_destroy failed\n");
        }
      }
      return CMP_ERROR_OOM;
    }
    memcpy(badge->text, final_text, len + 1);
  }

  if (translated.data) {
    rc = cmp_string_destroy(&translated);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_badge_set_text: cmp_string_destroy failed\n");
    }
  }

  return rc;
}

/**
 * @brief cmp_ui_badge_bind_a11y
 *
 * @param widget The widget.
 * @param tree The accessibility tree.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_badge_bind_a11y(cmp_ui_badge_t *widget, cmp_a11y_tree_t *tree) {
  int rc = CMP_SUCCESS;
  if (!widget || !tree) {
    LOG_DEBUG("cmp_ui_badge_bind_a11y: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (!widget->node_root || !widget->node_root->layout) {
    LOG_DEBUG("cmp_ui_badge_bind_a11y: Missing layout\n");
    return CMP_ERROR_INVALID_ARG;
  }
  rc = cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "status",
                              "Badge");
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_badge_bind_a11y: cmp_a11y_tree_add_node failed\n");

    return rc;
  }
  return rc;
}
