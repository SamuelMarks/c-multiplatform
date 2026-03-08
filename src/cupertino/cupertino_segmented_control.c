#include "cupertino/cupertino_segmented_control.h"
#include <string.h>

CMP_API int CMP_CALL cupertino_segmented_control_init(CupertinoSegmentedControl *control, const CMPTextBackend *text_backend) {
    if (control == NULL || text_backend == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    memset(control, 0, sizeof(*control));
    control->text_backend = *text_backend;
    
    control->bounds.width = 200; 
    control->bounds.height = 32; /* Standard iOS height */
    
    control->selected_index = 0;
    control->pressed_index = -1;
    control->is_dark_mode = CMP_FALSE;
    control->is_disabled = CMP_FALSE;

    cmp_anim_controller_init(&control->selection_anim);
    control->selection_anim.value = 0.0f; /* Corresponds to index 0 */

    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_segmented_control_add_segment(CupertinoSegmentedControl *control, const char *label_utf8) {
    if (control == NULL || label_utf8 == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    if (control->segment_count >= CUPERTINO_SEGMENTED_CONTROL_MAX_SEGMENTS) {
        return CMP_ERR_OUT_OF_MEMORY;
    }

    control->segments[control->segment_count] = label_utf8;
    control->segment_lengths[control->segment_count] = strlen(label_utf8);
    control->segment_count++;

    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_segmented_control_set_selected(CupertinoSegmentedControl *control, cmp_i32 index, CMPBool animated) {
    CMPScalar target;
    
    if (control == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    if (index < 0 || (cmp_usize)index >= control->segment_count) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    control->selected_index = index;
    target = (CMPScalar)index;

    if (animated) {
        /* Apple uses a responsive spring for segment transitions */
        cmp_anim_controller_start_spring(&control->selection_anim, control->selection_anim.value, target, 400.0f, 30.0f, 1.0f);
    } else {
        cmp_anim_controller_stop(&control->selection_anim);
        control->selection_anim.value = target;
    }

    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_segmented_control_update(CupertinoSegmentedControl *control, double delta_time) {
    CMPBool finished;
    CMPScalar out_val;
    
    if (control == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    cmp_anim_controller_step(&control->selection_anim, (CMPScalar)delta_time, &out_val, &finished);

    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_segmented_control_paint(const CupertinoSegmentedControl *control, CMPPaintContext *ctx) {
    CMPColor bg_color;
    CMPColor thumb_color;
    CMPColor thumb_shadow_color;
    CMPColor divider_color;
    CMPColor text_color_selected;
    CMPColor text_color_normal;
    CMPScalar corner_radius;
    CMPScalar padding = 2.0f;
    CMPScalar segment_w;
    CMPScalar thumb_x;
    CMPRect thumb_rect;
    cmp_usize i;
    
    if (control == NULL || ctx == NULL || ctx->gfx == NULL || ctx->gfx->vtable == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    if (control->segment_count == 0) {
        return CMP_OK;
    }

    corner_radius = 8.0f; /* iOS standard radius for segmented control */
    segment_w = (control->bounds.width - (padding * 2.0f)) / (CMPScalar)control->segment_count;

    /* Colors */
    if (control->is_dark_mode) {
        bg_color.r = 0.11f; bg_color.g = 0.11f; bg_color.b = 0.118f; bg_color.a = 1.0f; /* iOS 13+ dark mode background */
        thumb_color.r = 0.38f; thumb_color.g = 0.38f; thumb_color.b = 0.4f; thumb_color.a = 1.0f;
        divider_color.r = 0.38f; divider_color.g = 0.38f; divider_color.b = 0.4f; divider_color.a = 0.5f;
        text_color_selected.r = 1.0f; text_color_selected.g = 1.0f; text_color_selected.b = 1.0f; text_color_selected.a = 1.0f;
        text_color_normal.r = 1.0f; text_color_normal.g = 1.0f; text_color_normal.b = 1.0f; text_color_normal.a = 1.0f;
    } else {
        bg_color.r = 0.93f; bg_color.g = 0.93f; bg_color.b = 0.937f; bg_color.a = 1.0f; /* iOS 13+ light mode background */
        thumb_color.r = 1.0f; thumb_color.g = 1.0f; thumb_color.b = 1.0f; thumb_color.a = 1.0f;
        divider_color.r = 0.55f; divider_color.g = 0.55f; divider_color.b = 0.55f; divider_color.a = 0.5f;
        text_color_selected.r = 0.0f; text_color_selected.g = 0.0f; text_color_selected.b = 0.0f; text_color_selected.a = 1.0f;
        text_color_normal.r = 0.0f; text_color_normal.g = 0.0f; text_color_normal.b = 0.0f; text_color_normal.a = 1.0f;
    }

    /* Track Background */
    if (ctx->gfx->vtable->draw_rect) {
        ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &control->bounds, bg_color, corner_radius);
    }

    /* Dividers (drawn only where thumb is NOT present, typically) */
    /* Simplified for this implementation */
    if (ctx->gfx->vtable->draw_line) {
        for (i = 1; i < control->segment_count; i++) {
            CMPScalar div_x = control->bounds.x + padding + (CMPScalar)i * segment_w;
            /* Don't draw divider if it's right next to the selected segment to mimic iOS behavior closely */
            /* Simplified: just draw all dividers, the thumb will draw over them */
            ctx->gfx->vtable->draw_line(ctx->gfx->ctx, div_x, control->bounds.y + 6.0f, div_x, control->bounds.y + control->bounds.height - 6.0f, divider_color, 1.0f);
        }
    }

    /* Thumb Position & Rect */
    thumb_x = control->bounds.x + padding + (control->selection_anim.value * segment_w);
    thumb_rect.x = thumb_x;
    thumb_rect.y = control->bounds.y + padding;
    thumb_rect.width = segment_w;
    thumb_rect.height = control->bounds.height - (padding * 2.0f);

    /* Thumb Shadow */
    thumb_shadow_color.r = 0.0f; thumb_shadow_color.g = 0.0f; thumb_shadow_color.b = 0.0f; thumb_shadow_color.a = 0.15f;
    
    if (ctx->gfx->vtable->draw_rect) {
        CMPRect shadow_rect = thumb_rect;
        shadow_rect.y += 1.0f; /* Slight drop shadow */
        ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &shadow_rect, thumb_shadow_color, corner_radius - padding);
        /* Thumb */
        ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &thumb_rect, thumb_color, corner_radius - padding);
    }

    /* Labels */
    for (i = 0; i < control->segment_count; i++) {
        CMPTextStyle text_style;
        CMPHandle font = {0};
        CMPTextMetrics metrics = {0};
        
        memset(&text_style, 0, sizeof(text_style));
        text_style.size_px = 13.0f;
        text_style.weight = 500; /* Medium for selected, Regular for unselected usually, but using Medium for both for simplicity */
        text_style.color = (i == (cmp_usize)control->selected_index) ? text_color_selected : text_color_normal;

        if (cmp_text_font_create((void*)&control->text_backend, &text_style, &font) == CMP_OK) {
            cmp_text_measure_utf8((void*)&control->text_backend, font, control->segments[i], control->segment_lengths[i], 0, &metrics);
            
            CMPScalar seg_center_x = control->bounds.x + padding + ((CMPScalar)i * segment_w) + (segment_w / 2.0f);
            CMPScalar text_x = seg_center_x - (metrics.width / 2.0f);
            CMPScalar text_y = control->bounds.y + (control->bounds.height / 2.0f) - (metrics.height / 2.0f) + metrics.baseline;

            cmp_text_draw_utf8_gfx(ctx->gfx, font, control->segments[i], control->segment_lengths[i], 0, text_x, text_y, text_style.color);
            
            cmp_text_font_destroy((void*)&control->text_backend, font);
        }
    }

    return CMP_OK;
}
