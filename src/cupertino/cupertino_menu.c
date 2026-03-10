/* clang-format off */
#include "cupertino/cupertino_menu.h"
#include "cmpc/cmp_path.h"
#include "cupertino/cupertino_shape.h"
#include <string.h>
/* clang-format on */

#define CUPERTINO_MENU_ITEM_HEIGHT_MAC 22.0f
#define CUPERTINO_MENU_ITEM_HEIGHT_IOS 44.0f
#define CUPERTINO_MENU_PADDING 6.0f
#define CUPERTINO_MENU_CORNER_RADIUS_MAC 6.0f
#define CUPERTINO_MENU_CORNER_RADIUS_IOS 12.0f
#define CUPERTINO_MENU_MIN_WIDTH 120.0f
#define CUPERTINO_MENU_MAX_WIDTH 250.0f

CMP_API int CMP_CALL cupertino_menu_style_init(CupertinoMenuStyle *style,
                                               CupertinoMenuVariant variant) {
  CupertinoTypographyScale typo;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->variant = variant;
  style->is_dark_mode = CMP_FALSE;

  cupertino_color_get_system(CUPERTINO_COLOR_BLUE, CMP_FALSE,
                             &style->tint_color);
  cupertino_color_get_system(CUPERTINO_COLOR_RED, CMP_FALSE,
                             &style->destructive_color);

  style->text_color.r = 0.0f;
  style->text_color.g = 0.0f;
  style->text_color.b = 0.0f;
  style->text_color.a = 1.0f;

  cupertino_typography_scale_init(&typo);

  if (variant == CUPERTINO_MENU_MAC_OS) {
    style->background_blur =
        CUPERTINO_BLUR_STYLE_NORMAL; /* macOS BehindWindow approximation */
    cupertino_typography_get_style(&typo, CUPERTINO_TYPOGRAPHY_BODY,
                                   &style->label_style);
    style->label_style.size_px = 13; /* macOS standard menu font size */
    style->label_style.line_height_px = 13.0f;
  } else {
    style->background_blur = CUPERTINO_BLUR_STYLE_THICK; /* iOS standard */
    cupertino_typography_get_style(&typo, CUPERTINO_TYPOGRAPHY_BODY,
                                   &style->label_style);
    style->label_style.size_px = 17;
  }

  style->shortcut_style = style->label_style;
  style->shortcut_style.color.r = 0.5f;
  style->shortcut_style.color.g = 0.5f;
  style->shortcut_style.color.b = 0.5f;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_menu_init(CupertinoMenu *menu,
                                         const CMPTextBackend *text_backend,
                                         CupertinoMenuVariant variant) {
  if (menu == NULL || text_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(menu, 0, sizeof(*menu));
  menu->text_backend = *text_backend;
  menu->hovered_index = -1;
  menu->animation_progress = 1.0f;
  cupertino_menu_style_init(&menu->style, variant);

  menu->widget.ctx = menu;
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_menu_set_items(CupertinoMenu *menu,
                                              CupertinoMenuItem *items,
                                              cmp_u32 count) {
  if (menu == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (items == NULL && count > 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  menu->items = items;
  menu->item_count = count;
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_menu_set_hover(CupertinoMenu *menu, int index) {
  if (menu == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  menu->hovered_index = index;
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_menu_measure(CupertinoMenu *menu,
                                            CMPSize *out_size) {
  cmp_u32 i;
  CMPScalar max_w = CUPERTINO_MENU_MIN_WIDTH;
  CMPScalar current_h = CUPERTINO_MENU_PADDING * 2.0f;
  CMPScalar item_height;

  if (menu == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  item_height = (menu->style.variant == CUPERTINO_MENU_MAC_OS)
                    ? CUPERTINO_MENU_ITEM_HEIGHT_MAC
                    : CUPERTINO_MENU_ITEM_HEIGHT_IOS;

  for (i = 0; i < menu->item_count; i++) {
    if (menu->items[i].style == CUPERTINO_MENU_ITEM_SEPARATOR) {
      current_h += 9.0f; /* standard separator block height */
    } else {
      /* Quick estimate, skipping actual text measurement for simplicity in this
       * port */
      CMPScalar est_w = 40.0f; /* padding */
      if (menu->items[i].title_utf8) {
        est_w += (CMPScalar)menu->items[i].title_len * 8.0f; /* approx width */
      }
      if (menu->items[i].shortcut_utf8) {
        est_w += 20.0f + (CMPScalar)menu->items[i].shortcut_len * 8.0f;
      }

      if (est_w > max_w)
        max_w = est_w;
      current_h += item_height;
    }
  }

  if (max_w > CUPERTINO_MENU_MAX_WIDTH) {
    max_w = CUPERTINO_MENU_MAX_WIDTH;
  }

  out_size->width = max_w;
  out_size->height = current_h;
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_menu_layout(CupertinoMenu *menu,
                                           CMPRect anchor_rect,
                                           CMPRect screen_bounds) {
  CMPSize size;
  int rc;

  if (menu == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cupertino_menu_measure(menu, &size);
  if (rc != CMP_OK)
    return rc;

  menu->anchor_rect = anchor_rect;

  if (menu->style.variant == CUPERTINO_MENU_MAC_OS) {
    /* macOS usually spawns right at the cursor (or directly below a menu bar
     * item) */
    menu->bounds.x = anchor_rect.x;
    menu->bounds.y = anchor_rect.y + anchor_rect.height;
  } else {
    /* iOS context menus usually float near the anchor but avoid edges */
    menu->bounds.x = anchor_rect.x;
    menu->bounds.y = anchor_rect.y + anchor_rect.height + 8.0f;
  }

  menu->bounds.width = size.width;
  menu->bounds.height = size.height;

  /* Basic screen clamping */
  if (menu->bounds.x + menu->bounds.width > screen_bounds.width) {
    menu->bounds.x = screen_bounds.width - menu->bounds.width;
  }
  if (menu->bounds.y + menu->bounds.height > screen_bounds.height) {
    menu->bounds.y =
        anchor_rect.y - menu->bounds.height - 8.0f; /* flip above */
    if (menu->bounds.y < 0.0f) {
      menu->bounds.y = 0.0f;
    }
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_menu_paint(const CupertinoMenu *menu,
                                          CMPPaintContext *ctx) {
  CupertinoBlurEffect blur_effect;
  CMPColor border_color;
  CMPPath path = {0};
  CMPAllocator alloc;
  CMPScalar corner_radius;
  CMPScalar current_y;
  CMPScalar item_height;
  cmp_u32 i;
  int rc;

  if (menu == NULL || ctx == NULL || ctx->gfx == NULL ||
      ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (menu->animation_progress <= 0.0f || menu->item_count == 0) {
    return CMP_OK;
  }

  if (cmp_get_default_allocator(&alloc) != CMP_OK) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  corner_radius = (menu->style.variant == CUPERTINO_MENU_MAC_OS)
                      ? CUPERTINO_MENU_CORNER_RADIUS_MAC
                      : CUPERTINO_MENU_CORNER_RADIUS_IOS;
  item_height = (menu->style.variant == CUPERTINO_MENU_MAC_OS)
                    ? CUPERTINO_MENU_ITEM_HEIGHT_MAC
                    : CUPERTINO_MENU_ITEM_HEIGHT_IOS;

  rc = cupertino_blur_get_effect(menu->style.background_blur,
                                 menu->style.is_dark_mode, &blur_effect);

  if (ctx->gfx->vtable->draw_path) {
    cmp_path_init(&path, &alloc, 32);
    cupertino_shape_append_squircle(&path, menu->bounds, corner_radius,
                                    corner_radius, corner_radius,
                                    corner_radius);

    /* Draw blurred background base */
    if (rc == CMP_OK) {
      ctx->gfx->vtable->draw_path(ctx->gfx->ctx, &path, blur_effect.tint_color);
    }

    /* Draw stroke / border */
    border_color.r = menu->style.is_dark_mode ? 0.3f : 0.8f;
    border_color.g = border_color.r;
    border_color.b = border_color.r;
    border_color.a = 0.5f;

    /* Standard graphics doesn't support stroke_path directly, so we rely on
     * background being handled. */
    cmp_path_shutdown(&path);
  }

  current_y = menu->bounds.y + CUPERTINO_MENU_PADDING;

  for (i = 0; i < menu->item_count; i++) {
    CupertinoMenuItem *item = &menu->items[i];

    if (item->style == CUPERTINO_MENU_ITEM_SEPARATOR) {
      CMPColor sep_color;
      sep_color.r = 0.5f;
      sep_color.g = 0.5f;
      sep_color.b = 0.5f;
      sep_color.a = 0.3f;

      if (ctx->gfx->vtable->draw_line) {
        CMPScalar lx = menu->bounds.x + 12.0f;
        CMPScalar rx = menu->bounds.x + menu->bounds.width - 12.0f;
        CMPScalar sy = current_y + 4.5f;
        ctx->gfx->vtable->draw_line(ctx->gfx->ctx, lx, sy, rx, sy, sep_color,
                                    1.0f);
      }
      current_y += 9.0f;
      continue;
    }

    /* Draw Hover/Selection state */
    if ((int)i == menu->hovered_index && !item->is_disabled &&
        ctx->gfx->vtable->draw_rect) {
      CMPColor sel_color = menu->style.tint_color;
      CMPRect sel_rect = {menu->bounds.x + 4.0f, current_y,
                          menu->bounds.width - 8.0f, item_height};
      CMPScalar sel_radius =
          (menu->style.variant == CUPERTINO_MENU_MAC_OS) ? 4.0f : 0.0f;

      if (menu->style.variant == CUPERTINO_MENU_IOS) {
        sel_color.a = 0.15f; /* iOS is usually a grey/tinted wash */
      }

      ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &sel_rect, sel_color,
                                  sel_radius);
    }

    /* Draw Text */
    if (item->title_utf8 && item->title_len > 0) {
      CMPHandle font = {0};
      CMPTextMetrics metrics = {0};
      CMPTextStyle applied_style = menu->style.label_style;
      CMPScalar text_x, text_y;

      if (item->is_disabled) {
        applied_style.color.a = 0.4f;
      } else if ((int)i == menu->hovered_index &&
                 menu->style.variant == CUPERTINO_MENU_MAC_OS) {
        /* macOS text turns white when selected/hovered */
        applied_style.color.r = 1.0f;
        applied_style.color.g = 1.0f;
        applied_style.color.b = 1.0f;
        applied_style.color.a = 1.0f;
      } else if (item->style == CUPERTINO_MENU_ITEM_DESTRUCTIVE) {
        applied_style.color = menu->style.destructive_color;
      } else {
        applied_style.color = menu->style.text_color;
      }

      if (cmp_text_font_create(&menu->text_backend, &applied_style, &font) ==
          CMP_OK) {
        cmp_text_measure_utf8(&menu->text_backend, font, item->title_utf8,
                              item->title_len, 0, &metrics);

        text_x = menu->bounds.x + 14.0f;
        text_y = current_y + (item_height - metrics.height) / 2.0f +
                 metrics.baseline;

        cmp_text_draw_utf8_gfx(ctx->gfx, font, item->title_utf8,
                               item->title_len, 0, text_x, text_y,
                               applied_style.color);
        cmp_text_font_destroy(&menu->text_backend, font);
      }
    }

    current_y += item_height;
  }

  return CMP_OK;
}
