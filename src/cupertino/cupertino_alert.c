/* clang-format off */
#include "cupertino/cupertino_alert.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL cupertino_alert_init(CupertinoAlert *alert,
                                          const CMPTextBackend *text_backend) {
  if (alert == NULL || text_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(alert, 0, sizeof(*alert));
  alert->text_backend = *text_backend;

  alert->bounds.width = 270;  /* Standard iOS alert width */
  alert->bounds.height = 150; /* Base height, should be measured */

  alert->is_dark_mode = CMP_FALSE;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_alert_set_content(CupertinoAlert *alert,
                                                 const char *title_utf8,
                                                 const char *message_utf8) {
  if (alert == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alert->title_utf8 = title_utf8;
  alert->title_len = title_utf8 ? strlen(title_utf8) : 0;

  alert->message_utf8 = message_utf8;
  alert->message_len = message_utf8 ? strlen(message_utf8) : 0;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_alert_add_action(CupertinoAlert *alert,
                                                const char *label_utf8,
                                                CMPBool is_destructive) {
  CupertinoButton *btn;
  CupertinoButtonStyle style;

  if (alert == NULL || label_utf8 == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (alert->action_count >= CUPERTINO_ALERT_MAX_ACTIONS) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  btn = &alert->actions[alert->action_count];
  cupertino_button_init(btn, &alert->text_backend);

  cupertino_button_style_init(&style);
  style.variant = CUPERTINO_BUTTON_VARIANT_PLAIN;

  if (is_destructive) {
    cupertino_color_get_system(CUPERTINO_COLOR_RED, alert->is_dark_mode,
                               &style.tint_color);
  } else {
    cupertino_color_get_system(CUPERTINO_COLOR_BLUE, alert->is_dark_mode,
                               &style.tint_color);
  }

  cupertino_button_set_style(btn, &style);
  btn->label_utf8 = label_utf8;
  btn->label_len = strlen(label_utf8);

  alert->action_count++;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_alert_paint(const CupertinoAlert *alert,
                                           CMPPaintContext *ctx) {
  CMPRect alert_rect;
  CMPColor bg_color;
  CMPColor div_color;
  CMPColor text_color;
  CMPScalar corner_radius = 14.0f; /* Standard iOS alert radius */
  CMPScalar content_y;
  cmp_usize i;

  if (alert == NULL || ctx == NULL || ctx->gfx == NULL ||
      ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alert_rect = alert->bounds;

  /* Colors */
  if (alert->is_dark_mode) {
    /* iOS dark mode alert is typically a dark gray slightly translucent */
    bg_color.r = 0.15f;
    bg_color.g = 0.15f;
    bg_color.b = 0.15f;
    bg_color.a = 0.95f;
    div_color.r = 0.3f;
    div_color.g = 0.3f;
    div_color.b = 0.3f;
    div_color.a = 0.5f;
    text_color.r = 1.0f;
    text_color.g = 1.0f;
    text_color.b = 1.0f;
    text_color.a = 1.0f;
  } else {
    bg_color.r = 0.95f;
    bg_color.g = 0.95f;
    bg_color.b = 0.95f;
    bg_color.a = 0.95f;
    div_color.r = 0.8f;
    div_color.g = 0.8f;
    div_color.b = 0.8f;
    div_color.a = 0.5f;
    text_color.r = 0.0f;
    text_color.g = 0.0f;
    text_color.b = 0.0f;
    text_color.a = 1.0f;
  }

  /* Background */
  if (ctx->gfx->vtable->draw_rect) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &alert_rect, bg_color,
                                corner_radius);
  }

  content_y = alert_rect.y + 20.0f;

  /* Title */
  if (alert->title_len > 0) {
    CMPTextStyle title_style;
    CMPHandle font = {0};
    CMPTextMetrics metrics = {0};

    memset(&title_style, 0, sizeof(title_style));
    title_style.size_px = 17.0f;
    title_style.weight = 600; /* Semibold */
    title_style.color = text_color;

    if (cmp_text_font_create((void *)&alert->text_backend, &title_style,
                             &font) == CMP_OK) {
      CMPScalar title_x;
      cmp_text_measure_utf8((void *)&alert->text_backend, font,
                            alert->title_utf8, alert->title_len, 0, &metrics);

      title_x = alert_rect.x + (alert_rect.width - metrics.width) / 2.0f;
      cmp_text_draw_utf8_gfx(ctx->gfx, font, alert->title_utf8,
                             alert->title_len, 0, title_x,
                             content_y + metrics.baseline, text_color);

      content_y += metrics.height + 4.0f;
      cmp_text_font_destroy((void *)&alert->text_backend, font);
    }
  }

  /* Message */
  if (alert->message_len > 0) {
    CMPTextStyle msg_style;
    CMPHandle font = {0};
    CMPTextMetrics metrics = {0};

    memset(&msg_style, 0, sizeof(msg_style));
    msg_style.size_px = 13.0f;
    msg_style.weight = 400; /* Regular */
    msg_style.color = text_color;

    if (cmp_text_font_create((void *)&alert->text_backend, &msg_style, &font) ==
        CMP_OK) {
      CMPScalar msg_x;
      /* Basic multi-line measure would be here, assuming single line or simple
       * layout for now */
      cmp_text_measure_utf8((void *)&alert->text_backend, font,
                            alert->message_utf8, alert->message_len, 0,
                            &metrics);

      msg_x = alert_rect.x + (alert_rect.width - metrics.width) / 2.0f;
      content_y += 4.0f;
      cmp_text_draw_utf8_gfx(ctx->gfx, font, alert->message_utf8,
                             alert->message_len, 0, msg_x,
                             content_y + metrics.baseline, text_color);

      content_y += metrics.height + 20.0f;
      cmp_text_font_destroy((void *)&alert->text_backend, font);
    }
  } else {
    content_y += 20.0f;
  }

  /* Top Divider */
  if (ctx->gfx->vtable->draw_line) {
    ctx->gfx->vtable->draw_line(ctx->gfx->ctx, alert_rect.x, content_y,
                                alert_rect.x + alert_rect.width, content_y,
                                div_color, 1.0f);
  }

  /* Actions (Buttons) */
  if (alert->action_count == 2) {
    /* Side by side layout */
    CMPScalar btn_w = alert_rect.width / 2.0f;
    CMPScalar btn_h = 44.0f;

    for (i = 0; i < 2; i++) {
      CupertinoButton *btn = (CupertinoButton *)&alert->actions[i];
      CMPRect btn_rect;
      btn_rect.x = alert_rect.x + (i * btn_w);
      btn_rect.y = content_y;
      btn_rect.width = btn_w;
      btn_rect.height = btn_h;

      cupertino_button_layout(btn, btn_rect);
      cupertino_button_paint(btn, ctx);

      /* Vertical divider between the two buttons */
      if (i == 0 && ctx->gfx->vtable->draw_line) {
        ctx->gfx->vtable->draw_line(ctx->gfx->ctx, alert_rect.x + btn_w,
                                    content_y, alert_rect.x + btn_w,
                                    content_y + btn_h, div_color, 1.0f);
      }
    }
  } else if (alert->action_count > 0) {
    /* Stacked layout */
    CMPScalar btn_h = 44.0f;
    for (i = 0; i < alert->action_count; i++) {
      CupertinoButton *btn = (CupertinoButton *)&alert->actions[i];
      CMPRect btn_rect;

      if (i > 0 && ctx->gfx->vtable->draw_line) {
        /* Horizontal divider between stacked buttons */
        ctx->gfx->vtable->draw_line(ctx->gfx->ctx, alert_rect.x, content_y,
                                    alert_rect.x + alert_rect.width, content_y,
                                    div_color, 1.0f);
      }

      btn_rect.x = alert_rect.x;
      btn_rect.y = content_y;
      btn_rect.width = alert_rect.width;
      btn_rect.height = btn_h;

      cupertino_button_layout(btn, btn_rect);
      cupertino_button_paint(btn, ctx);

      content_y += btn_h;
    }
  }

  return CMP_OK;
}
