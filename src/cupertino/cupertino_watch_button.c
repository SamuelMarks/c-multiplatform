/* clang-format off */
#include "cupertino/cupertino_watch_button.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL cupertino_watch_button_init(
    CupertinoWatchButton *button, const CMPTextBackend *text_backend) {
  if (button == NULL || text_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(button, 0, sizeof(*button));
  button->text_backend = *text_backend;

  button->bounds.width = 160;
  button->bounds.height = 44;

  button->is_pressed = CMP_FALSE;
  button->is_disabled = CMP_FALSE;

  /* watchOS buttons are typically a dark gray when not specifically tinted */
  button->tint_color.r = 0.15f;
  button->tint_color.g = 0.15f;
  button->tint_color.b = 0.15f;
  button->tint_color.a = 1.0f;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_watch_button_set_title(
    CupertinoWatchButton *button, const char *title_utf8) {
  if (button == NULL)
    return CMP_ERR_INVALID_ARGUMENT;

  if (title_utf8) {
    cmp_usize len = strlen(title_utf8);
    if (len >= 64)
      len = 63;
    memcpy(button->title_utf8, title_utf8, len);
    button->title_utf8[len] = '\0';
    button->title_len = len;
  } else {
    button->title_utf8[0] = '\0';
    button->title_len = 0;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_watch_button_paint(
    const CupertinoWatchButton *button, CMPPaintContext *ctx) {
  CMPColor bg_color;
  CMPColor text_color;
  CMPScalar corner_radius;
  CMPRect draw_rect;

  if (button == NULL || ctx == NULL || ctx->gfx == NULL ||
      ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  draw_rect = button->bounds;

  /* Full pill shape */
  corner_radius = button->bounds.height / 2.0f;

  bg_color = button->tint_color;
  text_color.r = 1.0f;
  text_color.g = 1.0f;
  text_color.b = 1.0f;
  text_color.a = 1.0f;

  if (button->is_disabled) {
    bg_color.a = 0.5f;
    text_color.a = 0.5f;
  } else if (button->is_pressed) {
    CMPScalar shrink = 2.0f;
    /* watchOS shrinks the button slightly and dims it when pressed */
    bg_color.r *= 0.8f;
    bg_color.g *= 0.8f;
    bg_color.b *= 0.8f;

    draw_rect.x += shrink;
    draw_rect.y += shrink;
    draw_rect.width -= shrink * 2.0f;
    draw_rect.height -= shrink * 2.0f;
    corner_radius = draw_rect.height / 2.0f;
  }

  if (ctx->gfx->vtable->draw_rect) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &draw_rect, bg_color,
                                corner_radius);
  }

  if (button->title_len > 0) {
    CMPTextStyle t_style;
    CMPHandle font = {0};
    CMPTextMetrics metrics = {0};

    memset(&t_style, 0, sizeof(t_style));
    t_style.size_px = 15.0f;
    t_style.weight = 500;

    if (cmp_text_font_create((void *)&button->text_backend, &t_style, &font) ==
        CMP_OK) {
      CMPScalar tx;
      CMPScalar ty;
      cmp_text_measure_utf8((void *)&button->text_backend, font,
                            button->title_utf8, button->title_len, 0, &metrics);

      tx = draw_rect.x + (draw_rect.width / 2.0f) - (metrics.width / 2.0f);
      ty = draw_rect.y + (draw_rect.height / 2.0f) - (metrics.height / 2.0f) +
           metrics.baseline;

      cmp_text_draw_utf8_gfx(ctx->gfx, font, button->title_utf8,
                             button->title_len, 0, tx, ty, text_color);
      cmp_text_font_destroy((void *)&button->text_backend, font);
    }
  }

  return CMP_OK;
}
