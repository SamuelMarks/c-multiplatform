/* clang-format off */
#include "greatest.h"
#include "oauth2_gui.h"
#include <cmpc/cmp_backend_gtk4.h>
#include <cmpc/cmp_backend_win32.h>
#include <cmpc/cmp_router_view.h>
#include <cmpc/cmp_api_gfx.h>
#include <cmpc/cmp_fs.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* clang-format on */

#ifndef OAUTH2_TEST_DIR
#define OAUTH2_TEST_DIR "."
#endif

static int compare_files(const char *f1, const char *f2) {
  FILE *fp1 = fopen(f1, "rb");
  FILE *fp2 = fopen(f2, "rb");
  if (!fp1 || !fp2) {
    if (fp1)
      fclose(fp1);
    if (fp2)
      fclose(fp2);
    return -1;
  }
  int match = 1;
  while (1) {
    int c1 = fgetc(fp1);
    int c2 = fgetc(fp2);
    if (c1 != c2) {
      match = 0;
      break;
    }
    if (c1 == EOF) {
      break;
    }
  }
  fclose(fp1);
  fclose(fp2);
  return match;
}

TEST test_init_shutdown(void) {
  OAuth2Gui gui;
  CMPAllocator alloc;
  CMPTextBackend backend;
  int rc;

  cmp_get_default_allocator(&alloc);
  memset(&backend, 0, sizeof(backend));

  rc = oauth2_gui_init(&gui, alloc, backend, OAUTH2_GUI_THEME_M3);
  ASSERT_EQ(0, rc);

  rc = oauth2_gui_shutdown(&gui);
  ASSERT_EQ(0, rc);

  PASS();
}

TEST test_tick_null(void) {
  int rc;
  rc = oauth2_gui_tick(NULL);
  ASSERT_EQ(CMP_ERR_INVALID_ARGUMENT, rc);
  PASS();
}

