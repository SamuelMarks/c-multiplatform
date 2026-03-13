/* clang-format off */
#include "cupertino/cupertino_text_field.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL cupertino_text_field_init(
    CupertinoTextField *field, const CMPTextBackend *text_backend) {
  if (field == NULL || text_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(field, 0, sizeof(*field));
  field->text_backend = *text_backend;

  field->bounds.width = 200;
  field->bounds.height = 34; /* Standard iOS text field height */

  field->style = CUPERTINO_TEXT_FIELD_STYLE_ROUNDED_RECT;

  field->is_dark_mode = CMP_FALSE;
  field->is_focused = CMP_FALSE;
  field->is_disabled = CMP_FALSE;

  field->cursor_blink_time = 0.0;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_text_field_set_text(CupertinoTextField *field,
                                                   const char *text_utf8) {
  if (field == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (text_utf8) {
    cmp_usize len = strlen(text_utf8);
    if (len >= CUPERTINO_TEXT_FIELD_MAX_LENGTH) {
      len = CUPERTINO_TEXT_FIELD_MAX_LENGTH - 1;
    }
    memcpy(field->text, text_utf8, len);
    field->text[len] = '\0';
    field->text_len = len;
  } else {
    field->text[0] = '\0';
    field->text_len = 0;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_text_field_set_placeholder(
    CupertinoTextField *field, const char *placeholder_utf8) {
  if (field == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (placeholder_utf8) {
    cmp_usize len = strlen(placeholder_utf8);
    if (len >= CUPERTINO_TEXT_FIELD_MAX_LENGTH) {
      len = CUPERTINO_TEXT_FIELD_MAX_LENGTH - 1;
    }
    memcpy(field->placeholder, placeholder_utf8, len);
    field->placeholder[len] = '\0';
    field->placeholder_len = len;
  } else {
    field->placeholder[0] = '\0';
    field->placeholder_len = 0;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_text_field_update(CupertinoTextField *field,
                                                 double delta_time) {
  if (field == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (field->is_focused) {
    field->cursor_blink_time += delta_time;
    /* ~1 second blink cycle (500ms on, 500ms off) */
    while (field->cursor_blink_time >= 1.0) {
      field->cursor_blink_time -= 1.0;
    }
  } else {
    field->cursor_blink_time = 0.0;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_text_field_paint(const CupertinoTextField *field,
                                                CMPPaintContext *ctx) {
  CMPColor bg_color;
  CMPColor border_color;
  CMPColor text_color;
  CMPColor placeholder_color;
  CMPColor cursor_color;
  CMPColor focus_ring_color;

  CMPScalar corner_radius = 5.0f; /* Standard rounding for iOS text field */
  CMPScalar padding_x = 10.0f;
  CMPScalar content_x;

  CMPTextStyle txt_style;
  CMPHandle font = {0};
  CMPTextMetrics metrics = {0};

  if (field == NULL || ctx == NULL || ctx->gfx == NULL ||
      ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  /* Colors setup based on mode */
  if (field->is_dark_mode) {
    bg_color.r = 0.11f;
    bg_color.g = 0.11f;
    bg_color.b = 0.118f;
    bg_color.a = 1.0f;
    border_color.r = 0.2f;
    border_color.g = 0.2f;
    border_color.b = 0.2f;
    border_color.a = 1.0f;
    text_color.r = 1.0f;
    text_color.g = 1.0f;
    text_color.b = 1.0f;
    text_color.a = 1.0f;
    placeholder_color.r = 0.5f;
    placeholder_color.g = 0.5f;
    placeholder_color.b = 0.55f;
    placeholder_color.a = 1.0f;
  } else {
    bg_color.r = 1.0f;
    bg_color.g = 1.0f;
    bg_color.b = 1.0f;
    bg_color.a = 1.0f;
    border_color.r = 0.8f;
    border_color.g = 0.8f;
    border_color.b = 0.8f;
    border_color.a = 1.0f;
    text_color.r = 0.0f;
    text_color.g = 0.0f;
    text_color.b = 0.0f;
    text_color.a = 1.0f;
    placeholder_color.r = 0.78f;
    placeholder_color.g = 0.78f;
    placeholder_color.b = 0.8f;
    placeholder_color.a = 1.0f;
  }

  /* System Blue for cursor and focus rings */
  cupertino_color_get_system(CUPERTINO_COLOR_BLUE, field->is_dark_mode,
                             &cursor_color);
  focus_ring_color = cursor_color;
  focus_ring_color.a = 0.5f;

  /* Base Styling */
  if (field->style == CUPERTINO_TEXT_FIELD_STYLE_ROUNDED_RECT) {
    if (ctx->gfx->vtable->draw_rect) {
      /* Draw outer border/focus ring first if focused */
      if (field->is_focused && !field->is_disabled) {
        CMPRect ring = field->bounds;
        ring.x -= 2.0f;
        ring.y -= 2.0f;
        ring.width += 4.0f;
        ring.height += 4.0f;
        ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &ring, focus_ring_color,
                                    corner_radius + 2.0f);
      }

      /* Background */
      ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &field->bounds, bg_color,
                                  corner_radius);

      /* Border */
      if (!field->is_focused && !field->is_dark_mode) {
        CMPRect bdr = field->bounds;
        /* Simulated inner stroke by drawing a slightly smaller rect?
           Assuming draw_rect handles fills, we just draw the border manually or
           rely on a draw_rounded_rect variant if it existed. Using simple line
           strokes around the edges as fallback if needed, but skipping for
           brevity unless we have stroke API. */

        /* Drawing a thin border by drawing background color slightly inset */
        ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &bdr, border_color,
                                    corner_radius);
        bdr.x += 1.0f;
        bdr.y += 1.0f;
        bdr.width -= 2.0f;
        bdr.height -= 2.0f;
        ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &bdr, bg_color,
                                    corner_radius - 1.0f);
      }
    }
  } else if (field->style == CUPERTINO_TEXT_FIELD_STYLE_LINE) {
    if (ctx->gfx->vtable->draw_line) {
      CMPColor line_color = field->is_focused ? cursor_color : border_color;
      ctx->gfx->vtable->draw_line(ctx->gfx->ctx, field->bounds.x,
                                  field->bounds.y + field->bounds.height,
                                  field->bounds.x + field->bounds.width,
                                  field->bounds.y + field->bounds.height,
                                  line_color, field->is_focused ? 2.0f : 1.0f);
    }
    padding_x = 0.0f; /* Line style usually flush left */
  }

  content_x = field->bounds.x + padding_x;

  /* Text & Placeholder */
  memset(&txt_style, 0, sizeof(txt_style));
  txt_style.size_px = 15.0f;
  txt_style.weight = 400;

  if (cmp_text_font_create((void *)&field->text_backend, &txt_style, &font) ==
      CMP_OK) {
    CMPScalar text_width = 0.0f;
    CMPScalar text_y;

    /* Measure to find baseline */
    cmp_text_measure_utf8((void *)&field->text_backend, font, "Ay", 2, 0,
                          &metrics);
    text_y = field->bounds.y + (field->bounds.height / 2.0f) -
             (metrics.height / 2.0f) + metrics.baseline;

    if (field->text_len > 0) {
      /* Draw Text */
      cmp_text_measure_utf8((void *)&field->text_backend, font, field->text,
                            field->text_len, 0, &metrics);
      text_width = metrics.width;
      cmp_text_draw_utf8_gfx(ctx->gfx, font, field->text, field->text_len, 0,
                             content_x, text_y, text_color);
    } else if (field->placeholder_len > 0) {
      /* Draw Placeholder */
      cmp_text_draw_utf8_gfx(ctx->gfx, font, field->placeholder,
                             field->placeholder_len, 0, content_x, text_y,
                             placeholder_color);
    }

    /* Cursor */
    if (field->is_focused && field->cursor_blink_time < 0.5) {
      if (ctx->gfx->vtable->draw_line) {
        CMPScalar cx = content_x + text_width;
        /* Typical iOS cursor is blue and slightly taller than the caps height
         */
        CMPScalar cy1 = field->bounds.y + (field->bounds.height / 2.0f) - 8.0f;
        CMPScalar cy2 = field->bounds.y + (field->bounds.height / 2.0f) + 8.0f;

        ctx->gfx->vtable->draw_line(ctx->gfx->ctx, cx, cy1, cx, cy2,
                                    cursor_color, 2.0f);
      }
    }

    cmp_text_font_destroy((void *)&field->text_backend, font);
  }

  return CMP_OK;
}
