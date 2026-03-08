#include "cupertino/cupertino_progress_bar.h"
#include <string.h>

CMP_API int CMP_CALL cupertino_progress_bar_init(CupertinoProgressBar *bar) {
    if (bar == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    memset(bar, 0, sizeof(*bar));
    
    bar->bounds.width = 150; 
    bar->bounds.height = 4; /* Standard iOS progress bar height */
    
    bar->progress = 0.0f;
    bar->is_dark_mode = CMP_FALSE;

    cupertino_color_get_system(CUPERTINO_COLOR_BLUE, CMP_FALSE, &bar->progress_color);
    /* Track color is usually a very light gray */
    bar->track_color.r = 0.9f; bar->track_color.g = 0.9f; bar->track_color.b = 0.9f; bar->track_color.a = 1.0f;

    cmp_anim_controller_init(&bar->anim);
    bar->anim.value = 0.0f;

    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_progress_bar_set_progress(CupertinoProgressBar *bar, CMPScalar progress, CMPBool animated) {
    if (bar == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    bar->progress = progress;

    if (animated) {
        /* Standard spring for progress */
        cmp_anim_controller_start_spring(&bar->anim, bar->anim.value, progress, 200.0f, 20.0f, 1.0f);
    } else {
        cmp_anim_controller_stop(&bar->anim);
        bar->anim.value = progress;
    }

    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_progress_bar_update(CupertinoProgressBar *bar, double delta_time) {
    CMPBool finished;
    CMPScalar out_val;
    
    if (bar == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    cmp_anim_controller_step(&bar->anim, (CMPScalar)delta_time, &out_val, &finished);

    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_progress_bar_paint(const CupertinoProgressBar *bar, CMPPaintContext *ctx) {
    CMPColor draw_track_color;
    CMPRect track_rect;
    CMPRect fill_rect;
    CMPScalar radius;
    CMPScalar current_progress;
    
    if (bar == NULL || ctx == NULL || ctx->gfx == NULL || ctx->gfx->vtable == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    radius = bar->bounds.height / 2.0f;
    current_progress = bar->anim.value;

    if (current_progress < 0.0f) current_progress = 0.0f;
    if (current_progress > 1.0f) current_progress = 1.0f;

    draw_track_color = bar->track_color;
    if (bar->is_dark_mode) {
        /* Darker track for dark mode */
        draw_track_color.r = 0.2f; draw_track_color.g = 0.2f; draw_track_color.b = 0.2f;
    }

    /* Background Track */
    track_rect = bar->bounds;
    
    /* Fill */
    fill_rect = bar->bounds;
    fill_rect.width = bar->bounds.width * current_progress;

    if (ctx->gfx->vtable->draw_rect) {
        /* Draw Track */
        ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &track_rect, draw_track_color, radius);
        
        /* Draw Fill (if visible) */
        if (fill_rect.width > 0.1f) {
            ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &fill_rect, bar->progress_color, radius);
        }
    }

    return CMP_OK;
}
