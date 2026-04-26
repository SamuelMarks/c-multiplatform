/* clang-format off */
#include "cmp_ui_diff.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_diff {
  cmp_ui_node_t *node_root;
  char *old_text;
  char *new_text;
};

/**
 * @brief cmp_ui_diff_create
 *
 * @param out_diff Parameter description.
 * @param old_text Parameter description.
 * @param new_text Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_diff_create(cmp_ui_diff_t **out_diff, const char *old_text,
                       const char *new_text) {
  cmp_ui_diff_t *diff;
  int rc;
  size_t len;

  if (!out_diff) {
    LOG_DEBUG("cmp_ui_diff_create: out_diff is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_diff_t), (void **)&diff);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_diff_create: OOM\n");
    return rc;
  }
  memset(diff, 0, sizeof(cmp_ui_diff_t));

  diff->old_text = NULL;
  if (old_text) {
    len = strlen(old_text);
    rc = CMP_MALLOC(len + 1, (void **)&diff->old_text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_diff_create: OOM old_text\n");
      CMP_FREE(diff);
      return CMP_ERROR_OOM;
    }
#if defined(_MSC_VER)
    if (memcpy_s(diff->old_text, len + 1, old_text, len + 1) != 0) {
      LOG_DEBUG("cmp_ui_diff_create: memcpy_s failed\n");
      CMP_FREE(diff->old_text);
      CMP_FREE(diff);
      return CMP_ERROR_GENERAL;
    }
#else
    memcpy(diff->old_text, old_text, len + 1);
#endif
  }

  diff->new_text = NULL;
  if (new_text) {
    len = strlen(new_text);
    rc = CMP_MALLOC(len + 1, (void **)&diff->new_text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_diff_create: OOM new_text\n");
      if (diff->old_text)
        CMP_FREE(diff->old_text);
      CMP_FREE(diff);
      return CMP_ERROR_OOM;
    }
#if defined(_MSC_VER)
    if (memcpy_s(diff->new_text, len + 1, new_text, len + 1) != 0) {
      LOG_DEBUG("cmp_ui_diff_create: memcpy_s failed\n");
      CMP_FREE(diff->new_text);
      if (diff->old_text)
        CMP_FREE(diff->old_text);
      CMP_FREE(diff);
      return CMP_ERROR_GENERAL;
    }
#else
    memcpy(diff->new_text, new_text, len + 1);
#endif
  }

  rc = cmp_ui_box_create(&diff->node_root);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_diff_create: cmp_ui_box_create failed\n");
    if (diff->old_text)
      CMP_FREE(diff->old_text);
    if (diff->new_text)
      CMP_FREE(diff->new_text);
    CMP_FREE(diff);
    return rc;
  }

  *out_diff = diff;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_diff_destroy
 *
 * @param diff Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_diff_destroy(cmp_ui_diff_t *diff) {
  int rc;

  if (!diff) {
    LOG_DEBUG("cmp_ui_diff_destroy: diff is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (diff->old_text) {
    rc = CMP_FREE(diff->old_text);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_diff_destroy: CMP_FREE old_text failed\n");
  }
  if (diff->new_text) {
    rc = CMP_FREE(diff->new_text);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_diff_destroy: CMP_FREE new_text failed\n");
  }
  if (diff->node_root) {
    rc = cmp_ui_node_destroy(diff->node_root);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_diff_destroy: cmp_ui_node_destroy failed\n");
  }

  rc = CMP_FREE(diff);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_diff_destroy: CMP_FREE diff failed\n");
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_diff_get_node
 *
 * @param diff Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_diff_get_node(cmp_ui_diff_t *diff, cmp_ui_node_t **out_node) {
  if (!diff || !out_node) {
    LOG_DEBUG("cmp_ui_diff_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = diff->node_root;
  return CMP_SUCCESS;
}