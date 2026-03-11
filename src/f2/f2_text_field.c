/* clang-format off */
#include "f2/f2_text_field.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL f2_text_field_style_init(F2TextFieldStyle *style) {
  int res;
  if (!style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(style, 0, sizeof(*style));

  res = cmp_text_field_style_init(&style->core);
  if (res != CMP_OK) {
    return res;
  }
  style->core.padding_x = F2_TEXT_FIELD_DEFAULT_PADDING_X;
  style->core.padding_y = F2_TEXT_FIELD_DEFAULT_PADDING_Y;
  style->core.min_height = F2_TEXT_FIELD_DEFAULT_MIN_HEIGHT;
  style->core.corner_radius = F2_TEXT_FIELD_DEFAULT_CORNER_RADIUS;

  style->core.text_style.utf8_family =
      "Segoe UI Variable, system-ui, -apple-system, Roboto, Ubuntu";
  style->core.text_style.size_px = 14;
  style->core.text_style.weight = 400;

  style->core.label_style.utf8_family = style->core.text_style.utf8_family;
  style->core.label_style.size_px = 12;
  style->core.label_style.weight = 400;

  style->core.container_color = cmp_color_rgba(0.99f, 0.99f, 0.99f, 1.0f);
  style->core.outline_color =
      cmp_color_rgba(0.0f, 0.0f, 0.0f, 0.0f); /* Handled as bottom line */

  style->bottom_line_color = cmp_color_rgba(0.4f, 0.4f, 0.4f, 1.0f);
  style->bottom_line_focus_color = cmp_color_rgba(0.0f, 0.36f, 0.77f, 1.0f);
  style->error_color = cmp_color_rgba(0.8f, 0.0f, 0.0f, 1.0f);

  return CMP_OK;
}

static int CMP_CALL f2_text_field_core_on_change(void *ctx, CMPTextField *core,
                                                 const char *utf8_text,
                                                 cmp_usize len) {
  F2TextField *text_field = (F2TextField *)ctx;
  (void)core;
  (void)len;
  if (text_field && text_field->on_change) {
    return text_field->on_change(text_field->on_change_ctx, text_field,
                                 utf8_text);
  }
  return CMP_OK;
}

CMP_API int CMP_CALL f2_text_field_init(F2TextField *text_field,
                                        const CMPTextBackend *backend,
                                        const F2TextFieldStyle *style,
                                        CMPAllocator *allocator) {
  int res;
  if (!text_field || !backend || !style || !allocator) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(text_field, 0, sizeof(*text_field));
  text_field->style = *style;

  res = cmp_text_field_init(&text_field->core, backend, &style->core, allocator,
                            NULL, 0);
  if (res != CMP_OK) {
    return res;
  }

  cmp_text_field_set_on_change(&text_field->core, f2_text_field_core_on_change,
                               text_field);
  return CMP_OK;
}

CMP_API int CMP_CALL f2_text_field_set_text(F2TextField *text_field,
                                            const char *utf8_text) {
  cmp_usize len = 0;
  if (!text_field) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8_text) {
    len = strlen(utf8_text);
  }
  return cmp_text_field_set_text(&text_field->core, utf8_text, len);
}

CMP_API int CMP_CALL f2_text_field_set_error(F2TextField *text_field,
                                             CMPBool is_error) {
  if (!text_field) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  text_field->is_error = is_error;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_text_field_set_on_change(F2TextField *text_field,
                                                 F2TextFieldOnChange on_change,
                                                 void *ctx) {
  if (!text_field) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  text_field->on_change = on_change;
  text_field->on_change_ctx = ctx;
  return CMP_OK;
}
