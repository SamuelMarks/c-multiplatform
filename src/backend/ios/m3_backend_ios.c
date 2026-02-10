#include "m3/m3_backend_ios.h"

#include "m3/m3_backend_null.h"

#include <string.h>

#define M3_IOS_RETURN_IF_ERROR(rc) \
    do { \
        if ((rc) != M3_OK) { \
            return (rc); \
        } \
    } while (0)

#define M3_IOS_DEFAULT_HANDLE_CAPACITY 64u

static int m3_ios_backend_validate_config(const M3IOSBackendConfig *config)
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
int M3_CALL m3_ios_backend_test_validate_config(const M3IOSBackendConfig *config)
{
    return m3_ios_backend_validate_config(config);
}
#endif

int M3_CALL m3_ios_backend_is_available(M3Bool *out_available)
{
    if (out_available == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#if defined(M3_IOS_AVAILABLE)
    *out_available = M3_TRUE;
#else
    *out_available = M3_FALSE;
#endif
    return M3_OK;
}

int M3_CALL m3_ios_backend_config_init(M3IOSBackendConfig *config)
{
    if (config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    config->allocator = NULL;
    config->handle_capacity = M3_IOS_DEFAULT_HANDLE_CAPACITY;
    config->clipboard_limit = (m3_usize)~(m3_usize)0;
    config->enable_logging = M3_TRUE;
    config->inline_tasks = M3_TRUE;
    return M3_OK;
}

#if defined(M3_IOS_AVAILABLE)

struct M3IOSBackend {
    M3Allocator allocator;
    M3NullBackend *null_backend;
    M3WS ws;
    M3Gfx gfx;
    M3Env env;
    M3Bool initialized;
};

static int m3_ios_backend_cleanup(M3Allocator *allocator, struct M3IOSBackend *backend, int primary_error)
{
    int rc;
    int result;

    if (allocator == NULL || backend == NULL) {
        return primary_error;
    }

    result = primary_error;

    if (backend->null_backend != NULL) {
        rc = m3_null_backend_destroy(backend->null_backend);
        if (result == M3_OK && rc != M3_OK) {
            result = rc;
        }
    }

    rc = allocator->free(allocator->ctx, backend);
    if (result == M3_OK && rc != M3_OK) {
        result = rc;
    }

    return result;
}

int M3_CALL m3_ios_backend_create(const M3IOSBackendConfig *config, M3IOSBackend **out_backend)
{
    M3IOSBackendConfig local_config;
    M3Allocator allocator;
    M3NullBackendConfig null_config;
    struct M3IOSBackend *backend;
    int rc;

    if (out_backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_backend = NULL;

    if (config == NULL) {
        rc = m3_ios_backend_config_init(&local_config);
        M3_IOS_RETURN_IF_ERROR(rc);
        config = &local_config;
    }

    rc = m3_ios_backend_validate_config(config);
    M3_IOS_RETURN_IF_ERROR(rc);

    if (config->allocator == NULL) {
        rc = m3_get_default_allocator(&allocator);
        M3_IOS_RETURN_IF_ERROR(rc);
    } else {
        allocator = *config->allocator;
    }

    if (allocator.alloc == NULL || allocator.realloc == NULL || allocator.free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = allocator.alloc(allocator.ctx, sizeof(M3IOSBackend), (void **)&backend);
    M3_IOS_RETURN_IF_ERROR(rc);

    memset(backend, 0, sizeof(*backend));
    backend->allocator = allocator;

    rc = m3_null_backend_config_init(&null_config);
    if (rc != M3_OK) {
        return m3_ios_backend_cleanup(&allocator, backend, rc);
    }

    null_config.allocator = &allocator;
    null_config.handle_capacity = config->handle_capacity;
    null_config.clipboard_limit = config->clipboard_limit;
    null_config.enable_logging = config->enable_logging;
    null_config.inline_tasks = config->inline_tasks;

    rc = m3_null_backend_create(&null_config, &backend->null_backend);
    if (rc != M3_OK) {
        return m3_ios_backend_cleanup(&allocator, backend, rc);
    }

    rc = m3_null_backend_get_ws(backend->null_backend, &backend->ws);
    if (rc != M3_OK) {
        return m3_ios_backend_cleanup(&allocator, backend, rc);
    }

    rc = m3_null_backend_get_gfx(backend->null_backend, &backend->gfx);
    if (rc != M3_OK) {
        return m3_ios_backend_cleanup(&allocator, backend, rc);
    }

    rc = m3_null_backend_get_env(backend->null_backend, &backend->env);
    if (rc != M3_OK) {
        return m3_ios_backend_cleanup(&allocator, backend, rc);
    }

    backend->initialized = M3_TRUE;
    *out_backend = backend;
    return M3_OK;
}

int M3_CALL m3_ios_backend_destroy(M3IOSBackend *backend)
{
    int rc;

    if (backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return M3_ERR_STATE;
    }

    rc = m3_null_backend_destroy(backend->null_backend);
    M3_IOS_RETURN_IF_ERROR(rc);

    backend->null_backend = NULL;
    backend->initialized = M3_FALSE;
    return backend->allocator.free(backend->allocator.ctx, backend);
}

int M3_CALL m3_ios_backend_get_ws(M3IOSBackend *backend, M3WS *out_ws)
{
    if (backend == NULL || out_ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return M3_ERR_STATE;
    }
    *out_ws = backend->ws;
    return M3_OK;
}

int M3_CALL m3_ios_backend_get_gfx(M3IOSBackend *backend, M3Gfx *out_gfx)
{
    if (backend == NULL || out_gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return M3_ERR_STATE;
    }
    *out_gfx = backend->gfx;
    return M3_OK;
}

int M3_CALL m3_ios_backend_get_env(M3IOSBackend *backend, M3Env *out_env)
{
    if (backend == NULL || out_env == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return M3_ERR_STATE;
    }
    *out_env = backend->env;
    return M3_OK;
}

#else

int M3_CALL m3_ios_backend_create(const M3IOSBackendConfig *config, M3IOSBackend **out_backend)
{
    M3IOSBackendConfig local_config;
    int rc;

    if (out_backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_backend = NULL;

    if (config == NULL) {
        rc = m3_ios_backend_config_init(&local_config);
        M3_IOS_RETURN_IF_ERROR(rc);
        config = &local_config;
    }

    rc = m3_ios_backend_validate_config(config);
    M3_IOS_RETURN_IF_ERROR(rc);

    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_ios_backend_destroy(M3IOSBackend *backend)
{
    if (backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_ios_backend_get_ws(M3IOSBackend *backend, M3WS *out_ws)
{
    if (backend == NULL || out_ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    memset(out_ws, 0, sizeof(*out_ws));
    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_ios_backend_get_gfx(M3IOSBackend *backend, M3Gfx *out_gfx)
{
    if (backend == NULL || out_gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    memset(out_gfx, 0, sizeof(*out_gfx));
    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_ios_backend_get_env(M3IOSBackend *backend, M3Env *out_env)
{
    if (backend == NULL || out_env == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    memset(out_env, 0, sizeof(*out_env));
    return M3_ERR_UNSUPPORTED;
}

#endif
