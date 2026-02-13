#include "cmpc/cmp_backend_cocoa.h"

#include <string.h>

#define CMP_COCOA_RETURN_IF_ERROR(rc)                                           \
  do {                                                                         \
    if ((rc) != CMP_OK) {                                                       \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#define CMP_COCOA_DEFAULT_HANDLE_CAPACITY 64

int CMP_CALL cmp_cocoa_backend_is_available(CMPBool *out_available) {
  if (out_available == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(CMP_COCOA_AVAILABLE)
  *out_available = CMP_TRUE;
#else
  *out_available = CMP_FALSE;
#endif
  return CMP_OK;
}

int CMP_CALL cmp_cocoa_backend_config_init(CMPCocoaBackendConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  config->allocator = NULL;
  config->handle_capacity = CMP_COCOA_DEFAULT_HANDLE_CAPACITY;
  config->clipboard_limit = (cmp_usize) ~(cmp_usize)0;
  config->enable_logging = CMP_TRUE;
  config->inline_tasks = CMP_TRUE;
  return CMP_OK;
}

#if !defined(CMP_COCOA_AVAILABLE)

static int
cmp_cocoa_backend_validate_config(const CMPCocoaBackendConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->handle_capacity == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->allocator != NULL) {
    if (config->allocator->alloc == NULL ||
        config->allocator->realloc == NULL || config->allocator->free == NULL) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
  }
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL
cmp_cocoa_backend_test_validate_config(const CMPCocoaBackendConfig *config) {
  return cmp_cocoa_backend_validate_config(config);
}
#endif

int CMP_CALL cmp_cocoa_backend_create(const CMPCocoaBackendConfig *config,
                                    CMPCocoaBackend **out_backend) {
  CMPCocoaBackendConfig local_config;
  int rc;

  if (out_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = cmp_cocoa_backend_config_init(&local_config);
    CMP_COCOA_RETURN_IF_ERROR(rc);
    config = &local_config;
  }

  rc = cmp_cocoa_backend_validate_config(config);
  CMP_COCOA_RETURN_IF_ERROR(rc);

  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_cocoa_backend_destroy(CMPCocoaBackend *backend) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_cocoa_backend_get_ws(CMPCocoaBackend *backend, CMPWS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_ws, 0, sizeof(*out_ws));
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_cocoa_backend_get_gfx(CMPCocoaBackend *backend, CMPGfx *out_gfx) {
  if (backend == NULL || out_gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_gfx, 0, sizeof(*out_gfx));
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_cocoa_backend_get_env(CMPCocoaBackend *backend, CMPEnv *out_env) {
  if (backend == NULL || out_env == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_env, 0, sizeof(*out_env));
  return CMP_ERR_UNSUPPORTED;
}

#endif
