#include "app/demo_app.h"
#include "cmpc/cmp_backend_win32.h"
#include <stdio.h>
#include <windows.h>

/* Helper to allow viewing printf output in a Win32 GUI app */
void CreateDebugConsole(void) {
  AllocConsole();
  freopen("CONIN$", "r", stdin);
  freopen("CONOUT$", "w", stdout);
  freopen("CONOUT$", "w", stderr);
  printf("Debug console attached.\n");
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine,
                   int nCmdShow) {
  (void)hInstance;
  (void)hPrev;
  (void)lpCmdLine;
  (void)nCmdShow;

  /* 1. Attach Console so we can see why it dies */
#ifdef _DEBUG
  CreateDebugConsole();
#endif

  CMPWin32BackendConfig config = {0};
  CMPWin32Backend *backend = NULL;

  /* 2. Fix: Use stack allocated structures, NOT pointers */
  CMPWS ws = {0};
  CMPGfx gfx = {0};
  CMPEnv env = {0};

  CMPHandle window = {0};
  DemoApp *app = NULL;
  CMPAllocator alloc;
  int rc;

  /* 3. Initialize Config & Force Defaults */
  rc = cmp_win32_backend_config_init(&config);
  if (rc != CMP_OK) {
    fprintf(stderr, "Backend config init failed: %d\n", rc);
    return 1;
  }

  /* Manually enforce capacity if init didn't set it */
  if (config.handle_capacity == 0) {
    config.handle_capacity = 128;
  }
  config.enable_logging = CMP_TRUE;

  /* 4. Create Backend */
  printf("Creating Win32 Backend...\n");
  rc = cmp_win32_backend_create(&config, &backend);
  if (rc != CMP_OK) {
    fprintf(stderr, "Failed to create Win32 backend. Error Code: %d\n", rc);
    /* Keep console open for a moment so we can read the error */
    if (IsDebuggerPresent()) {
      printf("Press Enter to exit...");
      getchar();
    }
    return 1;
  }

  /* 5. Get Interface V-Tables */
  /* Pass the address of the stack structs */
  cmp_win32_backend_get_ws(backend, &ws);
  cmp_win32_backend_get_gfx(backend, &gfx);
  cmp_win32_backend_get_env(backend, &env);

  printf("Backend created. Creating window...\n");

  CMPWSWindowConfig wincfg = {400, 600, "CMPC Demo (Win32)",
                              CMP_WS_WINDOW_RESIZABLE};

  /* Access via the struct directly (ws.vtable) */
  rc = ws.vtable->create_window(ws.ctx, &wincfg, &window);
  if (rc != CMP_OK) {
    fprintf(stderr, "Failed to create window: %d\n", rc);
    return 1;
  }

  ws.vtable->show_window(ws.ctx, window);

  cmp_get_default_allocator(&alloc);

  printf("Creating Demo App...\n");
  demo_app_create(&alloc, &app);

  printf("Initializing Resources...\n");
  // Pass pointers to the stack structs
  demo_app_init_resources(app, &gfx, &env);

  printf("Running Loop...\n");
  int running = 1;
  CMPInputEvent event;
  CMPBool has_event;

  while (running) {
    /* Poll events */
    while (1) {
      ws.vtable->poll_event(ws.ctx, &event, &has_event);
      if (!has_event)
        break;
      if (event.type == CMP_INPUT_WINDOW_CLOSE)
        running = 0;

      CMPBool handled;
      demo_app_handle_event(app, &event, &handled);
    }

    /* Update & Render */
    demo_app_update(app, 0.016);

    int w, h;
    ws.vtable->get_window_size(ws.ctx, window, &w, &h);
    demo_app_render(app, &gfx, window, w, h, 1.0f);

    Sleep(16);
  }

  printf("Shutting down...\n");
  demo_app_destroy(app);
  ws.vtable->destroy_window(ws.ctx, window);
  cmp_win32_backend_destroy(backend);

#ifdef _DEBUG
  FreeConsole();
#endif
  return 0;
}