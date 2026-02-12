#include "m3/m3_text.h"

#include <string.h>

#ifdef M3_TESTING
static m3_usize g_m3_text_cstr_limit = 0;
#endif

static int m3_text_validate_color(const M3Color *color) {
  if (color == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!(color->r >= 0.0f && color->r <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->g >= 0.0f && color->g <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->b >= 0.0f && color->b <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->a >= 0.0f && color->a <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_text_validate_style(const M3TextStyle *style) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (style->utf8_family == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (style->size_px <= 0) {
    return M3_ERR_RANGE;
  }
  if (style->weight < 100 || style->weight > 900) {
    return M3_ERR_RANGE;
  }
  if (style->italic != M3_FALSE && style->italic != M3_TRUE) {
    return M3_ERR_RANGE;
  }

  rc = m3_text_validate_color(&style->color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_text_validate_backend(const M3TextBackend *backend) {
  if (backend == NULL || backend->vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_OK;
}

static int m3_text_validate_measure_spec(M3MeasureSpec spec) {
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.mode != M3_MEASURE_EXACTLY &&
      spec.mode != M3_MEASURE_AT_MOST) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_text_validate_rect(const M3Rect *rect) {
  if (rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_text_metrics_update(M3TextWidget *widget) {
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (widget->metrics_valid == M3_TRUE) {
    return M3_OK;
  }

  rc = m3_text_measure_utf8(&widget->backend, widget->font, widget->utf8,
                            widget->utf8_len, &widget->metrics);
  if (rc != M3_OK) {
    return rc;
  }

  widget->metrics_valid = M3_TRUE;
  return M3_OK;
}

static int m3_text_widget_measure(void *widget, M3MeasureSpec width,
                                  M3MeasureSpec height, M3Size *out_size) {
  M3TextWidget *text;
  M3TextMetrics metrics;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_text_validate_measure_spec(width);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_validate_measure_spec(height);
  if (rc != M3_OK) {
    return rc;
  }

  text = (M3TextWidget *)widget;
  rc = m3_text_metrics_update(text);
  if (rc != M3_OK) {
    return rc;
  }

  metrics = text->metrics;

  if (width.mode == M3_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == M3_MEASURE_AT_MOST) {
    out_size->width = (metrics.width > width.size) ? width.size : metrics.width;
  } else {
    out_size->width = metrics.width;
  }

  if (height.mode == M3_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == M3_MEASURE_AT_MOST) {
    out_size->height =
        (metrics.height > height.size) ? height.size : metrics.height;
  } else {
    out_size->height = metrics.height;
  }

  return M3_OK;
}

static int m3_text_widget_layout(void *widget, M3Rect bounds) {
  M3TextWidget *text;
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_text_validate_rect(&bounds);
  if (rc != M3_OK) {
    return rc;
  }

  text = (M3TextWidget *)widget;
  text->bounds = bounds;
  return M3_OK;
}

static int m3_text_widget_paint(void *widget, M3PaintContext *ctx) {
  M3TextWidget *text;
  M3TextMetrics metrics;
  M3Scalar x;
  M3Scalar y;
  int rc;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->text_vtable == NULL ||
      ctx->gfx->text_vtable->draw_text == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  text = (M3TextWidget *)widget;
  rc = m3_text_metrics_update(text);
  if (rc != M3_OK) {
    return rc;
  }

  metrics = text->metrics;
  x = text->bounds.x;
  y = text->bounds.y + metrics.baseline;
  return ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, text->font, text->utf8,
                                          text->utf8_len, x, y,
                                          text->style.color);
}

static int m3_text_widget_event(void *widget, const M3InputEvent *event,
                                M3Bool *out_handled) { /* GCOVR_EXCL_LINE */
  if (widget == NULL || event == NULL || out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_handled = M3_FALSE;
  return M3_OK;
}

static int m3_text_widget_get_semantics(void *widget,
                                        M3Semantics *out_semantics) {
  M3TextWidget *text;

  if (widget == NULL || out_semantics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  text = (M3TextWidget *)widget;
  out_semantics->role = M3_SEMANTIC_TEXT;
  out_semantics->flags = 0;
  out_semantics->utf8_label = text->utf8;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return M3_OK;
}

static int m3_text_widget_destroy(void *widget) {
  M3TextWidget *text;
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  text = (M3TextWidget *)widget;
  rc = M3_OK;
  if (text->owns_font == M3_TRUE &&
      (text->font.id != 0u || text->font.generation != 0u)) {
    if (text->backend.vtable != NULL &&
        text->backend.vtable->destroy_font != NULL) {
      rc = text->backend.vtable->destroy_font(text->backend.ctx, text->font);
    } else {
      rc = M3_ERR_UNSUPPORTED;
    }
  }

  text->font.id = 0u;
  text->font.generation = 0u;
  text->utf8 = NULL;
  text->utf8_len = 0;
  text->metrics_valid = M3_FALSE;
  text->owns_font = M3_FALSE;
  text->backend.ctx = NULL;
  text->backend.vtable = NULL;
  text->widget.ctx = NULL;
  text->widget.vtable = NULL;
  return rc;
}

static const M3WidgetVTable g_m3_text_widget_vtable = {
    m3_text_widget_measure,       m3_text_widget_layout,
    m3_text_widget_paint,         m3_text_widget_event,
    m3_text_widget_get_semantics, m3_text_widget_destroy};

int M3_CALL m3_text_style_init(M3TextStyle *style) {
  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  style->utf8_family = NULL;
  style->size_px = 14;
  style->weight = 400;
  style->italic = M3_FALSE;
  style->color.r = 0.0f;
  style->color.g = 0.0f;
  style->color.b = 0.0f;
  style->color.a = 1.0f;
  return M3_OK;
}

int M3_CALL m3_text_backend_from_gfx(const M3Gfx *gfx,
                                     M3TextBackend *out_backend) {
  if (gfx == NULL || out_backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (gfx->text_vtable == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  out_backend->ctx = gfx->ctx;
  out_backend->vtable = gfx->text_vtable;
  return M3_OK;
}

int M3_CALL m3_text_font_create(const M3TextBackend *backend,
                                const M3TextStyle *style, M3Handle *out_font) {
  int rc;

  if (backend == NULL || style == NULL || out_font == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_text_validate_backend(backend);
  if (rc != M3_OK) {
    return rc;
  }
  if (backend->vtable->create_font == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  rc = m3_text_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }

  out_font->id = 0u;
  out_font->generation = 0u;
  return backend->vtable->create_font(backend->ctx, style->utf8_family,
                                      style->size_px, style->weight,
                                      style->italic, out_font);
}

int M3_CALL m3_text_font_destroy(const M3TextBackend *backend, M3Handle font) {
  int rc;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  rc = m3_text_validate_backend(backend);
  if (rc != M3_OK) {
    return rc;
  }
  if (backend->vtable->destroy_font == NULL) {
    return M3_ERR_UNSUPPORTED;
  }
  if (font.id == 0u && font.generation == 0u) {
    return M3_OK;
  }

  return backend->vtable->destroy_font(backend->ctx, font);
}

int M3_CALL m3_text_measure_utf8(const M3TextBackend *backend, M3Handle font,
                                 const char *utf8, m3_usize utf8_len,
                                 M3TextMetrics *out_metrics) {
  M3Scalar width;
  M3Scalar height;
  M3Scalar baseline;
  int rc;

  if (backend == NULL || out_metrics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_text_validate_backend(backend);
  if (rc != M3_OK) {
    return rc;
  }
  if (backend->vtable->measure_text == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  rc = backend->vtable->measure_text(backend->ctx, font, utf8, utf8_len, &width,
                                     &height, &baseline);
  if (rc != M3_OK) {
    return rc;
  }
  if (width < 0.0f || height < 0.0f || baseline < 0.0f) {
    return M3_ERR_RANGE;
  }

  out_metrics->width = width;
  out_metrics->height = height;
  out_metrics->baseline = baseline;
  return M3_OK;
}

static int m3_text_cstrlen(const char *cstr, m3_usize *out_len) {
  m3_usize len;
  m3_usize max_len;

  if (cstr == NULL || out_len == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  max_len = (m3_usize) ~(m3_usize)0;
#ifdef M3_TESTING
  if (g_m3_text_cstr_limit != 0) {
    max_len = g_m3_text_cstr_limit;
  }
#endif
  len = 0;
  while (cstr[len] != '\0') {
    if (len == max_len) {
      return M3_ERR_OVERFLOW;
    }
    len += 1;
  }

  *out_len = len;
  return M3_OK;
}

int M3_CALL m3_text_measure_cstr(const M3TextBackend *backend, M3Handle font,
                                 const char *utf8, M3TextMetrics *out_metrics) {
  m3_usize len;
  int rc;

  if (utf8 == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_text_cstrlen(utf8, &len);
  if (rc != M3_OK) {
    return rc;
  }

  return m3_text_measure_utf8(backend, font, utf8, len, out_metrics);
}

int M3_CALL m3_text_font_metrics(const M3TextBackend *backend, M3Handle font,
                                 M3TextMetrics *out_metrics) {
  return m3_text_measure_utf8(backend, font, NULL, 0, out_metrics);
}

int M3_CALL m3_text_widget_init(M3TextWidget *widget,
                                const M3TextBackend *backend,
                                const M3TextStyle *style, const char *utf8,
                                m3_usize utf8_len) {
  int rc;

  if (widget == NULL || backend == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_text_validate_backend(backend);
  if (rc != M3_OK) {
    return rc;
  }
  if (backend->vtable->create_font == NULL ||
      backend->vtable->measure_text == NULL ||
      backend->vtable->draw_text == NULL ||
      backend->vtable->destroy_font == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  rc = m3_text_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }

  memset(widget, 0, sizeof(*widget));
  widget->backend = *backend;
  widget->style = *style;
  widget->utf8 = utf8;
  widget->utf8_len = utf8_len;
  widget->owns_font = M3_TRUE;
  widget->metrics_valid = M3_FALSE;

  rc = m3_text_font_create(backend, style, &widget->font);
  if (rc != M3_OK) {
    return rc;
  }

  widget->widget.ctx = widget;
  widget->widget.vtable = &g_m3_text_widget_vtable;
  widget->widget.handle.id = 0u;
  widget->widget.handle.generation = 0u;
  widget->widget.flags = 0u;
  return M3_OK;
}

int M3_CALL m3_text_widget_set_text(M3TextWidget *widget, const char *utf8,
                                    m3_usize utf8_len) {
  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  widget->utf8 = utf8;
  widget->utf8_len = utf8_len;
  widget->metrics_valid = M3_FALSE;
  return M3_OK;
}

int M3_CALL m3_text_widget_set_style(M3TextWidget *widget,
                                     const M3TextStyle *style) {
  M3Handle new_font;
  int rc; /* GCOVR_EXCL_LINE */

  if (widget == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_text_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_text_font_create(&widget->backend, style, &new_font);
  if (rc != M3_OK) {
    return rc;
  }

  if (widget->owns_font == M3_TRUE) {
    m3_text_font_destroy(&widget->backend, widget->font);
  }

  widget->style = *style;
  widget->font = new_font;
  widget->owns_font = M3_TRUE;
  widget->metrics_valid = M3_FALSE;
  return M3_OK;
}

int M3_CALL m3_text_widget_get_metrics(
    M3TextWidget *widget, M3TextMetrics *out_metrics) { /* GCOVR_EXCL_LINE */
  int rc;

  if (widget == NULL || out_metrics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_text_metrics_update(widget);
  if (rc != M3_OK) {
    return rc;
  }

  *out_metrics = widget->metrics;
  return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_text_test_validate_color(const M3Color *color) {
  return m3_text_validate_color(color);
}

int M3_CALL m3_text_test_validate_style(const M3TextStyle *style) {
  return m3_text_validate_style(style);
}

int M3_CALL m3_text_test_validate_backend(const M3TextBackend *backend) {
  return m3_text_validate_backend(backend);
}

int M3_CALL m3_text_test_validate_measure_spec(M3MeasureSpec spec) {
  return m3_text_validate_measure_spec(spec);
}

int M3_CALL m3_text_test_validate_rect(const M3Rect *rect) {
  return m3_text_validate_rect(rect);
}

int M3_CALL m3_text_test_metrics_update(M3TextWidget *widget) {
  return m3_text_metrics_update(widget);
}

int M3_CALL m3_text_test_cstrlen(const char *cstr, m3_usize *out_len) {
  return m3_text_cstrlen(cstr, out_len);
}

int M3_CALL m3_text_test_set_cstr_limit(m3_usize max_len) {
  g_m3_text_cstr_limit = max_len;
  return M3_OK;
}
#endif
