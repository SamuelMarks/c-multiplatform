#include "cupertino/cupertino_date_picker.h"
#include <stdio.h>
#include <string.h>

CMP_API int CMP_CALL cupertino_date_picker_init(
    CupertinoDatePicker *picker, const CMPTextBackend *text_backend) {
  if (picker == NULL || text_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(picker, 0, sizeof(*picker));
  picker->text_backend = *text_backend;

  picker->bounds.width = 180;
  picker->bounds.height = 34;

  picker->year = 2026;
  picker->month = 3;
  picker->day = 8;
  picker->hour = 12;
  picker->minute = 0;

  picker->is_time_only = CMP_FALSE;
  picker->is_dark_mode = CMP_FALSE;
  picker->is_pressed = CMP_FALSE;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_date_picker_set_datetime(
    CupertinoDatePicker *picker, cmp_i32 year, cmp_i32 month, cmp_i32 day,
    cmp_i32 hour, cmp_i32 minute) {
  if (picker == NULL)
    return CMP_ERR_INVALID_ARGUMENT;

  picker->year = year;
  picker->month = month;
  picker->day = day;
  picker->hour = hour;
  picker->minute = minute;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_date_picker_paint(
    const CupertinoDatePicker *picker, CMPPaintContext *ctx) {
  CMPColor bg_color;
  CMPColor text_color;
  CMPScalar corner_radius = 6.0f;
  char display_text[64];
  cmp_usize text_len;

  if (picker == NULL || ctx == NULL || ctx->gfx == NULL ||
      ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  /* Colors */
  if (picker->is_dark_mode) {
    bg_color.r = 0.15f;
    bg_color.g = 0.15f;
    bg_color.b = 0.16f;
    bg_color.a = 1.0f;
    text_color.r = 1.0f;
    text_color.g = 1.0f;
    text_color.b = 1.0f;
    text_color.a = 1.0f;
  } else {
    bg_color.r = 0.92f;
    bg_color.g = 0.92f;
    bg_color.b = 0.94f;
    bg_color.a = 1.0f;
    text_color.r = 0.0f;
    text_color.g = 0.0f;
    text_color.b = 0.0f;
    text_color.a = 1.0f;
  }

  if (picker->is_pressed) {
    /* Dim slightly when pressed */
    if (picker->is_dark_mode) {
      bg_color.r *= 1.2f;
      bg_color.g *= 1.2f;
      bg_color.b *= 1.2f;
    } else {
      bg_color.r *= 0.9f;
      bg_color.g *= 0.9f;
      bg_color.b *= 0.9f;
    }
  }

  if (ctx->gfx->vtable->draw_rect) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &picker->bounds, bg_color,
                                corner_radius);
  }

  /* Format text */
  if (picker->is_time_only) {
    int h = picker->hour;
    const char *ampm = "AM";
    if (h >= 12) {
      ampm = "PM";
      if (h > 12)
        h -= 12;
    }
    if (h == 0)
      h = 12;

#if defined(NUM_FORMAT)
/* Just using standard sprintf for basic formatting in this simulation */
#endif
#if defined(_MSC_VER)
    sprintf_s(display_text, sizeof(display_text), "%d:%02d %s", h,
              picker->minute, ampm);
#else
    sprintf(display_text, "%d:%02d %s", h, picker->minute, ampm);
#endif
  } else {
    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    int m = picker->month - 1;
    if (m < 0)
      m = 0;
    if (m > 11)
      m = 11;

#if defined(_MSC_VER)
    sprintf_s(display_text, sizeof(display_text), "%s %d, %d", months[m],
              picker->day, picker->year);
#else
    sprintf(display_text, "%s %d, %d", months[m], picker->day, picker->year);
#endif
  }
  text_len = strlen(display_text);

  if (text_len > 0) {
    CMPTextStyle t_style;
    CMPHandle font = {0};
    CMPTextMetrics metrics = {0};

    memset(&t_style, 0, sizeof(t_style));
    t_style.size_px = 15.0f;
    t_style.weight = 400; /* Regular */

    if (cmp_text_font_create((void *)&picker->text_backend, &t_style, &font) ==
        CMP_OK) {
      cmp_text_measure_utf8((void *)&picker->text_backend, font, display_text,
                            text_len, 0, &metrics);

      CMPScalar tx = picker->bounds.x + (picker->bounds.width / 2.0f) -
                     (metrics.width / 2.0f);
      CMPScalar ty = picker->bounds.y + (picker->bounds.height / 2.0f) -
                     (metrics.height / 2.0f) + metrics.baseline;

      /* Time picker in iOS often colors the text differently (e.g. system
       * blue/red) but inline date is usually primary text colored */
      cmp_text_draw_utf8_gfx(ctx->gfx, font, display_text, text_len, 0, tx, ty,
                             text_color);
      cmp_text_font_destroy((void *)&picker->text_backend, font);
    }
  }

  return CMP_OK;
}
