#include "m3/m3_list.h"
#include "cmpc/cmp_core.h"
#include "m3/m3_typography.h"
#include <string.h>

static int m3_list_item_measure(void *widget, CMPMeasureSpec width,
                                CMPMeasureSpec height, CMPSize *out_size);
static int m3_list_item_layout(void *widget, CMPRect bounds);
static int m3_list_item_paint(void *widget, CMPPaintContext *ctx);
static int m3_list_item_event(void *widget, const CMPInputEvent *event,
                              CMPBool *out_handled);
static int m3_list_item_get_semantics(void *widget,
                                      CMPSemantics *out_semantics);
static int m3_list_item_destroy(void *widget);

static const CMPWidgetVTable m3_list_item_vtable = {
    m3_list_item_measure, m3_list_item_layout,        m3_list_item_paint,
    m3_list_item_event,   m3_list_item_get_semantics, m3_list_item_destroy};

CMP_API int CMP_CALL m3_list_item_style_init(M3ListItemStyle *style,
                                             cmp_u32 variant) {
  M3TypographyScale typography;
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (variant != M3_LIST_ITEM_VARIANT_1_LINE &&
      variant != M3_LIST_ITEM_VARIANT_2_LINE &&
      variant != M3_LIST_ITEM_VARIANT_3_LINE) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  m3_typography_scale_init(&typography);

  style->variant = variant;

  m3_typography_get_style(&typography, M3_TYPOGRAPHY_BODY_LARGE,
                          &style->headline_style);
  m3_typography_get_style(&typography, M3_TYPOGRAPHY_BODY_MEDIUM,
                          &style->supporting_style);
  m3_typography_get_style(&typography, M3_TYPOGRAPHY_LABEL_SMALL,
                          &style->trailing_style);

  style->background_color.r = 0.0f;
  style->background_color.g = 0.0f;
  style->background_color.b = 0.0f;
  style->background_color.a = 0.0f;
  style->ripple_color.r = 0.0f;
  style->ripple_color.g = 0.0f;
  style->ripple_color.b = 0.0f;
  style->ripple_color.a = 0.12f;
  style->icon_color.r = 0.286f;
  style->icon_color.g = 0.270f;
  style->icon_color.b = 0.309f;
  style->icon_color.a = 1.0f;

  style->padding_x = M3_LIST_ITEM_DEFAULT_PADDING_X;

  if (variant == M3_LIST_ITEM_VARIANT_1_LINE) {
    style->min_height = 56.0f;
  } else if (variant == M3_LIST_ITEM_VARIANT_2_LINE) {
    style->min_height = 72.0f;
  } else {
    style->min_height = 88.0f;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL m3_list_item_init(M3ListItem *item,
                                       const M3ListItemStyle *style,
                                       CMPTextBackend text_backend) {
  if (item == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  item->widget.vtable = &m3_list_item_vtable;
  item->text_backend = text_backend;
  item->style = *style;
  item->utf8_headline = NULL;
  item->utf8_supporting = NULL;
  item->utf8_trailing = NULL;
  item->leading_widget = NULL;
  item->trailing_widget = NULL;
  item->bounds.x = 0.0f;
  item->bounds.y = 0.0f;
  item->bounds.width = 0.0f;
  item->bounds.height = 0.0f;
  item->pressed = CMP_FALSE;
  item->on_press = NULL;
  item->on_press_ctx = NULL;

  cmp_ripple_init(&item->ripple);

  return CMP_OK;
}

CMP_API int CMP_CALL m3_list_item_set_headline(M3ListItem *item,
                                               const char *utf8_text) {
  if (item == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  item->utf8_headline = utf8_text;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_list_item_set_supporting(M3ListItem *item,
                                                 const char *utf8_text) {
  if (item == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  item->utf8_supporting = utf8_text;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_list_item_set_trailing(M3ListItem *item,
                                               const char *utf8_text) {
  if (item == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  item->utf8_trailing = utf8_text;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_list_item_set_widgets(M3ListItem *item,
                                              CMPWidget *leading,
                                              CMPWidget *trailing) {
  if (item == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  item->leading_widget = leading;
  item->trailing_widget = trailing;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_list_item_set_on_press(M3ListItem *item,
                                               M3ListItemOnPress on_press,
                                               void *ctx) {
  if (item == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  item->on_press = on_press;
  item->on_press_ctx = ctx;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_list_test_helper(void) { return CMP_OK; }

static int m3_list_item_measure(void *widget, CMPMeasureSpec width,
                                CMPMeasureSpec height, CMPSize *out_size) {
  M3ListItem *item = (M3ListItem *)widget;
  CMPScalar w = 0.0f;
  CMPSize child_size;

  if (item == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (width.mode == CMP_MEASURE_EXACTLY) {
    w = width.size;
  } else if (width.mode == CMP_MEASURE_AT_MOST) {
    w = width.size; /* List items usually expand to fill horizontally */
  }

  out_size->width = w;
  out_size->height = item->style.min_height;

  if (height.mode == CMP_MEASURE_EXACTLY) {
    out_size->height = height.size;
  }

  if (item->leading_widget != NULL) {
    item->leading_widget->vtable->measure(item->leading_widget, width, height,
                                          &child_size);
  }
  if (item->trailing_widget != NULL) {
    item->trailing_widget->vtable->measure(item->trailing_widget, width, height,
                                           &child_size);
  }

  return CMP_OK;
}

static int m3_list_item_layout(void *widget, CMPRect bounds) {
  M3ListItem *item = (M3ListItem *)widget;
  CMPRect child_bounds;
  CMPSize child_size;
  CMPMeasureSpec spec_unspec = {CMP_MEASURE_UNSPECIFIED, 0.0f};

  if (item == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  item->bounds = bounds;

  if (item->leading_widget != NULL) {
    item->leading_widget->vtable->measure(item->leading_widget, spec_unspec,
                                          spec_unspec, &child_size);
    child_bounds.x = bounds.x + item->style.padding_x;
    child_bounds.y = bounds.y + (bounds.height - child_size.height) * 0.5f;
    child_bounds.width = child_size.width;
    child_bounds.height = child_size.height;
    item->leading_widget->vtable->layout(item->leading_widget, child_bounds);
  }

  if (item->trailing_widget != NULL) {
    item->trailing_widget->vtable->measure(item->trailing_widget, spec_unspec,
                                           spec_unspec, &child_size);
    child_bounds.x =
        bounds.x + bounds.width - item->style.padding_x - child_size.width;
    child_bounds.y = bounds.y + (bounds.height - child_size.height) * 0.5f;
    child_bounds.width = child_size.width;
    child_bounds.height = child_size.height;
    item->trailing_widget->vtable->layout(item->trailing_widget, child_bounds);
  }

  return CMP_OK;
}

static int m3_list_item_paint(void *widget, CMPPaintContext *ctx) {
  M3ListItem *item = (M3ListItem *)widget;
  CMPHandle font = {0};
  CMPScalar text_x, text_y, w, h, baseline;
  int rc;

  if (item == NULL || ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (item->style.background_color.a > 0.0f && ctx->gfx != NULL) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &item->bounds,
                                item->style.background_color, 0.0f);
  }

  text_x = item->bounds.x + item->style.padding_x;
  if (item->leading_widget != NULL) {
    text_x += 40.0f + 16.0f; /* approximate icon width + padding */
  }
  text_y = item->bounds.y + (item->style.min_height * 0.5f) - 8.0f;

  if (item->utf8_headline != NULL && item->text_backend.vtable != NULL) {
    rc = item->text_backend.vtable->create_font(
        item->text_backend.ctx, item->style.headline_style.utf8_family,
        item->style.headline_style.size_px, item->style.headline_style.weight,
        item->style.headline_style.italic, &font);
    if (rc == CMP_OK) {
      item->text_backend.vtable->measure_text(
          item->text_backend.ctx, font, item->utf8_headline,
          strlen(item->utf8_headline), &w, &h, &baseline);
      item->text_backend.vtable->draw_text(
          item->text_backend.ctx, font, item->utf8_headline,
          strlen(item->utf8_headline), text_x, text_y + baseline,
          item->style.headline_style.color);
      item->text_backend.vtable->destroy_font(item->text_backend.ctx, font);
    }
  }

  if (item->leading_widget != NULL) {
    item->leading_widget->vtable->paint(item->leading_widget, ctx);
  }
  if (item->trailing_widget != NULL) {
    item->trailing_widget->vtable->paint(item->trailing_widget, ctx);
  }

  return CMP_OK;
}

static int m3_list_item_event(void *widget, const CMPInputEvent *event,
                              CMPBool *out_handled) {
  M3ListItem *item = (M3ListItem *)widget;
  CMPRect bounds;

  if (item == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;

  if (item->leading_widget != NULL) {
    CMPBool child_handled = CMP_FALSE;
    item->leading_widget->vtable->event(item->leading_widget, event,
                                        &child_handled);
    if (child_handled) {
      *out_handled = CMP_TRUE;
      return CMP_OK;
    }
  }

  if (item->trailing_widget != NULL) {
    CMPBool child_handled = CMP_FALSE;
    item->trailing_widget->vtable->event(item->trailing_widget, event,
                                         &child_handled);
    if (child_handled) {
      *out_handled = CMP_TRUE;
      return CMP_OK;
    }
  }

  bounds = item->bounds;
  if (event->type == CMP_INPUT_POINTER_DOWN) {
    if (event->data.pointer.x >= bounds.x &&
        event->data.pointer.x <= bounds.x + bounds.width &&
        event->data.pointer.y >= bounds.y &&
        event->data.pointer.y <= bounds.y + bounds.height) {
      item->pressed = CMP_TRUE;
      cmp_ripple_start(&item->ripple, event->data.pointer.x,
                       event->data.pointer.y, bounds.width, 0.3f,
                       item->style.ripple_color);
      *out_handled = CMP_TRUE;
    }
  } else if (event->type == CMP_INPUT_POINTER_UP) {
    if (item->pressed) {
      item->pressed = CMP_FALSE;
      cmp_ripple_release(&item->ripple, 0.2f);
      if (item->on_press != NULL) {
        item->on_press(item->on_press_ctx, item);
      }
      *out_handled = CMP_TRUE;
    }
  }

  return CMP_OK;
}

static int m3_list_item_get_semantics(void *widget,
                                      CMPSemantics *out_semantics) {
  M3ListItem *item = (M3ListItem *)widget;
  if (item == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  out_semantics->role = CMP_SEMANTIC_BUTTON;
  out_semantics->flags = CMP_SEMANTIC_FLAG_FOCUSABLE;
  out_semantics->utf8_label = item->utf8_headline;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_list_item_destroy(void *widget) {
  M3ListItem *item = (M3ListItem *)widget;
  if (item == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}
