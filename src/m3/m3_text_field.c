#include "m3/m3_text_field.h"
#include "cmpc/cmp_api_env.h"
#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_core.h"
#include "m3/m3_typography.h"
#include <string.h>

static int m3_text_field_measure(void *widget, CMPMeasureSpec width,
                                 CMPMeasureSpec height, CMPSize *out_size);
static int m3_text_field_layout(void *widget, CMPRect bounds);
static int m3_text_field_paint(void *widget, CMPPaintContext *ctx);
static int m3_text_field_event(void *widget, const CMPInputEvent *event,
                               CMPBool *out_handled);
static int m3_text_field_get_semantics(void *widget,
                                       CMPSemantics *out_semantics);
static int m3_text_field_destroy(void *widget);

static const CMPWidgetVTable m3_text_field_vtable = {
    m3_text_field_measure, m3_text_field_layout,        m3_text_field_paint,
    m3_text_field_event,   m3_text_field_get_semantics, m3_text_field_destroy};

CMP_API int CMP_CALL m3_text_field_style_init(M3TextFieldStyle *style,
                                              cmp_u32 variant) {
  M3TypographyScale typography;
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (variant != M3_TEXT_FIELD_VARIANT_FILLED &&
      variant != M3_TEXT_FIELD_VARIANT_OUTLINED) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  m3_typography_scale_init(&typography);

  style->variant = variant;
  cmp_text_field_style_init(&style->core);

  m3_typography_get_style(&typography, M3_TYPOGRAPHY_BODY_LARGE,
                          &style->core.text_style);
  m3_typography_get_style(&typography, M3_TYPOGRAPHY_LABEL_LARGE,
                          &style->core.label_style);
  m3_typography_get_style(&typography, M3_TYPOGRAPHY_BODY_SMALL,
                          &style->supporting_text_style);

  style->core.padding_x = M3_TEXT_FIELD_DEFAULT_PADDING_X;
  style->core.padding_y = M3_TEXT_FIELD_DEFAULT_PADDING_Y;
  style->core.min_height = M3_TEXT_FIELD_DEFAULT_MIN_HEIGHT;
  style->core.label_float_offset = 8.0f;

  if (variant == M3_TEXT_FIELD_VARIANT_FILLED) {
    style->core.container_color.r = 0.949f;
    style->core.container_color.g = 0.949f;
    style->core.container_color.b = 0.949f;
    style->core.container_color.a = 1.0f;
    style->core.outline_color.r = 0.0f;
    style->core.outline_color.g = 0.0f;
    style->core.outline_color.b = 0.0f;
    style->core.outline_color.a = 0.0f;
    style->core.focused_outline_color.r = 0.384f;
    style->core.focused_outline_color.g = 0.0f;
    style->core.focused_outline_color.b = 0.933f;
    style->core.focused_outline_color.a = 1.0f;
    style->core.corner_radius = M3_TEXT_FIELD_DEFAULT_FILLED_RADIUS;
  } else {
    style->core.container_color.r = 0.0f;
    style->core.container_color.g = 0.0f;
    style->core.container_color.b = 0.0f;
    style->core.container_color.a = 0.0f;
    style->core.outline_color.r = 0.474f;
    style->core.outline_color.g = 0.454f;
    style->core.outline_color.b = 0.494f;
    style->core.outline_color.a = 1.0f;
    style->core.focused_outline_color.r = 0.384f;
    style->core.focused_outline_color.g = 0.0f;
    style->core.focused_outline_color.b = 0.933f;
    style->core.focused_outline_color.a = 1.0f;
    style->core.corner_radius = M3_TEXT_FIELD_DEFAULT_OUTLINED_RADIUS;
  }

  style->error_color.r = 0.701f;
  style->error_color.g = 0.149f;
  style->error_color.b = 0.117f;
  style->error_color.a = 1.0f;
  style->supporting_color.r = 0.286f;
  style->supporting_color.g = 0.270f;
  style->supporting_color.b = 0.309f;
  style->supporting_color.a = 1.0f;
  style->icon_color.r = 0.286f;
  style->icon_color.g = 0.270f;
  style->icon_color.b = 0.309f;
  style->icon_color.a = 1.0f;
  style->icon_size = 24.0f;

  return CMP_OK;
}

