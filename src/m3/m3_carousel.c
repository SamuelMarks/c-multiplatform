#include "m3/m3_carousel.h"
#include "cmpc/cmp_core.h"
#include <stdlib.h>

static int m3_carousel_measure(void *widget, CMPMeasureSpec width,
                               CMPMeasureSpec height, CMPSize *out_size);
static int m3_carousel_layout(void *widget, CMPRect bounds);
static int m3_carousel_paint(void *widget, CMPPaintContext *ctx);
static int m3_carousel_event(void *widget, const CMPInputEvent *event,
                             CMPBool *out_handled);
static int m3_carousel_get_semantics(void *widget, CMPSemantics *out_semantics);
static int m3_carousel_destroy(void *widget);

static const CMPWidgetVTable m3_carousel_vtable = {
    m3_carousel_measure, m3_carousel_layout,        m3_carousel_paint,
    m3_carousel_event,   m3_carousel_get_semantics, m3_carousel_destroy};

CMP_API int CMP_CALL m3_carousel_style_init(M3CarouselStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  style->item_spacing = 8.0f;
  style->snap_velocity = 500.0f;
  style->background_color.r = 0.0f;
  style->background_color.g = 0.0f;
  style->background_color.b = 0.0f;
  style->background_color.a = 0.0f;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_carousel_init(M3Carousel *carousel,
                                      const M3CarouselStyle *style,
                                      CMPWidget **items, cmp_usize item_count) {
  cmp_usize i;
  if (carousel == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  carousel->widget.vtable = &m3_carousel_vtable;
  carousel->style = *style;
  carousel->item_count = item_count;
  carousel->capacity = item_count > 0 ? item_count : 4;
  carousel->items =
      (CMPWidget **)malloc(carousel->capacity * sizeof(CMPWidget *));
  if (carousel->items == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  if (items != NULL && item_count > 0) {
    for (i = 0; i < item_count; i++) {
      carousel->items[i] = items[i];
    }
  }

  carousel->bounds.x = 0.0f;
  carousel->bounds.y = 0.0f;
  carousel->bounds.width = 0.0f;
  carousel->bounds.height = 0.0f;
  carousel->selected_index = 0;
  carousel->on_select = NULL;
  carousel->on_select_ctx = NULL;

  carousel->scroll_offset = 0.0f;
  carousel->content_extent = 0.0f;
  carousel->dragging = CMP_FALSE;
  carousel->last_pointer_x = 0;
  carousel->last_time_ms = 0;
  carousel->velocity = 0.0f;

  return CMP_OK;
}

CMP_API int CMP_CALL m3_carousel_add_item(M3Carousel *carousel,
                                          CMPWidget *item) {
  if (carousel == NULL || item == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (carousel->item_count >= carousel->capacity) {
    cmp_usize new_cap = carousel->capacity * 2;
    CMPWidget **new_items =
        (CMPWidget **)realloc(carousel->items, new_cap * sizeof(CMPWidget *));
    if (new_items == NULL) {
      return CMP_ERR_OUT_OF_MEMORY;
    }
    carousel->items = new_items;
    carousel->capacity = new_cap;
  }
  carousel->items[carousel->item_count] = item;
  carousel->item_count += 1;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_carousel_set_on_select(M3Carousel *carousel,
                                               M3CarouselOnSelect on_select,
                                               void *ctx) {
  if (carousel == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  carousel->on_select = on_select;
  carousel->on_select_ctx = ctx;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_carousel_test_helper(void) { return CMP_OK; }

static int m3_carousel_measure(void *widget, CMPMeasureSpec width,
                               CMPMeasureSpec height, CMPSize *out_size) {
  M3Carousel *carousel = (M3Carousel *)widget;
  CMPSize child_size;
  cmp_usize i;
  CMPScalar max_h = 0.0f;
  CMPMeasureSpec unspec = {CMP_MEASURE_UNSPECIFIED, 0.0f};

  if (carousel == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < carousel->item_count; i++) {
    carousel->items[i]->vtable->measure(carousel->items[i], unspec, height,
                                        &child_size);
    if (child_size.height > max_h) {
      max_h = child_size.height;
    }
  }

  out_size->width = width.size;
  out_size->height = height.mode == CMP_MEASURE_EXACTLY ? height.size : max_h;

  return CMP_OK;
}

static int m3_carousel_layout(void *widget, CMPRect bounds) {
  M3Carousel *carousel = (M3Carousel *)widget;
  CMPRect child_bounds;
  CMPSize child_size;
  cmp_usize i;
  CMPScalar x_offset;
  CMPMeasureSpec unspec = {CMP_MEASURE_UNSPECIFIED, 0.0f};

  if (carousel == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  carousel->bounds = bounds;
  x_offset = bounds.x - carousel->scroll_offset;

  for (i = 0; i < carousel->item_count; i++) {
    carousel->items[i]->vtable->measure(carousel->items[i], unspec, unspec,
                                        &child_size);
    child_bounds.x = x_offset;
    child_bounds.y = bounds.y + (bounds.height - child_size.height) * 0.5f;
    child_bounds.width = child_size.width;
    child_bounds.height = child_size.height;

    carousel->items[i]->vtable->layout(carousel->items[i], child_bounds);

    x_offset += child_size.width + carousel->style.item_spacing;
  }

  carousel->content_extent = x_offset - bounds.x + carousel->scroll_offset -
                             carousel->style.item_spacing;

  return CMP_OK;
}

static int m3_carousel_paint(void *widget, CMPPaintContext *ctx) {
  M3Carousel *carousel = (M3Carousel *)widget;
  cmp_usize i;

  if (carousel == NULL || ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (carousel->style.background_color.a > 0.0f && ctx->gfx != NULL) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &carousel->bounds,
                                carousel->style.background_color, 0.0f);
  }

  for (i = 0; i < carousel->item_count; i++) {
    carousel->items[i]->vtable->paint(carousel->items[i], ctx);
  }

  return CMP_OK;
}

static int m3_carousel_event(void *widget, const CMPInputEvent *event,
                             CMPBool *out_handled) {
  M3Carousel *carousel = (M3Carousel *)widget;
  cmp_usize i;
  CMPBool child_handled = CMP_FALSE;

  if (carousel == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;

  for (i = 0; i < carousel->item_count; i++) {
    carousel->items[i]->vtable->event(carousel->items[i], event,
                                      &child_handled);
    if (child_handled) {
      *out_handled = CMP_TRUE;
      return CMP_OK;
    }
  }

  if (event->type == CMP_INPUT_POINTER_DOWN) {
    carousel->dragging = CMP_TRUE;
    carousel->last_pointer_x = event->data.pointer.x;
    carousel->last_time_ms = event->time_ms;
    carousel->velocity = 0.0f;
    *out_handled = CMP_TRUE;
  } else if (event->type == CMP_INPUT_POINTER_MOVE && carousel->dragging) {
    CMPScalar dx =
        (CMPScalar)(carousel->last_pointer_x - event->data.pointer.x);
    cmp_u32 dt = event->time_ms - carousel->last_time_ms;

    if (dt > 0) {
      carousel->velocity = dx / ((CMPScalar)dt / 1000.0f);
    }

    carousel->scroll_offset += dx;
    if (carousel->scroll_offset < 0.0f)
      carousel->scroll_offset = 0.0f;
    if (carousel->scroll_offset >
        carousel->content_extent - carousel->bounds.width) {
      carousel->scroll_offset =
          carousel->content_extent - carousel->bounds.width;
      if (carousel->scroll_offset < 0.0f)
        carousel->scroll_offset = 0.0f;
    }

    carousel->last_pointer_x = event->data.pointer.x;
    carousel->last_time_ms = event->time_ms;
    *out_handled = CMP_TRUE;
  } else if (event->type == CMP_INPUT_POINTER_UP) {
    carousel->dragging = CMP_FALSE;
    *out_handled = CMP_TRUE;
  }

  return CMP_OK;
}

static int m3_carousel_get_semantics(void *widget,
                                     CMPSemantics *out_semantics) {
  M3Carousel *carousel = (M3Carousel *)widget;
  if (carousel == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  out_semantics->role = CMP_SEMANTIC_NONE;
  out_semantics->flags = CMP_SEMANTIC_FLAG_FOCUSABLE;
  out_semantics->utf8_label = "Carousel";
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_carousel_destroy(void *widget) {
  M3Carousel *carousel = (M3Carousel *)widget;
  if (carousel == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (carousel->items != NULL) {
    free(carousel->items);
  }
  return CMP_OK;
}
