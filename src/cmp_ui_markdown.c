/* clang-format off */
#include "cmp_ui_markdown.h"
#include <stdlib.h>
#include <string.h>
#include "cmp_log.h"
/* clang-format on */

struct cmp_ui_markdown {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_text;
  char *text;
};

/**
 * @brief cmp_ui_markdown_create
 *
 * @param out_md Parameter description.
 * @param text Parameter description.
 * @param bg_color Parameter description.
 * @param text_color Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_markdown_create(cmp_ui_markdown_t **out_md, const char *text,
                           uint32_t bg_color, uint32_t text_color) {
  int rc;
  rc = CMP_SUCCESS;
  cmp_ui_markdown_t *md;
  int err;
  size_t len;

  if (!out_md) {
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_markdown_t), (void **)&(md));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  md->text = NULL;
  if (text) {
    len = strlen(text);
    rc = CMP_MALLOC(len + 1, (void **)&(md->text));
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("OOM\n");
      return CMP_ERROR_OOM;
    }
    if (md->text) {
      memcpy(md->text, text, len + 1);
    }
  }

  err = cmp_ui_box_create(&md->node_root);
  if (err != 0) {
    rc = CMP_FREE(md->text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    rc = CMP_FREE(md);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    return err;
  }

  md->node_root->bg_color = bg_color;

  err = cmp_ui_text_create(&md->node_text, md->text ? md->text : "", -1);
  if (err != 0) {
    rc = CMP_FREE(md->text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    rc = CMP_FREE(md);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    return err;
  }

  md->node_text->text_color = text_color;

  cmp_ui_node_add_child(md->node_root, md->node_text);

  *out_md = md;
  return 0;
}

/**
 * @brief cmp_ui_markdown_destroy
 *
 * @param md Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_markdown_destroy(cmp_ui_markdown_t *md) {
  int rc;
  rc = CMP_SUCCESS;
  if (!md) {
    return CMP_ERROR_INVALID_ARG;
  }
  rc = CMP_FREE(md->text);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  rc = CMP_FREE(md);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  return 0;
}

/**
 * @brief cmp_ui_markdown_get_node
 *
 * @param md Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_markdown_get_node(cmp_ui_markdown_t *md, cmp_ui_node_t **out_node) {
  int rc;
  rc = 0;
  if (!md || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = md->node_root;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_ui_markdown_set_text
 *
 * @param md Parameter description.
 * @param text Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_markdown_set_text(cmp_ui_markdown_t *md, const char *text) {
  int rc;
  rc = CMP_SUCCESS;
  size_t len;

  if (!md) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (md->text) {
    rc = CMP_FREE(md->text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    md->text = NULL;
  }

  if (text) {
    len = strlen(text);
    rc = CMP_MALLOC(len + 1, (void **)&(md->text));
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("OOM\n");
      return CMP_ERROR_OOM;
    }
    memcpy(md->text, text, len + 1);
  }

  return 0;
}