static int CMP_CALL m3_text_field_on_core_change(void *ctx,
                                                 CMPTextField *core_field,
                                                 const char *utf8_text,
                                                 cmp_usize utf8_len) {
  M3TextField *text_field = (M3TextField *)ctx;
  (void)core_field;
  (void)utf8_len;
  if (text_field->on_change != NULL) {
    return text_field->on_change(text_field->on_change_ctx, text_field,
                                 utf8_text);
  }
  return CMP_OK;
}

CMP_API int CMP_CALL m3_text_field_init(M3TextField *text_field,
                                        const M3TextFieldStyle *style,
                                        CMPAllocator allocator,
                                        CMPTextBackend text_backend) {
  int rc;
  if (text_field == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  text_field->widget.vtable = &m3_text_field_vtable;
  text_field->style = *style;
  text_field->error = CMP_FALSE;
  text_field->disabled = CMP_FALSE;
  text_field->utf8_supporting = NULL;
  text_field->utf8_error = NULL;
  text_field->on_change = NULL;
  text_field->on_change_ctx = NULL;

  rc = cmp_text_field_init(&text_field->core_field, &text_backend, &style->core,
                           &allocator, NULL, 0);
  if (rc != CMP_OK) {
    return rc;
  }

  cmp_text_field_set_on_change(&text_field->core_field,
                               m3_text_field_on_core_change, text_field);

  return CMP_OK;
}

CMP_API int CMP_CALL m3_text_field_set_style(M3TextField *text_field,
                                             const M3TextFieldStyle *style) {
  if (text_field == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  text_field->style = *style;
  if (text_field->error) {
    CMPTextFieldStyle error_style = style->core;
    error_style.focused_outline_color = style->error_color;
    error_style.outline_color = style->error_color;
    error_style.label_style.color = style->error_color;
    return cmp_text_field_set_style(&text_field->core_field, &error_style);
  }
  return cmp_text_field_set_style(&text_field->core_field, &style->core);
}

CMP_API int CMP_CALL m3_text_field_set_label(M3TextField *text_field,
                                             const char *utf8_label) {
  if (text_field == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return cmp_text_field_set_label(&text_field->core_field, utf8_label,
                                  utf8_label ? strlen(utf8_label) : 0);
}

CMP_API int CMP_CALL m3_text_field_set_placeholder(
    M3TextField *text_field, const char *utf8_placeholder) {
  if (text_field == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return cmp_text_field_set_placeholder(
      &text_field->core_field, utf8_placeholder,
      utf8_placeholder ? strlen(utf8_placeholder) : 0);
}

CMP_API int CMP_CALL m3_text_field_set_supporting_text(M3TextField *text_field,
                                                       const char *utf8_text) {
  if (text_field == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  text_field->utf8_supporting = utf8_text;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_text_field_set_error(M3TextField *text_field,
                                             const char *utf8_error) {
  if (text_field == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  text_field->utf8_error = utf8_error;
  text_field->error = (utf8_error != NULL) ? CMP_TRUE : CMP_FALSE;
  return m3_text_field_set_style(text_field, &text_field->style);
}

CMP_API int CMP_CALL m3_text_field_set_disabled(M3TextField *text_field,
                                                CMPBool disabled) {
  if (text_field == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  text_field->disabled = disabled;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_text_field_set_on_change(M3TextField *text_field,
                                                 M3TextFieldOnChange on_change,
                                                 void *ctx) {
  if (text_field == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  text_field->on_change = on_change;
  text_field->on_change_ctx = ctx;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_text_field_set_text(M3TextField *text_field,
                                            const char *utf8_text) {
  if (text_field == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return cmp_text_field_set_text(&text_field->core_field, utf8_text,
                                 utf8_text ? strlen(utf8_text) : 0);
}

CMP_API int CMP_CALL m3_text_field_get_text_len(const M3TextField *text_field,
                                                cmp_usize *out_len) {
  if (text_field == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (out_len != NULL) {
    *out_len = text_field->core_field.utf8_len;
  }
  return CMP_OK;
}

CMP_API int CMP_CALL m3_text_field_test_helper(void) { return CMP_OK; }

static int m3_text_field_measure(void *widget, CMPMeasureSpec width,
                                 CMPMeasureSpec height, CMPSize *out_size) {
  M3TextField *field = (M3TextField *)widget;
  CMPSize core_size;
  int rc;

  if (field == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = field->core_field.widget.vtable->measure(&field->core_field, width,
                                                height, &core_size);
  if (rc != CMP_OK) {
    return rc;
  }

  if (field->utf8_supporting != NULL || field->utf8_error != NULL) {
    core_size.height += field->style.supporting_text_style.size_px + 8.0f;
  }

  *out_size = core_size;
  return CMP_OK;
}

static int m3_text_field_layout(void *widget, CMPRect bounds) {
  M3TextField *field = (M3TextField *)widget;
  CMPRect core_bounds;

  if (field == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  core_bounds = bounds;
  if (field->utf8_supporting != NULL || field->utf8_error != NULL) {
    core_bounds.height -= (field->style.supporting_text_style.size_px + 8.0f);
    if (core_bounds.height < 0.0f)
      core_bounds.height = 0.0f;
  }

  return field->core_field.widget.vtable->layout(&field->core_field,
                                                 core_bounds);
}

static int m3_text_field_paint(void *widget, CMPPaintContext *ctx) {
  M3TextField *field = (M3TextField *)widget;
  int rc;
  const char *sub_text;
  CMPColor sub_color;
  CMPRect text_bounds;
  CMPHandle font = {0};

  if (field == NULL || ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = field->core_field.widget.vtable->paint(&field->core_field, ctx);
  if (rc != CMP_OK) {
    return rc;
  }

  if (field->error && field->utf8_error != NULL) {
    sub_text = field->utf8_error;
    sub_color = field->style.error_color;
  } else if (field->utf8_supporting != NULL) {
    sub_text = field->utf8_supporting;
    sub_color = field->style.supporting_color;
  } else {
    sub_text = NULL;
  }

  if (sub_text != NULL) {
    text_bounds.x = field->core_field.bounds.x + 16.0f;
    text_bounds.y =
        field->core_field.bounds.y + field->core_field.bounds.height + 4.0f;
    text_bounds.width = field->core_field.bounds.width - 32.0f;
    text_bounds.height = field->style.supporting_text_style.size_px + 4.0f;

    rc = field->core_field.text_backend.vtable->create_font(
        field->core_field.text_backend.ctx,
        field->style.supporting_text_style.utf8_family,
        field->style.supporting_text_style.size_px,
        field->style.supporting_text_style.weight,
        field->style.supporting_text_style.italic, &font);
    if (rc == CMP_OK) {
      CMPScalar w = 0, h = 0, baseline = 0;
      field->core_field.text_backend.vtable->measure_text(
          field->core_field.text_backend.ctx, font, sub_text, strlen(sub_text),
          &w, &h, &baseline);
      field->core_field.text_backend.vtable->draw_text(
          field->core_field.text_backend.ctx, font, sub_text, strlen(sub_text),
          text_bounds.x, text_bounds.y + baseline, sub_color);
      field->core_field.text_backend.vtable->destroy_font(
          field->core_field.text_backend.ctx, font);
    }
  }

  return CMP_OK;
}

static int m3_text_field_event(void *widget, const CMPInputEvent *event,
                               CMPBool *out_handled) {
  M3TextField *field = (M3TextField *)widget;
  if (field == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (field->disabled) {
    *out_handled = CMP_FALSE;
    return CMP_OK;
  }
  return field->core_field.widget.vtable->event(&field->core_field, event,
                                                out_handled);
}

static int m3_text_field_get_semantics(void *widget,
                                       CMPSemantics *out_semantics) {
  M3TextField *field = (M3TextField *)widget;
  if (field == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return field->core_field.widget.vtable->get_semantics(&field->core_field,
                                                        out_semantics);
}

static int m3_text_field_destroy(void *widget) {
  M3TextField *field = (M3TextField *)widget;
  if (field == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return field->core_field.widget.vtable->destroy(&field->core_field);
}
