#include "app/demo_app.h"
#include "cmpc/cmp_backend_win32.h"
#include <stdio.h>
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine,
                   int nCmdShow) {
  (void)hInstance;
  (void)hPrev;
  (void)lpCmdLine;
  (void)nCmdShow;

  CMPWin32BackendConfig config;
  CMPWin32Backend *backend = NULL;
  CMPWS *ws;
  CMPGfx *gfx;
  CMPEnv *env;
  CMPHandle window;
  DemoApp *app = NULL;
  CMPAllocator alloc;

  cmp_win32_backend_config_init(&config);
  if (cmp_win32_backend_create(&config, &backend) != CMP_OK)
    return 1;

  cmp_win32_backend_get_ws(backend, &ws);
  cmp_win32_backend_get_gfx(backend, &gfx);
  cmp_win32_backend_get_env(backend, &env);

  CMPWSWindowConfig wincfg = {400, 600, "CMPC Demo (Win32)",
                              CMP_WS_WINDOW_RESIZABLE};
  ws->vtable->create_window(ws->ctx, &wincfg, &window);
  ws->vtable->show_window(ws->ctx, window);

  cmp_get_default_allocator(&alloc);
  demo_app_create(&alloc, &app);
  demo_app_init_resources(app, gfx, env);

  int running = 1;
  CMPInputEvent event;
  CMPBool has_event;

  while (running) {
    // Simple loop matching SDL3 structure for clarity
    // Real Win32 needs MsgWaitForMultipleObjects or similar to not burn CPU
    while (1) {
      ws->vtable->poll_event(ws->ctx, &event, &has_event);
      if (!has_event)
        break;
      if (event.type == CMP_INPUT_WINDOW_CLOSE)
        running = 0;
      CMPBool handled;
      demo_app_handle_event(app, &event, &handled);
    }

    demo_app_update(app, 0.016);

    int w, h;
    ws->vtable->get_window_size(ws->ctx, window, &w, &h);
    demo_app_render(app, gfx, window, w, h, 1.0f);

    Sleep(16);
  }

  demo_app_destroy(app);
  ws->vtable->destroy_window(ws->ctx, window);
  cmp_win32_backend_destroy(backend);
  return 0;
}