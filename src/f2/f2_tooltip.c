/* clang-format off */
#include "f2/f2_tooltip.h"
#include "cmpc/cmp_core.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL f2_tooltip_style_init(F2TooltipStyle *style) {
  if (!style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(style, 0, sizeof(*style));

  style->background_color = (CMPColor){1.0f, 1.0f, 1.0f, 1.0f};
  style->border_color = (CMPColor){0.9f, 0.9f, 0.9f, 1.0f};
  style->text_color = (CMPColor){0.14f, 0.14f, 0.14f, 1.0f};

  style->corner_radius = F2_TOOLTIP_DEFAULT_CORNER_RADIUS;
  style->padding_x = F2_TOOLTIP_DEFAULT_PADDING;
  style->padding_y = F2_TOOLTIP_DEFAULT_PADDING;
  style->border_width = 1.0f;

  style->text_style.utf8_family =
      "Segoe UI Variable, system-ui, -apple-system, Roboto, Ubuntu";
  style->text_style.size_px = 12;
  style->text_style.weight = 400;
  style->text_style.color = style->text_color;

  return CMP_OK;
}

CMP_API int CMP_CALL f2_tooltip_init(F2Tooltip *tooltip,
                                     const F2TooltipStyle *style,
                                     const char *utf8_text) {
  if (!tooltip || !style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(tooltip, 0, sizeof(*tooltip));
  tooltip->style = *style;
  tooltip->utf8_text = utf8_text;
  tooltip->is_visible = CMP_FALSE;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_tooltip_show(F2Tooltip *tooltip,
                                     CMPRect anchor_bounds) {
  if (!tooltip)
    return CMP_ERR_INVALID_ARGUMENT;
  tooltip->anchor_bounds = anchor_bounds;
  tooltip->is_visible = CMP_TRUE;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_tooltip_hide(F2Tooltip *tooltip) {
  if (!tooltip)
    return CMP_ERR_INVALID_ARGUMENT;
  tooltip->is_visible = CMP_FALSE;
  return CMP_OK;
}
