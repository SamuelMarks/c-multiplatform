#include "m3/m3_badge.h"
#include "cmpc/cmp_core.h"
#include <string.h>

static int m3_badge_measure(void *widget, CMPMeasureSpec width,
                            CMPMeasureSpec height, CMPSize *out_size);
static int m3_badge_layout(void *widget, CMPRect bounds);
static int m3_badge_paint(void *widget, CMPPaintContext *ctx);
static int m3_badge_event(void *widget, const CMPInputEvent *event,
                          CMPBool *out_handled);
static int m3_badge_get_semantics(void *widget, CMPSemantics *out_semantics);
static int m3_badge_destroy(void *widget);

static const CMPWidgetVTable m3_badge_vtable = {
    m3_badge_measure, m3_badge_layout,        m3_badge_paint,
    m3_badge_event,   m3_badge_get_semantics, m3_badge_destroy};

CMP_API int CMP_CALL m3_badge_style_init(M3BadgeStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  /* Default Error color */
  style->background_color.r = 0.698f;
  style->background_color.g = 0.149f;
  style->background_color.b = 0.118f;
  style->background_color.a = 1.0f;

  /* Default On Error color */
  style->text_color.r = 1.0f;
  style->text_color.g = 1.0f;
  style->text_color.b = 1.0f;
  style->text_color.a = 1.0f;

  /* Basic text style for badge */
  cmp_text_style_init(&style->text_style);
  style->text_style.size_px = 11;
  style->text_style.weight = 500;

  style->dot_diameter = M3_BADGE_DEFAULT_DOT_DIAMETER;
  style->min_width = M3_BADGE_DEFAULT_MIN_WIDTH;
  style->height = M3_BADGE_DEFAULT_HEIGHT;
  style->padding_x = M3_BADGE_DEFAULT_PADDING_X;

  return CMP_OK;
}

CMP_API int CMP_CALL m3_badge_init(M3Badge *badge,
                                   const CMPTextBackend *backend,
                                   const M3BadgeStyle *style,
                                   const char *utf8_label, cmp_usize utf8_len) {
  int rc;

  if (badge == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8_label == NULL && utf8_len > 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->dot_diameter <= 0.0f || style->min_width <= 0.0f ||
      style->height <= 0.0f || style->padding_x < 0.0f) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(badge, 0, sizeof(*badge));
  badge->widget.vtable = &m3_badge_vtable;
  badge->style = *style;

  if (backend != NULL) {
    badge->text_backend = *backend;
  }

  badge->utf8_label = utf8_label;
  badge->utf8_len = utf8_len;
  badge->font.id = 0;
  badge->font.generation = 0;

  if (backend != NULL && backend->vtable != NULL && utf8_label != NULL) {
    rc = cmp_text_font_create(backend, &style->text_style, &badge->font);
    if (rc != CMP_OK) {
      return rc;
    }
    rc = cmp_text_font_metrics(backend, badge->font, &badge->metrics);
    if (rc != CMP_OK) {
      cmp_text_font_destroy(backend, badge->font);
      badge->font.id = 0;
      badge->font.generation = 0;
      return rc;
    }
  }

  return CMP_OK;
}

