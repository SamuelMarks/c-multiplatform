/* clang-format off */
#include "cupertino/cupertino_mac_sidebar.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL cupertino_mac_sidebar_init(
    CupertinoMacSidebar *sidebar, const CMPTextBackend *text_backend) {
  if (sidebar == NULL || text_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(sidebar, 0, sizeof(*sidebar));
  sidebar->text_backend = *text_backend;

  sidebar->bounds.width = 250;
  sidebar->bounds.height = 600;

  sidebar->is_dark_mode = CMP_FALSE;
  sidebar->is_window_active = CMP_TRUE;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_mac_sidebar_add_item(
    CupertinoMacSidebar *sidebar, const char *title_utf8,
    CMPBool is_section_header) {
  CupertinoMacSidebarRow *row;

  if (sidebar == NULL || title_utf8 == NULL)
    return CMP_ERR_INVALID_ARGUMENT;
  if (sidebar->row_count >= CUPERTINO_MAC_SIDEBAR_MAX_ROWS)
    return CMP_ERR_OUT_OF_MEMORY;

  row = &sidebar->rows[sidebar->row_count++];
  memset(row, 0, sizeof(*row));

  {
    cmp_usize len = strlen(title_utf8);
    if (len >= CUPERTINO_MAC_SIDEBAR_MAX_TEXT)
      len = CUPERTINO_MAC_SIDEBAR_MAX_TEXT - 1;
    memcpy(row->title_utf8, title_utf8, len);
    row->title_len = len;
  }

  row->is_section_header = is_section_header;
  row->is_selected = CMP_FALSE;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_mac_sidebar_set_selected(
    CupertinoMacSidebar *sidebar, cmp_i32 index) {
  cmp_usize i;
  if (sidebar == NULL)
    return CMP_ERR_INVALID_ARGUMENT;

  if (index >= 0 && (cmp_usize)index >= sidebar->row_count) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (index >= 0 && sidebar->rows[index].is_section_header) {
    return CMP_ERR_INVALID_ARGUMENT; /* Cannot select header */
  }

  for (i = 0; i < sidebar->row_count; i++) {
    sidebar->rows[i].is_selected = ((cmp_i32)i == index) ? CMP_TRUE : CMP_FALSE;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_mac_sidebar_paint(
    const CupertinoMacSidebar *sidebar, CMPPaintContext *ctx) {
  CMPColor bg_color;
  CMPColor border_color;
  CMPColor sel_color_active;
  CMPColor sel_color_inactive;

  CMPColor text_color;
  CMPColor header_color;
  CMPColor sel_text_color;

  CMPScalar row_height = 28.0f;
  CMPScalar content_y;
  CMPScalar padding_x = 12.0f;
  cmp_usize i;

  if (sidebar == NULL || ctx == NULL || ctx->gfx == NULL ||
      ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  /* Colors representing macOS vibrancy */
  if (sidebar->is_dark_mode) {
    bg_color.r = 0.15f;
    bg_color.g = 0.15f;
    bg_color.b = 0.15f;
    bg_color.a = 0.95f; /* Needs real material blur */
    border_color.r = 0.0f;
    border_color.g = 0.0f;
    border_color.b = 0.0f;
    border_color.a = 1.0f;
    sel_color_active.r = 0.2f;
    sel_color_active.g = 0.4f;
    sel_color_active.b = 0.9f;
    sel_color_active.a = 1.0f;
    sel_color_inactive.r = 0.3f;
    sel_color_inactive.g = 0.3f;
    sel_color_inactive.b = 0.3f;
    sel_color_inactive.a = 1.0f;
    text_color.r = 1.0f;
    text_color.g = 1.0f;
    text_color.b = 1.0f;
    text_color.a = 0.85f;
    header_color.r = 1.0f;
    header_color.g = 1.0f;
    header_color.b = 1.0f;
    header_color.a = 0.5f;
    sel_text_color.r = 1.0f;
    sel_text_color.g = 1.0f;
    sel_text_color.b = 1.0f;
    sel_text_color.a = 1.0f;
  } else {
    bg_color.r = 0.95f;
    bg_color.g = 0.95f;
    bg_color.b = 0.95f;
    bg_color.a = 0.95f;
    border_color.r = 0.8f;
    border_color.g = 0.8f;
    border_color.b = 0.8f;
    border_color.a = 1.0f;
    sel_color_active.r = 0.0f;
    sel_color_active.g = 0.4f;
    sel_color_active.b = 0.8f;
    sel_color_active.a = 1.0f; /* macOS accent color */
    sel_color_inactive.r = 0.8f;
    sel_color_inactive.g = 0.8f;
    sel_color_inactive.b = 0.8f;
    sel_color_inactive.a = 1.0f;
    text_color.r = 0.0f;
    text_color.g = 0.0f;
    text_color.b = 0.0f;
    text_color.a = 0.85f;
    header_color.r = 0.0f;
    header_color.g = 0.0f;
    header_color.b = 0.0f;
    header_color.a = 0.4f;
    sel_text_color.r = 1.0f;
    sel_text_color.g = 1.0f;
    sel_text_color.b = 1.0f;
    sel_text_color.a = 1.0f;
  }

  if (!sidebar->is_window_active && !sidebar->is_dark_mode) {
    sel_text_color.r = 0.0f;
    sel_text_color.g = 0.0f;
    sel_text_color.b = 0.0f;
    sel_text_color.a = 0.85f;
  }

  /* Background */
  if (ctx->gfx->vtable->draw_rect) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &sidebar->bounds, bg_color,
                                0.0f);
  }

  /* Right border */
  if (ctx->gfx->vtable->draw_line) {
    ctx->gfx->vtable->draw_line(
        ctx->gfx->ctx, sidebar->bounds.x + sidebar->bounds.width - 1.0f,
        sidebar->bounds.y, sidebar->bounds.x + sidebar->bounds.width - 1.0f,
        sidebar->bounds.y + sidebar->bounds.height, border_color, 1.0f);
  }

  content_y = sidebar->bounds.y + 16.0f; /* Top padding */

  /* Rows */
  for (i = 0; i < sidebar->row_count; i++) {
    const CupertinoMacSidebarRow *row = &sidebar->rows[i];
    CMPRect row_rect;
    row_rect.x = sidebar->bounds.x + 8.0f;
    row_rect.y = content_y;
    row_rect.width =
        sidebar->bounds.width - 16.0f; /* 8px padding on both sides */
    row_rect.height = row->is_section_header ? 24.0f : row_height;

    /* Selection Background */
    if (row->is_selected && ctx->gfx->vtable->draw_rect) {
      CMPColor sc =
          sidebar->is_window_active ? sel_color_active : sel_color_inactive;
      ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &row_rect, sc,
                                  6.0f); /* macOS typical selection radius */
    }

    /* Text drawing */
    if (row->title_len > 0) {
      CMPTextStyle row_style;
      CMPHandle font = {0};
      CMPTextMetrics metrics = {0};
      memset(&row_style, 0, sizeof(row_style));

      if (row->is_section_header) {
        row_style.size_px = 11.0f;
        row_style.weight = 600; /* Semibold */
        row_style.color = header_color;
      } else {
        row_style.size_px = 13.0f;
        row_style.weight =
            row->is_selected ? 500 : 400; /* Medium if selected */
        row_style.color = row->is_selected ? sel_text_color : text_color;
      }

      if (cmp_text_font_create((void *)&sidebar->text_backend, &row_style,
                               &font) == CMP_OK) {
        CMPScalar text_y;
        CMPScalar indent;
        cmp_text_measure_utf8((void *)&sidebar->text_backend, font, "Ay", 2, 0,
                              &metrics);
        text_y = row_rect.y + (row_rect.height / 2.0f) -
                 (metrics.height / 2.0f) + metrics.baseline;

        /* Indent normal rows if they follow a section header? Standard is just
         * padding. */
        indent = row->is_section_header ? 0.0f : 8.0f;

        cmp_text_draw_utf8_gfx(ctx->gfx, font, row->title_utf8, row->title_len,
                               0, row_rect.x + padding_x + indent, text_y,
                               row_style.color);

        cmp_text_font_destroy((void *)&sidebar->text_backend, font);
      }
    }

    content_y += row_rect.height;
  }

  return CMP_OK;
}
