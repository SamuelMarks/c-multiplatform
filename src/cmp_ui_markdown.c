/* clang-format off */
#include "cmp_ui_markdown.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_markdown {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_text;
  char *text;
};

int cmp_ui_markdown_create(cmp_ui_markdown_t **out_md, const char *text,
                           uint32_t bg_color, uint32_t text_color) {
  cmp_ui_markdown_t *md;
  int err;
  size_t len;

  if (!out_md) {
    return CMP_ERROR_INVALID_ARG;
  }

  md = (cmp_ui_markdown_t *)malloc(sizeof(cmp_ui_markdown_t));
  if (!md) {
    return CMP_ERROR_OOM;
  }

  md->text = NULL;
  if (text) {
    len = strlen(text);
    md->text = (char *)malloc(len + 1);
    if (md->text) {
      memcpy(md->text, text, len + 1);
    }
  }

  err = cmp_ui_box_create(&md->node_root);
  if (err != 0) {
    free(md->text);
    free(md);
    return err;
  }

  md->node_root->bg_color = bg_color;

  err = cmp_ui_text_create(&md->node_text, md->text ? md->text : "", -1);
  if (err != 0) {
    free(md->text);
    free(md);
    return err;
  }

  md->node_text->text_color = text_color;

  cmp_ui_node_add_child(md->node_root, md->node_text);

  *out_md = md;
  return 0;
}

int cmp_ui_markdown_destroy(cmp_ui_markdown_t *md) {
  if (!md) {
    return CMP_ERROR_INVALID_ARG;
  }
  free(md->text);
  free(md);
  return 0;
}

int cmp_ui_markdown_get_node(cmp_ui_markdown_t *md, cmp_ui_node_t **out_node) {
  if (!md || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = md->node_root;
  return 0;
}

int cmp_ui_markdown_set_text(cmp_ui_markdown_t *md, const char *text) {
  size_t len;

  if (!md) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (md->text) {
    free(md->text);
    md->text = NULL;
  }

  if (text) {
    len = strlen(text);
    md->text = (char *)malloc(len + 1);
    if (!md->text) {
      return CMP_ERROR_OOM;
    }
    memcpy(md->text, text, len + 1);
  }

  return 0;
}
