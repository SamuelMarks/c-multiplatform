#include <SDL3/SDL.h>
#include <app/demo_app.h>
#include <cmpc/cmp_backend_sdl3.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  CMPSDL3BackendConfig config;
  CMPSDL3Backend *backend = NULL;
  CMPWS ws;
  CMPGfx gfx;
  CMPEnv env;
  CMPHandle window;
  DemoApp *app = NULL;
  CMPAllocator alloc;

  cmp_sdl3_backend_config_init(&config);
  if (cmp_sdl3_backend_create(&config, &backend) != CMP_OK) {
    fprintf(stderr, "Failed to create SDL3 backend\n");
    return 1;
  }

  cmp_sdl3_backend_get_ws(backend, &ws);
  cmp_sdl3_backend_get_gfx(backend, &gfx);
  cmp_sdl3_backend_get_env(backend, &env);

  CMPWSWindowConfig wincfg = {.width = 400,
                              .height = 700,
                              .utf8_title = "CMPC Demo (SDL3)",
                              .flags = CMP_WS_WINDOW_RESIZABLE};
  ws.vtable->create_window(ws.ctx, &wincfg, &window);
  ws.vtable->show_window(ws.ctx, window);

  cmp_get_default_allocator(&alloc);
  demo_app_create(&alloc, &app);
  demo_app_init_resources(app, &gfx, &env);

  int running = 1;
  CMPInputEvent event;
  CMPBool has_event;

  while (running) {
    while (1) {
      ws.vtable->poll_event(ws.ctx, &event, &has_event);
      if (!has_event)
        break;

      if (event.type == CMP_INPUT_WINDOW_CLOSE) {
        running = 0;
      }
      CMPBool handled;
      demo_app_handle_event(app, &event, &handled);
    }

    demo_app_update(app, 0.016);

    int w = 0, h = 0;
    ws.vtable->get_window_size(ws.ctx, window, &w, &h);
    demo_app_render(app, &gfx, window, w, h, 1.0f); // Default DPI for SDL3 stub

    SDL_Delay(16);
  }

  demo_app_destroy(app);
  ws.vtable->destroy_window(ws.ctx, window);
  cmp_sdl3_backend_destroy(backend);
  return 0;
}