CMP_API int CMP_CALL m3_badge_set_label(M3Badge *badge, const char *utf8_label,
                                        cmp_usize utf8_len) {
  if (badge == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8_label == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  badge->utf8_label = utf8_label;
  badge->utf8_len = utf8_len;

  return CMP_OK;
}

CMP_API int CMP_CALL m3_badge_set_style(M3Badge *badge,
                                        const M3BadgeStyle *style) {
  CMPHandle new_font = {0, 0};
  CMPTextMetrics new_metrics = {0};
  int rc;

  if (badge == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->dot_diameter <= 0.0f || style->min_width <= 0.0f ||
      style->height <= 0.0f || style->padding_x < 0.0f) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (badge->text_backend.vtable != NULL && badge->utf8_label != NULL) {
    rc = cmp_text_font_create(&badge->text_backend, &style->text_style,
                              &new_font);
    if (rc != CMP_OK) {
      return rc;
    }
    rc = cmp_text_font_metrics(&badge->text_backend, new_font, &new_metrics);
    if (rc != CMP_OK) {
      cmp_text_font_destroy(&badge->text_backend, new_font);
      return rc;
    }
    if (badge->font.id != 0 || badge->font.generation != 0) {
      cmp_text_font_destroy(&badge->text_backend, badge->font);
    }
    badge->font = new_font;
    badge->metrics = new_metrics;
  }

  badge->style = *style;
  return CMP_OK;
}

static int m3_badge_measure(void *widget, CMPMeasureSpec width,
                            CMPMeasureSpec height, CMPSize *out_size) {
  M3Badge *badge = (M3Badge *)widget;
  CMPSize text_size = {0.0f, 0.0f};

  if (badge == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (badge->utf8_label == NULL) {
    out_size->width = badge->style.dot_diameter;
    out_size->height = badge->style.dot_diameter;
  } else {
    if (badge->text_backend.vtable != NULL &&
        (badge->font.id != 0 || badge->font.generation != 0)) {
      CMPTextMetrics temp_metrics = {0};
      cmp_text_measure_utf8(&badge->text_backend, badge->font,
                            badge->utf8_label, badge->utf8_len, &temp_metrics);
      text_size.width = temp_metrics.width;
      text_size.height = temp_metrics.height;
    }
    out_size->width = text_size.width + (badge->style.padding_x * 2.0f);
    if (out_size->width < badge->style.min_width) {
      out_size->width = badge->style.min_width;
    }
    out_size->height = badge->style.height;
  }

  if (width.mode == CMP_MEASURE_EXACTLY) {
    out_size->width = width.size;
  }
  if (height.mode == CMP_MEASURE_EXACTLY) {
    out_size->height = height.size;
  }

  return CMP_OK;
}

static int m3_badge_layout(void *widget, CMPRect bounds) {
  M3Badge *badge = (M3Badge *)widget;
  if (badge == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  badge->bounds = bounds;
  return CMP_OK;
}

static int m3_badge_paint(void *widget, CMPPaintContext *ctx) {
  M3Badge *badge = (M3Badge *)widget;
  CMPScalar corner_radius;

  if (badge == NULL || ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (ctx->gfx == NULL || badge->style.background_color.a <= 0.0f) {
    return CMP_OK;
  }

  corner_radius = badge->bounds.height * 0.5f;

  /* Draw background */
  if (ctx->gfx->vtable != NULL && ctx->gfx->vtable->draw_rect != NULL) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &badge->bounds,
                                badge->style.background_color, corner_radius);
  }

  /* Draw text if labeled */
  if (badge->utf8_label != NULL && badge->text_backend.vtable != NULL &&
      (badge->font.id != 0 || badge->font.generation != 0) &&
      badge->style.text_color.a > 0.0f) {

    if (ctx->gfx->text_vtable != NULL &&
        ctx->gfx->text_vtable->draw_text != NULL) {
      CMPTextMetrics text_metrics = {0};
      CMPScalar text_x, text_y;

      cmp_text_measure_utf8(&badge->text_backend, badge->font,
                            badge->utf8_label, badge->utf8_len, &text_metrics);

      text_x =
          badge->bounds.x + (badge->bounds.width - text_metrics.width) * 0.5f;
      text_y = badge->bounds.y +
               (badge->bounds.height - badge->metrics.height) * 0.5f +
               badge->metrics.baseline;

      ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, badge->font,
                                       badge->utf8_label, badge->utf8_len,
                                       text_x, text_y, badge->style.text_color);
    }
  }

  return CMP_OK;
}

static int m3_badge_event(void *widget, const CMPInputEvent *event,
                          CMPBool *out_handled) {
  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_handled = CMP_FALSE;
  return CMP_OK;
}

static int m3_badge_get_semantics(void *widget, CMPSemantics *out_semantics) {
  M3Badge *badge = (M3Badge *)widget;
  if (badge == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  out_semantics->role = CMP_SEMANTIC_TEXT;
  out_semantics->flags = 0;
  out_semantics->utf8_label = badge->utf8_label;
  out_semantics->utf8_hint = "Badge";
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_badge_destroy(void *widget) {
  M3Badge *badge = (M3Badge *)widget;
  if (badge == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (badge->text_backend.vtable != NULL &&
      (badge->font.id != 0 || badge->font.generation != 0)) {
    cmp_text_font_destroy(&badge->text_backend, badge->font);
    badge->font.id = 0;
    badge->font.generation = 0;
  }
  return CMP_OK;
}
