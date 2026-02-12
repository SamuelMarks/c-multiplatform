#include "m3/m3_menu.h"

#include <string.h>

static int m3_menu_validate_rect(const M3Rect *rect) {
  if (rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_menu_validate_color(const M3Color *color) {
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

static int m3_menu_color_set(M3Color *color, M3Scalar r, M3Scalar g, M3Scalar b,
                             M3Scalar a) {
  if (color == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!(r >= 0.0f && r <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(g >= 0.0f && g <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(b >= 0.0f && b <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(a >= 0.0f && a <= 1.0f)) {
    return M3_ERR_RANGE;
  }

  color->r = r;
  color->g = g;
  color->b = b;
  color->a = a;
  return M3_OK;
}

static int m3_menu_validate_edges(const M3LayoutEdges *edges) {
  if (edges == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->right < 0.0f || edges->top < 0.0f ||
      edges->bottom < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_menu_validate_text_style(const M3TextStyle *style,
                                       M3Bool require_family) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (require_family == M3_TRUE && style->utf8_family == NULL) {
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

  rc = m3_menu_validate_color(&style->color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_menu_validate_style(const M3MenuStyle *style,
                                  M3Bool require_family) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (style->item_height <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->item_spacing < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->min_width < 0.0f || style->max_width < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->max_width > 0.0f && style->max_width < style->min_width) {
    return M3_ERR_RANGE;
  }
  if (style->corner_radius < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->anchor_gap < 0.0f) {
    return M3_ERR_RANGE;
  }

  rc = m3_menu_validate_edges(&style->padding);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_menu_validate_text_style(&style->text_style, require_family);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_menu_validate_color(&style->background_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_menu_validate_color(&style->disabled_text_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_menu_validate_backend(const M3TextBackend *backend) {
  if (backend == NULL || backend->vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_OK;
}

static int m3_menu_validate_items(const M3MenuItem *items, m3_usize count) {
  m3_usize i;

  if (count == 0) {
    return M3_OK;
  }
  if (items == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < count; ++i) {
    const M3MenuItem *item = &items[i];
    if (item->utf8_label == NULL && item->utf8_len != 0u) {
      return M3_ERR_INVALID_ARGUMENT;
    }
    if (item->enabled != M3_FALSE && item->enabled != M3_TRUE) {
      return M3_ERR_RANGE;
    }
  }
  return M3_OK;
}

static int m3_menu_validate_measure_spec(M3MeasureSpec spec) {
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.mode != M3_MEASURE_EXACTLY &&
      spec.mode != M3_MEASURE_AT_MOST) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_menu_validate_anchor(const M3MenuAnchor *anchor) {
  int rc;

  if (anchor == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (anchor->type == M3_MENU_ANCHOR_RECT) {
    rc = m3_menu_validate_rect(&anchor->rect);
    if (rc != M3_OK) {
      return rc;
    }
  } else if (anchor->type == M3_MENU_ANCHOR_POINT) {
    return M3_OK;
  } else {
    return M3_ERR_RANGE;
  }

  return M3_OK;
}

static int m3_menu_validate_placement(const M3MenuPlacement *placement) {
  if (placement == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  switch (placement->direction) {
  case M3_MENU_DIRECTION_DOWN:
  case M3_MENU_DIRECTION_UP:
  case M3_MENU_DIRECTION_RIGHT:
  case M3_MENU_DIRECTION_LEFT:
    break;
  default:
    return M3_ERR_RANGE;
  }

  switch (placement->align) {
  case M3_MENU_ALIGN_START:
  case M3_MENU_ALIGN_CENTER:
  case M3_MENU_ALIGN_END:
    break;
  default:
    return M3_ERR_RANGE;
  }

  return M3_OK;
}

static M3Scalar m3_menu_clamp(M3Scalar value, M3Scalar min_value,
                              M3Scalar max_value) {
  if (value < min_value) {
    return min_value;
  }
  if (value > max_value) {
    return max_value;
  }
  return value;
}

static int m3_menu_update_metrics(M3Menu *menu, M3Scalar *out_text_width,
                                  M3Bool *out_has_label) {
  M3TextMetrics metrics;
  M3Scalar max_width;
  M3Bool has_label;
  m3_usize i;
  int rc;

  if (menu == NULL || out_text_width == NULL || out_has_label == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_menu_validate_items(menu->items, menu->item_count);
  if (rc != M3_OK) {
    return rc;
  }

  max_width = 0.0f;
  has_label = M3_FALSE;
  for (i = 0; i < menu->item_count; ++i) {
    if (menu->items[i].utf8_len > 0u) {
      has_label = M3_TRUE;
      break;
    }
  }

  if (has_label == M3_TRUE) {
    if (menu->text_backend.vtable == NULL ||
        menu->text_backend.vtable->measure_text == NULL) {
      return M3_ERR_UNSUPPORTED;
    }
    if (menu->font.id == 0u && menu->font.generation == 0u) {
      return M3_ERR_STATE;
    }

    rc = m3_text_font_metrics(&menu->text_backend, menu->font,
                              &menu->font_metrics);
    if (rc != M3_OK) {
      return rc;
    }

    for (i = 0; i < menu->item_count; ++i) {
      if (menu->items[i].utf8_len == 0u) {
        continue;
      }
      rc = m3_text_measure_utf8(&menu->text_backend, menu->font,
                                menu->items[i].utf8_label,
                                menu->items[i].utf8_len, &metrics);
      if (rc != M3_OK) {
        return rc;
      }
      if (metrics.width < 0.0f) {
        return M3_ERR_RANGE;
      }
      if (metrics.width > max_width) {
        max_width = metrics.width;
      }
    }
  } else {
    menu->font_metrics.width = 0.0f;
    menu->font_metrics.height = 0.0f;
    menu->font_metrics.baseline = 0.0f;
  }

  *out_text_width = max_width;
  *out_has_label = has_label;
  return M3_OK;
}

static int m3_menu_compute_panel_size(M3Menu *menu, M3Scalar *out_width,
                                      M3Scalar *out_height,
                                      M3Bool *out_has_label) {
  M3Scalar text_width;
  M3Scalar width;
  M3Scalar height;
  int rc;

  if (menu == NULL || out_width == NULL || out_height == NULL ||
      out_has_label == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_menu_validate_style(&menu->style, M3_FALSE);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_menu_update_metrics(menu, &text_width, out_has_label);
  if (rc != M3_OK) {
    return rc;
  }

  width = menu->style.padding.left + menu->style.padding.right + text_width;
  if (width < menu->style.min_width) {
    width = menu->style.min_width;
  }
  if (menu->style.max_width > 0.0f && width > menu->style.max_width) {
    width = menu->style.max_width;
  }

  height = menu->style.padding.top + menu->style.padding.bottom;
  if (menu->item_count > 0u) {
    height += menu->style.item_height * (M3Scalar)menu->item_count;
    height += menu->style.item_spacing * (M3Scalar)(menu->item_count - 1u);
  }

  if (width < 0.0f || height < 0.0f) {
    return M3_ERR_RANGE;
  }

  *out_width = width;
  *out_height = height;
  return M3_OK;
}

static int
m3_menu_compute_panel_bounds(const M3Menu *menu, const M3Rect *overlay,
                             M3Scalar panel_width, M3Scalar panel_height,
                             M3Rect *out_bounds, m3_u32 *out_direction) {
  M3Rect anchor;
  M3Scalar anchor_x;
  M3Scalar anchor_y;
  M3Scalar anchor_w;
  M3Scalar anchor_h;
  M3Scalar space_before;
  M3Scalar space_after;
  M3Scalar x;
  M3Scalar y;
  M3Scalar min_x;
  M3Scalar max_x;
  M3Scalar min_y;
  M3Scalar max_y;
  m3_u32 direction;
  int rc;

  if (menu == NULL || overlay == NULL || out_bounds == NULL ||
      out_direction == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_menu_validate_rect(overlay);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_menu_validate_anchor(&menu->anchor);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_menu_validate_placement(&menu->placement);
  if (rc != M3_OK) {
    return rc;
  }
  if (panel_width < 0.0f || panel_height < 0.0f) {
    return M3_ERR_RANGE;
  }

  if (overlay->width >= 0.0f && panel_width > overlay->width) {
    panel_width = overlay->width;
  }
  if (overlay->height >= 0.0f && panel_height > overlay->height) {
    panel_height = overlay->height;
  }

  if (menu->anchor.type == M3_MENU_ANCHOR_RECT) {
    anchor = menu->anchor.rect;
  } else {
    anchor.x = menu->anchor.point.x;
    anchor.y = menu->anchor.point.y;
    anchor.width = 0.0f;
    anchor.height = 0.0f;
  }

  anchor_x = anchor.x;
  anchor_y = anchor.y;
  anchor_w = anchor.width;
  anchor_h = anchor.height;

  direction = menu->placement.direction;
  if (direction == M3_MENU_DIRECTION_DOWN ||
      direction == M3_MENU_DIRECTION_UP) {
    space_before = anchor_y - overlay->y;
    space_after = overlay->y + overlay->height - (anchor_y + anchor_h);

    if (direction == M3_MENU_DIRECTION_DOWN) {
      if (panel_height <= space_after) {
        direction = M3_MENU_DIRECTION_DOWN;
      } else if (panel_height <= space_before) {
        direction = M3_MENU_DIRECTION_UP;
      } else if (space_before > space_after) {
        direction = M3_MENU_DIRECTION_UP;
      }
    } else {
      if (panel_height <= space_before) {
        direction = M3_MENU_DIRECTION_UP;
      } else if (panel_height <= space_after) {
        direction = M3_MENU_DIRECTION_DOWN;
      } else if (space_after > space_before) {
        direction = M3_MENU_DIRECTION_DOWN;
      }
    }

    if (menu->placement.align == M3_MENU_ALIGN_START) {
      x = anchor_x;
    } else if (menu->placement.align == M3_MENU_ALIGN_CENTER) {
      x = anchor_x + (anchor_w - panel_width) * 0.5f;
    } else {
      x = anchor_x + anchor_w - panel_width;
    }

    if (direction == M3_MENU_DIRECTION_DOWN) {
      y = anchor_y + anchor_h + menu->style.anchor_gap;
    } else {
      y = anchor_y - panel_height - menu->style.anchor_gap;
    }
  } else {
    space_before = anchor_x - overlay->x;
    space_after = overlay->x + overlay->width - (anchor_x + anchor_w);

    if (direction == M3_MENU_DIRECTION_RIGHT) {
      if (panel_width <= space_after) {
        direction = M3_MENU_DIRECTION_RIGHT;
      } else if (panel_width <= space_before) {
        direction = M3_MENU_DIRECTION_LEFT;
      } else if (space_before > space_after) {
        direction = M3_MENU_DIRECTION_LEFT;
      }
    } else {
      if (panel_width <= space_before) {
        direction = M3_MENU_DIRECTION_LEFT;
      } else if (panel_width <= space_after) {
        direction = M3_MENU_DIRECTION_RIGHT;
      } else if (space_after > space_before) {
        direction = M3_MENU_DIRECTION_RIGHT;
      }
    }

    if (menu->placement.align == M3_MENU_ALIGN_START) {
      y = anchor_y;
    } else if (menu->placement.align == M3_MENU_ALIGN_CENTER) {
      y = anchor_y + (anchor_h - panel_height) * 0.5f;
    } else {
      y = anchor_y + anchor_h - panel_height;
    }

    if (direction == M3_MENU_DIRECTION_RIGHT) {
      x = anchor_x + anchor_w + menu->style.anchor_gap;
    } else {
      x = anchor_x - panel_width - menu->style.anchor_gap;
    }
  }

  min_x = overlay->x;
  max_x = overlay->x + overlay->width - panel_width;
  if (max_x < min_x) {
    max_x = min_x;
  }
  min_y = overlay->y;
  max_y = overlay->y + overlay->height - panel_height;
  if (max_y < min_y) {
    max_y = min_y;
  }

  x = m3_menu_clamp(x, min_x, max_x);
  y = m3_menu_clamp(y, min_y, max_y);

  out_bounds->x = x;
  out_bounds->y = y;
  out_bounds->width = panel_width;
  out_bounds->height = panel_height;
  *out_direction = direction;
  return M3_OK;
}

static int m3_menu_hit_test(const M3Menu *menu, M3Scalar x, M3Scalar y,
                            M3Bool *out_inside, m3_usize *out_index) {
  M3Bool inside;
  M3Scalar content_y;
  M3Scalar stride;
  M3Scalar offset;
  m3_usize index;
  int rc;

  if (menu == NULL || out_inside == NULL || out_index == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_inside = M3_FALSE;
  *out_index = M3_MENU_INVALID_INDEX;

  rc = m3_menu_validate_rect(&menu->menu_bounds);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_rect_contains_point(&menu->menu_bounds, x, y, &inside);
  if (rc != M3_OK) {
    return rc;
  }
  if (inside == M3_FALSE) {
    return M3_OK;
  }

  content_y = menu->menu_bounds.y + menu->style.padding.top;
  if (y < content_y) {
    *out_inside = M3_TRUE;
    return M3_OK;
  }

  if (menu->item_count == 0u) {
    *out_inside = M3_TRUE;
    return M3_OK;
  }

  stride = menu->style.item_height + menu->style.item_spacing;
  if (stride <= 0.0f) {
    return M3_ERR_RANGE;
  }

  offset = y - content_y;
  index = (m3_usize)(offset / stride);
  if (index >= menu->item_count) {
    *out_inside = M3_TRUE;
    return M3_OK;
  }

  if (offset - (M3Scalar)index * stride >= menu->style.item_height) {
    *out_inside = M3_TRUE;
    return M3_OK;
  }

  *out_inside = M3_TRUE;
  *out_index = index;
  return M3_OK;
}

static int m3_menu_widget_measure(void *widget, M3MeasureSpec width,
                                  M3MeasureSpec height, M3Size *out_size) {
  M3Menu *menu;
  M3Scalar panel_width;
  M3Scalar panel_height;
  M3Scalar desired_width;
  M3Scalar desired_height;
  M3Bool has_label;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_menu_validate_measure_spec(width);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_menu_validate_measure_spec(height);
  if (rc != M3_OK) {
    return rc;
  }

  menu = (M3Menu *)widget;
  rc =
      m3_menu_compute_panel_size(menu, &panel_width, &panel_height, &has_label);
  if (rc != M3_OK) {
    return rc;
  }
  M3_UNUSED(has_label);

  if (width.mode == M3_MEASURE_EXACTLY) {
    desired_width = width.size;
  } else if (width.mode == M3_MEASURE_AT_MOST) {
    desired_width = panel_width;
    if (desired_width > width.size) {
      desired_width = width.size;
    }
  } else {
    desired_width = panel_width;
  }

  if (height.mode == M3_MEASURE_EXACTLY) {
    desired_height = height.size;
  } else if (height.mode == M3_MEASURE_AT_MOST) {
    desired_height = panel_height;
    if (desired_height > height.size) {
      desired_height = height.size;
    }
  } else {
    desired_height = panel_height;
  }

  out_size->width = desired_width;
  out_size->height = desired_height;
  return M3_OK;
}

static int m3_menu_widget_layout(void *widget, M3Rect bounds) {
  M3Menu *menu;
  M3Rect panel_bounds;
  M3Scalar panel_width;
  M3Scalar panel_height;
  M3Bool has_label;
  m3_u32 resolved_direction;
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_menu_validate_rect(&bounds);
  if (rc != M3_OK) {
    return rc;
  }

  menu = (M3Menu *)widget;
  rc =
      m3_menu_compute_panel_size(menu, &panel_width, &panel_height, &has_label);
  if (rc != M3_OK) {
    return rc;
  }
  M3_UNUSED(has_label);

  rc = m3_menu_compute_panel_bounds(menu, &bounds, panel_width, panel_height,
                                    &panel_bounds, &resolved_direction);
  if (rc != M3_OK) {
    return rc;
  }

  menu->overlay_bounds = bounds;
  menu->menu_bounds = panel_bounds;
  menu->resolved_direction = resolved_direction;
  return M3_OK;
}

static int m3_menu_widget_paint(void *widget, M3PaintContext *ctx) {
  M3Menu *menu;
  M3Rect clip;
  const M3Rect *shadow_clip;
  M3Scalar content_x;
  M3Scalar content_y;
  M3Scalar stride;
  M3Scalar text_offset;
  M3Bool has_label;
  m3_usize i;
  int rc;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable == NULL || ctx->gfx->vtable->draw_rect == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  menu = (M3Menu *)widget;
  rc = m3_menu_validate_style(&menu->style, M3_FALSE);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_menu_validate_rect(&menu->menu_bounds);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_menu_update_metrics(menu, &text_offset, &has_label);
  if (rc != M3_OK) {
    return rc;
  }

  shadow_clip = NULL;
  if (ctx->gfx->vtable->push_clip != NULL &&
      ctx->gfx->vtable->pop_clip != NULL) {
    clip = ctx->clip;
    shadow_clip = &clip;
  }

  if (menu->style.shadow_enabled == M3_TRUE) {
    rc = m3_shadow_paint(&menu->style.shadow, ctx->gfx, &menu->menu_bounds,
                         shadow_clip);
    if (rc != M3_OK) {
      return rc;
    }
  }

  rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &menu->menu_bounds,
                                   menu->style.background_color,
                                   menu->style.corner_radius);
  if (rc != M3_OK) {
    return rc;
  }

  if (has_label == M3_FALSE) {
    return M3_OK;
  }
  if (ctx->gfx->text_vtable == NULL ||
      ctx->gfx->text_vtable->draw_text == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  content_x = menu->menu_bounds.x + menu->style.padding.left;
  content_y = menu->menu_bounds.y + menu->style.padding.top;
  stride = menu->style.item_height + menu->style.item_spacing;

  text_offset = (menu->style.item_height - menu->font_metrics.height) * 0.5f;

  for (i = 0; i < menu->item_count; ++i) {
    const M3MenuItem *item = &menu->items[i];
    M3Scalar text_x;
    M3Scalar text_y;
    M3Color color;

    if (item->utf8_len == 0u) {
      continue;
    }

    text_x = content_x;
    text_y = content_y + (M3Scalar)i * stride + text_offset +
             menu->font_metrics.baseline;
    color = menu->style.text_style.color;
    if (item->enabled == M3_FALSE) {
      color = menu->style.disabled_text_color;
    }

    rc = ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, menu->font,
                                          item->utf8_label, item->utf8_len,
                                          text_x, text_y, color);
    if (rc != M3_OK) {
      return rc;
    }
  }

  return M3_OK;
}

static int m3_menu_widget_event(void *widget, const M3InputEvent *event,
                                M3Bool *out_handled) {
  M3Menu *menu;
  M3Bool inside;
  m3_usize index;
  int rc;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_handled = M3_FALSE;

  menu = (M3Menu *)widget;
  if (menu->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    return M3_OK;
  }
  if (menu->widget.flags & M3_WIDGET_FLAG_HIDDEN) {
    return M3_OK;
  }

  switch (event->type) {
  case M3_INPUT_POINTER_DOWN:
  case M3_INPUT_POINTER_UP:
  case M3_INPUT_POINTER_MOVE:
  case M3_INPUT_POINTER_SCROLL:
    rc = m3_menu_hit_test(menu, (M3Scalar)event->data.pointer.x,
                          (M3Scalar)event->data.pointer.y, &inside, &index);
    if (rc != M3_OK) {
      return rc;
    }

    if (event->type == M3_INPUT_POINTER_DOWN) {
      if (inside == M3_FALSE) {
        menu->pressed_index = M3_MENU_INVALID_INDEX;
        if (menu->on_action != NULL) {
          rc = menu->on_action(menu->on_action_ctx, menu,
                               M3_MENU_ACTION_DISMISS, M3_MENU_INVALID_INDEX);
          if (rc != M3_OK) {
            return rc;
          }
        }
        *out_handled = M3_TRUE;
        return M3_OK;
      }

      if (index != M3_MENU_INVALID_INDEX &&
          menu->items[index].enabled == M3_TRUE) {
        menu->pressed_index = index;
      } else {
        menu->pressed_index = M3_MENU_INVALID_INDEX;
      }
      *out_handled = M3_TRUE;
      return M3_OK;
    }

    if (event->type == M3_INPUT_POINTER_UP) {
      m3_usize pressed = menu->pressed_index;

      menu->pressed_index = M3_MENU_INVALID_INDEX;
      if (pressed != M3_MENU_INVALID_INDEX && inside == M3_TRUE &&
          index == pressed && menu->items[pressed].enabled == M3_TRUE) {
        if (menu->on_action != NULL) {
          rc = menu->on_action(menu->on_action_ctx, menu, M3_MENU_ACTION_SELECT,
                               pressed);
          if (rc != M3_OK) {
            return rc;
          }
        }
      }
      *out_handled = M3_TRUE;
      return M3_OK;
    }

    *out_handled = M3_TRUE;
    return M3_OK;
  default:
    return M3_OK;
  }
}

static int m3_menu_widget_get_semantics(void *widget,
                                        M3Semantics *out_semantics) {
  M3Menu *menu;

  if (widget == NULL || out_semantics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  menu = (M3Menu *)widget;
  out_semantics->role = M3_SEMANTIC_NONE;
  out_semantics->flags = 0u;
  if (menu->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_DISABLED;
  }
  if (menu->widget.flags & M3_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_FOCUSABLE;
  }
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return M3_OK;
}

static int m3_menu_widget_destroy(void *widget) {
  M3Menu *menu;
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  menu = (M3Menu *)widget;
  rc = M3_OK;
  if (menu->owns_font == M3_TRUE &&
      (menu->font.id != 0u || menu->font.generation != 0u)) {
    if (menu->text_backend.vtable != NULL &&
        menu->text_backend.vtable->destroy_font != NULL) {
      rc = menu->text_backend.vtable->destroy_font(menu->text_backend.ctx,
                                                   menu->font);
    } else {
      rc = M3_ERR_UNSUPPORTED;
    }
  }

  menu->font.id = 0u;
  menu->font.generation = 0u;
  menu->items = NULL;
  menu->item_count = 0u;
  menu->pressed_index = M3_MENU_INVALID_INDEX;
  menu->owns_font = M3_FALSE;
  menu->open = M3_FALSE;
  menu->on_action = NULL;
  menu->on_action_ctx = NULL;
  menu->text_backend.ctx = NULL;
  menu->text_backend.vtable = NULL;
  menu->widget.ctx = NULL;
  menu->widget.vtable = NULL;
  return rc;
}

static const M3WidgetVTable g_m3_menu_widget_vtable = {
    m3_menu_widget_measure,       m3_menu_widget_layout,
    m3_menu_widget_paint,         m3_menu_widget_event,
    m3_menu_widget_get_semantics, m3_menu_widget_destroy};

int M3_CALL m3_menu_style_init(M3MenuStyle *style) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));

  rc = m3_text_style_init(&style->text_style);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_shadow_init(&style->shadow);
  if (rc != M3_OK) {
    return rc;
  }

  style->item_height = M3_MENU_DEFAULT_ITEM_HEIGHT;
  style->item_spacing = M3_MENU_DEFAULT_ITEM_SPACING;
  style->min_width = M3_MENU_DEFAULT_MIN_WIDTH;
  style->max_width = M3_MENU_DEFAULT_MAX_WIDTH;
  style->corner_radius = M3_MENU_DEFAULT_CORNER_RADIUS;
  style->anchor_gap = M3_MENU_DEFAULT_ANCHOR_GAP;
  style->padding.left = M3_MENU_DEFAULT_PADDING_X;
  style->padding.right = M3_MENU_DEFAULT_PADDING_X;
  style->padding.top = M3_MENU_DEFAULT_PADDING_Y;
  style->padding.bottom = M3_MENU_DEFAULT_PADDING_Y;
  style->shadow_enabled = M3_TRUE;

  rc = m3_menu_color_set(&style->background_color, 1.0f, 1.0f, 1.0f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_menu_color_set(&style->disabled_text_color, 0.0f, 0.0f, 0.0f, 0.38f);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_menu_init(M3Menu *menu, const M3TextBackend *backend,
                         const M3MenuStyle *style, const M3MenuItem *items,
                         m3_usize item_count) {
  int rc;

  if (menu == NULL || backend == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_menu_validate_backend(backend);
  if (rc != M3_OK) {
    return rc;
  }
  if (backend->vtable->create_font == NULL ||
      backend->vtable->destroy_font == NULL ||
      backend->vtable->measure_text == NULL ||
      backend->vtable->draw_text == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  rc = m3_menu_validate_style(style, M3_TRUE);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_menu_validate_items(items, item_count);
  if (rc != M3_OK) {
    return rc;
  }

  memset(menu, 0, sizeof(*menu));
  menu->text_backend = *backend;
  menu->style = *style;
  menu->items = items;
  menu->item_count = item_count;
  menu->anchor.type = M3_MENU_ANCHOR_POINT;
  menu->anchor.point.x = 0.0f;
  menu->anchor.point.y = 0.0f;
  menu->placement.direction = M3_MENU_DIRECTION_DOWN;
  menu->placement.align = M3_MENU_ALIGN_START;
  menu->resolved_direction = menu->placement.direction;
  menu->overlay_bounds.x = 0.0f;
  menu->overlay_bounds.y = 0.0f;
  menu->overlay_bounds.width = 0.0f;
  menu->overlay_bounds.height = 0.0f;
  menu->menu_bounds = menu->overlay_bounds;
  menu->pressed_index = M3_MENU_INVALID_INDEX;
  menu->owns_font = M3_TRUE;
  menu->open = M3_TRUE;

  rc = m3_text_font_create(backend, &style->text_style, &menu->font);
  if (rc != M3_OK) {
    return rc;
  }

  menu->widget.ctx = menu;
  menu->widget.vtable = &g_m3_menu_widget_vtable;
  menu->widget.handle.id = 0u;
  menu->widget.handle.generation = 0u;
  menu->widget.flags = M3_WIDGET_FLAG_FOCUSABLE;
  return M3_OK;
}

int M3_CALL m3_menu_set_items(M3Menu *menu, const M3MenuItem *items,
                              m3_usize item_count) {
  int rc;

  if (menu == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_menu_validate_items(items, item_count);
  if (rc != M3_OK) {
    return rc;
  }

  menu->items = items;
  menu->item_count = item_count;
  menu->pressed_index = M3_MENU_INVALID_INDEX;
  return M3_OK;
}

int M3_CALL m3_menu_set_style(M3Menu *menu, const M3MenuStyle *style) {
  M3Handle new_font;
  int rc;

  if (menu == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_menu_validate_style(style, M3_TRUE);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_text_font_create(&menu->text_backend, &style->text_style, &new_font);
  if (rc != M3_OK) {
    return rc;
  }

  if (menu->owns_font == M3_TRUE) {
    rc = m3_text_font_destroy(&menu->text_backend, menu->font);
    if (rc != M3_OK) {
      m3_text_font_destroy(&menu->text_backend, new_font);
      return rc;
    }
  }

  menu->style = *style;
  menu->font = new_font;
  menu->owns_font = M3_TRUE;
  return M3_OK;
}

int M3_CALL m3_menu_set_anchor_rect(M3Menu *menu, const M3Rect *rect) {
  int rc;

  if (menu == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_menu_validate_rect(rect);
  if (rc != M3_OK) {
    return rc;
  }

  menu->anchor.type = M3_MENU_ANCHOR_RECT;
  menu->anchor.rect = *rect;
  return M3_OK;
}

int M3_CALL m3_menu_set_anchor_point(M3Menu *menu, M3Scalar x, M3Scalar y) {
  if (menu == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  menu->anchor.type = M3_MENU_ANCHOR_POINT;
  menu->anchor.point.x = x;
  menu->anchor.point.y = y;
  return M3_OK;
}

int M3_CALL m3_menu_set_placement(M3Menu *menu,
                                  const M3MenuPlacement *placement) {
  int rc;

  if (menu == NULL || placement == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_menu_validate_placement(placement);
  if (rc != M3_OK) {
    return rc;
  }

  menu->placement = *placement;
  return M3_OK;
}

int M3_CALL m3_menu_set_open(M3Menu *menu, M3Bool open) {
  if (menu == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (open != M3_FALSE && open != M3_TRUE) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  menu->open = open;
  if (open == M3_TRUE) {
    menu->widget.flags &= (m3_u32)~M3_WIDGET_FLAG_HIDDEN;
  } else {
    menu->widget.flags |= M3_WIDGET_FLAG_HIDDEN;
  }
  return M3_OK;
}

int M3_CALL m3_menu_get_open(const M3Menu *menu, M3Bool *out_open) {
  if (menu == NULL || out_open == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_open = menu->open;
  return M3_OK;
}

int M3_CALL m3_menu_get_bounds(const M3Menu *menu, M3Rect *out_bounds) {
  if (menu == NULL || out_bounds == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_bounds = menu->menu_bounds;
  return M3_OK;
}

int M3_CALL m3_menu_get_overlay_bounds(const M3Menu *menu, M3Rect *out_bounds) {
  if (menu == NULL || out_bounds == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_bounds = menu->overlay_bounds;
  return M3_OK;
}

int M3_CALL m3_menu_get_resolved_direction(const M3Menu *menu,
                                           m3_u32 *out_direction) {
  if (menu == NULL || out_direction == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_direction = menu->resolved_direction;
  return M3_OK;
}

int M3_CALL m3_menu_get_item_bounds(const M3Menu *menu, m3_usize index,
                                    M3Rect *out_bounds) {
  M3Scalar content_x;
  M3Scalar content_y;
  M3Scalar stride;

  if (menu == NULL || out_bounds == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (index >= menu->item_count) {
    return M3_ERR_RANGE;
  }

  content_x = menu->menu_bounds.x + menu->style.padding.left;
  content_y = menu->menu_bounds.y + menu->style.padding.top;
  stride = menu->style.item_height + menu->style.item_spacing;
  if (stride <= 0.0f) {
    return M3_ERR_RANGE;
  }

  out_bounds->x = content_x;
  out_bounds->y = content_y + (M3Scalar)index * stride;
  out_bounds->width = menu->menu_bounds.width - menu->style.padding.left -
                      menu->style.padding.right;
  out_bounds->height = menu->style.item_height;
  return M3_OK;
}

int M3_CALL m3_menu_set_on_action(M3Menu *menu, M3MenuOnAction on_action,
                                  void *ctx) {
  if (menu == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  menu->on_action = on_action;
  menu->on_action_ctx = ctx;
  return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_menu_test_validate_color(const M3Color *color) {
  return m3_menu_validate_color(color);
}

int M3_CALL m3_menu_test_color_set(M3Color *color, M3Scalar r, M3Scalar g,
                                   M3Scalar b, M3Scalar a) {
  return m3_menu_color_set(color, r, g, b, a);
}

int M3_CALL m3_menu_test_validate_edges(const M3LayoutEdges *edges) {
  return m3_menu_validate_edges(edges);
}

int M3_CALL m3_menu_test_validate_text_style(const M3TextStyle *style,
                                             M3Bool require_family) {
  return m3_menu_validate_text_style(style, require_family);
}

int M3_CALL m3_menu_test_validate_measure_spec(M3MeasureSpec spec) {
  return m3_menu_validate_measure_spec(spec);
}

int M3_CALL m3_menu_test_validate_rect(const M3Rect *rect) {
  return m3_menu_validate_rect(rect);
}

int M3_CALL m3_menu_test_validate_anchor(const M3MenuAnchor *anchor) {
  return m3_menu_validate_anchor(anchor);
}

int M3_CALL m3_menu_test_validate_placement(const M3MenuPlacement *placement) {
  return m3_menu_validate_placement(placement);
}

int M3_CALL m3_menu_test_validate_items(const M3MenuItem *items,
                                        m3_usize count) {
  return m3_menu_validate_items(items, count);
}
#endif
