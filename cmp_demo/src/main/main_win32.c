#include "app/demo_app.h"
#include "cmpc/cmp_backend_win32.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

/* clang-format off */
#include <windef.h>
#include <winbase.h>
#include <winuser.h>
/* clang-format on */

#if defined(_MSC_VER)
#define NUM_FORMAT "%d"
#else
#define NUM_FORMAT "%d"
#endif

static int demo_printf_err(const char *fmt, ...) {
  int ret;
  va_list args;
  va_start(args, fmt);
#if defined(_MSC_VER)
  ret = vfprintf_s(stderr, fmt, args);
#else
  ret = vfprintf(stderr, fmt, args);
#endif
  va_end(args);
  return ret;
}

static int demo_printf_out(const char *fmt, ...) {
  int ret;
  va_list args;
  va_start(args, fmt);
#if defined(_MSC_VER)
  ret = vprintf_s(fmt, args);
#else
  ret = vprintf(fmt, args);
#endif
  va_end(args);
  return ret;
}

/* Helper to allow viewing printf output in a Win32 GUI app */
void CreateDebugConsole(void) {
  AllocConsole();
#if defined(_MSC_VER)
  {
    FILE *dummy;
    freopen_s(&dummy, "CONIN$", "r", stdin);
    freopen_s(&dummy, "CONOUT$", "w", stdout);
    freopen_s(&dummy, "CONOUT$", "w", stderr);
  }
#else
  freopen("CONIN$", "r", stdin);
  freopen("CONOUT$", "w", stdout);
  freopen("CONOUT$", "w", stderr);
#endif
  demo_printf_out("Debug console attached.\n");
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
    demo_printf_err("Backend config init failed: " NUM_FORMAT "\n", rc);
    return 1;
  }

  /* Manually enforce capacity if init didn't set it */
  if (config.handle_capacity == 0) {
    config.handle_capacity = 128;
  }
  config.enable_logging = CMP_TRUE;

  /* 4. Create Backend */
  demo_printf_out("Creating Win32 Backend...\n");
  rc = cmp_win32_backend_create(&config, &backend);
  if (rc != CMP_OK) {
    demo_printf_err(
        "Failed to create Win32 backend. Error Code: " NUM_FORMAT "\n", rc);
    /* Keep console open for a moment so we can read the error */
    if (IsDebuggerPresent()) {
      demo_printf_out("Press Enter to exit...");
      getchar();
    }
    return 1;
  }

  /* 5. Get Interface V-Tables */
  /* Pass the address of the stack structs */
  cmp_win32_backend_get_ws(backend, &ws);
  cmp_win32_backend_get_gfx(backend, &gfx);
  cmp_win32_backend_get_env(backend, &env);

  demo_printf_out("Backend created. Creating window...\n");

  CMPWSWindowConfig wincfg = {400, 600, "CMPC Demo (Win32)",
                              CMP_WS_WINDOW_RESIZABLE};

  /* Access via the struct directly (ws.vtable) */
  rc = ws.vtable->create_window(ws.ctx, &wincfg, &window);
  if (rc != CMP_OK) {
    demo_printf_err("Failed to create window: " NUM_FORMAT "\n", rc);
    return 1;
  }

  ws.vtable->show_window(ws.ctx, window);

  cmp_get_default_allocator(&alloc);

  demo_printf_out("Creating Demo App...\n");
  demo_app_create(&alloc, &app);

  demo_printf_out("Initializing Resources...\n");
  // Pass pointers to the stack structs
  demo_app_init_resources(app, &gfx, &env);

  demo_printf_out("Running Loop...\n");
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

  demo_printf_out("Shutting down...\n");
  demo_app_destroy(app);
  ws.vtable->destroy_window(ws.ctx, window);
  cmp_win32_backend_destroy(backend);

#ifdef _DEBUG
  FreeConsole();
#endif
  return 0;
}