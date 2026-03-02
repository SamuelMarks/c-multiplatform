#include "m3/m3_tooltip.h"
#include "cmpc/cmp_core.h"
#include "m3/m3_typography.h"
#include <string.h>

static CMPWidgetVTable g_m3_tooltip_widget_vtable;

static int m3_tooltip_destroy_fonts(const CMPTextBackend *backend,
                                    CMPHandle *fonts, cmp_usize count) {
  cmp_usize i;
  int rc = CMP_OK;
  int cleanup_rc;
  for (i = 0; i < count; ++i) {
    if (fonts[i].id != 0u || fonts[i].generation != 0u) {
      cleanup_rc = cmp_text_font_destroy(backend, fonts[i]);
      if (cleanup_rc != CMP_OK) {
        rc = cleanup_rc; /* GCOVR_EXCL_LINE */
      }
      fonts[i].id = 0u;
      fonts[i].generation = 0u;
    }
  }
  return rc;
}

int CMP_CALL m3_tooltip_style_init_plain(CMPTooltipStyle *style) {
  int rc;
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  rc = cmp_tooltip_style_init_plain(style);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  style->text_style.size_px = 12;
  style->text_style.weight = 400;
  style->text_style.italic = CMP_FALSE;
  style->text_style.utf8_family = "sans-serif";

  style->background_color.r = 0.192f;
  style->background_color.g = 0.188f;
  style->background_color.b = 0.200f;
  style->background_color.a = 1.0f; /* Inverse surface */
  return CMP_OK;
}

int CMP_CALL m3_tooltip_style_init_rich(CMPTooltipStyle *style) {
  int rc;
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  rc = cmp_tooltip_style_init_rich(style);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  style->title_style.size_px = 14;
  style->title_style.weight = 500;
  style->title_style.italic = CMP_FALSE;
  style->title_style.utf8_family = "sans-serif";

  style->body_style.size_px = 14;
  style->body_style.weight = 400;
  style->body_style.italic = CMP_FALSE;
  style->body_style.utf8_family = "sans-serif";

  style->background_color.r = 1.0f;
  style->background_color.g = 1.0f;
  style->background_color.b = 1.0f;
  style->background_color.a = 1.0f; /* Surface */
  return CMP_OK;
}

