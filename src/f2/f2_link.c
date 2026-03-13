/* clang-format off */
#include "f2/f2_link.h"
#include "cmpc/cmp_core.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL f2_link_style_init(F2LinkStyle *style) {
  if (!style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(style, 0, sizeof(*style));

  style->text_style.utf8_family =
      "Segoe UI Variable, system-ui, -apple-system, Roboto, Ubuntu";
  style->text_style.size_px = 14;
  style->text_style.weight = 400;

  style->text_color =
      cmp_color_rgba(0.0f, 0.36f, 0.77f, 1.0f); /* Fluent link color */
  style->hover_color = cmp_color_rgba(0.0f, 0.28f, 0.65f, 1.0f);
  style->pressed_color = cmp_color_rgba(0.0f, 0.18f, 0.45f, 1.0f);
  style->disabled_color = cmp_color_rgba(0.8f, 0.8f, 0.8f, 1.0f);

  style->has_underline = CMP_TRUE;

  return CMP_OK;
}

CMP_API int CMP_CALL f2_link_init(F2Link *link, const F2LinkStyle *style,
                                  const char *utf8_text, cmp_usize utf8_len) {
  if (!link || !style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(link, 0, sizeof(*link));
  link->style = *style;
  link->utf8_text = utf8_text;
  link->utf8_len = utf8_len;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_link_set_text(F2Link *link, const char *utf8_text,
                                      cmp_usize utf8_len) {
  if (!link)
    return CMP_ERR_INVALID_ARGUMENT;
  link->utf8_text = utf8_text;
  link->utf8_len = utf8_len;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_link_set_disabled(F2Link *link, CMPBool disabled) {
  if (!link)
    return CMP_ERR_INVALID_ARGUMENT;
  link->disabled = disabled;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_link_set_on_click(F2Link *link, F2LinkOnClick on_click,
                                          void *ctx) {
  if (!link)
    return CMP_ERR_INVALID_ARGUMENT;
  link->on_click = on_click;
  link->on_click_ctx = ctx;
  return CMP_OK;
}
