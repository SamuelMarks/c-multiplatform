/* clang-format off */
#include "cupertino/cupertino_list_section.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL cupertino_list_section_init(
    CupertinoListSection *section, const CMPTextBackend *text_backend) {
  if (section == NULL || text_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(section, 0, sizeof(*section));
  section->text_backend = *text_backend;

  section->bounds.width = 300;
  /* height will depend on content */

  section->is_dark_mode = CMP_FALSE;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_list_section_set_headers(
    CupertinoListSection *section, const char *header_utf8,
    const char *footer_utf8) {
  if (section == NULL)
    return CMP_ERR_INVALID_ARGUMENT;

  if (header_utf8) {
    cmp_usize len = strlen(header_utf8);
    if (len >= CUPERTINO_LIST_MAX_TEXT)
      len = CUPERTINO_LIST_MAX_TEXT - 1;
    memcpy(section->header_utf8, header_utf8, len);
    section->header_utf8[len] = '\0';
    section->header_len = len;
  } else {
    section->header_len = 0;
  }

  if (footer_utf8) {
    cmp_usize len = strlen(footer_utf8);
    if (len >= CUPERTINO_LIST_MAX_TEXT)
      len = CUPERTINO_LIST_MAX_TEXT - 1;
    memcpy(section->footer_utf8, footer_utf8, len);
    section->footer_utf8[len] = '\0';
    section->footer_len = len;
  } else {
    section->footer_len = 0;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_list_section_add_row(
    CupertinoListSection *section, const char *title_utf8,
    const char *value_utf8, CMPBool has_chevron) {
  CupertinoListRow *row;

  if (section == NULL || title_utf8 == NULL)
    return CMP_ERR_INVALID_ARGUMENT;
  if (section->row_count >= CUPERTINO_LIST_MAX_ROWS)
    return CMP_ERR_OUT_OF_MEMORY;

  row = &section->rows[section->row_count++];
  memset(row, 0, sizeof(*row));

  {
    cmp_usize len = strlen(title_utf8);
    if (len >= CUPERTINO_LIST_MAX_TEXT)
      len = CUPERTINO_LIST_MAX_TEXT - 1;
    memcpy(row->title_utf8, title_utf8, len);
    row->title_len = len;
  }

  if (value_utf8) {
    cmp_usize len = strlen(value_utf8);
    if (len >= CUPERTINO_LIST_MAX_TEXT)
      len = CUPERTINO_LIST_MAX_TEXT - 1;
    memcpy(row->value_utf8, value_utf8, len);
    row->value_len = len;
  }

  row->has_chevron = has_chevron;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_list_section_paint(
    const CupertinoListSection *section, CMPPaintContext *ctx) {
  CMPColor bg_color;
  CMPColor pressed_color;
  CMPColor divider_color;
  CMPColor title_color;
  CMPColor value_color;
  CMPColor header_color;
  CMPColor chevron_color;

  CMPScalar row_height = 44.0f;
  CMPScalar corner_radius = 10.0f;
  CMPScalar content_y;
  CMPScalar padding_x = 16.0f;
  cmp_usize i;

  if (section == NULL || ctx == NULL || ctx->gfx == NULL ||
      ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  /* Colors */
  if (section->is_dark_mode) {
    bg_color.r = 0.11f;
    bg_color.g = 0.11f;
    bg_color.b = 0.118f;
    bg_color.a = 1.0f;
    pressed_color.r = 0.15f;
    pressed_color.g = 0.15f;
    pressed_color.b = 0.15f;
    pressed_color.a = 1.0f;
    divider_color.r = 0.2f;
    divider_color.g = 0.2f;
    divider_color.b = 0.2f;
    divider_color.a = 1.0f;
    title_color.r = 1.0f;
    title_color.g = 1.0f;
    title_color.b = 1.0f;
    title_color.a = 1.0f;
    value_color.r = 0.55f;
    value_color.g = 0.55f;
    value_color.b = 0.55f;
    value_color.a = 1.0f;
    header_color.r = 0.5f;
    header_color.g = 0.5f;
    header_color.b = 0.55f;
    header_color.a = 1.0f;
    chevron_color.r = 0.3f;
    chevron_color.g = 0.3f;
    chevron_color.b = 0.33f;
    chevron_color.a = 1.0f;
  } else {
    bg_color.r = 1.0f;
    bg_color.g = 1.0f;
    bg_color.b = 1.0f;
    bg_color.a = 1.0f;
    pressed_color.r = 0.9f;
    pressed_color.g = 0.9f;
    pressed_color.b = 0.9f;
    pressed_color.a = 1.0f;
    divider_color.r = 0.85f;
    divider_color.g = 0.85f;
    divider_color.b = 0.85f;
    divider_color.a = 1.0f;
    title_color.r = 0.0f;
    title_color.g = 0.0f;
    title_color.b = 0.0f;
    title_color.a = 1.0f;
    value_color.r = 0.55f;
    value_color.g = 0.55f;
    value_color.b = 0.57f;
    value_color.a = 1.0f;
    header_color.r = 0.45f;
    header_color.g = 0.45f;
    header_color.b = 0.47f;
    header_color.a = 1.0f;
    chevron_color.r = 0.78f;
    chevron_color.g = 0.78f;
    chevron_color.b = 0.8f;
    chevron_color.a = 1.0f;
  }

  content_y = section->bounds.y;

  /* Header */
  if (section->header_len > 0) {
    CMPTextStyle h_style;
    CMPHandle font = {0};
    CMPTextMetrics metrics = {0};
    memset(&h_style, 0, sizeof(h_style));
    h_style.size_px = 13.0f;
    h_style.weight = 400; /* Regular, all caps usually but we'll print as is */

    if (cmp_text_font_create((void *)&section->text_backend, &h_style, &font) ==
        CMP_OK) {
      cmp_text_measure_utf8((void *)&section->text_backend, font, "Ay", 2, 0,
                            &metrics);
      cmp_text_draw_utf8_gfx(ctx->gfx, font, section->header_utf8,
                             section->header_len, 0,
                             section->bounds.x + padding_x,
                             content_y + metrics.baseline, header_color);
      content_y += metrics.height + 8.0f;
      cmp_text_font_destroy((void *)&section->text_backend, font);
    }
  }

  /* List Background */
  if (section->row_count > 0 && ctx->gfx->vtable->draw_rect) {
    CMPRect list_rect = section->bounds;
    list_rect.y = content_y;
    list_rect.height = (CMPScalar)section->row_count * row_height;
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &list_rect, bg_color,
                                corner_radius);
  }

  /* Rows */
  for (i = 0; i < section->row_count; i++) {
    const CupertinoListRow *row = &section->rows[i];
    CMPRect row_rect;
    row_rect.x = section->bounds.x;
    row_rect.y = content_y;
    row_rect.width = section->bounds.width;
    row_rect.height = row_height;

    /* Highlight pressed row */
    if (row->is_pressed && ctx->gfx->vtable->draw_rect) {
      /* Since we don't have clipping masks active, we just draw the rect.
         In a real impl, we'd clip it to the rounded section bounds. */
      ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &row_rect, pressed_color,
                                  0.0f);
    }

    /* Divider (Not on the first row, drawn at top of current row) */
    if (i > 0 && ctx->gfx->vtable->draw_line) {
      ctx->gfx->vtable->draw_line(ctx->gfx->ctx, row_rect.x + padding_x,
                                  row_rect.y, row_rect.x + row_rect.width,
                                  row_rect.y, divider_color, 1.0f);
    }

    /* Text drawing */
    {
      CMPTextStyle row_style;
      CMPHandle font = {0};
      CMPTextMetrics metrics = {0};
      memset(&row_style, 0, sizeof(row_style));
      row_style.size_px = 17.0f; /* Standard body text */
      row_style.weight = 400;

      if (cmp_text_font_create((void *)&section->text_backend, &row_style,
                               &font) == CMP_OK) {
        CMPScalar text_y;
        CMPScalar right_x;
        cmp_text_measure_utf8((void *)&section->text_backend, font, "Ay", 2, 0,
                              &metrics);
        text_y = row_rect.y + (row_height / 2.0f) - (metrics.height / 2.0f) +
                 metrics.baseline;

        /* Title */
        cmp_text_draw_utf8_gfx(ctx->gfx, font, row->title_utf8, row->title_len,
                               0, row_rect.x + padding_x, text_y, title_color);

        /* Value & Chevron positioning */
        right_x = row_rect.x + row_rect.width - padding_x;

        if (row->has_chevron && ctx->gfx->vtable->draw_line) {
          /* Draw Chevron > */
          CMPScalar cx = right_x - 6.0f;
          CMPScalar cy = row_rect.y + (row_height / 2.0f);
          CMPScalar size = 5.0f;
          ctx->gfx->vtable->draw_line(ctx->gfx->ctx, cx, cy - size, cx + size,
                                      cy, chevron_color, 2.0f);
          ctx->gfx->vtable->draw_line(ctx->gfx->ctx, cx + size, cy, cx,
                                      cy + size, chevron_color, 2.0f);
          right_x -= 16.0f; /* Space for chevron */
        }

        if (row->value_len > 0) {
          /* Right aligned value */
          CMPTextMetrics v_metrics = {0};
          cmp_text_measure_utf8((void *)&section->text_backend, font,
                                row->value_utf8, row->value_len, 0, &v_metrics);
          cmp_text_draw_utf8_gfx(ctx->gfx, font, row->value_utf8,
                                 row->value_len, 0, right_x - v_metrics.width,
                                 text_y, value_color);
        }

        cmp_text_font_destroy((void *)&section->text_backend, font);
      }
    }

    content_y += row_height;
  }

  /* Footer */
  if (section->footer_len > 0) {
    CMPTextStyle f_style;
    CMPHandle font = {0};
    CMPTextMetrics metrics = {0};
    content_y += 8.0f;
    memset(&f_style, 0, sizeof(f_style));
    f_style.size_px = 13.0f;
    f_style.weight = 400;

    if (cmp_text_font_create((void *)&section->text_backend, &f_style, &font) ==
        CMP_OK) {
      cmp_text_measure_utf8((void *)&section->text_backend, font, "Ay", 2, 0,
                            &metrics);
      cmp_text_draw_utf8_gfx(ctx->gfx, font, section->footer_utf8,
                             section->footer_len, 0,
                             section->bounds.x + padding_x,
                             content_y + metrics.baseline, header_color);
      cmp_text_font_destroy((void *)&section->text_backend, font);
    }
  }

  return CMP_OK;
}
