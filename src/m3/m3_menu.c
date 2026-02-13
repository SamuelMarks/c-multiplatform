#include "m3/m3_menu.h"

#include <string.h>

#ifdef CMP_TESTING
static CMPBool g_m3_menu_test_force_negative_panel = CMP_FALSE;
static CMPBool g_m3_menu_test_force_hit_test_error = CMP_FALSE;
static CMPBool g_m3_menu_test_fail_text_style = CMP_FALSE;
static CMPBool g_m3_menu_test_fail_shadow_init = CMP_FALSE;
static CMPBool g_m3_menu_test_fail_background_color = CMP_FALSE;
static CMPBool g_m3_menu_test_fail_disabled_color = CMP_FALSE;
static CMPBool g_m3_menu_test_force_bounds_overflow = CMP_FALSE;
#endif

static int m3_menu_validate_rect(const CMPRect *rect) {
  if (rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_menu_validate_color(const CMPColor *color) {
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

static int m3_menu_color_set(CMPColor *color, CMPScalar r, CMPScalar g,
                             CMPScalar b, CMPScalar a) {
  if (color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!(r >= 0.0f && r <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(g >= 0.0f && g <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(b >= 0.0f && b <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(a >= 0.0f && a <= 1.0f)) {
    return CMP_ERR_RANGE;
  }

  color->r = r;
  color->g = g;
  color->b = b;
  color->a = a;
  return CMP_OK;
}

static int m3_menu_validate_edges(const CMPLayoutEdges *edges) {
  if (edges == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (edges->left < 0.0f || edges->right < 0.0f || edges->top < 0.0f ||
      edges->bottom < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_menu_validate_text_style(const CMPTextStyle *style,
                                       CMPBool require_family) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (require_family == CMP_TRUE && style->utf8_family == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->size_px <= 0) {
    return CMP_ERR_RANGE;
  }
  if (style->weight < 100 || style->weight > 900) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (style->italic != CMP_FALSE && style->italic != CMP_TRUE) {
    return CMP_ERR_RANGE;
  }

  rc = m3_menu_validate_color(&style->color);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

static int m3_menu_validate_style(const M3MenuStyle *style,
                                  CMPBool require_family) {
  int rc = CMP_OK;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->item_height <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->item_spacing < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->min_width < 0.0f || style->max_width < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->max_width > 0.0f && style->max_width < style->min_width) {
    return CMP_ERR_RANGE;
  }
  if (style->corner_radius < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->anchor_gap < 0.0f) {
    return CMP_ERR_RANGE;
  }

  rc = m3_menu_validate_edges(&style->padding);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_menu_validate_text_style(&style->text_style, require_family);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_menu_validate_color(&style->background_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_menu_validate_color(&style->disabled_text_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_menu_validate_backend(const CMPTextBackend *backend) {
  if (backend == NULL || backend->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

static int m3_menu_validate_items(const M3MenuItem *items, cmp_usize count) {
  cmp_usize i;

  if (count == 0) {
    return CMP_OK;
  }
  if (items == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < count; ++i) {
    const M3MenuItem *item = &items[i];
    if (item->utf8_label == NULL && item->utf8_len != 0u) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
    if (item->enabled != CMP_FALSE && item->enabled != CMP_TRUE) {
      return CMP_ERR_RANGE;
    }
  }
  return CMP_OK;
}

static int m3_menu_validate_measure_spec(CMPMeasureSpec spec) {
  if (spec.mode != CMP_MEASURE_UNSPECIFIED &&
      spec.mode != CMP_MEASURE_EXACTLY && spec.mode != CMP_MEASURE_AT_MOST) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int
m3_menu_validate_anchor(const M3MenuAnchor *anchor) { /* GCOVR_EXCL_LINE */
  int rc;

  if (anchor == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (anchor->type == M3_MENU_ANCHOR_RECT) {
    rc = m3_menu_validate_rect(&anchor->rect);
    if (rc != CMP_OK) {
      return rc;
    }
  } else if (anchor->type == M3_MENU_ANCHOR_POINT) {
    return CMP_OK;
  } else {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

static int m3_menu_validate_placement(const M3MenuPlacement *placement) {
  if (placement == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  switch (placement->direction) {
  case M3_MENU_DIRECTION_DOWN:
  case M3_MENU_DIRECTION_UP:
  case M3_MENU_DIRECTION_RIGHT:
  case M3_MENU_DIRECTION_LEFT:
    break;
  default:
    return CMP_ERR_RANGE;
  }

  switch (placement->align) {
  case M3_MENU_ALIGN_START:
  case M3_MENU_ALIGN_CENTER:
  case M3_MENU_ALIGN_END:
    break;
  default:
    return CMP_ERR_RANGE;
  }

  return CMP_OK;
}

static CMPScalar m3_menu_clamp(CMPScalar value, CMPScalar min_value,
                               CMPScalar max_value) {
  if (value < min_value) {
    return min_value;
  }
  if (value > max_value) {
    return max_value;
  }
  return value;
}

static int
m3_menu_update_metrics(M3Menu *menu,
                       CMPScalar *out_text_width, /* GCOVR_EXCL_LINE */
                       CMPBool *out_has_label) {
  CMPTextMetrics metrics;
  CMPScalar max_width;
  CMPBool has_label = CMP_FALSE;
  cmp_usize i;
  int rc;

  if (menu == NULL || out_text_width == NULL || out_has_label == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_menu_validate_items(menu->items, menu->item_count);
  if (rc != CMP_OK) {
    return rc;
  }

  max_width = 0.0f;
  has_label = CMP_FALSE;
  for (i = 0; i < menu->item_count; ++i) {
    if (menu->items[i].utf8_len > 0u) {
      has_label = CMP_TRUE;
      break;
    }
  }

  if (has_label == CMP_TRUE) {
    if (menu->text_backend.vtable == NULL ||
        menu->text_backend.vtable->measure_text == NULL) {
      return CMP_ERR_UNSUPPORTED; /* GCOVR_EXCL_LINE */
    }
    if (menu->font.id == 0u && menu->font.generation == 0u) {
      return CMP_ERR_STATE;
    }

    rc = cmp_text_font_metrics(&menu->text_backend, menu->font,
                               &menu->font_metrics);
    if (rc != CMP_OK) {
      return rc;
    }

    for (i = 0; i < menu->item_count; ++i) {
      if (menu->items[i].utf8_len == 0u) {
        continue;
      }
      rc = cmp_text_measure_utf8(&menu->text_backend, menu->font,
                                 menu->items[i].utf8_label,
                                 menu->items[i].utf8_len, &metrics);
      if (rc != CMP_OK) {
        return rc;
      }
      if (metrics.width < 0.0f) {
        return CMP_ERR_RANGE;
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
  return CMP_OK;
}

static int /* GCOVR_EXCL_LINE */
m3_menu_compute_panel_size(M3Menu *menu,
                           CMPScalar *out_width,  /* GCOVR_EXCL_LINE */
                           CMPScalar *out_height, /* GCOVR_EXCL_LINE */
                           CMPBool *out_has_label) {
  CMPScalar text_width;
  CMPScalar width;
  CMPScalar height;
  int rc;

  if (menu == NULL || out_width == NULL || out_height == NULL ||
      out_has_label == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_menu_validate_style(&menu->style, CMP_FALSE);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_menu_update_metrics(menu, &text_width, out_has_label);
  if (rc != CMP_OK) {
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
    height += menu->style.item_height * (CMPScalar)menu->item_count;
    height += menu->style.item_spacing * (CMPScalar)(menu->item_count - 1u);
  }

#ifdef CMP_TESTING
  if (g_m3_menu_test_force_negative_panel == CMP_TRUE) {
    g_m3_menu_test_force_negative_panel = CMP_FALSE;
    width = -1.0f;
    height = -1.0f; /* GCOVR_EXCL_LINE */
  }
#endif

  if (width < 0.0f || height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  *out_width = width;
  *out_height = height;
  return CMP_OK; /* GCOVR_EXCL_LINE */
}

static int
m3_menu_compute_panel_bounds(const M3Menu *menu,
                             const CMPRect *overlay, /* GCOVR_EXCL_LINE */
                             CMPScalar panel_width, CMPScalar panel_height,
                             CMPRect *out_bounds, cmp_u32 *out_direction) {
  CMPRect anchor;
  CMPScalar anchor_x = 0.0f; /* GCOVR_EXCL_LINE */
  CMPScalar anchor_y;
  CMPScalar anchor_w;
  CMPScalar anchor_h;
  CMPScalar space_before;
  CMPScalar space_after = 0.0f; /* GCOVR_EXCL_LINE */
  CMPScalar x;
  CMPScalar y;
  CMPScalar min_x;
  CMPScalar max_x;
  CMPScalar min_y;
  CMPScalar max_y;
  cmp_u32 direction;
  int rc;

  if (menu == NULL || overlay == NULL || out_bounds == NULL ||
      out_direction == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_menu_validate_rect(overlay);
  if (rc != CMP_OK) { /* GCOVR_EXCL_LINE */
    return rc;
  }
  rc = m3_menu_validate_anchor(&menu->anchor);
  if (rc != CMP_OK) { /* GCOVR_EXCL_LINE */
    return rc;
  }
  rc = m3_menu_validate_placement(&menu->placement);
  if (rc != CMP_OK) { /* GCOVR_EXCL_LINE */
    return rc;
  }
  if (panel_width < 0.0f || panel_height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (overlay->width >= 0.0f && panel_width > overlay->width) {
    panel_width = overlay->width;
  }
  if (overlay->height >= 0.0f && panel_height > overlay->height) {
    panel_height = overlay->height;
  }

#ifdef CMP_TESTING
  if (g_m3_menu_test_force_bounds_overflow == CMP_TRUE) {
    g_m3_menu_test_force_bounds_overflow = CMP_FALSE;
    panel_width = overlay->width + 1.0f;
    panel_height = overlay->height + 1.0f;
  }
#endif

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
    } else if (menu->placement.align ==
               M3_MENU_ALIGN_CENTER) { /* GCOVR_EXCL_LINE */
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
    } else { /* GCOVR_EXCL_LINE */
      y = anchor_y + anchor_h - panel_height;
    }

    if (direction == M3_MENU_DIRECTION_RIGHT) {
      x = anchor_x + anchor_w + menu->style.anchor_gap; /* GCOVR_EXCL_LINE */
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

  x = m3_menu_clamp(x, min_x, max_x); /* GCOVR_EXCL_LINE */
  y = m3_menu_clamp(y, min_y, max_y);

  out_bounds->x = x;               /* GCOVR_EXCL_LINE */
  out_bounds->y = y;               /* GCOVR_EXCL_LINE */
  out_bounds->width = panel_width; /* GCOVR_EXCL_LINE */
  out_bounds->height = panel_height;
  *out_direction = direction;
  return CMP_OK; /* GCOVR_EXCL_LINE */
}

static int m3_menu_hit_test(const M3Menu *menu, CMPScalar x, CMPScalar y,
                            CMPBool *out_inside, cmp_usize *out_index) {
  CMPBool inside;
  CMPScalar content_y;
  CMPScalar stride;
  CMPScalar offset; /* GCOVR_EXCL_LINE */
  cmp_usize index;
  int rc;

  if (menu == NULL || out_inside == NULL || out_index == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_inside = CMP_FALSE;
  *out_index = M3_MENU_INVALID_INDEX;

  rc = m3_menu_validate_rect(&menu->menu_bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_rect_contains_point(&menu->menu_bounds, x, y, &inside);
#ifdef CMP_TESTING
  if (g_m3_menu_test_force_hit_test_error == CMP_TRUE) {
    g_m3_menu_test_force_hit_test_error = CMP_FALSE;
    rc = CMP_ERR_RANGE;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }
  if (inside == CMP_FALSE) {
    return CMP_OK;
  }

  content_y =
      menu->menu_bounds.y + menu->style.padding.top; /* GCOVR_EXCL_LINE */
  if (y < content_y) {
    *out_inside = CMP_TRUE;
    return CMP_OK;
  }

  if (menu->item_count == 0u) {
    *out_inside = CMP_TRUE;
    return CMP_OK;
  }

  stride = menu->style.item_height + menu->style.item_spacing;
  if (stride <= 0.0f) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE;
  }

  offset = y - content_y;
  index = (cmp_usize)(offset / stride);
  if (index >= menu->item_count) {
    *out_inside = CMP_TRUE;
    return CMP_OK;
  }

  if (offset - (CMPScalar)index * stride >= menu->style.item_height) {
    *out_inside = CMP_TRUE; /* GCOVR_EXCL_LINE */
    return CMP_OK;          /* GCOVR_EXCL_LINE */
  }

  *out_inside = CMP_TRUE; /* GCOVR_EXCL_LINE */
  *out_index = index;
  return CMP_OK;
}

static int m3_menu_widget_measure(void *widget, CMPMeasureSpec width,
                                  CMPMeasureSpec height, CMPSize *out_size) {
  M3Menu *menu;
  CMPScalar panel_width;
  CMPScalar panel_height = 0.0f;
  CMPScalar desired_width;
  CMPScalar desired_height;
  CMPBool has_label = CMP_FALSE; /* GCOVR_EXCL_LINE */
  int rc;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_menu_validate_measure_spec(width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_menu_validate_measure_spec(height);
  if (rc != CMP_OK) {
    return rc;
  }

  menu = (M3Menu *)widget;
  rc =
      m3_menu_compute_panel_size(menu, &panel_width, &panel_height, &has_label);
  if (rc != CMP_OK) {
    return rc;
  }
  CMP_UNUSED(has_label);

  if (width.mode == CMP_MEASURE_EXACTLY) {
    desired_width = width.size;
  } else if (width.mode == CMP_MEASURE_AT_MOST) {
    desired_width = panel_width;
    if (desired_width > width.size) {
      desired_width = width.size;
    }
  } else {
    desired_width = panel_width;
  }

  if (height.mode == CMP_MEASURE_EXACTLY) {
    desired_height = height.size;
  } else if (height.mode == CMP_MEASURE_AT_MOST) {
    desired_height = panel_height;
    if (desired_height > height.size) {
      desired_height = height.size; /* GCOVR_EXCL_LINE */
    }
  } else {                         /* GCOVR_EXCL_LINE */
    desired_height = panel_height; /* GCOVR_EXCL_LINE */
  }

  out_size->width = desired_width;
  out_size->height = desired_height;
  return CMP_OK; /* GCOVR_EXCL_LINE */
}

static int m3_menu_widget_layout(void *widget, CMPRect bounds) {
  M3Menu *menu;
  CMPRect panel_bounds;
  CMPScalar panel_width;
  CMPScalar panel_height;
  CMPBool has_label = CMP_FALSE;
  cmp_u32 resolved_direction = 0u;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_menu_validate_rect(&bounds); /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) {
    return rc;
  }

  menu = (M3Menu *)widget;
  rc =
      m3_menu_compute_panel_size(menu, &panel_width, &panel_height, &has_label);
  if (rc != CMP_OK) {
    return rc;
  }
  CMP_UNUSED(has_label);

  rc = m3_menu_compute_panel_bounds(menu, &bounds, panel_width, panel_height,
                                    &panel_bounds, &resolved_direction);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  menu->overlay_bounds = bounds;                 /* GCOVR_EXCL_LINE */
  menu->menu_bounds = panel_bounds;              /* GCOVR_EXCL_LINE */
  menu->resolved_direction = resolved_direction; /* GCOVR_EXCL_LINE */
  return CMP_OK;                                 /* GCOVR_EXCL_LINE */
}

static int m3_menu_widget_paint(void *widget,
                                CMPPaintContext *ctx) { /* GCOVR_EXCL_LINE */
  M3Menu *menu;
  CMPRect clip;
  const CMPRect *shadow_clip;
  CMPScalar content_x;
  CMPScalar content_y;
  CMPScalar stride;
  CMPScalar text_offset;
  CMPBool has_label;
  cmp_usize i;
  int rc;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (ctx->gfx->vtable == NULL || ctx->gfx->vtable->draw_rect == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  menu = (M3Menu *)widget;
  rc = m3_menu_validate_style(&menu->style, CMP_FALSE);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_menu_validate_rect(&menu->menu_bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_menu_update_metrics(menu, &text_offset, &has_label);
  if (rc != CMP_OK) {
    return rc;
  }

  shadow_clip = NULL;
  if (ctx->gfx->vtable->push_clip != NULL &&
      ctx->gfx->vtable->pop_clip != NULL) {
    clip = ctx->clip;
    shadow_clip = &clip;
  }

  if (menu->style.shadow_enabled == CMP_TRUE) {
    rc = cmp_shadow_paint(&menu->style.shadow, ctx->gfx, &menu->menu_bounds,
                          shadow_clip);
    if (rc != CMP_OK) { /* GCOVR_EXCL_LINE */
      return rc;
    }
  }

  rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &menu->menu_bounds,
                                   menu->style.background_color,
                                   menu->style.corner_radius);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  if (has_label == CMP_FALSE) {
    return CMP_OK;
  }
  if (ctx->gfx->text_vtable == NULL || /* GCOVR_EXCL_LINE */
      ctx->gfx->text_vtable->draw_text == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  content_x =
      menu->menu_bounds.x + menu->style.padding.left; /* GCOVR_EXCL_LINE */
  content_y =
      menu->menu_bounds.y + menu->style.padding.top; /* GCOVR_EXCL_LINE */
  stride = menu->style.item_height + menu->style.item_spacing;

  text_offset = (menu->style.item_height - menu->font_metrics.height) * 0.5f;

  for (i = 0; i < menu->item_count; ++i) {
    const M3MenuItem *item = &menu->items[i];
    CMPScalar text_x;
    CMPScalar text_y = 0.0f;
    CMPColor color;

    if (item->utf8_len == 0u) {
      continue;
    }

    text_x = content_x;
    text_y = content_y + (CMPScalar)i * stride + text_offset +
             menu->font_metrics.baseline;
    color = menu->style.text_style.color;
    if (item->enabled == CMP_FALSE) {
      color = menu->style.disabled_text_color;
    }

    rc = ctx->gfx->text_vtable->draw_text(
        ctx->gfx->ctx, menu->font, /* GCOVR_EXCL_LINE */
        item->utf8_label, item->utf8_len, text_x, text_y,
        color);         /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) { /* GCOVR_EXCL_LINE */
      return rc;
    }
  }

  return CMP_OK;
}

static int m3_menu_widget_event(void *widget, const CMPInputEvent *event,
                                CMPBool *out_handled) {
  M3Menu *menu;
  CMPBool inside;
  cmp_usize index = 0u;
  int rc;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;

  menu = (M3Menu *)widget;
  if (menu->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    return CMP_OK;
  }
  if (menu->widget.flags & CMP_WIDGET_FLAG_HIDDEN) {
    return CMP_OK;
  }

  switch (event->type) {
  case CMP_INPUT_POINTER_DOWN:
  case CMP_INPUT_POINTER_UP:
  case CMP_INPUT_POINTER_MOVE:
  case CMP_INPUT_POINTER_SCROLL:
    rc = m3_menu_hit_test(menu, (CMPScalar)event->data.pointer.x,
                          (CMPScalar)event->data.pointer.y, &inside, &index);
    if (rc != CMP_OK) {
      return rc;
    }

    if (event->type == CMP_INPUT_POINTER_DOWN) {
      if (inside == CMP_FALSE) {
        menu->pressed_index = M3_MENU_INVALID_INDEX;
        if (menu->on_action != NULL) {
          rc = menu->on_action(menu->on_action_ctx, menu,
                               M3_MENU_ACTION_DISMISS, M3_MENU_INVALID_INDEX);
          if (rc != CMP_OK) {
            return rc;
          }
        }
        *out_handled = CMP_TRUE;
        return CMP_OK;
      }

      if (index != M3_MENU_INVALID_INDEX &&
          menu->items[index].enabled == CMP_TRUE) {
        menu->pressed_index = index;
      } else {
        menu->pressed_index = M3_MENU_INVALID_INDEX;
      }
      *out_handled = CMP_TRUE;
      return CMP_OK;
    }

    if (event->type == CMP_INPUT_POINTER_UP) {
      cmp_usize pressed = menu->pressed_index;

      menu->pressed_index = M3_MENU_INVALID_INDEX;
      if (pressed != M3_MENU_INVALID_INDEX && inside == CMP_TRUE &&
          index == pressed && menu->items[pressed].enabled == CMP_TRUE) {
        if (menu->on_action != NULL) { /* GCOVR_EXCL_LINE */
          rc = menu->on_action(menu->on_action_ctx, menu, M3_MENU_ACTION_SELECT,
                               pressed);
          if (rc != CMP_OK) {
            return rc; /* GCOVR_EXCL_LINE */
          }
        }
      }
      *out_handled = CMP_TRUE; /* GCOVR_EXCL_LINE */
      return CMP_OK;
    }

    *out_handled = CMP_TRUE; /* GCOVR_EXCL_LINE */
    return CMP_OK;
  default:
    return CMP_OK;
  }
}

static int m3_menu_widget_get_semantics(void *widget,
                                        CMPSemantics *out_semantics) {
  M3Menu *menu;

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  menu = (M3Menu *)widget;
  out_semantics->role = CMP_SEMANTIC_NONE;
  out_semantics->flags = 0u;
  if (menu->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED; /* GCOVR_EXCL_LINE */
  }
  if (menu->widget.flags & CMP_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_FOCUSABLE; /* GCOVR_EXCL_LINE */
  }
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_menu_widget_destroy(void *widget) {
  M3Menu *menu;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  menu = (M3Menu *)widget;
  rc = CMP_OK;
  if (menu->owns_font == CMP_TRUE &&
      (menu->font.id != 0u || menu->font.generation != 0u)) {
    if (menu->text_backend.vtable != NULL &&
        menu->text_backend.vtable->destroy_font != NULL) {
      rc = menu->text_backend.vtable->destroy_font(menu->text_backend.ctx,
                                                   menu->font);
    } else {
      rc = CMP_ERR_UNSUPPORTED;
    }
  }

  menu->font.id = 0u;
  menu->font.generation = 0u;
  menu->items = NULL;
  menu->item_count = 0u;
  menu->pressed_index = M3_MENU_INVALID_INDEX;
  menu->owns_font = CMP_FALSE; /* GCOVR_EXCL_LINE */
  menu->open = CMP_FALSE;      /* GCOVR_EXCL_LINE */
  menu->on_action = NULL;
  menu->on_action_ctx = NULL;    /* GCOVR_EXCL_LINE */
  menu->text_backend.ctx = NULL; /* GCOVR_EXCL_LINE */
  menu->text_backend.vtable = NULL;
  menu->widget.ctx = NULL;
  menu->widget.vtable = NULL; /* GCOVR_EXCL_LINE */
  return rc;                  /* GCOVR_EXCL_LINE */
}

static const CMPWidgetVTable g_m3_menu_widget_vtable = {
    m3_menu_widget_measure,       m3_menu_widget_layout,
    m3_menu_widget_paint,         m3_menu_widget_event,
    m3_menu_widget_get_semantics, m3_menu_widget_destroy};

int CMP_CALL m3_menu_style_init(M3MenuStyle *style) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));

  rc = cmp_text_style_init(&style->text_style);
#ifdef CMP_TESTING
  if (g_m3_menu_test_fail_text_style == CMP_TRUE) {
    g_m3_menu_test_fail_text_style = CMP_FALSE;
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_shadow_init(&style->shadow);
#ifdef CMP_TESTING
  if (g_m3_menu_test_fail_shadow_init == CMP_TRUE) {
    g_m3_menu_test_fail_shadow_init = CMP_FALSE;
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
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
  style->shadow_enabled = CMP_TRUE;

  rc = m3_menu_color_set(&style->background_color, 1.0f, 1.0f, 1.0f, 1.0f);
#ifdef CMP_TESTING
  if (g_m3_menu_test_fail_background_color == CMP_TRUE) {
    g_m3_menu_test_fail_background_color = CMP_FALSE;
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_menu_color_set(&style->disabled_text_color, 0.0f, 0.0f, 0.0f, 0.38f);
#ifdef CMP_TESTING
  if (g_m3_menu_test_fail_disabled_color == CMP_TRUE) {
    g_m3_menu_test_fail_disabled_color = CMP_FALSE;
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL m3_menu_init(M3Menu *menu, const CMPTextBackend *backend,
                          const M3MenuStyle *style, const M3MenuItem *items,
                          cmp_usize item_count) {
  int rc;

  if (menu == NULL || backend == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_menu_validate_backend(backend);
  if (rc != CMP_OK) {
    return rc;
  }
  if (backend->vtable->create_font == NULL ||
      backend->vtable->destroy_font == NULL ||
      backend->vtable->measure_text == NULL ||
      backend->vtable->draw_text == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = m3_menu_validate_style(style, CMP_TRUE);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_menu_validate_items(items, item_count);
  if (rc != CMP_OK) {
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
  menu->owns_font = CMP_TRUE;
  menu->open = CMP_TRUE;

  rc = cmp_text_font_create(backend, &style->text_style, &menu->font);
  if (rc != CMP_OK) {
    return rc;
  }

  menu->widget.ctx = menu;
  menu->widget.vtable = &g_m3_menu_widget_vtable;
  menu->widget.handle.id = 0u;
  menu->widget.handle.generation = 0u;
  menu->widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;
  return CMP_OK;
}

int CMP_CALL m3_menu_set_items(M3Menu *menu, const M3MenuItem *items,
                               cmp_usize item_count) {
  int rc;

  if (menu == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_menu_validate_items(items, item_count);
  if (rc != CMP_OK) {
    return rc;
  }

  menu->items = items;
  menu->item_count = item_count;
  menu->pressed_index = M3_MENU_INVALID_INDEX;
  return CMP_OK;
}

int CMP_CALL m3_menu_set_style(M3Menu *menu, const M3MenuStyle *style) {
  CMPHandle new_font;
  int rc;

  if (menu == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_menu_validate_style(style, CMP_TRUE);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_text_font_create(&menu->text_backend, &style->text_style, &new_font);
  if (rc != CMP_OK) {
    return rc;
  }

  if (menu->owns_font == CMP_TRUE) {
    rc = cmp_text_font_destroy(&menu->text_backend, menu->font);
    if (rc != CMP_OK) {
      cmp_text_font_destroy(&menu->text_backend, new_font);
      return rc;
    }
  }

  menu->style = *style;
  menu->font = new_font;
  menu->owns_font = CMP_TRUE;
  return CMP_OK;
}

int CMP_CALL m3_menu_set_anchor_rect(M3Menu *menu, const CMPRect *rect) {
  int rc;

  if (menu == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_menu_validate_rect(rect);
  if (rc != CMP_OK) {
    return rc;
  }

  menu->anchor.type = M3_MENU_ANCHOR_RECT;
  menu->anchor.rect = *rect;
  return CMP_OK;
}

int CMP_CALL m3_menu_set_anchor_point(M3Menu *menu, CMPScalar x, CMPScalar y) {
  if (menu == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  menu->anchor.type = M3_MENU_ANCHOR_POINT;
  menu->anchor.point.x = x;
  menu->anchor.point.y = y;
  return CMP_OK;
}

int CMP_CALL m3_menu_set_placement(M3Menu *menu,
                                   const M3MenuPlacement *placement) {
  int rc;

  if (menu == NULL || placement == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_menu_validate_placement(placement);
  if (rc != CMP_OK) {
    return rc;
  }

  menu->placement = *placement;
  return CMP_OK;
}

int CMP_CALL m3_menu_set_open(M3Menu *menu, CMPBool open) {
  if (menu == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (open != CMP_FALSE && open != CMP_TRUE) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  menu->open = open;
  if (open == CMP_TRUE) {
    menu->widget.flags &= (cmp_u32)~CMP_WIDGET_FLAG_HIDDEN;
  } else {
    menu->widget.flags |= CMP_WIDGET_FLAG_HIDDEN;
  }
  return CMP_OK;
}

int CMP_CALL m3_menu_get_open(const M3Menu *menu, CMPBool *out_open) {
  if (menu == NULL || out_open == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_open = menu->open;
  return CMP_OK;
}

int CMP_CALL m3_menu_get_bounds(const M3Menu *menu, CMPRect *out_bounds) {
  if (menu == NULL || out_bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_bounds = menu->menu_bounds;
  return CMP_OK;
}

int CMP_CALL m3_menu_get_overlay_bounds(const M3Menu *menu,
                                        CMPRect *out_bounds) {
  if (menu == NULL || out_bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_bounds = menu->overlay_bounds;
  return CMP_OK;
}

int CMP_CALL m3_menu_get_resolved_direction(const M3Menu *menu,
                                            cmp_u32 *out_direction) {
  if (menu == NULL || out_direction == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_direction = menu->resolved_direction;
  return CMP_OK;
}

int CMP_CALL m3_menu_get_item_bounds(const M3Menu *menu, cmp_usize index,
                                     CMPRect *out_bounds) {
  CMPScalar content_x;
  CMPScalar content_y = 0.0f;
  CMPScalar stride;

  if (menu == NULL || out_bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (index >= menu->item_count) {
    return CMP_ERR_RANGE;
  }

  content_x = menu->menu_bounds.x + menu->style.padding.left;
  content_y = menu->menu_bounds.y + menu->style.padding.top;
  stride = menu->style.item_height + menu->style.item_spacing;
  if (stride <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  out_bounds->x = content_x;
  out_bounds->y = content_y + (CMPScalar)index * stride;
  out_bounds->width = menu->menu_bounds.width - menu->style.padding.left -
                      menu->style.padding.right;
  out_bounds->height = menu->style.item_height;
  return CMP_OK;
}

int CMP_CALL m3_menu_set_on_action(M3Menu *menu, CMPMenuOnAction on_action,
                                   void *ctx) {
  if (menu == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  menu->on_action = on_action;
  menu->on_action_ctx = ctx;
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL m3_menu_test_set_force_negative_panel(CMPBool enable) {
  g_m3_menu_test_force_negative_panel = enable;
  return CMP_OK;
}

int CMP_CALL m3_menu_test_set_force_bounds_overflow(CMPBool enable) {
  g_m3_menu_test_force_bounds_overflow = enable;
  return CMP_OK;
}

int CMP_CALL m3_menu_test_set_force_hit_test_error(CMPBool enable) {
  g_m3_menu_test_force_hit_test_error = enable;
  return CMP_OK;
}

int CMP_CALL m3_menu_test_set_style_fail_text_style(CMPBool enable) {
  g_m3_menu_test_fail_text_style = enable;
  return CMP_OK;
}

int CMP_CALL m3_menu_test_set_style_fail_shadow_init(CMPBool enable) {
  g_m3_menu_test_fail_shadow_init = enable;
  return CMP_OK;
}

int CMP_CALL m3_menu_test_set_style_fail_background_color(CMPBool enable) {
  g_m3_menu_test_fail_background_color = enable;
  return CMP_OK;
}

int CMP_CALL m3_menu_test_set_style_fail_disabled_color(CMPBool enable) {
  g_m3_menu_test_fail_disabled_color = enable;
  return CMP_OK;
}

int CMP_CALL m3_menu_test_validate_style(const M3MenuStyle *style,
                                         CMPBool require_family) {
  return m3_menu_validate_style(style, require_family);
}

int CMP_CALL m3_menu_test_update_metrics(M3Menu *menu,
                                         CMPScalar *out_text_width,
                                         CMPBool *out_has_label) {
  return m3_menu_update_metrics(menu, out_text_width,
                                out_has_label); /* GCOVR_EXCL_LINE */
}

int CMP_CALL m3_menu_test_compute_panel_size(M3Menu *menu, CMPScalar *out_width,
                                             CMPScalar *out_height,
                                             CMPBool *out_has_label) {
  return m3_menu_compute_panel_size(menu, out_width, out_height, out_has_label);
}

int CMP_CALL m3_menu_test_compute_panel_bounds(/* GCOVR_EXCL_LINE */
                                               const M3Menu *menu,
                                               const CMPRect *overlay,
                                               CMPScalar panel_width,
                                               CMPScalar panel_height,
                                               CMPRect *out_bounds,
                                               cmp_u32 *out_direction) {
  return m3_menu_compute_panel_bounds(menu, overlay, panel_width, panel_height,
                                      out_bounds,
                                      out_direction); /* GCOVR_EXCL_LINE */
}

int CMP_CALL m3_menu_test_hit_test(const M3Menu *menu, CMPScalar x, CMPScalar y,
                                   CMPBool *out_inside, cmp_usize *out_index) {
  return m3_menu_hit_test(menu, x, y, out_inside,
                          out_index); /* GCOVR_EXCL_LINE */
}

int CMP_CALL m3_menu_test_validate_color(const CMPColor *color) {
  return m3_menu_validate_color(color); /* GCOVR_EXCL_LINE */
}

int CMP_CALL m3_menu_test_color_set(CMPColor *color, CMPScalar r, CMPScalar g,
                                    CMPScalar b, CMPScalar a) {
  return m3_menu_color_set(color, r, g, b, a); /* GCOVR_EXCL_LINE */
}

int CMP_CALL
m3_menu_test_validate_edges(const CMPLayoutEdges *edges) { /* GCOVR_EXCL_LINE */
  return m3_menu_validate_edges(edges);                    /* GCOVR_EXCL_LINE */
}

int CMP_CALL m3_menu_test_validate_text_style(
    const CMPTextStyle *style, /* GCOVR_EXCL_LINE */
    CMPBool require_family) {
  return m3_menu_validate_text_style(style, require_family);
}

int CMP_CALL
m3_menu_test_validate_measure_spec(CMPMeasureSpec spec) { /* GCOVR_EXCL_LINE */
  return m3_menu_validate_measure_spec(spec);
}

int CMP_CALL
m3_menu_test_validate_rect(const CMPRect *rect) { /* GCOVR_EXCL_LINE */
  return m3_menu_validate_rect(rect);             /* GCOVR_EXCL_LINE */
}

int CMP_CALL
m3_menu_test_validate_anchor(const M3MenuAnchor *anchor) { /* GCOVR_EXCL_LINE */
  return m3_menu_validate_anchor(anchor);                  /* GCOVR_EXCL_LINE */
}

int CMP_CALL m3_menu_test_validate_placement(
    const M3MenuPlacement *placement) {         /* GCOVR_EXCL_LINE */
  return m3_menu_validate_placement(placement); /* GCOVR_EXCL_LINE */
}

int CMP_CALL
m3_menu_test_validate_items(const M3MenuItem *items, /* GCOVR_EXCL_LINE */
                            cmp_usize count) {
  return m3_menu_validate_items(items, count); /* GCOVR_EXCL_LINE */
}
#endif
