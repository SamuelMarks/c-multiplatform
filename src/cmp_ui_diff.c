/* clang-format off */
#include "cmp_ui_diff.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Opaque internal structure for UI Diff widget.
 */
struct cmp_ui_diff {
  /** @brief The root node of the diff component */
  cmp_ui_node_t *node_root;
  /** @brief The old text string */
  char *old_text;
  /** @brief The new text string */
  char *new_text;
};

/**
 * @brief cmp_ui_diff_create
 *
 * @param out_diff Pointer to store the created diff handle.
 * @param old_text The original text.
 * @param new_text The updated text.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_diff_create(cmp_ui_diff_t **out_diff, const char *old_text,
                       const char *new_text) {
  cmp_ui_diff_t *diff = NULL;
  int rc = CMP_SUCCESS;
  size_t len;

  if (!out_diff) {
    LOG_DEBUG("cmp_ui_diff_create: out_diff is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_diff_t), (void **)&diff);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_diff_create: OOM\n");
    return CMP_ERROR_OOM;
  }
  memset(diff, 0, sizeof(cmp_ui_diff_t));

  diff->old_text = NULL;
  if (old_text) {
    len = strlen(old_text);
    rc = CMP_MALLOC(len + 1, (void **)&diff->old_text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_diff_create: OOM old_text\n");
      rc = CMP_FREE(diff);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_diff_create: CMP_FREE diff failed\n");
      }
      return CMP_ERROR_OOM;
    }
    memcpy(diff->old_text, old_text, len + 1);
  }

  diff->new_text = NULL;
  if (new_text) {
    len = strlen(new_text);
    rc = CMP_MALLOC(len + 1, (void **)&diff->new_text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_diff_create: OOM new_text\n");
      if (diff->old_text) {
        int free_rc = CMP_FREE(diff->old_text);
        if (free_rc != CMP_SUCCESS)
          LOG_DEBUG("cmp_ui_diff_create: CMP_FREE old_text failed\n");
      }
      rc = CMP_FREE(diff);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_diff_create: CMP_FREE diff failed\n");
      }
      return CMP_ERROR_OOM;
    }
    memcpy(diff->new_text, new_text, len + 1);
  }

  rc = cmp_ui_box_create(&diff->node_root);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_diff_create: cmp_ui_box_create failed\n");
    if (diff->old_text) {
      int free_rc = CMP_FREE(diff->old_text);
      if (free_rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_diff_create: CMP_FREE old_text failed\n");
    }
    if (diff->new_text) {
      int free_rc = CMP_FREE(diff->new_text);
      if (free_rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_diff_create: CMP_FREE new_text failed\n");
    }
    rc = CMP_FREE(diff);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_diff_create: CMP_FREE diff failed\n");
    }
    return CMP_ERROR_GENERAL;
  }

  *out_diff = diff;
  return rc;
}

/**
 * @brief cmp_ui_diff_destroy
 *
 * @param diff The diff component.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_diff_destroy(cmp_ui_diff_t *diff) {
  int rc = CMP_SUCCESS;

  if (!diff) {
    LOG_DEBUG("cmp_ui_diff_destroy: diff is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (diff->old_text) {
    rc = CMP_FREE(diff->old_text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_diff_destroy: CMP_FREE old_text failed\n");
    }
  }
  if (diff->new_text) {
    rc = CMP_FREE(diff->new_text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_diff_destroy: CMP_FREE new_text failed\n");
    }
  }
  if (diff->node_root) {
    rc = cmp_ui_node_destroy(diff->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_diff_destroy: cmp_ui_node_destroy failed\n");
    }
  }

  rc = CMP_FREE(diff);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_diff_destroy: CMP_FREE diff failed\n");

    return rc;
  }
  return rc;
}

/**
 * @brief cmp_ui_diff_get_node
 *
 * @param diff The diff component.
 * @param out_node Pointer to store the underlying UI node.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_diff_get_node(cmp_ui_diff_t *diff, cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  if (!diff || !out_node) {
    LOG_DEBUG("cmp_ui_diff_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = diff->node_root;

  return rc;
}