/* clang-format off */
#include "cmp_ui_breadcrumbs.h"
#include <stdlib.h>
#include <string.h>
#include "cmp_log.h"
/* clang-format on */

struct cmp_ui_breadcrumbs {
  cmp_ui_node_t *node_root;
  int segment_count;
};

/**
 * @brief cmp_ui_breadcrumbs_create
 *
 * @param out_breadcrumbs Parameter description.
 * @param bg_color Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_breadcrumbs_create(cmp_ui_breadcrumbs_t **out_breadcrumbs,
                              uint32_t bg_color) {
  int rc = CMP_SUCCESS;
  cmp_ui_breadcrumbs_t *breadcrumbs;
  int err;

  if (!out_breadcrumbs) {
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_breadcrumbs_t), (void **)&(breadcrumbs));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  breadcrumbs->segment_count = 0;

  err = cmp_ui_box_create(&breadcrumbs->node_root);
  if (err != 0) {
    rc = CMP_FREE(breadcrumbs);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    return err;
  }

  breadcrumbs->node_root->bg_color = bg_color;

  *out_breadcrumbs = breadcrumbs;
  return 0;
}

/**
 * @brief cmp_ui_breadcrumbs_destroy
 *
 * @param breadcrumbs Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_breadcrumbs_destroy(cmp_ui_breadcrumbs_t *breadcrumbs) {
  int rc = CMP_SUCCESS;
  if (!breadcrumbs) {
    return CMP_ERROR_INVALID_ARG;
  }
  rc = CMP_FREE(breadcrumbs);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  return 0;
}

/**
 * @brief cmp_ui_breadcrumbs_get_node
 *
 * @param breadcrumbs Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_breadcrumbs_get_node(cmp_ui_breadcrumbs_t *breadcrumbs,
                                cmp_ui_node_t **out_node) {
  if (!breadcrumbs || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = breadcrumbs->node_root;
  return 0;
}

/**
 * @brief cmp_ui_breadcrumbs_add_segment
 *
 * @param breadcrumbs Parameter description.
 * @param segment Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_breadcrumbs_add_segment(cmp_ui_breadcrumbs_t *breadcrumbs,
                                   const char *segment) {
  cmp_ui_node_t *node_text;
  int err;
  cmp_string_t translated = {NULL, 0, 0};
  const char *final_segment = segment;

  if (!breadcrumbs || !segment) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (breadcrumbs->segment_count > 0) {
    cmp_ui_node_t *node_sep;
    err = cmp_ui_text_create(&node_sep, " > ", -1);
    if (err == 0) {
      cmp_ui_node_add_child(breadcrumbs->node_root, node_sep);
    }
  }

  if (cmp_i18n_translate(segment, &translated) == 0 && translated.data) {
    final_segment = translated.data;
  }

  err = cmp_ui_text_create(&node_text, final_segment, -1);

  if (translated.data) {
    cmp_string_destroy(&translated);
  }

  if (err != 0) {
    return err;
  }

  cmp_ui_node_add_child(breadcrumbs->node_root, node_text);
  breadcrumbs->segment_count++;

  return 0;
}
