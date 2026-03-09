#include "cupertino/cupertino_tv_card.h"
#include <string.h>

CMP_API int CMP_CALL cupertino_tv_card_init(
    CupertinoTVCard *card, const CMPTextBackend *text_backend) {
  if (card == NULL || text_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(card, 0, sizeof(*card));
  card->text_backend = *text_backend;

  card->bounds.width = 400;
  card->bounds.height = 300;

  card->is_focused = CMP_FALSE;
  card->remote_touch_x = 0.0f;
  card->remote_touch_y = 0.0f;
  card->is_dark_mode =
      CMP_TRUE; /* tvOS defaults to dark usually, but parameterized */

  cmp_anim_controller_init(&card->focus_anim);
  card->focus_anim.value = 0.0f;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_tv_card_set_title(CupertinoTVCard *card,
                                                 const char *title_utf8) {
  if (card == NULL)
    return CMP_ERR_INVALID_ARGUMENT;

  if (title_utf8) {
    cmp_usize len = strlen(title_utf8);
    if (len >= 64)
      len = 63;
    memcpy(card->title_utf8, title_utf8, len);
    card->title_utf8[len] = '\0';
    card->title_len = len;
  } else {
    card->title_utf8[0] = '\0';
    card->title_len = 0;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_tv_card_set_focused(CupertinoTVCard *card,
                                                   CMPBool is_focused,
                                                   CMPBool animated) {
  CMPScalar target;
  if (card == NULL)
    return CMP_ERR_INVALID_ARGUMENT;

  card->is_focused = is_focused;
  target = is_focused ? 1.0f : 0.0f;

  if (animated) {
    /* Bouncy tvOS spring */
    cmp_anim_controller_start_spring(&card->focus_anim, card->focus_anim.value,
                                     target, 250.0f, 20.0f, 1.0f);
  } else {
    cmp_anim_controller_stop(&card->focus_anim);
    card->focus_anim.value = target;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_tv_card_update(CupertinoTVCard *card,
                                              double delta_time) {
  CMPBool finished;
  CMPScalar out_val;
  if (card == NULL)
    return CMP_ERR_INVALID_ARGUMENT;

  cmp_anim_controller_step(&card->focus_anim, (CMPScalar)delta_time, &out_val,
                           &finished);
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_tv_card_paint(const CupertinoTVCard *card,
                                             CMPPaintContext *ctx) {
  CMPColor bg_color;
  CMPColor shadow_color;
  CMPColor text_color;
  CMPScalar corner_radius = 16.0f;
  CMPScalar focus_progress;
  CMPScalar scale;
  CMPRect draw_rect;

  if (card == NULL || ctx == NULL || ctx->gfx == NULL ||
      ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  focus_progress = card->focus_anim.value;

  /* Typically scales to 1.1x */
  scale = 1.0f + (0.1f * focus_progress);

  /* Expand bounds based on scale */
  draw_rect.width = card->bounds.width * scale;
  draw_rect.height = card->bounds.height * scale;
  draw_rect.x =
      card->bounds.x - ((draw_rect.width - card->bounds.width) / 2.0f);
  draw_rect.y =
      card->bounds.y - ((draw_rect.height - card->bounds.height) / 2.0f);

  /* Apply parallax translation if focused */
  if (focus_progress > 0.0f) {
    CMPScalar max_parallax_x = 10.0f * focus_progress;
    CMPScalar max_parallax_y = 10.0f * focus_progress;
    draw_rect.x += card->remote_touch_x * max_parallax_x;
    draw_rect.y += card->remote_touch_y * max_parallax_y;
  }

  /* Colors */
  if (card->is_dark_mode) {
    bg_color.r = 0.2f;
    bg_color.g = 0.2f;
    bg_color.b = 0.2f;
    bg_color.a = 1.0f;
    text_color.r = 1.0f;
    text_color.g = 1.0f;
    text_color.b = 1.0f;
    text_color.a = 1.0f;
    shadow_color.r = 0.0f;
    shadow_color.g = 0.0f;
    shadow_color.b = 0.0f;
    shadow_color.a = 0.5f * focus_progress;
  } else {
    bg_color.r = 0.9f;
    bg_color.g = 0.9f;
    bg_color.b = 0.9f;
    bg_color.a = 1.0f;
    text_color.r = 0.0f;
    text_color.g = 0.0f;
    text_color.b = 0.0f;
    text_color.a = 1.0f;
    shadow_color.r = 0.0f;
    shadow_color.g = 0.0f;
    shadow_color.b = 0.0f;
    shadow_color.a = 0.3f * focus_progress;
  }

  /* Shadow (simulated by dropping down and scaling slightly based on focus) */
  if (shadow_color.a > 0.0f && ctx->gfx->vtable->draw_rect) {
    CMPRect s_rect = draw_rect;
    s_rect.y += 20.0f * focus_progress; /* Shadow drops further when focused */
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &s_rect, shadow_color,
                                corner_radius);
  }

  /* Card Background */
  if (ctx->gfx->vtable->draw_rect) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &draw_rect, bg_color,
                                corner_radius);
  }

  /* Title below the card, appearing/scaling with focus */
  if (card->title_len > 0) {
    CMPTextStyle t_style;
    CMPHandle font = {0};
    CMPTextMetrics metrics = {0};

    memset(&t_style, 0, sizeof(t_style));
    t_style.size_px = 30.0f; /* large for tv */
    t_style.weight = 500;

    if (cmp_text_font_create((void *)&card->text_backend, &t_style, &font) ==
        CMP_OK) {
      cmp_text_measure_utf8((void *)&card->text_backend, font, card->title_utf8,
                            card->title_len, 0, &metrics);

      CMPScalar tx =
          card->bounds.x + (card->bounds.width / 2.0f) - (metrics.width / 2.0f);
      CMPScalar ty = card->bounds.y + card->bounds.height + 40.0f +
                     metrics.baseline; /* Standard gap */

      /* Text appears solid when focused, faded when unfocused */
      CMPColor tc = text_color;
      tc.a = 0.5f + (0.5f * focus_progress);

      cmp_text_draw_utf8_gfx(ctx->gfx, font, card->title_utf8, card->title_len,
                             0, tx, ty, tc);
      cmp_text_font_destroy((void *)&card->text_backend, font);
    }
  }

  return CMP_OK;
}