TEST test_screenshots(void) {
  CMPBool has_gtk4 = CMP_FALSE;
  CMPBool has_win32 = CMP_FALSE;
  cmp_gtk4_backend_is_available(&has_gtk4);
  cmp_win32_backend_is_available(&has_win32);

  printf("has_gtk4=%d, has_win32=%d\n", has_gtk4, has_win32);

  if (!has_gtk4 && !has_win32) {
    PASS(); /* Skip test if neither backend is available */
  }

  CMPGTK4Backend *be_gtk4 = NULL;
  CMPWin32Backend *be_win32 = NULL;
  CMPWS ws;
  CMPGfx gfx;
  CMPEnv env;

  if (has_gtk4) {
    CMPGTK4BackendConfig cfg;
    cmp_gtk4_backend_config_init(&cfg);
    cmp_gtk4_backend_create(&cfg, &be_gtk4);
    cmp_gtk4_backend_get_ws(be_gtk4, &ws);
    cmp_gtk4_backend_get_gfx(be_gtk4, &gfx);
    cmp_gtk4_backend_get_env(be_gtk4, &env);
  } else if (has_win32) {
    CMPWin32BackendConfig cfg;
    cmp_win32_backend_config_init(&cfg);
    cmp_win32_backend_create(&cfg, &be_win32);
    cmp_win32_backend_get_ws(be_win32, &ws);
    cmp_win32_backend_get_gfx(be_win32, &gfx);
    cmp_win32_backend_get_env(be_win32, &env);
  }

  CMPWSWindowConfig wcfg;
  CMPHandle win;
  int actual_w = 400;
  int actual_h = 400;
  CMPTextBackend text_backend;
  CMPAllocator alloc;
  OAuth2Gui gui;
  CMPRouterView view;
  const char *routes[] = {"login", "loading", "token"};
  int i, j;

  memset(&wcfg, 0, sizeof(wcfg));
  wcfg.width = 400;
  wcfg.height = 400;
  wcfg.utf8_title = "OAuth2 Screenshots Test";

  ws.vtable->create_window(ws.ctx, &wcfg, &win);

  if (ws.vtable->show_window) {
    ws.vtable->show_window(ws.ctx, win);
  }

  if (ws.vtable->get_window_size) {
    ws.vtable->get_window_size(ws.ctx, win, &actual_w, &actual_h);
  }

  for (i = 0; i < 10; i++) {
    CMPInputEvent ev;
    CMPBool has;
    ws.vtable->poll_event(ws.ctx, &ev, &has);
  }

  text_backend.ctx = gfx.ctx;
  text_backend.vtable = gfx.text_vtable;

  cmp_get_default_allocator(&alloc);

  oauth2_gui_init(&gui, alloc, text_backend, OAUTH2_GUI_THEME_M3);

  cmp_router_view_init(&view, &gui.router);

  for (i = 0; i < 3; i++) {
    if (i == 0) {
      cmp_router_navigate(&gui.router, "/login?redirect_uri=/", NULL);
    } else if (i == 1) {
      cmp_form_set_submitting(&gui.form, CMP_TRUE);
    } else if (i == 2) {
      if (gui.access_token) {
        free(gui.access_token);
      }
      gui.access_token = malloc(64);
      strcpy(gui.access_token, "fake_token_abc123");
      cmp_router_navigate(&gui.router, "/", NULL);
    }

    /* Pump events */
    for (j = 0; j < 10; j++) {
      oauth2_gui_tick(&gui);
    }

    gfx.vtable->begin_frame(gfx.ctx, win, 400, 400, 1.0f);
    {
      CMPColor white = {255, 255, 255, 255};
      CMPWidget *w_ptr = (CMPWidget *)&view;
      CMPSize size;
      CMPMeasureSpec ms_w = {CMP_MEASURE_EXACTLY, (CMPScalar)actual_w};
      CMPMeasureSpec ms_h = {CMP_MEASURE_EXACTLY, (CMPScalar)actual_h};
      CMPRect bounds = {0, 0, (CMPScalar)actual_w, (CMPScalar)actual_h};
      CMPPaintContext pctx;
      char actual_path[1024];
      char expected_path[1024];
      FILE *f_expected;

      gfx.vtable->clear(gfx.ctx, white);
      w_ptr->vtable->measure(w_ptr, ms_w, ms_h, &size);
      w_ptr->vtable->layout(w_ptr, bounds);

      memset(&pctx, 0, sizeof(pctx));
      pctx.gfx = &gfx;
      pctx.clip = bounds;
      pctx.dpi_scale = 1.0f;
      w_ptr->vtable->paint(w_ptr, &pctx);

      gfx.vtable->end_frame(gfx.ctx, win);

      if (has_gtk4) {
        int rc_s;
        snprintf(actual_path, sizeof(actual_path), "%s/actual/%s.png",
                 OAUTH2_TEST_DIR, routes[i]);
        snprintf(expected_path, sizeof(expected_path), "%s/expected/%s.png",
                 OAUTH2_TEST_DIR, routes[i]);
        rc_s = cmp_gtk4_backend_save_png(be_gtk4, win, actual_path);
        if (rc_s != CMP_OK) {
          printf("cmp_gtk4_backend_save_png failed: %d\n", rc_s);
        }
        ASSERT_EQ(CMP_OK, rc_s);
      } else if (has_win32) {
        int rc_s;
        snprintf(actual_path, sizeof(actual_path), "%s/actual/%s.bmp",
                 OAUTH2_TEST_DIR, routes[i]);
        snprintf(expected_path, sizeof(expected_path), "%s/expected/%s.bmp",
                 OAUTH2_TEST_DIR, routes[i]);
        rc_s = cmp_win32_backend_save_bmp(be_win32, win, actual_path);
        if (rc_s != CMP_OK) {
          printf("cmp_win32_backend_save_bmp failed: %d for %s\n", rc_s,
                 actual_path);
        }
        ASSERT_EQ(CMP_OK, rc_s);
      }

      f_expected = fopen(expected_path, "rb");
      if (!f_expected) {
        FILE *f_actual;
        printf("Populating missing expected screenshot: %s\n", expected_path);
        f_actual = fopen(actual_path, "rb");
        if (f_actual) {
          FILE *f_out = fopen(expected_path, "wb");
          if (f_out) {
            char buf[4096];
            size_t n;
            while ((n = fread(buf, 1, sizeof(buf), f_actual)) > 0) {
              fwrite(buf, 1, n, f_out);
            }
            fclose(f_out);
          }
          fclose(f_actual);
        }
      } else {
        fclose(f_expected);
      }

      /* Assert expected vs actual file */
      if (compare_files(expected_path, actual_path) != 1) {
        printf("Screenshot mismatch: expected %s, got %s\n", expected_path,
               actual_path);
        ASSERT(0);
      }
    }
  }

  oauth2_gui_shutdown(&gui);
  ws.vtable->destroy_window(ws.ctx, win);

  if (has_gtk4) {
    cmp_gtk4_backend_destroy(be_gtk4);
  } else if (has_win32) {
    cmp_win32_backend_destroy(be_win32);
  }

  PASS();
}

SUITE(oauth2_gui_suite) {
  RUN_TEST(test_init_shutdown);
  RUN_TEST(test_tick_null);
  RUN_TEST(test_screenshots);
}
