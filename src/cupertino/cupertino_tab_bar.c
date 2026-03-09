#include "cupertino/cupertino_tab_bar.h"
#include <string.h>

CMP_API int CMP_CALL cupertino_tab_bar_style_init(CupertinoTabBarStyle *style) {
  CupertinoTypographyScale typo;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->background_blur = CUPERTINO_BLUR_STYLE_CHROME;
  style->is_dark_mode = CMP_FALSE;

  cupertino_color_get_system(CUPERTINO_COLOR_BLUE, CMP_FALSE,
                             &style->tint_color);
  cupertino_color_get_system(CUPERTINO_COLOR_GRAY, CMP_FALSE,
                             &style->unselected_color);

  cupertino_typography_scale_init(&typo);
  cupertino_typography_get_style(&typo, CUPERTINO_TYPOGRAPHY_CAPTION_2,
                                 &style->label_style);
  style->label_style.weight = 500;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_tab_bar_init(
    CupertinoTabBar *bar, const CMPTextBackend *text_backend) {
  if (bar == NULL || text_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(bar, 0, sizeof(*bar));
  bar->text_backend = *text_backend;
  cupertino_tab_bar_style_init(&bar->style);

  bar->widget.ctx = bar;
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_tab_bar_set_items(
    CupertinoTabBar *bar, const CupertinoTabBarItem *items, cmp_u32 count) {
  cmp_u32 i;

  if (bar == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (count > CUPERTINO_TAB_BAR_MAX_ITEMS) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  bar->item_count = count;
  for (i = 0; i < count; i++) {
    bar->items[i] = items[i];
  }

  if (bar->selected_index >= count) {
    bar->selected_index = 0;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_tab_bar_layout(CupertinoTabBar *bar,
                                              CMPRect bounds,
                                              CMPRect safe_area) {
  CMPScalar tab_height = 49.0f; /* Standard iOS bottom tab bar height */
  CMPScalar bottom_inset;

  if (bar == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  /* safe_area.height here represents the bottom inset (Home indicator) */
  bottom_inset = safe_area.height > 0.0f ? safe_area.height : 0.0f;

  bar->bounds = bounds;
  bar->bounds.height = tab_height + bottom_inset;
  bar->bounds.y = bounds.height - bar->bounds.height; /* Pin to bottom */
  bar->safe_area_insets = safe_area;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_tab_bar_paint(const CupertinoTabBar *bar,
                                             CMPPaintContext *ctx) {
  CupertinoBlurEffect blur_effect;
  CMPColor border_color;
  int rc;
  cmp_u32 i;
  CMPScalar item_width;
  CMPScalar current_x;

  if (bar == NULL || ctx == NULL || ctx->gfx == NULL ||
      ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cupertino_blur_get_effect(bar->style.background_blur,
                                 bar->style.is_dark_mode, &blur_effect);
  if (rc == CMP_OK && ctx->gfx->vtable->draw_rect) {
    /* Base background representing the blurred glass */
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &bar->bounds,
                                blur_effect.tint_color, 0.0f);

    /* Top separator hairline */
    border_color.r = bar->style.is_dark_mode ? 0.3f : 0.7f;
    border_color.g = border_color.r;
    border_color.b = border_color.r;
    border_color.a = 0.5f;

    if (ctx->gfx->vtable->draw_line) {
      ctx->gfx->vtable->draw_line(ctx->gfx->ctx, bar->bounds.x, bar->bounds.y,
                                  bar->bounds.x + bar->bounds.width,
                                  bar->bounds.y, border_color, 1.0f);
    }
  }

  if (bar->item_count == 0) {
    return CMP_OK;
  }

  item_width = bar->bounds.width / (CMPScalar)bar->item_count;
  current_x = bar->bounds.x;

  for (i = 0; i < bar->item_count; i++) {
    CMPColor item_color = (i == bar->selected_index)
                              ? bar->style.tint_color
                              : bar->style.unselected_color;

    /* Draw icon placeholder (a simple rounded rect since we lack real SF
     * Symbols here) */
    if (ctx->gfx->vtable->draw_rect) {
      CMPRect icon_rect;
      icon_rect.width = 24.0f;
      icon_rect.height = 24.0f;
      icon_rect.x = current_x + (item_width - icon_rect.width) / 2.0f;
      icon_rect.y = bar->bounds.y + 6.0f;

      ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &icon_rect, item_color, 6.0f);

      /* Draw optional badge */
      if (bar->items[i].has_badge) {
        CMPRect badge_rect;
        badge_rect.width = 10.0f;
        badge_rect.height = 10.0f;
        badge_rect.x = icon_rect.x + icon_rect.width - 4.0f;
        badge_rect.y = icon_rect.y - 2.0f;
        ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &badge_rect,
                                    bar->items[i].badge_color, 5.0f);
      }
    }

    /* Draw label */
    if (bar->items[i].title_utf8 && bar->items[i].title_len > 0) {
      CMPHandle font = {0};
      CMPTextMetrics metrics = {0};
      CMPTextStyle applied_style = bar->style.label_style;
      CMPScalar text_x, text_y;

      if (cmp_text_font_create(&bar->text_backend, &applied_style, &font) ==
          CMP_OK) {
        cmp_text_measure_utf8(&bar->text_backend, font,
                              bar->items[i].title_utf8, bar->items[i].title_len,
                              0, &metrics);

        text_x = current_x + (item_width - metrics.width) / 2.0f;
        text_y = bar->bounds.y + 34.0f + metrics.baseline;

        cmp_text_draw_utf8_gfx(ctx->gfx, font, bar->items[i].title_utf8,
                               bar->items[i].title_len, 0, text_x, text_y,
                               item_color);
        cmp_text_font_destroy(&bar->text_backend, font);
      }
    }

    current_x += item_width;
  }

  return CMP_OK;
}
