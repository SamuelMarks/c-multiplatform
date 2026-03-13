/* clang-format off */
#include "cupertino/cupertino_mac_toolbar.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL cupertino_mac_toolbar_init(
    CupertinoMacToolbar *toolbar, const CMPTextBackend *text_backend) {
  if (toolbar == NULL || text_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(toolbar, 0, sizeof(*toolbar));
  toolbar->text_backend = *text_backend;

  toolbar->bounds.width = 600;
  toolbar->bounds.height =
      52; /* Standard macOS unified toolbar height is often 52 or 54 */

  toolbar->is_dark_mode = CMP_FALSE;
  toolbar->is_window_active = CMP_TRUE;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_mac_toolbar_add_item(
    CupertinoMacToolbar *toolbar, const char *title_utf8) {
  CupertinoMacToolbarItem *item;

  if (toolbar == NULL || title_utf8 == NULL)
    return CMP_ERR_INVALID_ARGUMENT;
  if (toolbar->item_count >= CUPERTINO_MAC_TOOLBAR_MAX_ITEMS)
    return CMP_ERR_OUT_OF_MEMORY;

  item = &toolbar->items[toolbar->item_count++];
  memset(item, 0, sizeof(*item));

  {
    cmp_usize len = strlen(title_utf8);
    if (len >= 64)
      len = 63;
    memcpy(item->title_utf8, title_utf8, len);
    item->title_len = len;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL
cupertino_mac_toolbar_add_flexible_space(CupertinoMacToolbar *toolbar) {
  CupertinoMacToolbarItem *item;
  if (toolbar == NULL)
    return CMP_ERR_INVALID_ARGUMENT;
  if (toolbar->item_count >= CUPERTINO_MAC_TOOLBAR_MAX_ITEMS)
    return CMP_ERR_OUT_OF_MEMORY;

  item = &toolbar->items[toolbar->item_count++];
  memset(item, 0, sizeof(*item));
  item->is_flexible_space = CMP_TRUE;

  return CMP_OK;
}

CMP_API int CMP_CALL
cupertino_mac_toolbar_add_search(CupertinoMacToolbar *toolbar) {
  CupertinoMacToolbarItem *item;
  if (toolbar == NULL)
    return CMP_ERR_INVALID_ARGUMENT;
  if (toolbar->item_count >= CUPERTINO_MAC_TOOLBAR_MAX_ITEMS)
    return CMP_ERR_OUT_OF_MEMORY;

  item = &toolbar->items[toolbar->item_count++];
  memset(item, 0, sizeof(*item));
  item->is_search = CMP_TRUE;
#if defined(_MSC_VER)
  strcpy_s(item->title_utf8, sizeof(item->title_utf8), "Search");
#else
  strcpy(item->title_utf8, "Search");
#endif
  item->title_len = 6;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_mac_toolbar_paint(
    const CupertinoMacToolbar *toolbar, CMPPaintContext *ctx) {
  CMPColor bg_color;
  CMPColor border_color;
  CMPColor text_color;
  CMPColor btn_bg_color;

  CMPScalar current_x;
  CMPScalar padding_y = 12.0f;
  CMPScalar item_height;
  cmp_usize i;
  cmp_usize flex_count = 0;
  CMPScalar total_fixed_width = 0.0f;
  CMPScalar flex_width = 0.0f;

  if (toolbar == NULL || ctx == NULL || ctx->gfx == NULL ||
      ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  item_height = toolbar->bounds.height - (padding_y * 2.0f);

  /* Calculate layout space */
  for (i = 0; i < toolbar->item_count; i++) {
    const CupertinoMacToolbarItem *item = &toolbar->items[i];
    if (item->is_flexible_space) {
      flex_count++;
    } else if (item->is_search) {
      total_fixed_width += 150.0f; /* Fixed search width */
    } else {
      /* Approximate text width without measuring for layout simplicity,
       * or we could measure each. Let's use a fixed button width for now. */
      total_fixed_width += 60.0f;
    }
    total_fixed_width += 8.0f; /* Gap */
  }

  if (flex_count > 0) {
    flex_width = (toolbar->bounds.width - total_fixed_width - 80.0f) /
                 (CMPScalar)flex_count; /* 80 for traffic lights space */
    if (flex_width < 0.0f)
      flex_width = 0.0f;
  }

  /* Colors */
  if (toolbar->is_dark_mode) {
    bg_color.r = 0.15f;
    bg_color.g = 0.15f;
    bg_color.b = 0.15f;
    bg_color.a = 1.0f;
    border_color.r = 0.0f;
    border_color.g = 0.0f;
    border_color.b = 0.0f;
    border_color.a = 1.0f;
    text_color.r = 0.9f;
    text_color.g = 0.9f;
    text_color.b = 0.9f;
    text_color.a = 1.0f;
    btn_bg_color.r = 0.3f;
    btn_bg_color.g = 0.3f;
    btn_bg_color.b = 0.3f;
    btn_bg_color.a = 1.0f;
  } else {
    bg_color.r = 0.95f;
    bg_color.g = 0.95f;
    bg_color.b = 0.95f;
    bg_color.a = 1.0f;
    border_color.r = 0.8f;
    border_color.g = 0.8f;
    border_color.b = 0.8f;
    border_color.a = 1.0f;
    text_color.r = 0.1f;
    text_color.g = 0.1f;
    text_color.b = 0.1f;
    text_color.a = 1.0f;
    btn_bg_color.r = 0.9f;
    btn_bg_color.g = 0.9f;
    btn_bg_color.b = 0.9f;
    btn_bg_color.a = 1.0f;
  }

  if (!toolbar->is_window_active) {
    text_color.a = 0.5f;
  }

  /* Background */
  if (ctx->gfx->vtable->draw_rect) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &toolbar->bounds, bg_color,
                                0.0f);
  }

  /* Bottom border separator */
  if (ctx->gfx->vtable->draw_line) {
    ctx->gfx->vtable->draw_line(
        ctx->gfx->ctx, toolbar->bounds.x,
        toolbar->bounds.y + toolbar->bounds.height - 1.0f,
        toolbar->bounds.x + toolbar->bounds.width,
        toolbar->bounds.y + toolbar->bounds.height - 1.0f, border_color, 1.0f);
  }

  /* Render Items */
  current_x = toolbar->bounds.x + 80.0f; /* Start after traffic lights */

  for (i = 0; i < toolbar->item_count; i++) {
    const CupertinoMacToolbarItem *item = &toolbar->items[i];
    CMPRect item_rect;
    CMPColor search_bg;

    if (item->is_flexible_space) {
      current_x += flex_width;
      continue;
    }

    item_rect.x = current_x;
    item_rect.y = toolbar->bounds.y + padding_y;
    item_rect.height = item_height;

    if (item->is_search) {
      item_rect.width = 150.0f;
      if (ctx->gfx->vtable->draw_rect) {
        /* Search box background */
        search_bg = toolbar->is_dark_mode
                        ? cmp_color_rgba(0.0f, 0.0f, 0.0f, 0.2f)
                        : cmp_color_rgba(1.0f, 1.0f, 1.0f, 0.5f);
        ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &item_rect, search_bg, 6.0f);
      }
    } else {
      item_rect.width = 60.0f;
      /* Simple button */
      if (ctx->gfx->vtable->draw_rect) {
        ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &item_rect, btn_bg_color,
                                    6.0f);
      }
    }

    /* Draw Text */
    if (item->title_len > 0) {
      CMPTextStyle row_style;
      CMPHandle font = {0};
      CMPTextMetrics metrics = {0};
      memset(&row_style, 0, sizeof(row_style));

      row_style.size_px = 13.0f;
      row_style.weight = 400;
      row_style.color = text_color;

      if (cmp_text_font_create((void *)&toolbar->text_backend, &row_style,
                               &font) == CMP_OK) {
        CMPScalar text_y;
        CMPScalar text_x;

        cmp_text_measure_utf8((void *)&toolbar->text_backend, font,
                              item->title_utf8, item->title_len, 0, &metrics);
        text_y = item_rect.y + (item_rect.height / 2.0f) -
                 (metrics.height / 2.0f) + metrics.baseline;

        if (item->is_search) {
          text_x = item_rect.x + 8.0f; /* left aligned */
        } else {
          text_x = item_rect.x + (item_rect.width / 2.0f) -
                   (metrics.width / 2.0f); /* center */
        }

        cmp_text_draw_utf8_gfx(ctx->gfx, font, item->title_utf8,
                               item->title_len, 0, text_x, text_y, text_color);
        cmp_text_font_destroy((void *)&toolbar->text_backend, font);
      }
    }

    current_x += item_rect.width + 8.0f;
  }

  return CMP_OK;
}
