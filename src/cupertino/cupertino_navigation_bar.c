/* clang-format off */
#include "cupertino/cupertino_navigation_bar.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL
cupertino_navigation_bar_style_init(CupertinoNavigationBarStyle *style) {
  CupertinoTypographyScale typo;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->variant = CUPERTINO_NAV_BAR_STANDARD;
  style->background_blur = CUPERTINO_BLUR_STYLE_CHROME;
  style->is_dark_mode = CMP_FALSE;

  cupertino_color_get_system(CUPERTINO_COLOR_BLUE, CMP_FALSE,
                             &style->tint_color);

  cupertino_typography_scale_init(&typo);
  cupertino_typography_get_style(&typo, CUPERTINO_TYPOGRAPHY_HEADLINE,
                                 &style->title_style);
  cupertino_typography_get_style(&typo, CUPERTINO_TYPOGRAPHY_LARGE_TITLE,
                                 &style->large_title_style);
  style->large_title_style.weight = 700; /* Bold large title */

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_navigation_bar_init(
    CupertinoNavigationBar *bar, const CMPTextBackend *text_backend) {
  if (bar == NULL || text_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(bar, 0, sizeof(*bar));
  bar->text_backend = *text_backend;
  cupertino_navigation_bar_style_init(&bar->style);

  bar->widget.ctx = bar;
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_navigation_bar_set_title(
    CupertinoNavigationBar *bar, const char *title_utf8, cmp_usize title_len) {
  if (bar == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  bar->title_utf8 = title_utf8;
  bar->title_len = title_len;
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_navigation_bar_layout(
    CupertinoNavigationBar *bar, CMPRect bounds, CMPRect safe_area) {
  CMPScalar nav_height = 44.0f; /* Standard iOS height */
  CMPScalar top_inset;

  if (bar == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  top_inset = safe_area.y > 0.0f ? safe_area.y : 0.0f;

  if (bar->style.variant == CUPERTINO_NAV_BAR_LARGE_TITLE) {
    /* 44 + 52 for large title extension minus scroll offset */
    CMPScalar large_ext = 52.0f - bar->scroll_offset;
    if (large_ext < 0.0f)
      large_ext = 0.0f;
    nav_height += large_ext;
  }

  bar->bounds = bounds;
  bar->bounds.y = 0.0f; /* Pin to top */
  bar->bounds.height = top_inset + nav_height;
  bar->safe_area_insets = safe_area;

  /* Layout children (buttons) */
  if (bar->left_item) {
    CMPRect btn_bounds = {
        16.0f, top_inset + (44.0f - bar->left_item->bounds.height) / 2.0f,
        bar->left_item->bounds.width, bar->left_item->bounds.height};
    cupertino_button_layout(bar->left_item, btn_bounds);
  }

  if (bar->right_item) {
    CMPRect btn_bounds = {
        bar->bounds.width - 16.0f - bar->right_item->bounds.width,
        top_inset + (44.0f - bar->right_item->bounds.height) / 2.0f,
        bar->right_item->bounds.width, bar->right_item->bounds.height};
    cupertino_button_layout(bar->right_item, btn_bounds);
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_navigation_bar_paint(
    const CupertinoNavigationBar *bar, CMPPaintContext *ctx) {
  CupertinoBlurEffect blur_effect;
  CMPColor border_color;
  CMPScalar top_inset;
  int rc;

  if (bar == NULL || ctx == NULL || ctx->gfx == NULL ||
      ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  top_inset = bar->safe_area_insets.y > 0.0f ? bar->safe_area_insets.y : 0.0f;

  rc = cupertino_blur_get_effect(bar->style.background_blur,
                                 bar->style.is_dark_mode, &blur_effect);
  if (rc == CMP_OK && ctx->gfx->vtable->draw_rect) {
    /* Hack: Standard CMPGfx doesn't support blurs natively yet, so we use the
       tinted base color. In a real iOS backend, this rect would be drawn with
       UIVisualEffectView. */
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &bar->bounds,
                                blur_effect.tint_color, 0.0f);

    /* 1px bottom separator (hairline) */
    border_color.r = bar->style.is_dark_mode ? 0.3f : 0.7f;
    border_color.g = border_color.r;
    border_color.b = border_color.r;
    border_color.a = 0.5f;

    if (ctx->gfx->vtable->draw_line) {
      ctx->gfx->vtable->draw_line(ctx->gfx->ctx, 0.0f, bar->bounds.height,
                                  bar->bounds.width, bar->bounds.height,
                                  border_color, 1.0f);
    }
  }

  /* Paint titles */
  if (bar->title_utf8 && bar->title_len > 0) {
    CMPHandle font = {0};
    CMPTextMetrics metrics = {0};
    CMPTextStyle applied_style;
    CMPScalar text_x, text_y;
    CMPBool show_inline = CMP_TRUE;
    CMPColor text_color = {0};

    text_color.r = bar->style.is_dark_mode ? 1.0f : 0.0f;
    text_color.g = text_color.r;
    text_color.b = text_color.r;
    text_color.a = 1.0f;

    if (bar->style.variant == CUPERTINO_NAV_BAR_LARGE_TITLE) {
      /* Transition inline title opacity based on scroll */
      if (bar->scroll_offset < 40.0f) {
        show_inline = CMP_FALSE; /* Fully hidden */
      } else if (bar->scroll_offset < 52.0f) {
        text_color.a = (bar->scroll_offset - 40.0f) / 12.0f; /* Fade in */
      }
    }

    if (show_inline) {
      applied_style = bar->style.title_style;
      if (cmp_text_font_create(&bar->text_backend, &applied_style, &font) ==
          CMP_OK) {
        cmp_text_measure_utf8(&bar->text_backend, font, bar->title_utf8,
                              bar->title_len, 0, &metrics);

        text_x = bar->bounds.x + (bar->bounds.width - metrics.width) / 2.0f;
        text_y = top_inset + (44.0f - metrics.height) / 2.0f + metrics.baseline;

        cmp_text_draw_utf8_gfx(ctx->gfx, font, bar->title_utf8, bar->title_len,
                               0, text_x, text_y, text_color);
        cmp_text_font_destroy(&bar->text_backend, font);
      }
    }

    if (bar->style.variant == CUPERTINO_NAV_BAR_LARGE_TITLE &&
        bar->scroll_offset < 52.0f) {
      CMPHandle large_font = {0};
      applied_style = bar->style.large_title_style;

      /* Fade out large title slightly as it collapses, though mostly it just
       * shrinks/moves */
      text_color.a = 1.0f - (bar->scroll_offset / 52.0f);
      if (text_color.a < 0.0f)
        text_color.a = 0.0f;

      if (cmp_text_font_create(&bar->text_backend, &applied_style,
                               &large_font) == CMP_OK) {
        cmp_text_measure_utf8(&bar->text_backend, large_font, bar->title_utf8,
                              bar->title_len, 0, &metrics);

        text_x = 16.0f; /* iOS large titles are left-aligned */
        /* Base line calculation for the extended area */
        text_y = top_inset + 44.0f +
                 (52.0f - bar->scroll_offset - metrics.height) / 2.0f +
                 metrics.baseline;

        cmp_text_draw_utf8_gfx(ctx->gfx, large_font, bar->title_utf8,
                               bar->title_len, 0, text_x, text_y, text_color);
        cmp_text_font_destroy(&bar->text_backend, large_font);
      }
    }
  }

  if (bar->left_item) {
    cupertino_button_paint(bar->left_item, ctx);
  }
  if (bar->right_item) {
    cupertino_button_paint(bar->right_item, ctx);
  }

  return CMP_OK;
}
