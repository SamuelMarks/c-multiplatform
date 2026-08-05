/* clang-format off */
#include "ui_form_node_internal.h"
#include "ui_form_group.h"
#include "ui_form_array.h"
/* clang-format on */

ui_error_t _ui_form_node_get_status_internal(ui_form_node_t node,
                                             enum ui_form_status *out_status,
                                             size_t depth) {
  ui_error_t rc;
  ui_signal_t *sig;
  union ui_signal_payload payload;

  if (!out_status) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (depth > UI_FORM_MAX_DEPTH) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  if (node.type == UI_FORM_NODE_CONTROL) {
    rc = ui_form_control_get_status_signal(node.node.control, &sig);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_signal_get(sig, &payload);
    if (rc != UI_ERROR_NONE)
      return rc;
    *out_status = (enum ui_form_status)payload.int_val;
    return UI_ERROR_NONE;
  } else if (node.type == UI_FORM_NODE_GROUP) {
    return _ui_form_group_get_status_internal(node.node.group, out_status,
                                              depth + 1);
  } else if (node.type == UI_FORM_NODE_ARRAY) {
    return _ui_form_array_get_status_internal(node.node.array, out_status,
                                              depth + 1);
  }

  return UI_ERROR_INVALID_ARGUMENT;
}
