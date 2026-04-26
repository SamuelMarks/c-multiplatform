/* clang-format off */
#include "cmp_ui_tooltip.h"
#include <stdlib.h>
#include <string.h>
#include "cmp_log.h"
/* clang-format on */

struct cmp_ui_tooltip {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_text;
  char *text;
};

/**
 * @brief cmp_ui_tooltip_create
 *
 * @param out_tooltip Parameter description.
 * @param text Parameter description.
 * @param bg_color Parameter description.
 * @param text_color Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_tooltip_create(cmp_ui_tooltip_t **out_tooltip, const char *text,
                          uint32_t bg_color, uint32_t text_color) {
  int rc = CMP_SUCCESS;
  cmp_ui_tooltip_t *tooltip;
  int err;
  size_t len;
  cmp_string_t translated = {NULL, 0, 0};
  const char *final_text = text;

  if (!out_tooltip) {
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_tooltip_t), (void **)&(tooltip));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  if (text && cmp_i18n_translate(text, &translated) == 0 && translated.data) {
    final_text = translated.data;
  }

  tooltip->text = NULL;
  if (final_text) {
    len = strlen(final_text);
    rc = CMP_MALLOC(len + 1, (void **)&(tooltip->text));
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("OOM\n");
      return CMP_ERROR_OOM;
    }
    if (tooltip->text) {
      memcpy(tooltip->text, final_text, len + 1);
    }
  }

  if (translated.data) {
    cmp_string_destroy(&translated);
  }

  err = cmp_ui_box_create(&tooltip->node_root);
  if (err != 0) {
    rc = CMP_FREE(tooltip->text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    rc = CMP_FREE(tooltip);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    return err;
  }

  tooltip->node_root->bg_color = bg_color;

  err = cmp_ui_text_create(&tooltip->node_text,
                           tooltip->text ? tooltip->text : "", -1);
  if (err != 0) {
    rc = CMP_FREE(tooltip->text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    rc = CMP_FREE(tooltip);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    return err;
  }

  tooltip->node_text->text_color = text_color;

  cmp_ui_node_add_child(tooltip->node_root, tooltip->node_text);

  *out_tooltip = tooltip;
  return 0;
}

/**
 * @brief cmp_ui_tooltip_destroy
 *
 * @param tooltip Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_tooltip_destroy(cmp_ui_tooltip_t *tooltip) {
  int rc = CMP_SUCCESS;
  if (!tooltip) {
    return CMP_ERROR_INVALID_ARG;
  }
  rc = CMP_FREE(tooltip->text);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  rc = CMP_FREE(tooltip);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  return 0;
}

/**
 * @brief cmp_ui_tooltip_get_node
 *
 * @param tooltip Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_tooltip_get_node(cmp_ui_tooltip_t *tooltip,
                            cmp_ui_node_t **out_node) {
  if (!tooltip || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = tooltip->node_root;
  return 0;
}

/**
 * @brief cmp_ui_tooltip_set_text
 *
 * @param tooltip Parameter description.
 * @param text Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_tooltip_set_text(cmp_ui_tooltip_t *tooltip, const char *text) {
  int rc = CMP_SUCCESS;
  size_t len;
  cmp_string_t translated = {NULL, 0, 0};
  const char *final_text = text;

  if (!tooltip) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (tooltip->text) {
    rc = CMP_FREE(tooltip->text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    tooltip->text = NULL;
  }

  if (text && cmp_i18n_translate(text, &translated) == 0 && translated.data) {
    final_text = translated.data;
  }

  if (final_text) {
    len = strlen(final_text);
    rc = CMP_MALLOC(len + 1, (void **)&(tooltip->text));
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("OOM\n");
      return CMP_ERROR_OOM;
    }
    memcpy(tooltip->text, final_text, len + 1);
  }

  if (translated.data) {
    cmp_string_destroy(&translated);
  }

  return 0;
}

/**
 * @brief cmp_ui_tooltip_bind_a11y
 *
 * @param widget Parameter description.
 * @param tree Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_tooltip_bind_a11y(cmp_ui_tooltip_t *widget, cmp_a11y_tree_t *tree) {
  if (!widget || !tree) {
    return CMP_ERROR_INVALID_ARG;
  }
  cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "tooltip",
                         "Tooltip");
  return 0;
}
