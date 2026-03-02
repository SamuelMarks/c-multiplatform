#include "m3/m3_divider.h"
#include "cmpc/cmp_core.h"
#include <string.h>

static int m3_divider_measure(void *widget, CMPMeasureSpec width,
                              CMPMeasureSpec height, CMPSize *out_size);
static int m3_divider_layout(void *widget, CMPRect bounds);
static int m3_divider_paint(void *widget, CMPPaintContext *ctx);
static int m3_divider_event(void *widget, const CMPInputEvent *event,
                            CMPBool *out_handled);
static int m3_divider_get_semantics(void *widget, CMPSemantics *out_semantics);
static int m3_divider_destroy(void *widget);

static const CMPWidgetVTable m3_divider_vtable = {
    m3_divider_measure, m3_divider_layout,        m3_divider_paint,
    m3_divider_event,   m3_divider_get_semantics, m3_divider_destroy};

CMP_API int CMP_CALL m3_divider_style_init(M3DividerStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  style->direction = CMP_LAYOUT_DIRECTION_ROW;

  /* Outline variant color (approx) */
  style->color.r = 0.792f;
  style->color.g = 0.776f;
  style->color.b = 0.812f;
  style->color.a = 1.0f;

  style->thickness = M3_DIVIDER_DEFAULT_THICKNESS;
  style->inset_start = 0.0f;
  style->inset_end = 0.0f;

  return CMP_OK;
}

CMP_API int CMP_CALL m3_divider_init(M3Divider *divider,
                                     const M3DividerStyle *style) {
  if (divider == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->thickness < 0.0f || style->inset_start < 0.0f ||
      style->inset_end < 0.0f) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(divider, 0, sizeof(*divider));
  divider->widget.vtable = &m3_divider_vtable;
  divider->style = *style;

  return CMP_OK;
}

CMP_API int CMP_CALL m3_divider_set_style(M3Divider *divider,
                                          const M3DividerStyle *style) {
  if (divider == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->thickness < 0.0f || style->inset_start < 0.0f ||
      style->inset_end < 0.0f) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  divider->style = *style;
  return CMP_OK;
}

static int m3_divider_measure(void *widget, CMPMeasureSpec width,
                              CMPMeasureSpec height, CMPSize *out_size) {
  M3Divider *divider = (M3Divider *)widget;

  if (divider == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (divider->style.direction == CMP_LAYOUT_DIRECTION_ROW) {
    /* Horizontal divider: width is available width, height is thickness */
    out_size->width = (width.mode == CMP_MEASURE_EXACTLY) ? width.size : 0.0f;
    out_size->height = (height.mode == CMP_MEASURE_EXACTLY)
                           ? height.size
                           : divider->style.thickness;
  } else {
    /* Vertical divider: height is available height, width is thickness */
    out_size->width = (width.mode == CMP_MEASURE_EXACTLY)
                          ? width.size
                          : divider->style.thickness;
    out_size->height =
        (height.mode == CMP_MEASURE_EXACTLY) ? height.size : 0.0f;
  }
  return CMP_OK;
}

static int m3_divider_layout(void *widget, CMPRect bounds) {
  M3Divider *divider = (M3Divider *)widget;
  if (divider == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  divider->bounds = bounds;
  return CMP_OK;
}

static int m3_divider_paint(void *widget, CMPPaintContext *ctx) {
  M3Divider *divider = (M3Divider *)widget;
  CMPRect paint_bounds;

  if (divider == NULL || ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (ctx->gfx == NULL || divider->style.color.a <= 0.0f ||
      divider->style.thickness <= 0.0f) {
    return CMP_OK; /* Nothing to draw */
  }

  paint_bounds = divider->bounds;

  if (divider->style.direction == CMP_LAYOUT_DIRECTION_ROW) {
    paint_bounds.x += divider->style.inset_start;
    paint_bounds.width -=
        (divider->style.inset_start + divider->style.inset_end);
    if (paint_bounds.width < 0.0f)
      paint_bounds.width = 0.0f;
    paint_bounds.height = divider->style.thickness;
  } else {
    paint_bounds.y += divider->style.inset_start;
    paint_bounds.height -=
        (divider->style.inset_start + divider->style.inset_end);
    if (paint_bounds.height < 0.0f)
      paint_bounds.height = 0.0f;
    paint_bounds.width = divider->style.thickness;
  }

  return ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &paint_bounds,
                                     divider->style.color, 0.0f);
}

static int m3_divider_event(void *widget, const CMPInputEvent *event,
                            CMPBool *out_handled) {
  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_handled = CMP_FALSE;
  return CMP_OK;
}

static int m3_divider_get_semantics(void *widget, CMPSemantics *out_semantics) {
  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  /* Dividers typically don't have semantic value unless explicitly set */
  out_semantics->role = CMP_SEMANTIC_NONE;
  out_semantics->flags = 0;
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_divider_destroy(void *widget) {
  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}
