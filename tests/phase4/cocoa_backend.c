#include "test_utils.h"

#include "m3/m3_backend_cocoa.h"

#include <string.h>

int main(void)
{
    M3CocoaBackendConfig config;
    M3CocoaBackend *backend;
    M3Bool available;

    M3_TEST_EXPECT(m3_cocoa_backend_config_init(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_cocoa_backend_config_init(&config));
    M3_TEST_ASSERT(config.handle_capacity != 0);
    M3_TEST_ASSERT(config.clipboard_limit != 0);
    M3_TEST_ASSERT(config.enable_logging == M3_TRUE);
    M3_TEST_ASSERT(config.inline_tasks == M3_TRUE);

    M3_TEST_EXPECT(m3_cocoa_backend_is_available(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_cocoa_backend_is_available(&available));

    backend = NULL;
    M3_TEST_EXPECT(m3_cocoa_backend_create(NULL, NULL), M3_ERR_INVALID_ARGUMENT);
    {
        int rc;
        rc = m3_cocoa_backend_create(NULL, &backend);
        if (!available) {
            M3_TEST_EXPECT(rc, M3_ERR_UNSUPPORTED);
            M3_TEST_ASSERT(backend == NULL);
        } else if (rc == M3_OK && backend != NULL) {
            M3_TEST_OK(m3_cocoa_backend_destroy(backend));
        }
    }

    M3_TEST_OK(m3_cocoa_backend_config_init(&config));
    config.handle_capacity = 0;
    M3_TEST_EXPECT(m3_cocoa_backend_create(&config, &backend), M3_ERR_INVALID_ARGUMENT);

    {
        M3Allocator bad_alloc;
        M3_TEST_OK(m3_cocoa_backend_config_init(&config));
        memset(&bad_alloc, 0, sizeof(bad_alloc));
        config.allocator = &bad_alloc;
        M3_TEST_EXPECT(m3_cocoa_backend_create(&config, &backend), M3_ERR_INVALID_ARGUMENT);
    }

    if (!available) {
        M3WS ws;
        M3Gfx gfx;
        M3Env env;

        M3_TEST_OK(m3_cocoa_backend_config_init(&config));
        backend = NULL;
        M3_TEST_EXPECT(m3_cocoa_backend_create(&config, &backend), M3_ERR_UNSUPPORTED);
        M3_TEST_ASSERT(backend == NULL);

        M3_TEST_EXPECT(m3_cocoa_backend_destroy(NULL), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_cocoa_backend_destroy((M3CocoaBackend *)1), M3_ERR_UNSUPPORTED);

        M3_TEST_EXPECT(m3_cocoa_backend_get_ws(NULL, &ws), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_cocoa_backend_get_ws((M3CocoaBackend *)1, NULL), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_cocoa_backend_get_ws((M3CocoaBackend *)1, &ws), M3_ERR_UNSUPPORTED);

        M3_TEST_EXPECT(m3_cocoa_backend_get_gfx(NULL, &gfx), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_cocoa_backend_get_gfx((M3CocoaBackend *)1, NULL), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_cocoa_backend_get_gfx((M3CocoaBackend *)1, &gfx), M3_ERR_UNSUPPORTED);

        M3_TEST_EXPECT(m3_cocoa_backend_get_env(NULL, &env), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_cocoa_backend_get_env((M3CocoaBackend *)1, NULL), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_cocoa_backend_get_env((M3CocoaBackend *)1, &env), M3_ERR_UNSUPPORTED);

        return 0;
    }

#if defined(M3_COCOA_AVAILABLE)
    {
        M3WS ws;
        M3Gfx gfx;
        M3Env env;
        int rc;

        M3_TEST_OK(m3_cocoa_backend_config_init(&config));
        backend = NULL;
        rc = m3_cocoa_backend_create(&config, &backend);
        if (rc != M3_OK) {
            return 0;
        }

        M3_TEST_OK(m3_cocoa_backend_get_ws(backend, &ws));
        M3_TEST_OK(m3_cocoa_backend_get_gfx(backend, &gfx));
        M3_TEST_OK(m3_cocoa_backend_get_env(backend, &env));

        M3_TEST_OK(m3_cocoa_backend_destroy(backend));
    }
#endif

    return 0;
}
