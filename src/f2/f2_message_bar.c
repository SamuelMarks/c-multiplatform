/* clang-format off */
#include "f2/f2_message_bar.h"
#include <string.h>
/* clang-format on */

static int f2_color_from_u32(cmp_u32 argb, CMPColor *out_color) {
  if (!out_color)
    return CMP_ERR_INVALID_ARGUMENT;
  out_color->a = ((argb >> 24) & 0xFF) / 255.0f;
  out_color->r = ((argb >> 16) & 0xFF) / 255.0f;
  out_color->g = ((argb >> 8) & 0xFF) / 255.0f;
  out_color->b = (argb & 0xFF) / 255.0f;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_message_bar_style_init(F2MessageBarStyle *style,
                                               F2Intent intent,
                                               const F2Theme *theme) {
  if (!style || !theme)
    return CMP_ERR_INVALID_ARGUMENT;
  memset(style, 0, sizeof(*style));

  style->intent = intent;
  style->padding = F2_MESSAGE_BAR_DEFAULT_PADDING;
  style->corner_radius = F2_MESSAGE_BAR_DEFAULT_CORNER_RADIUS;

  switch (intent) {
  case F2_INTENT_INFO:
    f2_color_from_u32(theme->semantics.info_background,
                      &style->background_color);
    f2_color_from_u32(theme->semantics.info_text, &style->text_color);
    break;
  case F2_INTENT_SUCCESS:
    f2_color_from_u32(theme->semantics.success_background,
                      &style->background_color);
    f2_color_from_u32(theme->semantics.success_text, &style->text_color);
    break;
  case F2_INTENT_WARNING:
    f2_color_from_u32(theme->semantics.warning_background,
                      &style->background_color);
    f2_color_from_u32(theme->semantics.warning_text, &style->text_color);
    break;
  case F2_INTENT_ERROR:
    f2_color_from_u32(theme->semantics.danger_background,
                      &style->background_color);
    f2_color_from_u32(theme->semantics.danger_text, &style->text_color);
    break;
  }

  style->icon_color = style->text_color;

  style->text_style.utf8_family =
      "Segoe UI Variable, system-ui, -apple-system, Roboto, Ubuntu";
  style->text_style.size_px = 14;
  style->text_style.weight = 400;
  style->text_style.color = style->text_color;

  return CMP_OK;
}

CMP_API int CMP_CALL f2_message_bar_init(F2MessageBar *bar,
                                         const F2MessageBarStyle *style,
                                         const char *utf8_text,
                                         cmp_usize text_len) {
  if (!bar || !style)
    return CMP_ERR_INVALID_ARGUMENT;
  memset(bar, 0, sizeof(*bar));
  bar->style = *style;
  bar->utf8_text = utf8_text;
  bar->text_len = text_len;
  bar->is_visible = CMP_FALSE;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_message_bar_show(F2MessageBar *bar) {
  if (!bar)
    return CMP_ERR_INVALID_ARGUMENT;
  bar->is_visible = CMP_TRUE;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_message_bar_hide(F2MessageBar *bar) {
  if (!bar)
    return CMP_ERR_INVALID_ARGUMENT;
  bar->is_visible = CMP_FALSE;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_message_bar_set_action(F2MessageBar *bar,
                                               const char *utf8_action,
                                               cmp_usize action_len,
                                               F2MessageBarOnAction on_action,
                                               void *ctx) {
  if (!bar)
    return CMP_ERR_INVALID_ARGUMENT;
  bar->utf8_action = utf8_action;
  bar->action_len = action_len;
  bar->on_action = on_action;
  bar->on_action_ctx = ctx;
  bar->style.has_action = CMP_TRUE;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_message_bar_set_dismiss(F2MessageBar *bar,
                                                F2MessageBarOnAction on_dismiss,
                                                void *ctx) {
  if (!bar)
    return CMP_ERR_INVALID_ARGUMENT;
  bar->on_dismiss = on_dismiss;
  bar->on_dismiss_ctx = ctx;
  bar->style.has_dismiss = CMP_TRUE;
  return CMP_OK;
}
