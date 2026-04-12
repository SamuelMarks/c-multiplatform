/* clang-format off */
#include "cmp_ui_accordion.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_accordion {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_title;
  char *title;
  int is_expanded;
};

int cmp_ui_accordion_create(cmp_ui_accordion_t **out_accordion,
                            const char *title, uint32_t bg_color) {
  cmp_ui_accordion_t *accordion;
  int err;
  size_t len;
  cmp_string_t translated = {NULL, 0, 0};
  const char *final_title = title;

  if (!out_accordion) {
    return CMP_ERROR_INVALID_ARG;
  }

  accordion = (cmp_ui_accordion_t *)malloc(sizeof(cmp_ui_accordion_t));
  if (!accordion) {
    return CMP_ERROR_OOM;
  }

  if (title && cmp_i18n_translate(title, &translated) == 0 && translated.data) {
    final_title = translated.data;
  }

  accordion->title = NULL;
  if (final_title) {
    len = strlen(final_title);
    accordion->title = (char *)malloc(len + 1);
    if (accordion->title) {
      memcpy(accordion->title, final_title, len + 1);
    }
  }

  if (translated.data) {
    cmp_string_destroy(&translated);
  }

  accordion->is_expanded = 0;

  err = cmp_ui_box_create(&accordion->node_root);
  if (err != 0) {
    free(accordion->title);
    free(accordion);
    return err;
  }

  accordion->node_root->bg_color = bg_color;

  err = cmp_ui_text_create(&accordion->node_title,
                           accordion->title ? accordion->title : "", -1);
  if (err != 0) {
    free(accordion->title);
    free(accordion);
    return err;
  }

  cmp_ui_node_add_child(accordion->node_root, accordion->node_title);

  *out_accordion = accordion;
  return 0;
}

int cmp_ui_accordion_destroy(cmp_ui_accordion_t *accordion) {
  if (!accordion) {
    return CMP_ERROR_INVALID_ARG;
  }
  free(accordion->title);
  free(accordion);
  return 0;
}

int cmp_ui_accordion_get_node(cmp_ui_accordion_t *accordion,
                              cmp_ui_node_t **out_node) {
  if (!accordion || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = accordion->node_root;
  return 0;
}

int cmp_ui_accordion_set_expanded(cmp_ui_accordion_t *accordion,
                                  int is_expanded) {
  if (!accordion) {
    return CMP_ERROR_INVALID_ARG;
  }
  accordion->is_expanded = is_expanded;
  return 0;
}
