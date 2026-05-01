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
 * @brief Creates a new UI markdown component.
 *
 * @param out_md Pointer to store the newly created markdown context.
 * @param text The markdown source text to render.
 * @param bg_color The background color.
 * @param text_color The foreground text color.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_markdown_create(cmp_ui_markdown_t **out_md, const char *text,
                           uint32_t bg_color, uint32_t text_color) {
  int rc = CMP_SUCCESS;
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
  return rc;
}

/**
 * @brief Destroys a UI markdown component and frees its resources.
 *
 * @param md Pointer to the markdown context to destroy.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_markdown_destroy(cmp_ui_markdown_t *md) {
  int rc = CMP_SUCCESS;
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
  return rc;
}

/**
 * @brief Retrieves the root UI node of the markdown component.
 *
 * @param md Pointer to the markdown context.
 * @param out_node Pointer to store the root UI node pointer.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_markdown_get_node(cmp_ui_markdown_t *md, cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  if (!md || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = md->node_root;

  return rc;
}

/**
 * @brief Updates the markdown text for the component.
 *
 * @param md Pointer to the markdown context.
 * @param text The new markdown source text.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_markdown_set_text(cmp_ui_markdown_t *md, const char *text) {
  int rc = CMP_SUCCESS;
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

  return rc;
}
