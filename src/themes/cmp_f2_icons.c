/* clang-format off */
#include "cmp.h"
#include "themes/cmp_f2_icons.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
/* clang-format on */

CMP_API int cmp_f2_icon_create(cmp_ui_node_t **out_node, uint32_t codepoint_regular,
                       uint32_t codepoint_filled, float size) {
  cmp_f2_icon_t *icon_data;
  int res;
  char codepoint_str[8];

  if (!out_node) {
    return CMP_ERROR_INVALID_ARG;
  }

  /* Arbitrary codepoint conversion to char[] logic. For true fluent mapping
     these would be actual UTF-8 translations */
  memset(codepoint_str, 0, sizeof(codepoint_str));
  codepoint_str[0] = (char)codepoint_regular; /* Simplistic fallback */

  res = cmp_ui_text_create(out_node, codepoint_str, -1);
  if (res != CMP_SUCCESS) {
    return res;
  }

  icon_data = (cmp_f2_icon_t *)malloc(sizeof(cmp_f2_icon_t));
  if (!icon_data) {
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

  return CMP_SUCCESS;
}

CMP_API int cmp_f2_icon_set_filled(cmp_ui_node_t *icon_node, int is_filled) {
  cmp_f2_icon_t *icon_data;
  char codepoint_str[8];

  if (!icon_node || !icon_node->properties) {
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
  return CMP_SUCCESS;
}
