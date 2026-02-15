#include "demo_app.h"
#include <android_native_app_glue.h>
#include <cmpc/cmp_backend_android.h>
#include <time.h>

typedef struct {
  struct android_app *app;
  CMPAndroidBackend *backend;
  CMPWS *ws;
  CMPGfx *gfx;
  CMPEnv *env;
  CMPHandle window;
  DemoApp *demo;
  int initialized;
} AndroidState;

static double get_time(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec / 1e9;
}

static void on_cmd(struct android_app *app, int32_t cmd) {
  AndroidState *state = (AndroidState *)app->userData;
  if (cmd == APP_CMD_INIT_WINDOW && app->window) {
    CMPAndroidBackendConfig cfg;
    cmp_android_backend_config_init(&cfg);
    cfg.java_vm = app->activity->vm;
    cfg.activity = app->activity->clazz;
    cfg.native_window = app->window;

    cmp_android_backend_create(&cfg, &state->backend);
    cmp_android_backend_get_ws(state->backend, &state->ws);
    cmp_android_backend_get_gfx(state->backend, &state->gfx);
    cmp_android_backend_get_env(state->backend, &state->env);

    CMPWSWindowConfig wcfg = {0};
    state->ws->vtable->create_window(state->ws->ctx, &wcfg, &state->window);

    CMPAllocator alloc;
    cmp_get_default_allocator(&alloc);
    demo_app_create(&alloc, &state->demo);
    demo_app_init_resources(state->demo, state->gfx, state->env);
    state->initialized = 1;
  }
  if (cmd == APP_CMD_TERM_WINDOW && state->initialized) {
    demo_app_destroy(state->demo);
    state->ws->vtable->destroy_window(state->ws->ctx, state->window);
    cmp_android_backend_destroy(state->backend);
    state->initialized = 0;
  }
}

void android_main(struct android_app *app) {
  AndroidState state = {0};
  state.app = app;
  app->userData = &state;
  app->onAppCmd = on_cmd;

  double last = get_time();

  while (1) {
    int events;
    struct android_poll_source *source;
    while (ALooper_pollAll(state.initialized ? 0 : -1, NULL, &events,
                           (void **)&source) >= 0) {
      if (source)
        source->process(app, source);
      if (app->destroyRequested)
        return;
    }

    if (state.initialized) {
      CMPInputEvent evt;
      CMPBool has;
      while (1) {
        state.ws->vtable->poll_event(state.ws->ctx, &evt, &has);
        if (!has)
          break;
        CMPBool h;
        demo_app_handle_event(state.demo, &evt, &h);
      }

      double now = get_time();
      demo_app_update(state.demo, now - last);
      last = now;

      int w, h;
      CMPScalar s;
      state.ws->vtable->get_window_size(state.ws->ctx, state.window, &w, &h);
      state.ws->vtable->get_window_dpi_scale(state.ws->ctx, state.window, &s);
      demo_app_render(state.demo, state.gfx, state.window, w, h, (float)s);
    }
  }
}