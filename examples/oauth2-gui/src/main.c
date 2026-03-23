/* clang-format off */
#include "oauth2_gui.h"
#include <cmpc/cmp_core.h>
#include <cmpc/cmp_router_view.h>
#include <cmpc/cmp_api_gfx.h>
#include <cmpc/cmp_api_ws.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* clang-format on */

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#include <cmpc/cmp_backend_win32.h>
__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);
#endif

int main(int argc, char **argv) {
  CMPWS ws;
  CMPGfx gfx;
  CMPEnv env;
  void *backend_ptr = NULL;
  OAuth2GuiTheme theme = OAUTH2_GUI_THEME_M3;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--theme=m3") == 0) {
      theme = OAUTH2_GUI_THEME_M3;
    } else if (strcmp(argv[i], "--theme=f2") == 0) {
      theme = OAUTH2_GUI_THEME_F2;
    } else if (strcmp(argv[i], "--theme=cupertino") == 0) {
      theme = OAUTH2_GUI_THEME_CUPERTINO;
    }
  }

  memset(&ws, 0, sizeof(ws));
  memset(&gfx, 0, sizeof(gfx));
  memset(&env, 0, sizeof(env));

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
  CMPBool has_win32 = CMP_FALSE;
  cmp_win32_backend_is_available(&has_win32);
  if (has_win32) {
    CMPWin32BackendConfig cfg;
    CMPWin32Backend *be_win32 = NULL;
    if (cmp_win32_backend_config_init(&cfg) == CMP_OK) {
      cfg.enable_logging = CMP_TRUE;
      if (cmp_win32_backend_create(&cfg, &be_win32) == CMP_OK) {
        cmp_win32_backend_get_ws(be_win32, &ws);
        cmp_win32_backend_get_gfx(be_win32, &gfx);
        cmp_win32_backend_get_env(be_win32, &env);
        backend_ptr = be_win32;
      }
    }
  }
#endif

  if (backend_ptr == NULL) {
    fprintf(stderr, "No supported backend could be initialized.\n");
    return 1;
  }

  CMPWSWindowConfig wcfg;
  memset(&wcfg, 0, sizeof(wcfg));
  wcfg.width = 800;
  wcfg.height = 600;
  wcfg.utf8_title = "Login";
  wcfg.flags = CMP_WS_WINDOW_RESIZABLE;

  CMPHandle win;
  if (ws.vtable->create_window(ws.ctx, &wcfg, &win) != CMP_OK) {
    fprintf(stderr, "Failed to create window.\n");
    return 1;
  }

  CMPTextBackend text_backend;
  memset(&text_backend, 0, sizeof(text_backend));
  text_backend.ctx = gfx.ctx;
  text_backend.vtable = gfx.text_vtable;

  CMPAllocator alloc;
  cmp_get_default_allocator(&alloc);

  OAuth2Gui gui;
  if (oauth2_gui_init(&gui, alloc, text_backend, theme) != 0) {
    fprintf(stderr, "Failed to initialize GUI\n");
    return 1;
  }

  /* Pre-paint the first frame to avoid white flashes */
  gfx.vtable->begin_frame(gfx.ctx, win, (cmp_u32)wcfg.width,
                          (cmp_u32)wcfg.height, 1.0f);
  gfx.vtable->clear(gfx.ctx, oauth2_gui_get_bg_color(&gui));
  gfx.vtable->end_frame(gfx.ctx, win);

  if (ws.vtable->show_window) {
    ws.vtable->show_window(ws.ctx, win);
  }

  /* Navigate to login view */
  cmp_router_navigate(&gui.router, "/login?redirect_uri=/", NULL);

  int running = 1;
  while (running) {
    CMPInputEvent ev;
    CMPBool has_event;

    while (1) {
      ws.vtable->poll_event(ws.ctx, &ev, &has_event);
      if (!has_event)
        break;

      if (ev.type == CMP_INPUT_WINDOW_CLOSE) {
        running = 0;
        break;
      }

      /* Dispatch to UI widget */
      CMPBool handled = CMP_FALSE;
      CMPWidget *w_ptr = gui.root_widget;
      if (w_ptr && w_ptr->vtable && w_ptr->vtable->event) {
        w_ptr->vtable->event(w_ptr->ctx, &ev, &handled);
      }
    }

    if (!running)
      break;

    oauth2_gui_tick(&gui);

    int w = 400, h = 600;
    CMPColor bg = {243.0f / 255.0f, 241.0f / 255.0f, 246.0f / 255.0f,
                   1.0f}; /* A more reasonable background color for M3 */
    CMPWidget *w_ptr = gui.root_widget;
    CMPSize size;
    CMPMeasureSpec ms_w;
    CMPMeasureSpec ms_h;
    CMPRect bounds;
    CMPPaintContext pctx;

    if (ws.vtable->get_window_size) {
      ws.vtable->get_window_size(ws.ctx, win, &w, &h);
    }

    if (w <= 0 || h <= 0) {
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
      Sleep(16);
#endif
      continue;
    }

    gfx.vtable->begin_frame(gfx.ctx, win, (cmp_u32)w, (cmp_u32)h, 1.0f);

    if (theme == OAUTH2_GUI_THEME_M3) {
      bg = oauth2_gui_get_bg_color(&gui);
    }
    gfx.vtable->clear(gfx.ctx, bg);

    ms_w.mode = CMP_MEASURE_EXACTLY;
    ms_w.size = (CMPScalar)w;
    ms_h.mode = CMP_MEASURE_EXACTLY;
    ms_h.size = (CMPScalar)h;

    printf("MEASURE... w_ptr=%p\n", w_ptr);
    if (w_ptr) {
      printf("vtable=%p\n", w_ptr->vtable);
      if (w_ptr->vtable) {
        printf("measure fn=%p\n", w_ptr->vtable->measure);
      }
    }
    fflush(stdout);

    w_ptr->vtable->measure(w_ptr->ctx, ms_w, ms_h, &size);

    bounds.x = 0;
    bounds.y = 0;
    bounds.width = (CMPScalar)w;
    bounds.height = (CMPScalar)h;

    printf("LAYOUT...\n");
    fflush(stdout);
    w_ptr->vtable->layout(w_ptr->ctx, bounds);

    memset(&pctx, 0, sizeof(pctx));
    pctx.gfx = &gfx;
    pctx.clip = bounds;
    pctx.dpi_scale = 1.0f;

    printf("PAINT...\n");
    fflush(stdout);
    w_ptr->vtable->paint(w_ptr->ctx, &pctx);

    printf("END_FRAME...\n");
    fflush(stdout);
    gfx.vtable->end_frame(gfx.ctx, win);
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
    Sleep(16);
#endif
  }

  oauth2_gui_shutdown(&gui);
  ws.vtable->destroy_window(ws.ctx, win);

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
  if (backend_ptr) {
    cmp_win32_backend_destroy((CMPWin32Backend *)backend_ptr);
  }
#endif

  return 0;
}
