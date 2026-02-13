#include "cmpc/cmp_text.h"

#include <string.h>

#ifdef CMP_TESTING
static cmp_usize g_cmp_text_cstr_limit = 0;
#endif

static int cmp_text_validate_color(const CMPColor *color) {
  if (color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!(color->r >= 0.0f && color->r <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->g >= 0.0f && color->g <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->b >= 0.0f && color->b <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->a >= 0.0f && color->a <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_text_validate_style(const CMPTextStyle *style) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->utf8_family == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->size_px <= 0) {
    return CMP_ERR_RANGE;
  }
  if (style->weight < 100 || style->weight > 900) {
    return CMP_ERR_RANGE;
  }
  if (style->italic != CMP_FALSE && style->italic != CMP_TRUE) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_text_validate_color(&style->color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int cmp_text_validate_backend(const CMPTextBackend *backend) {
  if (backend == NULL || backend->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

static int cmp_text_validate_measure_spec(CMPMeasureSpec spec) {
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.mode != CMP_MEASURE_EXACTLY &&
      spec.mode != CMP_MEASURE_AT_MOST) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_text_validate_rect(const CMPRect *rect) {
  if (rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_text_metrics_update(CMPTextWidget *widget) {
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (widget->metrics_valid == CMP_TRUE) {
    return CMP_OK;
  }

  rc = cmp_text_measure_utf8(&widget->backend, widget->font, widget->utf8,
                            widget->utf8_len, &widget->metrics);
  if (rc != CMP_OK) {
    return rc;
  }

  widget->metrics_valid = CMP_TRUE;
  return CMP_OK;
}

static int cmp_text_widget_measure(void *widget, CMPMeasureSpec width,
                                  CMPMeasureSpec height, CMPSize *out_size) {
  CMPTextWidget *text;
  CMPTextMetrics metrics;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_text_validate_measure_spec(width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_text_validate_measure_spec(height);
  if (rc != CMP_OK) {
    return rc;
  }

  text = (CMPTextWidget *)widget;
  rc = cmp_text_metrics_update(text);
  if (rc != CMP_OK) {
    return rc;
  }

  metrics = text->metrics;

  if (width.mode == CMP_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == CMP_MEASURE_AT_MOST) {
    out_size->width = (metrics.width > width.size) ? width.size : metrics.width;
  } else {
    out_size->width = metrics.width;
  }

  if (height.mode == CMP_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == CMP_MEASURE_AT_MOST) {
    out_size->height =
        (metrics.height > height.size) ? height.size : metrics.height;
  } else {
    out_size->height = metrics.height;
  }

  return CMP_OK;
}

static int cmp_text_widget_layout(void *widget, CMPRect bounds) {
  CMPTextWidget *text;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_text_validate_rect(&bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  text = (CMPTextWidget *)widget;
  text->bounds = bounds;
  return CMP_OK;
}

static int cmp_text_widget_paint(void *widget, CMPPaintContext *ctx) {
  CMPTextWidget *text;
  CMPTextMetrics metrics;
  CMPScalar x;
  CMPScalar y;
  int rc;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->text_vtable == NULL ||
      ctx->gfx->text_vtable->draw_text == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  text = (CMPTextWidget *)widget;
  rc = cmp_text_metrics_update(text);
  if (rc != CMP_OK) {
    return rc;
  }

  metrics = text->metrics;
  x = text->bounds.x;
  y = text->bounds.y + metrics.baseline;
  return ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, text->font, text->utf8,
                                          text->utf8_len, x, y,
                                          text->style.color);
}

static int cmp_text_widget_event(void *widget, const CMPInputEvent *event,
                                CMPBool *out_handled) { /* GCOVR_EXCL_LINE */
  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;
  return CMP_OK;
}

static int cmp_text_widget_get_semantics(void *widget,
                                        CMPSemantics *out_semantics) {
  CMPTextWidget *text;

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  text = (CMPTextWidget *)widget;
  out_semantics->role = CMP_SEMANTIC_TEXT;
  out_semantics->flags = 0;
  out_semantics->utf8_label = text->utf8;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int cmp_text_widget_destroy(void *widget) {
  CMPTextWidget *text;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  text = (CMPTextWidget *)widget;
  rc = CMP_OK;
  if (text->owns_font == CMP_TRUE &&
      (text->font.id != 0u || text->font.generation != 0u)) {
    if (text->backend.vtable != NULL &&
        text->backend.vtable->destroy_font != NULL) {
      rc = text->backend.vtable->destroy_font(text->backend.ctx, text->font);
    } else {
      rc = CMP_ERR_UNSUPPORTED;
    }
  }

  text->font.id = 0u;
  text->font.generation = 0u;
  text->utf8 = NULL;
  text->utf8_len = 0;
  text->metrics_valid = CMP_FALSE;
  text->owns_font = CMP_FALSE;
  text->backend.ctx = NULL;
  text->backend.vtable = NULL;
  text->widget.ctx = NULL;
  text->widget.vtable = NULL;
  return rc;
}

static const CMPWidgetVTable g_cmp_text_widget_vtable = {
    cmp_text_widget_measure,       cmp_text_widget_layout,
    cmp_text_widget_paint,         cmp_text_widget_event,
    cmp_text_widget_get_semantics, cmp_text_widget_destroy};

int CMP_CALL cmp_text_style_init(CMPTextStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  style->utf8_family = NULL;
  style->size_px = 14;
  style->weight = 400;
  style->italic = CMP_FALSE;
  style->color.r = 0.0f;
  style->color.g = 0.0f;
  style->color.b = 0.0f;
  style->color.a = 1.0f;
  return CMP_OK;
}

int CMP_CALL cmp_text_backend_from_gfx(const CMPGfx *gfx,
                                     CMPTextBackend *out_backend) {
  if (gfx == NULL || out_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (gfx->text_vtable == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  out_backend->ctx = gfx->ctx;
  out_backend->vtable = gfx->text_vtable;
  return CMP_OK;
}

int CMP_CALL cmp_text_font_create(const CMPTextBackend *backend,
                                const CMPTextStyle *style, CMPHandle *out_font) {
  int rc;

  if (backend == NULL || style == NULL || out_font == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_text_validate_backend(backend);
  if (rc != CMP_OK) {
    return rc;
  }
  if (backend->vtable->create_font == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = cmp_text_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  out_font->id = 0u;
  out_font->generation = 0u;
  return backend->vtable->create_font(backend->ctx, style->utf8_family,
                                      style->size_px, style->weight,
                                      style->italic, out_font);
}

int CMP_CALL cmp_text_font_destroy(const CMPTextBackend *backend, CMPHandle font) {
  int rc;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  rc = cmp_text_validate_backend(backend);
  if (rc != CMP_OK) {
    return rc;
  }
  if (backend->vtable->destroy_font == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_OK;
  }

  return backend->vtable->destroy_font(backend->ctx, font);
}

int CMP_CALL cmp_text_measure_utf8(const CMPTextBackend *backend, CMPHandle font,
                                 const char *utf8, cmp_usize utf8_len,
                                 CMPTextMetrics *out_metrics) {
  CMPScalar width;
  CMPScalar height;
  CMPScalar baseline;
  int rc;

  if (backend == NULL || out_metrics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_text_validate_backend(backend);
  if (rc != CMP_OK) {
    return rc;
  }
  if (backend->vtable->measure_text == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = backend->vtable->measure_text(backend->ctx, font, utf8, utf8_len, &width,
                                     &height, &baseline);
  if (rc != CMP_OK) {
    return rc;
  }
  if (width < 0.0f || height < 0.0f || baseline < 0.0f) {
    return CMP_ERR_RANGE;
  }

  out_metrics->width = width;
  out_metrics->height = height;
  out_metrics->baseline = baseline;
  return CMP_OK;
}

static int cmp_text_cstrlen(const char *cstr, cmp_usize *out_len) {
  cmp_usize len;
  cmp_usize max_len;

  if (cstr == NULL || out_len == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_len = (cmp_usize) ~(cmp_usize)0;
#ifdef CMP_TESTING
  if (g_cmp_text_cstr_limit != 0) {
    max_len = g_cmp_text_cstr_limit;
  }
#endif
  len = 0;
  while (cstr[len] != '\0') {
    if (len == max_len) {
      return CMP_ERR_OVERFLOW;
    }
    len += 1;
  }

  *out_len = len;
  return CMP_OK;
}

int CMP_CALL cmp_text_measure_cstr(const CMPTextBackend *backend, CMPHandle font,
                                 const char *utf8, CMPTextMetrics *out_metrics) {
  cmp_usize len;
  int rc;

  if (utf8 == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_text_cstrlen(utf8, &len);
  if (rc != CMP_OK) {
    return rc;
  }

  return cmp_text_measure_utf8(backend, font, utf8, len, out_metrics);
}

int CMP_CALL cmp_text_font_metrics(const CMPTextBackend *backend, CMPHandle font,
                                 CMPTextMetrics *out_metrics) {
  return cmp_text_measure_utf8(backend, font, NULL, 0, out_metrics);
}

int CMP_CALL cmp_text_widget_init(CMPTextWidget *widget,
                                const CMPTextBackend *backend,
                                const CMPTextStyle *style, const char *utf8,
                                cmp_usize utf8_len) {
  int rc;

  if (widget == NULL || backend == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_text_validate_backend(backend);
  if (rc != CMP_OK) {
    return rc;
  }
  if (backend->vtable->create_font == NULL ||
      backend->vtable->measure_text == NULL ||
      backend->vtable->draw_text == NULL ||
      backend->vtable->destroy_font == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = cmp_text_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  memset(widget, 0, sizeof(*widget));
  widget->backend = *backend;
  widget->style = *style;
  widget->utf8 = utf8;
  widget->utf8_len = utf8_len;
  widget->owns_font = CMP_TRUE;
  widget->metrics_valid = CMP_FALSE;

  rc = cmp_text_font_create(backend, style, &widget->font);
  if (rc != CMP_OK) {
    return rc;
  }

  widget->widget.ctx = widget;
  widget->widget.vtable = &g_cmp_text_widget_vtable;
  widget->widget.handle.id = 0u;
  widget->widget.handle.generation = 0u;
  widget->widget.flags = 0u;
  return CMP_OK;
}

int CMP_CALL cmp_text_widget_set_text(CMPTextWidget *widget, const char *utf8,
                                    cmp_usize utf8_len) {
  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  widget->utf8 = utf8;
  widget->utf8_len = utf8_len;
  widget->metrics_valid = CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL cmp_text_widget_set_style(CMPTextWidget *widget,
                                     const CMPTextStyle *style) {
  CMPHandle new_font;
  int rc; /* GCOVR_EXCL_LINE */

  if (widget == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_text_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_text_font_create(&widget->backend, style, &new_font);
  if (rc != CMP_OK) {
    return rc;
  }

  if (widget->owns_font == CMP_TRUE) {
    cmp_text_font_destroy(&widget->backend, widget->font);
  }

  widget->style = *style;
  widget->font = new_font;
  widget->owns_font = CMP_TRUE;
  widget->metrics_valid = CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL cmp_text_widget_get_metrics(
    CMPTextWidget *widget, CMPTextMetrics *out_metrics) { /* GCOVR_EXCL_LINE */
  int rc;

  if (widget == NULL || out_metrics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_text_metrics_update(widget);
  if (rc != CMP_OK) {
    return rc;
  }

  *out_metrics = widget->metrics;
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL cmp_text_test_validate_color(const CMPColor *color) {
  return cmp_text_validate_color(color);
}

int CMP_CALL cmp_text_test_validate_style(const CMPTextStyle *style) {
  return cmp_text_validate_style(style);
}

int CMP_CALL cmp_text_test_validate_backend(const CMPTextBackend *backend) {
  return cmp_text_validate_backend(backend);
}

int CMP_CALL cmp_text_test_validate_measure_spec(CMPMeasureSpec spec) {
  return cmp_text_validate_measure_spec(spec);
}

int CMP_CALL cmp_text_test_validate_rect(const CMPRect *rect) {
  return cmp_text_validate_rect(rect);
}

int CMP_CALL cmp_text_test_metrics_update(CMPTextWidget *widget) {
  return cmp_text_metrics_update(widget);
}

int CMP_CALL cmp_text_test_cstrlen(const char *cstr, cmp_usize *out_len) {
  return cmp_text_cstrlen(cstr, out_len);
}

int CMP_CALL cmp_text_test_set_cstr_limit(cmp_usize max_len) {
  g_cmp_text_cstr_limit = max_len;
  return CMP_OK;
}
#endif
