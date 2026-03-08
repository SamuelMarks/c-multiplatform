#include "cupertino/cupertino_picker.h"
#include <string.h>

CMP_API int CMP_CALL cupertino_picker_init(CupertinoPicker *picker, const CMPTextBackend *text_backend) {
    if (picker == NULL || text_backend == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    memset(picker, 0, sizeof(*picker));
    picker->text_backend = *text_backend;
    
    picker->bounds.width = 300; 
    picker->bounds.height = 216; /* Typical iOS picker height */
    
    picker->selected_index = 0;
    picker->scroll_offset = 0.0f;
    picker->is_dark_mode = CMP_FALSE;

    cmp_anim_controller_init(&picker->scroll_anim);
    picker->scroll_anim.value = 0.0f;

    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_picker_add_item(CupertinoPicker *picker, const char *item_utf8) {
    if (picker == NULL || item_utf8 == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    if (picker->item_count >= CUPERTINO_PICKER_MAX_ITEMS) {
        return CMP_ERR_OUT_OF_MEMORY;
    }

    picker->items[picker->item_count] = item_utf8;
    picker->item_lengths[picker->item_count] = strlen(item_utf8);
    picker->item_count++;

    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_picker_set_selected(CupertinoPicker *picker, cmp_i32 index, CMPBool animated) {
    CMPScalar target;
    
    if (picker == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    if (index < 0 || (cmp_usize)index >= picker->item_count) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    picker->selected_index = index;
    target = (CMPScalar)index;

    if (animated) {
        cmp_anim_controller_start_spring(&picker->scroll_anim, picker->scroll_offset, target, 300.0f, 25.0f, 1.0f);
    } else {
        cmp_anim_controller_stop(&picker->scroll_anim);
        picker->scroll_anim.value = target;
        picker->scroll_offset = target;
    }

    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_picker_update(CupertinoPicker *picker, double delta_time) {
    CMPBool finished;
    CMPScalar out_val;
    
    if (picker == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    if (picker->scroll_anim.running) {
        cmp_anim_controller_step(&picker->scroll_anim, (CMPScalar)delta_time, &out_val, &finished);
        picker->scroll_offset = out_val;
    }

    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_picker_paint(const CupertinoPicker *picker, CMPPaintContext *ctx) {
    CMPColor bg_color;
    CMPColor selection_bg_color;
    CMPColor text_color;
    
    CMPScalar item_height = 32.0f;
    CMPScalar center_y;
    int i;
    
    if (picker == NULL || ctx == NULL || ctx->gfx == NULL || ctx->gfx->vtable == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    /* Colors */
    if (picker->is_dark_mode) {
        bg_color.r = 0.0f; bg_color.g = 0.0f; bg_color.b = 0.0f; bg_color.a = 0.0f; /* Usually transparent or slightly translucent over content */
        selection_bg_color.r = 0.2f; selection_bg_color.g = 0.2f; selection_bg_color.b = 0.2f; selection_bg_color.a = 0.5f;
        text_color.r = 1.0f; text_color.g = 1.0f; text_color.b = 1.0f; text_color.a = 1.0f;
    } else {
        bg_color.r = 1.0f; bg_color.g = 1.0f; bg_color.b = 1.0f; bg_color.a = 0.0f;
        selection_bg_color.r = 0.9f; selection_bg_color.g = 0.9f; selection_bg_color.b = 0.9f; selection_bg_color.a = 0.5f;
        text_color.r = 0.0f; text_color.g = 0.0f; text_color.b = 0.0f; text_color.a = 1.0f;
    }

    /* Background */
    if (bg_color.a > 0.0f && ctx->gfx->vtable->draw_rect) {
        ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &picker->bounds, bg_color, 0.0f);
    }

    center_y = picker->bounds.y + (picker->bounds.height / 2.0f);

    /* Selection Indicator */
    if (ctx->gfx->vtable->draw_rect) {
        CMPRect sel_rect;
        sel_rect.x = picker->bounds.x + 10.0f;
        sel_rect.y = center_y - (item_height / 2.0f);
        sel_rect.width = picker->bounds.width - 20.0f;
        sel_rect.height = item_height;
        ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &sel_rect, selection_bg_color, 8.0f);
    }

    /* Optional clip push if supported */
    if (ctx->gfx->vtable->push_clip) {
        ctx->gfx->vtable->push_clip(ctx->gfx->ctx, &picker->bounds);
    }

    /* Items */
    for (i = 0; i < (int)picker->item_count; i++) {
        CMPScalar dist_from_center_units = (CMPScalar)i - picker->scroll_offset;
        CMPScalar y_offset = dist_from_center_units * item_height;
        CMPScalar item_center_y = center_y + y_offset;
        
        /* Perspective/3D scaling effect simulation */
        CMPScalar scale = 1.0f - ((dist_from_center_units < 0 ? -dist_from_center_units : dist_from_center_units) * 0.1f);
        if (scale < 0.5f) scale = 0.5f;
        
        CMPScalar alpha = 1.0f - ((dist_from_center_units < 0 ? -dist_from_center_units : dist_from_center_units) * 0.3f);
        if (alpha < 0.0f) alpha = 0.0f;

        if (alpha > 0.0f && item_center_y >= picker->bounds.y && item_center_y <= picker->bounds.y + picker->bounds.height) {
            CMPTextStyle txt_style;
            CMPHandle font = {0};
            CMPTextMetrics metrics = {0};
            
            memset(&txt_style, 0, sizeof(txt_style));
            txt_style.size_px = 22.0f * scale; /* Standard size scaled */
            txt_style.weight = 400;
            
            CMPColor draw_color = text_color;
            draw_color.a *= alpha;

            if (cmp_text_font_create((void*)&picker->text_backend, &txt_style, &font) == CMP_OK) {
                cmp_text_measure_utf8((void*)&picker->text_backend, font, picker->items[i], picker->item_lengths[i], 0, &metrics);
                
                CMPScalar text_x = picker->bounds.x + (picker->bounds.width / 2.0f) - (metrics.width / 2.0f);
                CMPScalar text_y = item_center_y - (metrics.height / 2.0f) + metrics.baseline;

                cmp_text_draw_utf8_gfx(ctx->gfx, font, picker->items[i], picker->item_lengths[i], 0, text_x, text_y, draw_color);
                
                cmp_text_font_destroy((void*)&picker->text_backend, font);
            }
        }
    }

    if (ctx->gfx->vtable->pop_clip) {
        ctx->gfx->vtable->pop_clip(ctx->gfx->ctx);
    }

    return CMP_OK;
}
