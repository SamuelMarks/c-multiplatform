#include "cupertino/cupertino_mac_traffic_lights.h"
#include <string.h>

CMP_API int CMP_CALL cupertino_mac_traffic_lights_init(CupertinoMacTrafficLights *lights) {
    if (lights == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    memset(lights, 0, sizeof(*lights));
    
    /* Standard macOS dimensions (each button is ~12px, separated by ~8px) */
    lights->bounds.width = 54; 
    lights->bounds.height = 14; 
    
    lights->is_dark_mode = CMP_FALSE;
    lights->is_window_active = CMP_TRUE;
    lights->is_hovered = CMP_FALSE;
    lights->pressed_button = -1;

    return CMP_OK;
}

CMP_API int CMP_CALL cupertino_mac_traffic_lights_paint(const CupertinoMacTrafficLights *lights, CMPPaintContext *ctx) {
    CMPColor colors_active[3];
    CMPColor colors_inactive[3];
    CMPColor colors_pressed[3];
    CMPColor icon_color;
    CMPColor outline_color;
    CMPScalar btn_size = 12.0f;
    CMPScalar gap = 8.0f;
    CMPScalar radius = btn_size / 2.0f;
    int i;
    
    if (lights == NULL || ctx == NULL || ctx->gfx == NULL || ctx->gfx->vtable == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    /* Red, Yellow, Green */
    colors_active[0].r = 1.0f; colors_active[0].g = 0.36f; colors_active[0].b = 0.33f; colors_active[0].a = 1.0f;
    colors_active[1].r = 1.0f; colors_active[1].g = 0.75f; colors_active[1].b = 0.18f; colors_active[1].a = 1.0f;
    colors_active[2].r = 0.16f; colors_active[2].g = 0.78f; colors_active[2].b = 0.25f; colors_active[2].a = 1.0f;

    /* Darker states when pressed */
    colors_pressed[0].r = 0.75f; colors_pressed[0].g = 0.25f; colors_pressed[0].b = 0.25f; colors_pressed[0].a = 1.0f;
    colors_pressed[1].r = 0.75f; colors_pressed[1].g = 0.55f; colors_pressed[1].b = 0.10f; colors_pressed[1].a = 1.0f;
    colors_pressed[2].r = 0.10f; colors_pressed[2].g = 0.55f; colors_pressed[2].b = 0.15f; colors_pressed[2].a = 1.0f;

    /* Inactive window state (grayed out) */
    if (lights->is_dark_mode) {
        colors_inactive[0].r = 0.3f; colors_inactive[0].g = 0.3f; colors_inactive[0].b = 0.3f; colors_inactive[0].a = 1.0f;
        colors_inactive[1] = colors_inactive[0];
        colors_inactive[2] = colors_inactive[0];
        outline_color.r = 0.1f; outline_color.g = 0.1f; outline_color.b = 0.1f; outline_color.a = 0.5f;
        icon_color.r = 0.0f; icon_color.g = 0.0f; icon_color.b = 0.0f; icon_color.a = 0.6f;
    } else {
        colors_inactive[0].r = 0.85f; colors_inactive[0].g = 0.85f; colors_inactive[0].b = 0.85f; colors_inactive[0].a = 1.0f;
        colors_inactive[1] = colors_inactive[0];
        colors_inactive[2] = colors_inactive[0];
        outline_color.r = 0.8f; outline_color.g = 0.8f; outline_color.b = 0.8f; outline_color.a = 0.5f;
        icon_color.r = 0.0f; icon_color.g = 0.0f; icon_color.b = 0.0f; icon_color.a = 0.5f;
    }

    /* Draw Buttons */
    for (i = 0; i < 3; i++) {
        CMPRect btn_rect;
        CMPColor draw_color;
        
        btn_rect.x = lights->bounds.x + (CMPScalar)i * (btn_size + gap);
        btn_rect.y = lights->bounds.y + (lights->bounds.height / 2.0f) - radius;
        btn_rect.width = btn_size;
        btn_rect.height = btn_size;

        if (lights->is_window_active || lights->is_hovered) {
            if (lights->pressed_button == i) {
                draw_color = colors_pressed[i];
            } else {
                draw_color = colors_active[i];
            }
        } else {
            draw_color = colors_inactive[i];
        }

        if (ctx->gfx->vtable->draw_rect) {
            ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &btn_rect, draw_color, radius);
            
            /* Slight outline for contrast */
            CMPRect outline_rect = btn_rect;
            /* Inner stroke simulation */
            /* In a full implementation we'd draw an outline loop */
        }

        /* Draw icons if hovered and active/hovered */
        if (lights->is_hovered && (lights->is_window_active || lights->is_hovered)) {
            CMPScalar cx = btn_rect.x + radius;
            CMPScalar cy = btn_rect.y + radius;
            
            if (ctx->gfx->vtable->draw_line) {
                if (i == 0) {
                    /* Close (X) */
                    CMPScalar s = 2.5f;
                    ctx->gfx->vtable->draw_line(ctx->gfx->ctx, cx - s, cy - s, cx + s, cy + s, icon_color, 1.2f);
                    ctx->gfx->vtable->draw_line(ctx->gfx->ctx, cx + s, cy - s, cx - s, cy + s, icon_color, 1.2f);
                } else if (i == 1) {
                    /* Minimize (-) */
                    CMPScalar s = 3.5f;
                    ctx->gfx->vtable->draw_line(ctx->gfx->ctx, cx - s, cy, cx + s, cy, icon_color, 1.5f);
                } else if (i == 2) {
                    /* Zoom (+) or fullscreen arrows, simplified to + for now */
                    CMPScalar s = 3.0f;
                    ctx->gfx->vtable->draw_line(ctx->gfx->ctx, cx - s, cy, cx + s, cy, icon_color, 1.2f);
                    ctx->gfx->vtable->draw_line(ctx->gfx->ctx, cx, cy - s, cx, cy + s, icon_color, 1.2f);
                }
            }
        }
    }

    return CMP_OK;
}
