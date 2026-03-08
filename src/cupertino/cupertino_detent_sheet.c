#include "cupertino/cupertino_detent_sheet.h"
#include <string.h>

CMP_API int CMP_CALL cupertino_detent_sheet_init(CupertinoDetentSheet *sheet) {
    if (sheet == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    memset(sheet, 0, sizeof(*sheet));
    
    sheet->screen_bounds.width = 375; 
    sheet->screen_bounds.height = 812;
    
    sheet->detent_hidden = 812;
    sheet->detent_medium = 406;
    sheet->detent_large = 40;
    
    sheet->current_detent = 0;
    sheet->is_dark_mode = CMP_FALSE;

    cmp_anim_controller_init(&sheet->slide_anim);
    sheet->slide_anim.value = sheet->detent_hidden;

    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_detent_sheet_set_bounds(CupertinoDetentSheet *sheet, CMPRect bounds) {
    if (sheet == NULL) return CMP_ERR_INVALID_ARGUMENT;
    
    sheet->screen_bounds = bounds;
    
    /* Recalculate detents */
    sheet->detent_hidden = bounds.height;
    sheet->detent_medium = bounds.height * 0.5f;
    sheet->detent_large = bounds.height * 0.05f; /* Leave a small gap at top */
    
    /* Snap without animation if bounds change */
    if (!sheet->slide_anim.running) {
        if (sheet->current_detent == 0) sheet->slide_anim.value = sheet->detent_hidden;
        else if (sheet->current_detent == 1) sheet->slide_anim.value = sheet->detent_medium;
        else if (sheet->current_detent == 2) sheet->slide_anim.value = sheet->detent_large;
    }
    
    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_detent_sheet_set_detent(CupertinoDetentSheet *sheet, cmp_i32 detent, CMPBool animated) {
    CMPScalar target;
    if (sheet == NULL) return CMP_ERR_INVALID_ARGUMENT;
    
    if (detent < 0 || detent > 2) return CMP_ERR_INVALID_ARGUMENT;
    
    sheet->current_detent = detent;
    
    if (detent == 0) target = sheet->detent_hidden;
    else if (detent == 1) target = sheet->detent_medium;
    else target = sheet->detent_large;

    if (animated) {
        /* Standard iOS sheet spring */
        cmp_anim_controller_start_spring(&sheet->slide_anim, sheet->slide_anim.value, target, 350.0f, 30.0f, 1.0f);
    } else {
        cmp_anim_controller_stop(&sheet->slide_anim);
        sheet->slide_anim.value = target;
    }
    
    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_detent_sheet_update(CupertinoDetentSheet *sheet, double delta_time) {
    CMPBool finished;
    CMPScalar out_val;
    if (sheet == NULL) return CMP_ERR_INVALID_ARGUMENT;
    
    cmp_anim_controller_step(&sheet->slide_anim, (CMPScalar)delta_time, &out_val, &finished);
    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_detent_sheet_paint(const CupertinoDetentSheet *sheet, CMPPaintContext *ctx) {
    CMPColor scrim_color;
    CMPColor bg_color;
    CMPColor grabber_color;
    CMPScalar corner_radius = 10.0f; /* iOS sheets have rounded tops */
    CMPRect sheet_rect;
    CMPScalar current_y;
    CMPScalar progress;
    
    if (sheet == NULL || ctx == NULL || ctx->gfx == NULL || ctx->gfx->vtable == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    current_y = sheet->slide_anim.value;

    /* Don't render anything if fully hidden */
    if (current_y >= sheet->screen_bounds.height) {
        return CMP_OK;
    }

    /* Colors */
    if (sheet->is_dark_mode) {
        bg_color.r = 0.11f; bg_color.g = 0.11f; bg_color.b = 0.118f; bg_color.a = 1.0f;
        grabber_color.r = 0.3f; grabber_color.g = 0.3f; grabber_color.b = 0.3f; grabber_color.a = 1.0f;
    } else {
        bg_color.r = 1.0f; bg_color.g = 1.0f; bg_color.b = 1.0f; bg_color.a = 1.0f;
        grabber_color.r = 0.8f; grabber_color.g = 0.8f; grabber_color.b = 0.8f; grabber_color.a = 1.0f;
    }
    
    /* Scrim alpha based on how far up it is */
    progress = 1.0f - (current_y / sheet->screen_bounds.height);
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;
    
    scrim_color.r = 0.0f; scrim_color.g = 0.0f; scrim_color.b = 0.0f;
    scrim_color.a = 0.4f * progress;

    /* Draw Scrim */
    if (scrim_color.a > 0.01f && ctx->gfx->vtable->draw_rect) {
        ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &sheet->screen_bounds, scrim_color, 0.0f);
    }

    /* Draw Sheet Background */
    sheet_rect.x = sheet->screen_bounds.x;
    sheet_rect.y = sheet->screen_bounds.y + current_y;
    sheet_rect.width = sheet->screen_bounds.width;
    sheet_rect.height = sheet->screen_bounds.height - current_y + corner_radius; /* Overshoot to hide bottom corners */

    if (ctx->gfx->vtable->draw_rect) {
        ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &sheet_rect, bg_color, corner_radius);
    }

    /* Draw Grabber Handle */
    if (ctx->gfx->vtable->draw_rect) {
        CMPRect grabber_rect;
        grabber_rect.width = 36.0f;
        grabber_rect.height = 5.0f;
        grabber_rect.x = sheet_rect.x + (sheet_rect.width / 2.0f) - (grabber_rect.width / 2.0f);
        grabber_rect.y = sheet_rect.y + 8.0f;
        ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &grabber_rect, grabber_color, grabber_rect.height / 2.0f);
    }

    return CMP_OK;
}
