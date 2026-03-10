/* clang-format off */
#include "f2/f2_dialog.h"
#include "cmpc/cmp_core.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL f2_dialog_style_init(F2DialogStyle *style) {
  if (!style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(style, 0, sizeof(*style));

  style->background_color = (CMPColor){1.0f, 1.0f, 1.0f, 1.0f};
  style->scrim_color = (CMPColor){0.0f, 0.0f, 0.0f, 0.4f};

  style->corner_radius = F2_DIALOG_DEFAULT_CORNER_RADIUS;
  style->padding_x = F2_DIALOG_DEFAULT_PADDING;
  style->padding_y = F2_DIALOG_DEFAULT_PADDING;
  style->min_width = F2_DIALOG_DEFAULT_MIN_WIDTH;
  style->max_width = F2_DIALOG_DEFAULT_MAX_WIDTH;

  style->title_style.utf8_family =
      "Segoe UI Variable, system-ui, -apple-system, Roboto, Ubuntu";
  style->title_style.size_px = 20;
  style->title_style.weight = 600;
  style->title_style.color = (CMPColor){0.0f, 0.0f, 0.0f, 1.0f};

  style->body_style.utf8_family =
      "Segoe UI Variable, system-ui, -apple-system, Roboto, Ubuntu";
  style->body_style.size_px = 14;
  style->body_style.weight = 400;
  style->body_style.color = (CMPColor){0.2f, 0.2f, 0.2f, 1.0f};

  return CMP_OK;
}

CMP_API int CMP_CALL f2_dialog_init(F2Dialog *dialog,
                                    const F2DialogStyle *style,
                                    const char *utf8_title,
                                    const char *utf8_body) {
  if (!dialog || !style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(dialog, 0, sizeof(*dialog));
  dialog->style = *style;
  dialog->utf8_title = utf8_title;
  dialog->utf8_body = utf8_body;
  dialog->is_open = CMP_FALSE;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_dialog_open(F2Dialog *dialog) {
  if (!dialog)
    return CMP_ERR_INVALID_ARGUMENT;
  dialog->is_open = CMP_TRUE;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_dialog_close(F2Dialog *dialog) {
  if (!dialog)
    return CMP_ERR_INVALID_ARGUMENT;
  dialog->is_open = CMP_FALSE;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_dialog_set_on_action(F2Dialog *dialog,
                                             F2DialogOnAction on_action,
                                             void *ctx) {
  if (!dialog)
    return CMP_ERR_INVALID_ARGUMENT;
  dialog->on_action = on_action;
  dialog->on_action_ctx = ctx;
  return CMP_OK;
}
