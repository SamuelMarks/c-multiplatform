#include "m3/m3_search.h"
#include "cmpc/cmp_core.h"
#include "m3/m3_typography.h"
#include <string.h>

static int m3_search_bar_measure(void *widget, CMPMeasureSpec width,
                                 CMPMeasureSpec height, CMPSize *out_size);
static int m3_search_bar_layout(void *widget, CMPRect bounds);
static int m3_search_bar_paint(void *widget, CMPPaintContext *ctx);
static int m3_search_bar_event(void *widget, const CMPInputEvent *event,
                               CMPBool *out_handled);
static int m3_search_bar_get_semantics(void *widget,
                                       CMPSemantics *out_semantics);
static int m3_search_bar_destroy(void *widget);

static const CMPWidgetVTable m3_search_bar_vtable = {
    m3_search_bar_measure, m3_search_bar_layout,        m3_search_bar_paint,
    m3_search_bar_event,   m3_search_bar_get_semantics, m3_search_bar_destroy};

CMP_API int CMP_CALL m3_search_bar_style_init(M3SearchBarStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  m3_text_field_style_init(&style->field_style, M3_TEXT_FIELD_VARIANT_FILLED);

  style->field_style.core.outline_color.r = 0.0f;
  style->field_style.core.outline_color.g = 0.0f;
  style->field_style.core.outline_color.b = 0.0f;
  style->field_style.core.outline_color.a = 0.0f;

  style->field_style.core.focused_outline_color.r = 0.0f;
  style->field_style.core.focused_outline_color.g = 0.0f;
  style->field_style.core.focused_outline_color.b = 0.0f;
  style->field_style.core.focused_outline_color.a = 0.0f;

  style->field_style.core.container_color.r = 0.0f;
  style->field_style.core.container_color.g = 0.0f;
  style->field_style.core.container_color.b = 0.0f;
  style->field_style.core.container_color.a = 0.0f;

  style->background_color.r = 0.94f;
  style->background_color.g = 0.93f;
  style->background_color.b = 0.96f;
  style->background_color.a = 1.0f;

  style->corner_radius = 28.0f;
  style->min_height = 56.0f;

  return CMP_OK;
}

static int CMP_CALL m3_search_bar_on_field_change(void *ctx, M3TextField *field,
                                                  const char *utf8_text) {
  M3SearchBar *search_bar = (M3SearchBar *)ctx;
  (void)field;
  if (search_bar->on_change != NULL) {
    return search_bar->on_change(search_bar->on_change_ctx, search_bar,
                                 utf8_text);
  }
  return CMP_OK;
}

