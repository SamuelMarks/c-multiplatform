#include "test_utils.h"

#include "m3/m3_backend_sdl3.h"

#include <string.h>

int main(void)
{
    M3SDL3BackendConfig config;
    M3SDL3Backend *backend;
    M3Bool available;

    M3_TEST_EXPECT(m3_sdl3_backend_config_init(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_sdl3_backend_config_init(&config));
    M3_TEST_ASSERT(config.handle_capacity != 0);
    M3_TEST_ASSERT(config.renderer_flags != 0);
    M3_TEST_ASSERT(config.enable_logging == M3_TRUE);
    M3_TEST_ASSERT(config.enable_tasks == M3_TRUE);

    M3_TEST_EXPECT(m3_sdl3_backend_is_available(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_sdl3_backend_is_available(&available));

    backend = NULL;
    M3_TEST_EXPECT(m3_sdl3_backend_create(NULL, NULL), M3_ERR_INVALID_ARGUMENT);
    {
        int rc;
        rc = m3_sdl3_backend_create(NULL, &backend);
        if (!available) {
            M3_TEST_EXPECT(rc, M3_ERR_UNSUPPORTED);
            M3_TEST_ASSERT(backend == NULL);
        } else if (rc == M3_OK && backend != NULL) {
            M3_TEST_OK(m3_sdl3_backend_destroy(backend));
        }
    }

    M3_TEST_OK(m3_sdl3_backend_config_init(&config));
    config.handle_capacity = 0;
    M3_TEST_EXPECT(m3_sdl3_backend_create(&config, &backend), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_sdl3_backend_config_init(&config));
    config.renderer_flags = 0x80000000u;
    M3_TEST_EXPECT(m3_sdl3_backend_create(&config, &backend), M3_ERR_INVALID_ARGUMENT);

    {
        M3Allocator bad_alloc;
        M3_TEST_OK(m3_sdl3_backend_config_init(&config));
        memset(&bad_alloc, 0, sizeof(bad_alloc));
        config.allocator = &bad_alloc;
        M3_TEST_EXPECT(m3_sdl3_backend_create(&config, &backend), M3_ERR_INVALID_ARGUMENT);
    }

    if (!available) {
        M3WS ws;
        M3Gfx gfx;
        M3Env env;

        M3_TEST_OK(m3_sdl3_backend_config_init(&config));
        backend = NULL;
        M3_TEST_EXPECT(m3_sdl3_backend_create(&config, &backend), M3_ERR_UNSUPPORTED);
        M3_TEST_ASSERT(backend == NULL);

        M3_TEST_EXPECT(m3_sdl3_backend_destroy(NULL), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_sdl3_backend_destroy((M3SDL3Backend *)1), M3_ERR_UNSUPPORTED);

        M3_TEST_EXPECT(m3_sdl3_backend_get_ws(NULL, &ws), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_sdl3_backend_get_ws((M3SDL3Backend *)1, NULL), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_sdl3_backend_get_ws((M3SDL3Backend *)1, &ws), M3_ERR_UNSUPPORTED);

        M3_TEST_EXPECT(m3_sdl3_backend_get_gfx(NULL, &gfx), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_sdl3_backend_get_gfx((M3SDL3Backend *)1, NULL), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_sdl3_backend_get_gfx((M3SDL3Backend *)1, &gfx), M3_ERR_UNSUPPORTED);

        M3_TEST_EXPECT(m3_sdl3_backend_get_env(NULL, &env), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_sdl3_backend_get_env((M3SDL3Backend *)1, NULL), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_sdl3_backend_get_env((M3SDL3Backend *)1, &env), M3_ERR_UNSUPPORTED);

        return 0;
    }

#if defined(M3_SDL3_AVAILABLE)
    {
        M3WS ws;
        M3Gfx gfx;
        M3Env env;
        M3WSConfig ws_config;
        m3_u32 time_ms;
        int rc;

        M3_TEST_EXPECT(m3_sdl3_backend_get_ws(NULL, &ws), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_sdl3_backend_get_gfx(NULL, &gfx), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_sdl3_backend_get_env(NULL, &env), M3_ERR_INVALID_ARGUMENT);

        M3_TEST_OK(m3_sdl3_backend_config_init(&config));
        backend = NULL;
        rc = m3_sdl3_backend_create(&config, &backend);
        if (rc != M3_OK) {
            return 0;
        }

        M3_TEST_OK(m3_sdl3_backend_get_ws(backend, &ws));
        M3_TEST_OK(m3_sdl3_backend_get_gfx(backend, &gfx));
        M3_TEST_OK(m3_sdl3_backend_get_env(backend, &env));

        if (gfx.text_vtable != NULL) {
            M3Handle font;
            int text_rc;

            font.id = 0u;
            font.generation = 0u;

            M3_TEST_EXPECT(gfx.text_vtable->create_font(gfx.ctx, NULL, 12, 400, M3_FALSE, &font), M3_ERR_INVALID_ARGUMENT);
            M3_TEST_EXPECT(gfx.text_vtable->create_font(gfx.ctx, "missing.ttf", 0, 400, M3_FALSE, &font), M3_ERR_RANGE);
            M3_TEST_EXPECT(gfx.text_vtable->create_font(gfx.ctx, "missing.ttf", 12, 50, M3_FALSE, &font), M3_ERR_RANGE);

            text_rc = gfx.text_vtable->create_font(gfx.ctx, "missing.ttf", 12, 400, M3_FALSE, &font);
#if defined(M3_SDL3_TTF_AVAILABLE)
            if (text_rc == M3_OK) {
                M3Scalar width;
                M3Scalar height;
                M3Scalar baseline;

                M3_TEST_OK(gfx.text_vtable->measure_text(gfx.ctx, font, "", 0, &width, &height, &baseline));
                M3_TEST_OK(gfx.text_vtable->destroy_font(gfx.ctx, font));
            } else {
                M3_TEST_ASSERT(text_rc == M3_ERR_NOT_FOUND || text_rc == M3_ERR_UNKNOWN);
            }
#else
            M3_TEST_EXPECT(text_rc, M3_ERR_UNSUPPORTED);
#endif
        }

        ws_config.utf8_app_name = "LibM3C";
        ws_config.utf8_app_id = "com.libm3c.test";
        ws_config.reserved = 0u;
        M3_TEST_OK(ws.vtable->init(ws.ctx, &ws_config));
        M3_TEST_OK(ws.vtable->get_time_ms(ws.ctx, &time_ms));
        M3_TEST_OK(env.vtable->get_time_ms(env.ctx, &time_ms));
        M3_TEST_OK(ws.vtable->shutdown(ws.ctx));

        M3_TEST_OK(m3_sdl3_backend_destroy(backend));
    }
#endif

    return 0;
}
