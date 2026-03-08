#include "cupertino/cupertino_action_sheet.h"
#include <string.h>

#define CUPERTINO_SHEET_CORNER_RADIUS 13.0f
#define CUPERTINO_SHEET_MARGIN 8.0f
#define CUPERTINO_ACTION_HEIGHT 56.0f

CMP_API int CMP_CALL cupertino_action_sheet_style_init(CupertinoActionSheetStyle *style) {
    CupertinoTypographyScale typo;

    if (style == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    memset(style, 0, sizeof(*style));
    style->background_blur = CUPERTINO_BLUR_STYLE_THICK;
    style->is_dark_mode = CMP_FALSE;

    cupertino_color_get_system(CUPERTINO_COLOR_BLUE, CMP_FALSE, &style->tint_color);
    cupertino_color_get_system(CUPERTINO_COLOR_RED, CMP_FALSE, &style->destructive_color);

    cupertino_typography_scale_init(&typo);
    
    cupertino_typography_get_style(&typo, CUPERTINO_TYPOGRAPHY_FOOTNOTE, &style->title_style);
    style->title_style.weight = 600;
    style->title_style.color.r = 0.5f; style->title_style.color.g = 0.5f; style->title_style.color.b = 0.5f;

    cupertino_typography_get_style(&typo, CUPERTINO_TYPOGRAPHY_FOOTNOTE, &style->message_style);
    style->message_style.color.r = 0.5f; style->message_style.color.g = 0.5f; style->message_style.color.b = 0.5f;

    cupertino_typography_get_style(&typo, CUPERTINO_TYPOGRAPHY_TITLE_3, &style->action_style);
    style->action_style.color = style->tint_color;

    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_action_sheet_init(CupertinoActionSheet *sheet, const CMPTextBackend *text_backend) {
    if (sheet == NULL || text_backend == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    memset(sheet, 0, sizeof(*sheet));
    sheet->text_backend = *text_backend;
    sheet->animation_progress = 1.0f; /* Fully visible by default */
    cupertino_action_sheet_style_init(&sheet->style);
    
    sheet->widget.ctx = sheet;
    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_action_sheet_set_text(CupertinoActionSheet *sheet, 
                                                     const char *title_utf8, cmp_usize title_len,
                                                     const char *msg_utf8, cmp_usize msg_len) {
    if (sheet == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    sheet->title_utf8 = title_utf8;
    sheet->title_len = title_len;
    sheet->message_utf8 = msg_utf8;
    sheet->message_len = msg_len;
    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_action_sheet_add_action(CupertinoActionSheet *sheet, const CupertinoAction *action) {
    if (sheet == NULL || action == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (sheet->action_count >= CUPERTINO_ACTION_SHEET_MAX_ACTIONS) {
        return CMP_ERR_RANGE;
    }

    sheet->actions[sheet->action_count] = *action;
    sheet->action_count++;
    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_action_sheet_layout(CupertinoActionSheet *sheet, CMPRect bounds, CMPRect safe_area) {
    if (sheet == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    sheet->bounds = bounds;
    sheet->safe_area_insets = safe_area;

    return CMP_OK;
}

static void draw_text_centered(CMPPaintContext *ctx, const CMPTextBackend *backend, 
                               const char *text, cmp_usize len, 
                               CMPRect box, CMPTextStyle style) {
    CMPHandle font = {0};
    CMPTextMetrics metrics = {0};
    CMPScalar text_x, text_y;

    if (cmp_text_font_create(backend, &style, &font) == CMP_OK) {
        cmp_text_measure_utf8(backend, font, text, len, 0, &metrics);
        text_x = box.x + (box.width - metrics.width) / 2.0f;
        text_y = box.y + (box.height - metrics.height) / 2.0f + metrics.baseline;
        cmp_text_draw_utf8_gfx(ctx->gfx, font, text, len, 0, text_x, text_y, style.color);
        cmp_text_font_destroy(backend, font);
    }
}

CMP_API int CMP_CALL cupertino_action_sheet_paint(const CupertinoActionSheet *sheet, CMPPaintContext *ctx) {
    CupertinoBlurEffect blur_effect;
    CMPColor border_color, dim_bg;
    CMPScalar bottom_inset;
    CMPScalar content_height = 0.0f;
    CMPScalar header_height = 0.0f;
    CMPScalar current_y;
    CMPScalar sheet_y_offset;
    cmp_u32 i;
    int cancel_index = -1;
    cmp_u32 standard_action_count = 0;
    CMPRect main_group_rect, cancel_group_rect;
    CMPPath path = {0};
    CMPAllocator alloc;

    if (sheet == NULL || ctx == NULL || ctx->gfx == NULL || ctx->gfx->vtable == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    if (sheet->animation_progress <= 0.0f) {
        return CMP_OK;
    }

    bottom_inset = sheet->safe_area_insets.height > 0.0f ? sheet->safe_area_insets.height : CUPERTINO_SHEET_MARGIN;

    /* Calculate heights */
    if (sheet->title_utf8 || sheet->message_utf8) {
        header_height = 44.0f; /* Approx */
        content_height += header_height;
    }

    for (i = 0; i < sheet->action_count; i++) {
        if (sheet->actions[i].style == CUPERTINO_ACTION_STYLE_CANCEL) {
            cancel_index = (int)i;
        } else {
            standard_action_count++;
            content_height += CUPERTINO_ACTION_HEIGHT;
        }
    }

    /* Cancel button is in a separate group */
    if (cancel_index >= 0) {
        content_height += CUPERTINO_ACTION_HEIGHT + CUPERTINO_SHEET_MARGIN;
    }

    /* Total Y offset pushed down by animation progress */
    sheet_y_offset = sheet->bounds.height - (content_height + bottom_inset) * sheet->animation_progress;

    /* Draw full screen dim background */
    dim_bg.r = 0.0f; dim_bg.g = 0.0f; dim_bg.b = 0.0f; dim_bg.a = 0.4f * sheet->animation_progress;
    if (dim_bg.a > 0.0f && ctx->gfx->vtable->draw_rect) {
        ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &sheet->bounds, dim_bg, 0.0f);
    }

    if (cmp_get_default_allocator(&alloc) != CMP_OK) {
        return CMP_ERR_OUT_OF_MEMORY;
    }

    cupertino_blur_get_effect(sheet->style.background_blur, sheet->style.is_dark_mode, &blur_effect);
    
    border_color.r = sheet->style.is_dark_mode ? 0.3f : 0.7f;
    border_color.g = border_color.r;
    border_color.b = border_color.r;
    border_color.a = 0.5f;

    /* Main Group */
    main_group_rect.x = CUPERTINO_SHEET_MARGIN;
    main_group_rect.y = sheet_y_offset;
    main_group_rect.width = sheet->bounds.width - (CUPERTINO_SHEET_MARGIN * 2.0f);
    main_group_rect.height = header_height + (standard_action_count * CUPERTINO_ACTION_HEIGHT);

    if (main_group_rect.height > 0.0f && ctx->gfx->vtable->draw_path) {
        cmp_path_init(&path, &alloc, 32);
        cupertino_shape_append_squircle(&path, main_group_rect, CUPERTINO_SHEET_CORNER_RADIUS, CUPERTINO_SHEET_CORNER_RADIUS, CUPERTINO_SHEET_CORNER_RADIUS, CUPERTINO_SHEET_CORNER_RADIUS);
        ctx->gfx->vtable->draw_path(ctx->gfx->ctx, &path, blur_effect.tint_color);
        cmp_path_shutdown(&path);

        current_y = main_group_rect.y;

        if (header_height > 0.0f) {
            CMPRect header_box = {main_group_rect.x, current_y, main_group_rect.width, header_height};
            /* Simplification: Just draw title centered in header block */
            if (sheet->title_utf8) {
                draw_text_centered(ctx, &sheet->text_backend, sheet->title_utf8, sheet->title_len, header_box, sheet->style.title_style);
            }
            current_y += header_height;
        }

        for (i = 0; i < sheet->action_count; i++) {
            if ((int)i != cancel_index) {
                CMPRect action_box = {main_group_rect.x, current_y, main_group_rect.width, CUPERTINO_ACTION_HEIGHT};
                CMPTextStyle act_style = sheet->style.action_style;
                
                if (sheet->actions[i].style == CUPERTINO_ACTION_STYLE_DESTRUCTIVE) {
                    act_style.color = sheet->style.destructive_color;
                }
                
                /* Separator */
                if (current_y > main_group_rect.y && ctx->gfx->vtable->draw_line) {
                    ctx->gfx->vtable->draw_line(ctx->gfx->ctx, action_box.x, action_box.y, action_box.x + action_box.width, action_box.y, border_color, 1.0f);
                }

                draw_text_centered(ctx, &sheet->text_backend, sheet->actions[i].title_utf8, sheet->actions[i].title_len, action_box, act_style);
                current_y += CUPERTINO_ACTION_HEIGHT;
            }
        }
    }

    /* Cancel Group */
    if (cancel_index >= 0) {
        CMPTextStyle cancel_style = sheet->style.action_style;
        cancel_style.weight = 600; /* Cancel is bold */
        
        cancel_group_rect.x = CUPERTINO_SHEET_MARGIN;
        cancel_group_rect.y = main_group_rect.y + main_group_rect.height + CUPERTINO_SHEET_MARGIN;
        cancel_group_rect.width = sheet->bounds.width - (CUPERTINO_SHEET_MARGIN * 2.0f);
        cancel_group_rect.height = CUPERTINO_ACTION_HEIGHT;

        if (ctx->gfx->vtable->draw_path) {
            cmp_path_init(&path, &alloc, 32);
            cupertino_shape_append_squircle(&path, cancel_group_rect, CUPERTINO_SHEET_CORNER_RADIUS, CUPERTINO_SHEET_CORNER_RADIUS, CUPERTINO_SHEET_CORNER_RADIUS, CUPERTINO_SHEET_CORNER_RADIUS);
            
            /* Cancel button uses an opaque white/black instead of blur usually */
            dim_bg.r = sheet->style.is_dark_mode ? 0.11f : 1.0f;
            dim_bg.g = dim_bg.r; dim_bg.b = dim_bg.r; dim_bg.a = 1.0f;
            
            ctx->gfx->vtable->draw_path(ctx->gfx->ctx, &path, dim_bg);
            cmp_path_shutdown(&path);

            draw_text_centered(ctx, &sheet->text_backend, sheet->actions[cancel_index].title_utf8, sheet->actions[cancel_index].title_len, cancel_group_rect, cancel_style);
        }
    }

    return CMP_OK;
}