CMP_API int CMP_CALL m3_search_bar_init(M3SearchBar *search_bar,
                                        const M3SearchBarStyle *style,
                                        CMPAllocator allocator,
                                        CMPTextBackend text_backend) {
  if (search_bar == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  search_bar->widget.vtable = &m3_search_bar_vtable;
  search_bar->text_backend = text_backend;
  search_bar->style = *style;
  search_bar->leading_icon = NULL;
  search_bar->trailing_icon = NULL;
  search_bar->bounds.x = 0.0f;
  search_bar->bounds.y = 0.0f;
  search_bar->bounds.width = 0.0f;
  search_bar->bounds.height = 0.0f;
  search_bar->on_change = NULL;
  search_bar->on_change_ctx = NULL;

  m3_text_field_init(&search_bar->field, &style->field_style, allocator,
                     text_backend);
  m3_text_field_set_on_change(&search_bar->field, m3_search_bar_on_field_change,
                              search_bar);

  return CMP_OK;
}

CMP_API int CMP_CALL m3_search_bar_set_placeholder(
    M3SearchBar *search_bar, const char *utf8_placeholder) {
  if (search_bar == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return m3_text_field_set_placeholder(&search_bar->field, utf8_placeholder);
}

CMP_API int CMP_CALL m3_search_bar_set_icons(M3SearchBar *search_bar,
                                             CMPWidget *leading,
                                             CMPWidget *trailing) {
  if (search_bar == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  search_bar->leading_icon = leading;
  search_bar->trailing_icon = trailing;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_search_bar_set_on_change(M3SearchBar *search_bar,
                                                 M3SearchOnChange on_change,
                                                 void *ctx) {
  if (search_bar == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  search_bar->on_change = on_change;
  search_bar->on_change_ctx = ctx;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_search_test_helper(void) { return CMP_OK; }

static int m3_search_bar_measure(void *widget, CMPMeasureSpec width,
                                 CMPMeasureSpec height, CMPSize *out_size) {
  M3SearchBar *bar = (M3SearchBar *)widget;
  CMPScalar w = 0.0f;

  if (bar == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (width.mode == CMP_MEASURE_EXACTLY || width.mode == CMP_MEASURE_AT_MOST) {
    w = width.size;
  }

  out_size->width = w;
  out_size->height = bar->style.min_height;

  if (height.mode == CMP_MEASURE_EXACTLY) {
    out_size->height = height.size;
  }

  return CMP_OK;
}

static int m3_search_bar_layout(void *widget, CMPRect bounds) {
  M3SearchBar *bar = (M3SearchBar *)widget;
  CMPRect field_bounds;
  CMPScalar content_x;
  CMPScalar content_w;
  CMPSize icon_size;
  CMPRect icon_bounds;
  CMPMeasureSpec unspec = {CMP_MEASURE_UNSPECIFIED, 0.0f};

  if (bar == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  bar->bounds = bounds;
  content_x = bounds.x + 16.0f;
  content_w = bounds.width - 32.0f;

  if (bar->leading_icon != NULL) {
    bar->leading_icon->vtable->measure(bar->leading_icon, unspec, unspec,
                                       &icon_size);
    icon_bounds.x = content_x;
    icon_bounds.y = bounds.y + (bounds.height - icon_size.height) * 0.5f;
    icon_bounds.width = icon_size.width;
    icon_bounds.height = icon_size.height;
    bar->leading_icon->vtable->layout(bar->leading_icon, icon_bounds);

    content_x += icon_size.width + 16.0f;
    content_w -= icon_size.width + 16.0f;
  }

  if (bar->trailing_icon != NULL) {
    bar->trailing_icon->vtable->measure(bar->trailing_icon, unspec, unspec,
                                        &icon_size);
    icon_bounds.x = bounds.x + bounds.width - 16.0f - icon_size.width;
    icon_bounds.y = bounds.y + (bounds.height - icon_size.height) * 0.5f;
    icon_bounds.width = icon_size.width;
    icon_bounds.height = icon_size.height;
    bar->trailing_icon->vtable->layout(bar->trailing_icon, icon_bounds);

    content_w -= icon_size.width + 16.0f;
  }

  field_bounds.x = content_x;
  field_bounds.y = bounds.y;
  field_bounds.width = content_w;
  field_bounds.height = bounds.height;
  bar->field.widget.vtable->layout(&bar->field, field_bounds);

  return CMP_OK;
}

static int m3_search_bar_paint(void *widget, CMPPaintContext *ctx) {
  M3SearchBar *bar = (M3SearchBar *)widget;

  if (bar == NULL || ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (bar->style.background_color.a > 0.0f && ctx->gfx != NULL) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &bar->bounds,
                                bar->style.background_color,
                                bar->style.corner_radius);
  }

  bar->field.widget.vtable->paint(&bar->field, ctx);

  if (bar->leading_icon != NULL) {
    bar->leading_icon->vtable->paint(bar->leading_icon, ctx);
  }
  if (bar->trailing_icon != NULL) {
    bar->trailing_icon->vtable->paint(bar->trailing_icon, ctx);
  }

  return CMP_OK;
}

static int m3_search_bar_event(void *widget, const CMPInputEvent *event,
                               CMPBool *out_handled) {
  M3SearchBar *bar = (M3SearchBar *)widget;

  if (bar == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;

  if (bar->leading_icon != NULL) {
    CMPBool handled = CMP_FALSE;
    bar->leading_icon->vtable->event(bar->leading_icon, event, &handled);
    if (handled) {
      *out_handled = CMP_TRUE;
      return CMP_OK;
    }
  }

  if (bar->trailing_icon != NULL) {
    CMPBool handled = CMP_FALSE;
    bar->trailing_icon->vtable->event(bar->trailing_icon, event, &handled);
    if (handled) {
      *out_handled = CMP_TRUE;
      return CMP_OK;
    }
  }

  return bar->field.widget.vtable->event(&bar->field, event, out_handled);
}

static int m3_search_bar_get_semantics(void *widget,
                                       CMPSemantics *out_semantics) {
  M3SearchBar *bar = (M3SearchBar *)widget;
  if (bar == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return bar->field.widget.vtable->get_semantics(&bar->field, out_semantics);
}

static int m3_search_bar_destroy(void *widget) {
  M3SearchBar *bar = (M3SearchBar *)widget;
  if (bar == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return bar->field.widget.vtable->destroy(&bar->field);
}
