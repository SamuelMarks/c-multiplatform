/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include "themes/cmp_f2_icons.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
/* clang-format on */

/**
 * @brief cmp_f2_icon_create
 *
 * @param out_node Parameter description.
 * @param codepoint_regular Parameter description.
 * @param codepoint_filled Parameter description.
 * @param size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_icon_create(cmp_ui_node_t **out_node,
                               uint32_t codepoint_regular,
                               uint32_t codepoint_filled, float size) {
  int rc = 0;
  cmp_f2_icon_t *icon_data;
  int res;
  char codepoint_str[8];

  if (!out_node) {
    LOG_DEBUG("cmp_f2_icon_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* Arbitrary codepoint conversion to char[] logic. For true fluent mapping
     these would be actual UTF-8 translations */
  memset(codepoint_str, 0, sizeof(codepoint_str));
  codepoint_str[0] = (char)codepoint_regular; /* Simplistic fallback */

  res = cmp_ui_text_create(out_node, codepoint_str, -1);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_icon_create: cmp_ui_text_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_icon_t), (void **)&icon_data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_icon_create: OOM\n");
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  icon_data->codepoint_regular = codepoint_regular;
  icon_data->codepoint_filled = codepoint_filled;
  icon_data->is_filled = 0;
  icon_data->size = size;

  (*out_node)->properties = (void *)icon_data;

  /* Explicit sizing matching Fluent 2 (10, 12, 16, 20, 24, 28, 32, 48) */
  (*out_node)->layout->width = size;
  (*out_node)->layout->height = size;

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
 * @brief cmp_f2_icon_set_filled
 *
 * @param icon_node Parameter description.
 * @param is_filled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_icon_set_filled(cmp_ui_node_t *icon_node, int is_filled) {
  int rc = 0;
  cmp_f2_icon_t *icon_data;
  char codepoint_str[8];

  if (!icon_node || !icon_node->properties) {
    LOG_DEBUG("cmp_f2_icon_set_filled: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  icon_data = (cmp_f2_icon_t *)icon_node->properties;
  icon_data->is_filled = is_filled ? 1 : 0;

  memset(codepoint_str, 0, sizeof(codepoint_str));
  if (icon_data->is_filled) {
    codepoint_str[0] = (char)icon_data->codepoint_filled;
  } else {
    codepoint_str[0] = (char)icon_data->codepoint_regular;
  }

  /* Internal component update mapping not yet fully implemented in generic tree
   */
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