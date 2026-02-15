#include "app/demo_app.h"
#include "cmpc/cmp_backend_web.h"
#include <emscripten.h>
#include <stdio.h>

struct WebState {
  CMPWebBackend *backend;
  CMPWS *ws;
  CMPGfx *gfx;
  CMPEnv *env;
  CMPHandle window;
  DemoApp *app;
} g_state;

void loop_iter() {
  CMPInputEvent event;
  CMPBool has_event;

  while (1) {
    g_state.ws->vtable->poll_event(g_state.ws->ctx, &event, &has_event);
    if (!has_event)
      break;
    CMPBool handled;
    demo_app_handle_event(g_state.app, &event, &handled);
  }

  demo_app_update(g_state.app, 0.016);

  int w, h;
  g_state.ws->vtable->get_window_size(g_state.ws->ctx, g_state.window, &w, &h);
  demo_app_render(g_state.app, g_state.gfx, g_state.window, w, h, 1.0f);
}

int main() {
  CMPAllocator alloc;
  CMPWebBackendConfig config;

  cmp_web_backend_config_init(&config);
  if (cmp_web_backend_create(&config, &g_state.backend) != CMP_OK)
    return 1;

  cmp_web_backend_get_ws(g_state.backend, &g_state.ws);
  cmp_web_backend_get_gfx(g_state.backend, &g_state.gfx);

  // We don't usually create a "window" in web the same way, but the abstraction
  // holds
  CMPWSWindowConfig wincfg = {0, 0, "Web",
                              0}; // 0,0 lets browser determine canvas size
  g_state.ws->vtable->create_window(g_state.ws->ctx, &wincfg, &g_state.window);

  cmp_get_default_allocator(&alloc);
  demo_app_create(&alloc, &g_state.app);
  demo_app_init_resources(g_state.app, g_state.gfx, NULL);

  emscripten_set_main_loop(loop_iter, 0, 1);
  return 0;
}