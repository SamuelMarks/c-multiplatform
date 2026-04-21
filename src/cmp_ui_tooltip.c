/* clang-format off */
#include "cmp_ui_tooltip.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_tooltip {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_text;
  char *text;
};

int cmp_ui_tooltip_create(cmp_ui_tooltip_t **out_tooltip, const char *text,
                          uint32_t bg_color, uint32_t text_color) {
  cmp_ui_tooltip_t *tooltip;
  int err;
  size_t len;
  cmp_string_t translated = {NULL, 0, 0};
  const char *final_text = text;

  if (!out_tooltip) {
    return CMP_ERROR_INVALID_ARG;
  }

  tooltip = (cmp_ui_tooltip_t *)malloc(sizeof(cmp_ui_tooltip_t));
  if (!tooltip) {
    return CMP_ERROR_OOM;
  }

  if (text && cmp_i18n_translate(text, &translated) == 0 && translated.data) {
    final_text = translated.data;
  }

  tooltip->text = NULL;
  if (final_text) {
    len = strlen(final_text);
    tooltip->text = (char *)malloc(len + 1);
    if (tooltip->text) {
      memcpy(tooltip->text, final_text, len + 1);
    }
  }

  if (translated.data) {
    cmp_string_destroy(&translated);
  }

  err = cmp_ui_box_create(&tooltip->node_root);
  if (err != 0) {
    free(tooltip->text);
    free(tooltip);
    return err;
  }

  tooltip->node_root->bg_color = bg_color;

  err = cmp_ui_text_create(&tooltip->node_text,
                           tooltip->text ? tooltip->text : "", -1);
  if (err != 0) {
    free(tooltip->text);
    free(tooltip);
    return err;
  }

  tooltip->node_text->text_color = text_color;

  cmp_ui_node_add_child(tooltip->node_root, tooltip->node_text);

  *out_tooltip = tooltip;
  return 0;
}

int cmp_ui_tooltip_destroy(cmp_ui_tooltip_t *tooltip) {
  if (!tooltip) {
    return CMP_ERROR_INVALID_ARG;
  }
  free(tooltip->text);
  free(tooltip);
  return 0;
}

int cmp_ui_tooltip_get_node(cmp_ui_tooltip_t *tooltip,
                            cmp_ui_node_t **out_node) {
  if (!tooltip || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = tooltip->node_root;
  return 0;
}

int cmp_ui_tooltip_set_text(cmp_ui_tooltip_t *tooltip, const char *text) {
  size_t len;
  cmp_string_t translated = {NULL, 0, 0};
  const char *final_text = text;

  if (!tooltip) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (tooltip->text) {
    free(tooltip->text);
    tooltip->text = NULL;
  }

  if (text && cmp_i18n_translate(text, &translated) == 0 && translated.data) {
    final_text = translated.data;
  }

  if (final_text) {
    len = strlen(final_text);
    tooltip->text = (char *)malloc(len + 1);
    if (!tooltip->text) {
      if (translated.data) {
        cmp_string_destroy(&translated);
      }
      return CMP_ERROR_OOM;
    }
    memcpy(tooltip->text, final_text, len + 1);
  }

  if (translated.data) {
    cmp_string_destroy(&translated);
  }

  return 0;
}
