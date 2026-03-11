/* clang-format off */
#include "f2/f2_command_bar.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL f2_command_bar_style_init(F2CommandBarStyle *style) {
  if (!style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(style, 0, sizeof(*style));

  style->background_color = cmp_color_rgba(1.0f, 1.0f, 1.0f, 1.0f);
  style->border_color = cmp_color_rgba(0.9f, 0.9f, 0.9f, 1.0f);
  style->height = F2_COMMAND_BAR_DEFAULT_HEIGHT;
  style->padding = F2_COMMAND_BAR_DEFAULT_PADDING;

  f2_button_style_init_transparent(&style->item_style);

  return CMP_OK;
}

CMP_API int CMP_CALL f2_command_bar_init(
    F2CommandBar *bar, const F2CommandBarStyle *style,
    const F2CommandBarItem *primary_items, cmp_usize primary_count,
    const F2CommandBarItem *secondary_items, cmp_usize secondary_count) {
  if (!bar || !style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (primary_count > 0 && !primary_items) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (secondary_count > 0 && !secondary_items) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(bar, 0, sizeof(*bar));
  bar->style = *style;
  bar->primary_items = primary_items;
  bar->primary_count = primary_count;
  bar->secondary_items = secondary_items;
  bar->secondary_count = secondary_count;

  return CMP_OK;
}

CMP_API int CMP_CALL f2_command_bar_set_on_action(
    F2CommandBar *bar, F2CommandBarOnAction on_action, void *ctx) {
  if (!bar)
    return CMP_ERR_INVALID_ARGUMENT;
  bar->on_action = on_action;
  bar->on_action_ctx = ctx;
  return CMP_OK;
}
