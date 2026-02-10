#include "m3/m3_backend_cocoa.h"

#include <string.h>

#define M3_COCOA_RETURN_IF_ERROR(rc) \
    do { \
        if ((rc) != M3_OK) { \
            return (rc); \
        } \
    } while (0)

#define M3_COCOA_DEFAULT_HANDLE_CAPACITY 64

int M3_CALL m3_cocoa_backend_is_available(M3Bool *out_available)
{
    if (out_available == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#if defined(M3_COCOA_AVAILABLE)
    *out_available = M3_TRUE;
#else
    *out_available = M3_FALSE;
#endif
    return M3_OK;
}

int M3_CALL m3_cocoa_backend_config_init(M3CocoaBackendConfig *config)
{
    if (config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    config->allocator = NULL;
    config->handle_capacity = M3_COCOA_DEFAULT_HANDLE_CAPACITY;
    config->clipboard_limit = (m3_usize)~(m3_usize)0;
    config->enable_logging = M3_TRUE;
    config->inline_tasks = M3_TRUE;
    return M3_OK;
}

#if !defined(M3_COCOA_AVAILABLE)

static int m3_cocoa_backend_validate_config(const M3CocoaBackendConfig *config)
{
    if (config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (config->handle_capacity == 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (config->allocator != NULL) {
        if (config->allocator->alloc == NULL || config->allocator->realloc == NULL || config->allocator->free == NULL) {
            return M3_ERR_INVALID_ARGUMENT;
        }
    }
    return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_cocoa_backend_test_validate_config(const M3CocoaBackendConfig *config)
{
    return m3_cocoa_backend_validate_config(config);
}
#endif

int M3_CALL m3_cocoa_backend_create(const M3CocoaBackendConfig *config, M3CocoaBackend **out_backend)
{
    M3CocoaBackendConfig local_config;
    int rc;

    if (out_backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_backend = NULL;

    if (config == NULL) {
        rc = m3_cocoa_backend_config_init(&local_config);
        M3_COCOA_RETURN_IF_ERROR(rc);
        config = &local_config;
    }

    rc = m3_cocoa_backend_validate_config(config);
    M3_COCOA_RETURN_IF_ERROR(rc);

    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_cocoa_backend_destroy(M3CocoaBackend *backend)
{
    if (backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_cocoa_backend_get_ws(M3CocoaBackend *backend, M3WS *out_ws)
{
    if (backend == NULL || out_ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    memset(out_ws, 0, sizeof(*out_ws));
    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_cocoa_backend_get_gfx(M3CocoaBackend *backend, M3Gfx *out_gfx)
{
    if (backend == NULL || out_gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    memset(out_gfx, 0, sizeof(*out_gfx));
    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_cocoa_backend_get_env(M3CocoaBackend *backend, M3Env *out_env)
{
    if (backend == NULL || out_env == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    memset(out_env, 0, sizeof(*out_env));
    return M3_ERR_UNSUPPORTED;
}

#endif
