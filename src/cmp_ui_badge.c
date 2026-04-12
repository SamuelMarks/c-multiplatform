/* clang-format off */
#include "cmp_ui_badge.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_badge {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_text;
  char *text;
};

int cmp_ui_badge_create(cmp_ui_badge_t **out_badge, const char *text,
                        uint32_t bg_color, uint32_t text_color) {
  cmp_ui_badge_t *badge;
  int err;
  size_t len;
  cmp_string_t translated = {NULL, 0, 0};
  const char *final_text = text;

  if (!out_badge) {
    return CMP_ERROR_INVALID_ARG;
  }

  badge = (cmp_ui_badge_t *)malloc(sizeof(cmp_ui_badge_t));
  if (!badge) {
    return CMP_ERROR_OOM;
  }

  if (text && cmp_i18n_translate(text, &translated) == 0 && translated.data) {
    final_text = translated.data;
  }

  badge->text = NULL;
  if (final_text) {
    len = strlen(final_text);
    badge->text = (char *)malloc(len + 1);
    if (badge->text) {
      memcpy(badge->text, final_text, len + 1);
    }
  }

  if (translated.data) {
    cmp_string_destroy(&translated);
  }

  err = cmp_ui_box_create(&badge->node_root);
  if (err != 0) {
    free(badge->text);
    free(badge);
    return err;
  }

  badge->node_root->bg_color = bg_color;

  err =
      cmp_ui_text_create(&badge->node_text, badge->text ? badge->text : "", -1);
  if (err != 0) {
    free(badge->text);
    free(badge);
    return err;
  }

  badge->node_text->text_color = text_color;

  cmp_ui_node_add_child(badge->node_root, badge->node_text);

  *out_badge = badge;
  return 0;
}

int cmp_ui_badge_destroy(cmp_ui_badge_t *badge) {
  if (!badge) {
    return CMP_ERROR_INVALID_ARG;
  }
  free(badge->text);
  free(badge);
  return 0;
}

int cmp_ui_badge_get_node(cmp_ui_badge_t *badge, cmp_ui_node_t **out_node) {
  if (!badge || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = badge->node_root;
  return 0;
}

int cmp_ui_badge_set_text(cmp_ui_badge_t *badge, const char *text) {
  size_t len;
  cmp_string_t translated = {NULL, 0, 0};
  const char *final_text = text;

  if (!badge) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (badge->text) {
    free(badge->text);
    badge->text = NULL;
  }

  if (text && cmp_i18n_translate(text, &translated) == 0 && translated.data) {
    final_text = translated.data;
  }

  if (final_text) {
    len = strlen(final_text);
    badge->text = (char *)malloc(len + 1);
    if (!badge->text) {
      if (translated.data) {
        cmp_string_destroy(&translated);
      }
      return CMP_ERROR_OOM;
    }
    memcpy(badge->text, final_text, len + 1);
  }

  if (translated.data) {
    cmp_string_destroy(&translated);
  }

  return 0;
}
