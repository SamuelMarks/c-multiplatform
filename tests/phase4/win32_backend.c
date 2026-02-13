#include "test_utils.h"

#include "cmpc/cmp_backend_win32.h"

#include <string.h>

int main(void) {
  CMPWin32BackendConfig config;
  CMPWin32Backend *backend;
  CMPAllocator default_alloc;
  CMPBool available;

  CMP_TEST_EXPECT(cmp_win32_backend_config_init(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_win32_backend_config_init(&config));
  CMP_TEST_ASSERT(config.handle_capacity != 0);
  CMP_TEST_ASSERT(config.clipboard_limit != 0);
  CMP_TEST_ASSERT(config.enable_logging == CMP_TRUE);
  CMP_TEST_ASSERT(config.inline_tasks == CMP_TRUE);

  CMP_TEST_EXPECT(cmp_win32_backend_test_validate_config(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_get_default_allocator(&default_alloc));
  CMP_TEST_OK(cmp_win32_backend_config_init(&config));
  config.allocator = &default_alloc;
  CMP_TEST_OK(cmp_win32_backend_test_validate_config(&config));

  CMP_TEST_EXPECT(cmp_win32_backend_is_available(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_win32_backend_is_available(&available));

  backend = NULL;
  CMP_TEST_EXPECT(cmp_win32_backend_create(NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  {
    int rc;
    rc = cmp_win32_backend_create(NULL, &backend);
    if (!available) {
      CMP_TEST_EXPECT(rc, CMP_ERR_UNSUPPORTED);
      CMP_TEST_ASSERT(backend == NULL);
    } else if (rc == CMP_OK && backend != NULL) {
      CMP_TEST_OK(cmp_win32_backend_destroy(backend));
    }
  }

  CMP_TEST_OK(cmp_win32_backend_config_init(&config));
  config.handle_capacity = 0;
  CMP_TEST_EXPECT(cmp_win32_backend_create(&config, &backend),
                  CMP_ERR_INVALID_ARGUMENT);

  {
    CMPAllocator bad_alloc;
    CMP_TEST_OK(cmp_win32_backend_config_init(&config));
    memset(&bad_alloc, 0, sizeof(bad_alloc));
    config.allocator = &bad_alloc;
    CMP_TEST_EXPECT(cmp_win32_backend_create(&config, &backend),
                    CMP_ERR_INVALID_ARGUMENT);
  }

  if (!available) {
    CMPWS ws;
    CMPGfx gfx;
    CMPEnv env;

    CMP_TEST_OK(cmp_win32_backend_config_init(&config));
    backend = NULL;
    CMP_TEST_EXPECT(cmp_win32_backend_create(&config, &backend),
                    CMP_ERR_UNSUPPORTED);
    CMP_TEST_ASSERT(backend == NULL);

    CMP_TEST_EXPECT(cmp_win32_backend_destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_win32_backend_destroy((CMPWin32Backend *)1),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(cmp_win32_backend_get_ws(NULL, &ws),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_win32_backend_get_ws((CMPWin32Backend *)1, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_win32_backend_get_ws((CMPWin32Backend *)1, &ws),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(cmp_win32_backend_get_gfx(NULL, &gfx),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_win32_backend_get_gfx((CMPWin32Backend *)1, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_win32_backend_get_gfx((CMPWin32Backend *)1, &gfx),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(cmp_win32_backend_get_env(NULL, &env),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_win32_backend_get_env((CMPWin32Backend *)1, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_win32_backend_get_env((CMPWin32Backend *)1, &env),
                    CMP_ERR_UNSUPPORTED);

    return 0;
  }

#if defined(CMP_WIN32_AVAILABLE)
  {
    CMPWS ws;
    CMPGfx gfx;
    CMPEnv env;
    CMPWSConfig ws_config;
    CMPWSWindowConfig window_config;
    CMPHandle window;
    CMPRect src_rect;
    CMPRect dst_rect;
    CMPColor color;
    CMPHandle texture;
    cmp_u8 pixels[16];
    cmp_u8 update_pixels[4];
    cmp_u32 time_ms;
    int rc;

    CMP_TEST_EXPECT(cmp_win32_backend_get_ws(NULL, &ws),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_win32_backend_get_gfx(NULL, &gfx),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_win32_backend_get_env(NULL, &env),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(cmp_win32_backend_config_init(&config));
    backend = NULL;
    rc = cmp_win32_backend_create(&config, &backend);
    if (rc != CMP_OK) {
      return 0;
    }

    CMP_TEST_OK(cmp_win32_backend_get_ws(backend, &ws));
    CMP_TEST_OK(cmp_win32_backend_get_gfx(backend, &gfx));
    CMP_TEST_OK(cmp_win32_backend_get_env(backend, &env));

    if (gfx.text_vtable != NULL) {
      CMPHandle font;

      font.id = 0u;
      font.generation = 0u;

      CMP_TEST_EXPECT(gfx.text_vtable->create_font(gfx.ctx, NULL, 12, 400,
                                                   CMP_FALSE, &font),
                      CMP_ERR_INVALID_ARGUMENT);
      CMP_TEST_EXPECT(gfx.text_vtable->create_font(gfx.ctx, "Arial", 0, 400,
                                                   CMP_FALSE, &font),
                      CMP_ERR_RANGE);
      CMP_TEST_EXPECT(gfx.text_vtable->create_font(gfx.ctx, "Arial", 12, 50,
                                                   CMP_FALSE, &font),
                      CMP_ERR_RANGE);
      CMP_TEST_OK(gfx.text_vtable->create_font(gfx.ctx, "Arial", 12, 400,
                                               CMP_FALSE, &font));
      CMP_TEST_OK(gfx.text_vtable->destroy_font(gfx.ctx, font));
    }

    ws_config.utf8_app_name = "LibCMPC";
    ws_config.utf8_app_id = "com.libcmpc.test";
    ws_config.reserved = 0u;
    CMP_TEST_OK(ws.vtable->init(ws.ctx, &ws_config));

    window_config.width = 320;
    window_config.height = 240;
    window_config.utf8_title = "LibCMPC Win32";
    window_config.flags = CMP_WS_WINDOW_RESIZABLE;

    rc = ws.vtable->create_window(ws.ctx, &window_config, &window);
    if (rc == CMP_OK) {
      CMP_TEST_OK(gfx.vtable->begin_frame(gfx.ctx, window, window_config.width,
                                          window_config.height, 1.0f));

      memset(pixels, 255, sizeof(pixels));
      texture.id = 0u;
      texture.generation = 0u;
      CMP_TEST_OK(gfx.vtable->create_texture(gfx.ctx, 2, 2,
                                             CMP_TEX_FORMAT_RGBA8, pixels,
                                             sizeof(pixels), &texture));

      update_pixels[0] = 0u;
      update_pixels[1] = 128u;
      update_pixels[2] = 255u;
      update_pixels[3] = 255u;
      CMP_TEST_OK(gfx.vtable->update_texture(
          gfx.ctx, texture, 0, 0, 1, 1, update_pixels, sizeof(update_pixels)));

      src_rect.x = 0.0f;
      src_rect.y = 0.0f;
      src_rect.width = 2.0f;
      src_rect.height = 2.0f;
      dst_rect.x = 10.0f;
      dst_rect.y = 10.0f;
      dst_rect.width = 20.0f;
      dst_rect.height = 20.0f;

      color.r = 0.0f;
      color.g = 0.0f;
      color.b = 0.0f;
      color.a = 1.0f;
      CMP_TEST_OK(gfx.vtable->clear(gfx.ctx, color));
      CMP_TEST_OK(gfx.vtable->draw_texture(gfx.ctx, texture, &src_rect,
                                           &dst_rect, 1.0f));
      CMP_TEST_OK(gfx.vtable->destroy_texture(gfx.ctx, texture));
      CMP_TEST_OK(gfx.vtable->end_frame(gfx.ctx, window));
      CMP_TEST_OK(ws.vtable->destroy_window(ws.ctx, window));
    }

    CMP_TEST_OK(ws.vtable->get_time_ms(ws.ctx, &time_ms));
    CMP_TEST_OK(env.vtable->get_time_ms(env.ctx, &time_ms));
    CMP_TEST_OK(ws.vtable->shutdown(ws.ctx));

    CMP_TEST_OK(cmp_win32_backend_destroy(backend));
  }
#endif

  return 0;
}
