/* clang-format off */
#include "cmp_ui_breadcrumbs.h"
#include <stdlib.h>
#include <string.h>
#include "cmp_log.h"
/* clang-format on */

/**
 * @brief Opaque internal structure for UI Breadcrumbs widget.
 */
struct cmp_ui_breadcrumbs {
  /** @brief The root node of the breadcrumbs */
  cmp_ui_node_t *node_root;
  /** @brief The current number of breadcrumb segments */
  int segment_count;
};

/**
 * @brief cmp_ui_breadcrumbs_create
 *
 * @param out_breadcrumbs Pointer to output the newly created breadcrumbs.
 * @param bg_color The background color.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_breadcrumbs_create(cmp_ui_breadcrumbs_t **out_breadcrumbs,
                              uint32_t bg_color) {
  int rc;
  rc = CMP_SUCCESS;
  cmp_ui_breadcrumbs_t *breadcrumbs = NULL;

  if (!out_breadcrumbs) {
    LOG_DEBUG("cmp_ui_breadcrumbs_create: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_breadcrumbs_t), (void **)&breadcrumbs);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_breadcrumbs_create: OOM\n");
    return CMP_ERROR_OOM;
  }

  breadcrumbs->segment_count = 0;

  rc = cmp_ui_box_create(&breadcrumbs->node_root);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_breadcrumbs_create: cmp_ui_box_create failed\n");
    rc = CMP_FREE(breadcrumbs);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_breadcrumbs_create: CMP_FREE failed\n");
    }
    return CMP_ERROR_GENERAL;
  }

  breadcrumbs->node_root->bg_color = bg_color;

  *out_breadcrumbs = breadcrumbs;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_breadcrumbs_destroy
 *
 * @param breadcrumbs The breadcrumbs component.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_breadcrumbs_destroy(cmp_ui_breadcrumbs_t *breadcrumbs) {
  int rc;
  rc = CMP_SUCCESS;
  if (!breadcrumbs) {
    LOG_DEBUG("cmp_ui_breadcrumbs_destroy: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (breadcrumbs->node_root) {
    rc = cmp_ui_node_destroy(breadcrumbs->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_breadcrumbs_destroy: cmp_ui_node_destroy failed\n");
    }
  }
  rc = CMP_FREE(breadcrumbs);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_breadcrumbs_destroy: CMP_FREE failed\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_breadcrumbs_get_node
 *
 * @param breadcrumbs The breadcrumbs component.
 * @param out_node Pointer to store the UI node.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_breadcrumbs_get_node(cmp_ui_breadcrumbs_t *breadcrumbs,
                                cmp_ui_node_t **out_node) {
  int rc;
  rc = 0;
  if (!breadcrumbs || !out_node) {
    LOG_DEBUG("cmp_ui_breadcrumbs_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = breadcrumbs->node_root;
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
 * @brief cmp_ui_breadcrumbs_add_segment
 *
 * @param breadcrumbs The breadcrumbs component.
 * @param segment The segment label text.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_breadcrumbs_add_segment(cmp_ui_breadcrumbs_t *breadcrumbs,
                                   const char *segment) {
  cmp_ui_node_t *node_text = NULL;
  cmp_ui_node_t *node_sep = NULL;
  int rc;
  rc = CMP_SUCCESS;
  cmp_string_t translated = {NULL, 0, 0};
  const char *final_segment = segment;

  if (!breadcrumbs || !segment) {
    LOG_DEBUG("cmp_ui_breadcrumbs_add_segment: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (breadcrumbs->segment_count > 0) {
    rc = cmp_ui_text_create(&node_sep, " > ", -1);
    if (rc == CMP_SUCCESS) {
      rc = cmp_ui_node_add_child(breadcrumbs->node_root, node_sep);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_breadcrumbs_add_segment: cmp_ui_node_add_child "
                  "separator failed\n");
      }
    } else {
      LOG_DEBUG("cmp_ui_breadcrumbs_add_segment: cmp_ui_text_create separator "
                "failed\n");
    }
  }

  rc = cmp_i18n_translate(segment, &translated);
  if (rc == CMP_SUCCESS && translated.data) {
    final_segment = translated.data;
  } else if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_breadcrumbs_add_segment: cmp_i18n_translate failed\n");
  }

  rc = cmp_ui_text_create(&node_text, final_segment, -1);

  if (translated.data) {
    int destroy_rc = cmp_string_destroy(&translated);
    if (destroy_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_breadcrumbs_add_segment: cmp_string_destroy failed\n");
    }
  }

  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_breadcrumbs_add_segment: cmp_ui_text_create failed\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = cmp_ui_node_add_child(breadcrumbs->node_root, node_text);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_breadcrumbs_add_segment: cmp_ui_node_add_child failed\n");
  }

  breadcrumbs->segment_count++;

  return CMP_SUCCESS;
}
