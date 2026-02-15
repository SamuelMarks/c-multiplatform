#ifndef DEMO_APP_H
#define DEMO_APP_H

#include <cmpc/cmp_api_env.h>
#include <cmpc/cmp_api_gfx.h>
#include <cmpc/cmp_api_ws.h>
#include <cmpc/cmp_core.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DemoApp DemoApp;

/** Create the application state. */
int demo_app_create(CMPAllocator *allocator, DemoApp **out_app);

/** Destroy the application state. */
int demo_app_destroy(DemoApp *app);

/**
 * Initialize resources (fonts, styles) that require backend interfaces.
 * Must be called before render/update.
 */
int demo_app_init_resources(DemoApp *app, CMPGfx *gfx, CMPEnv *env);

/** Handle input events from the window system. */
int demo_app_handle_event(DemoApp *app, const CMPInputEvent *event,
                          CMPBool *out_handled);

/** Update animation state. */
int demo_app_update(DemoApp *app, double dt_seconds);

/** Perform layout and generate render commands. */
int demo_app_render(DemoApp *app, CMPGfx *gfx, CMPHandle window, int width,
                    int height, float dpi);

#ifdef __cplusplus
}
#endif

#endif