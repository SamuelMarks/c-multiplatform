#include "test_utils.h"

#include "m3/m3_backend_win32.h"

#include <string.h>

int main(void)
{
    M3Win32BackendConfig config;
    M3Win32Backend *backend;
    M3Bool available;

    M3_TEST_EXPECT(m3_win32_backend_config_init(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_win32_backend_config_init(&config));
    M3_TEST_ASSERT(config.handle_capacity != 0);
    M3_TEST_ASSERT(config.clipboard_limit != 0);
    M3_TEST_ASSERT(config.enable_logging == M3_TRUE);
    M3_TEST_ASSERT(config.inline_tasks == M3_TRUE);

    M3_TEST_EXPECT(m3_win32_backend_is_available(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_win32_backend_is_available(&available));

    backend = NULL;
    M3_TEST_EXPECT(m3_win32_backend_create(NULL, NULL), M3_ERR_INVALID_ARGUMENT);
    {
        int rc;
        rc = m3_win32_backend_create(NULL, &backend);
        if (!available) {
            M3_TEST_EXPECT(rc, M3_ERR_UNSUPPORTED);
            M3_TEST_ASSERT(backend == NULL);
        } else if (rc == M3_OK && backend != NULL) {
            M3_TEST_OK(m3_win32_backend_destroy(backend));
        }
    }

    M3_TEST_OK(m3_win32_backend_config_init(&config));
    config.handle_capacity = 0;
    M3_TEST_EXPECT(m3_win32_backend_create(&config, &backend), M3_ERR_INVALID_ARGUMENT);

    {
        M3Allocator bad_alloc;
        M3_TEST_OK(m3_win32_backend_config_init(&config));
        memset(&bad_alloc, 0, sizeof(bad_alloc));
        config.allocator = &bad_alloc;
        M3_TEST_EXPECT(m3_win32_backend_create(&config, &backend), M3_ERR_INVALID_ARGUMENT);
    }

    if (!available) {
        M3WS ws;
        M3Gfx gfx;
        M3Env env;

        M3_TEST_OK(m3_win32_backend_config_init(&config));
        backend = NULL;
        M3_TEST_EXPECT(m3_win32_backend_create(&config, &backend), M3_ERR_UNSUPPORTED);
        M3_TEST_ASSERT(backend == NULL);

        M3_TEST_EXPECT(m3_win32_backend_destroy(NULL), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_win32_backend_destroy((M3Win32Backend *)1), M3_ERR_UNSUPPORTED);

        M3_TEST_EXPECT(m3_win32_backend_get_ws(NULL, &ws), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_win32_backend_get_ws((M3Win32Backend *)1, NULL), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_win32_backend_get_ws((M3Win32Backend *)1, &ws), M3_ERR_UNSUPPORTED);

        M3_TEST_EXPECT(m3_win32_backend_get_gfx(NULL, &gfx), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_win32_backend_get_gfx((M3Win32Backend *)1, NULL), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_win32_backend_get_gfx((M3Win32Backend *)1, &gfx), M3_ERR_UNSUPPORTED);

        M3_TEST_EXPECT(m3_win32_backend_get_env(NULL, &env), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_win32_backend_get_env((M3Win32Backend *)1, NULL), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_win32_backend_get_env((M3Win32Backend *)1, &env), M3_ERR_UNSUPPORTED);

        return 0;
    }

#if defined(M3_WIN32_AVAILABLE)
    {
        M3WS ws;
        M3Gfx gfx;
        M3Env env;
        M3WSConfig ws_config;
        M3WSWindowConfig window_config;
        M3Handle window;
        M3Rect src_rect;
        M3Rect dst_rect;
        M3Color color;
        M3Handle texture;
        m3_u8 pixels[16];
        m3_u8 update_pixels[4];
        m3_u32 time_ms;
        int rc;

        M3_TEST_EXPECT(m3_win32_backend_get_ws(NULL, &ws), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_win32_backend_get_gfx(NULL, &gfx), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_win32_backend_get_env(NULL, &env), M3_ERR_INVALID_ARGUMENT);

        M3_TEST_OK(m3_win32_backend_config_init(&config));
        backend = NULL;
        rc = m3_win32_backend_create(&config, &backend);
        if (rc != M3_OK) {
            return 0;
        }

        M3_TEST_OK(m3_win32_backend_get_ws(backend, &ws));
        M3_TEST_OK(m3_win32_backend_get_gfx(backend, &gfx));
        M3_TEST_OK(m3_win32_backend_get_env(backend, &env));

        if (gfx.text_vtable != NULL) {
            M3Handle font;

            font.id = 0u;
            font.generation = 0u;

            M3_TEST_EXPECT(gfx.text_vtable->create_font(gfx.ctx, NULL, 12, 400, M3_FALSE, &font), M3_ERR_INVALID_ARGUMENT);
            M3_TEST_EXPECT(gfx.text_vtable->create_font(gfx.ctx, "Arial", 0, 400, M3_FALSE, &font), M3_ERR_RANGE);
            M3_TEST_EXPECT(gfx.text_vtable->create_font(gfx.ctx, "Arial", 12, 50, M3_FALSE, &font), M3_ERR_RANGE);
            M3_TEST_OK(gfx.text_vtable->create_font(gfx.ctx, "Arial", 12, 400, M3_FALSE, &font));
            M3_TEST_OK(gfx.text_vtable->destroy_font(gfx.ctx, font));
        }

        ws_config.utf8_app_name = "LibM3C";
        ws_config.utf8_app_id = "com.libm3c.test";
        ws_config.reserved = 0u;
        M3_TEST_OK(ws.vtable->init(ws.ctx, &ws_config));

        window_config.width = 320;
        window_config.height = 240;
        window_config.utf8_title = "LibM3C Win32";
        window_config.flags = M3_WS_WINDOW_RESIZABLE;

        rc = ws.vtable->create_window(ws.ctx, &window_config, &window);
        if (rc == M3_OK) {
            M3_TEST_OK(gfx.vtable->begin_frame(gfx.ctx, window, window_config.width, window_config.height, 1.0f));

            memset(pixels, 255, sizeof(pixels));
            texture.id = 0u;
            texture.generation = 0u;
            M3_TEST_OK(gfx.vtable->create_texture(gfx.ctx, 2, 2, M3_TEX_FORMAT_RGBA8, pixels, sizeof(pixels), &texture));

            update_pixels[0] = 0u;
            update_pixels[1] = 128u;
            update_pixels[2] = 255u;
            update_pixels[3] = 255u;
            M3_TEST_OK(gfx.vtable->update_texture(gfx.ctx, texture, 0, 0, 1, 1, update_pixels, sizeof(update_pixels)));

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
            M3_TEST_OK(gfx.vtable->clear(gfx.ctx, color));
            M3_TEST_OK(gfx.vtable->draw_texture(gfx.ctx, texture, &src_rect, &dst_rect, 1.0f));
            M3_TEST_OK(gfx.vtable->destroy_texture(gfx.ctx, texture));
            M3_TEST_OK(gfx.vtable->end_frame(gfx.ctx, window));
            M3_TEST_OK(ws.vtable->destroy_window(ws.ctx, window));
        }

        M3_TEST_OK(ws.vtable->get_time_ms(ws.ctx, &time_ms));
        M3_TEST_OK(env.vtable->get_time_ms(env.ctx, &time_ms));
        M3_TEST_OK(ws.vtable->shutdown(ws.ctx));

        M3_TEST_OK(m3_win32_backend_destroy(backend));
    }
#endif

    return 0;
}