static int m3_tooltip_metrics_update(M3Tooltip *tooltip) {
  int rc;

  if (tooltip == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (tooltip->metrics_valid == CMP_TRUE) {
    return CMP_OK;
  }

  memset(&tooltip->metrics, 0, sizeof(tooltip->metrics));

  if (tooltip->style.variant == CMP_TOOLTIP_VARIANT_PLAIN) {
    tooltip->metrics.has_title = CMP_FALSE;
    if (tooltip->body_len > 0u) {
      rc = cmp_text_measure_utf8(&tooltip->text_backend, tooltip->plain_font,
                                 tooltip->utf8_body, tooltip->body_len,
                                 &tooltip->metrics.body_metrics);
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
      tooltip->metrics.has_body = CMP_TRUE;
    }
  } else {
    /* Rich tooltip */
    if (tooltip->title_len > 0u && tooltip->utf8_title != NULL) {
      rc = cmp_text_measure_utf8(&tooltip->text_backend, tooltip->title_font,
                                 tooltip->utf8_title, tooltip->title_len,
                                 &tooltip->metrics.title_metrics);
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
      tooltip->metrics.has_title = CMP_TRUE;
    } else {
      tooltip->metrics.has_title = CMP_FALSE;
    }

    if (tooltip->body_len > 0u && tooltip->utf8_body != NULL) {
      rc = cmp_text_measure_utf8(&tooltip->text_backend, tooltip->body_font,
                                 tooltip->utf8_body, tooltip->body_len,
                                 &tooltip->metrics.body_metrics);
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
      tooltip->metrics.has_body = CMP_TRUE;
    } else {
      tooltip->metrics.has_body = CMP_FALSE;
    }
  }

  tooltip->metrics_valid = CMP_TRUE;
  return CMP_OK;
}

static int m3_tooltip_widget_measure(void *widget, CMPMeasureSpec width_spec,
                                     CMPMeasureSpec height_spec,
                                     CMPSize *out_size) {
  M3Tooltip *tooltip;
  int rc;
  CMPSize content_size;

  (void)width_spec;
  (void)height_spec;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  tooltip = (M3Tooltip *)widget;

  rc = m3_tooltip_metrics_update(tooltip);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_tooltip_compute_content_size(&tooltip->style, &tooltip->metrics,
                                        &content_size);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  /* We do not strictly use width_spec/height_spec for constraints unless
   * specified, as overlay bounding happens in compute_bounds. But we satisfy
   * the measure step. */
  out_size->width = content_size.width;
  out_size->height = content_size.height;

  return CMP_OK;
}

static int m3_tooltip_widget_layout(void *widget, CMPRect bounds) {
  M3Tooltip *tooltip;
  int rc;
  CMPSize content_size;

  (void)bounds;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  tooltip = (M3Tooltip *)widget;

  rc = m3_tooltip_metrics_update(tooltip);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_tooltip_compute_content_size(&tooltip->style, &tooltip->metrics,
                                        &content_size);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_tooltip_compute_bounds(
      &tooltip->style, &tooltip->anchor, &tooltip->placement, &tooltip->overlay,
      &content_size, &tooltip->bounds, &tooltip->resolved_direction);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

static int m3_tooltip_widget_paint(void *widget, CMPPaintContext *ctx) {
  M3Tooltip *tooltip;
  CMPColor text_color;
  int rc;
  CMPScalar text_x, text_y;

  if (widget == NULL || ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (ctx->gfx == NULL || ctx->gfx->vtable == NULL ||
      ctx->gfx->vtable->draw_rect == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  if (ctx->gfx->text_vtable == NULL ||
      ctx->gfx->text_vtable->draw_text == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  tooltip = (M3Tooltip *)widget;

  if (tooltip->bounds.width <= 0.0f || tooltip->bounds.height <= 0.0f) {
    return CMP_OK;
  }

  rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &tooltip->bounds,
                                   tooltip->style.background_color,
                                   tooltip->style.corner_radius);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  if (tooltip->style.variant == CMP_TOOLTIP_VARIANT_PLAIN) {
    text_color.r = 0.957f;
    text_color.g = 0.937f;
    text_color.b = 0.957f;
    text_color.a = 1.0f; /* Inverse on surface */

    text_x = tooltip->bounds.x + tooltip->style.padding.left;
    text_y = tooltip->bounds.y + tooltip->style.padding.top +
             tooltip->metrics.body_metrics.baseline;

    if (tooltip->metrics.has_body == CMP_TRUE) {
      rc = ctx->gfx->text_vtable->draw_text(
          ctx->gfx->ctx, tooltip->plain_font, tooltip->utf8_body,
          tooltip->body_len, text_x, text_y, text_color);
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
    }
  } else {
    /* Rich tooltip */
    text_color.r = 0.114f;
    text_color.g = 0.106f;
    text_color.b = 0.125f;
    text_color.a = 1.0f; /* On surface */

    text_x = tooltip->bounds.x + tooltip->style.padding.left;
    text_y = tooltip->bounds.y + tooltip->style.padding.top;

    if (tooltip->metrics.has_title == CMP_TRUE) {
      text_y += tooltip->metrics.title_metrics.baseline;
      rc = ctx->gfx->text_vtable->draw_text(
          ctx->gfx->ctx, tooltip->title_font, tooltip->utf8_title,
          tooltip->title_len, text_x, text_y, text_color);
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
      text_y += (tooltip->metrics.title_metrics.height -
                 tooltip->metrics.title_metrics.baseline) +
                tooltip->style.title_body_spacing;
    }

    if (tooltip->metrics.has_body == CMP_TRUE) {
      text_y += tooltip->metrics.body_metrics.baseline;
      rc = ctx->gfx->text_vtable->draw_text(
          ctx->gfx->ctx, tooltip->body_font, tooltip->utf8_body,
          tooltip->body_len, text_x, text_y, text_color);
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
    }
  }

  return CMP_OK;
}

static int m3_tooltip_widget_event(void *widget, const CMPInputEvent *event,
                                   CMPBool *out_handled) {
  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_handled = CMP_FALSE;
  /* Tooltips typically do not handle events unless rich and have actions. */
  return CMP_OK;
}

static int m3_tooltip_widget_get_semantics(void *widget,
                                           CMPSemantics *out_semantics) {
  M3Tooltip *tooltip;
  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  tooltip = (M3Tooltip *)widget;

  out_semantics->role = CMP_SEMANTIC_TEXT;
  out_semantics->flags = 0;
  out_semantics->utf8_label = tooltip->utf8_body;
  out_semantics->utf8_hint = tooltip->utf8_title;
  out_semantics->utf8_value = NULL;

  return CMP_OK;
}

static int m3_tooltip_widget_destroy(void *widget) {
  M3Tooltip *tooltip;
  CMPHandle fonts[3];
  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  tooltip = (M3Tooltip *)widget;

  if (tooltip->owns_fonts == CMP_TRUE) {
    fonts[0] = tooltip->plain_font;
    fonts[1] = tooltip->title_font;
    fonts[2] = tooltip->body_font;
    m3_tooltip_destroy_fonts(&tooltip->text_backend, fonts, 3u);
  }

  return CMP_OK;
}

static void m3_tooltip_widget_vtable_init(void) {
  if (g_m3_tooltip_widget_vtable.measure == NULL) {
    g_m3_tooltip_widget_vtable.measure = m3_tooltip_widget_measure;
    g_m3_tooltip_widget_vtable.layout = m3_tooltip_widget_layout;
    g_m3_tooltip_widget_vtable.paint = m3_tooltip_widget_paint;
    g_m3_tooltip_widget_vtable.event = m3_tooltip_widget_event;
    g_m3_tooltip_widget_vtable.get_semantics = m3_tooltip_widget_get_semantics;
    g_m3_tooltip_widget_vtable.destroy = m3_tooltip_widget_destroy;
  }
}

int CMP_CALL m3_tooltip_init(M3Tooltip *tooltip,
                             const CMPTextBackend *text_backend,
                             const CMPTooltipStyle *style,
                             const CMPTooltipAnchor *anchor,
                             const CMPTooltipPlacement *placement,
                             CMPRect overlay, const char *utf8_body,
                             cmp_usize body_len) {
  int rc;
  CMPHandle fonts[3];

  if (tooltip == NULL || text_backend == NULL || style == NULL ||
      anchor == NULL || placement == NULL || utf8_body == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (text_backend->vtable == NULL ||
      text_backend->vtable->create_font == NULL ||
      text_backend->vtable->destroy_font == NULL ||
      text_backend->vtable->measure_text == NULL ||
      text_backend->vtable->draw_text == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  memset(fonts, 0, sizeof(fonts));

  if (style->variant == CMP_TOOLTIP_VARIANT_PLAIN) {
    rc = cmp_text_font_create(text_backend, &style->text_style, &fonts[0]);
    if (rc != CMP_OK) {
      return rc;
    }
  } else {
    rc = cmp_text_font_create(text_backend, &style->title_style, &fonts[1]);
    if (rc != CMP_OK) {
      return rc;
    }
    rc = cmp_text_font_create(text_backend, &style->body_style, &fonts[2]);
    if (rc != CMP_OK) {
      m3_tooltip_destroy_fonts(text_backend, fonts, 3u);
      return rc;
    }
  }

  memset(tooltip, 0, sizeof(*tooltip));

  m3_tooltip_widget_vtable_init();
  tooltip->widget.ctx = tooltip;
  tooltip->widget.vtable = &g_m3_tooltip_widget_vtable;

  tooltip->text_backend = *text_backend;
  tooltip->style = *style;
  tooltip->anchor = *anchor;
  tooltip->placement = *placement;
  tooltip->overlay = overlay;
  tooltip->utf8_body = utf8_body;
  tooltip->body_len = body_len;

  tooltip->plain_font = fonts[0];
  tooltip->title_font = fonts[1];
  tooltip->body_font = fonts[2];
  tooltip->owns_fonts = CMP_TRUE;

  return CMP_OK;
}

int CMP_CALL m3_tooltip_set_title(M3Tooltip *tooltip, const char *utf8_title,
                                  cmp_usize title_len) {
  if (tooltip == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  tooltip->utf8_title = utf8_title;
  tooltip->title_len = title_len;
  tooltip->metrics_valid = CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL m3_tooltip_set_overlay(M3Tooltip *tooltip, CMPRect overlay) {
  if (tooltip == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  tooltip->overlay = overlay;
  return CMP_OK;
}

int CMP_CALL m3_tooltip_test_metrics_update(M3Tooltip *tooltip) {
  return m3_tooltip_metrics_update(tooltip);
}
