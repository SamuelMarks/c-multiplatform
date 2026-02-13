#include "cmpc/cmp_backend_ios.h"

#include "cmpc/cmp_backend_null.h"

#include <string.h>

#define CMP_IOS_RETURN_IF_ERROR(rc)                                            \
  do {                                                                         \
    if ((rc) != CMP_OK) {                                                      \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#define CMP_IOS_DEFAULT_HANDLE_CAPACITY 64u

static int cmp_ios_backend_validate_config(const CMPIOSBackendConfig *config) {
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
  if (config->predictive_back != NULL &&
      config->predictive_back->initialized != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL
cmp_ios_backend_test_validate_config(const CMPIOSBackendConfig *config) {
  return cmp_ios_backend_validate_config(config);
}
#endif

int CMP_CALL cmp_ios_backend_is_available(CMPBool *out_available) {
  if (out_available == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(CMP_IOS_AVAILABLE)
  *out_available = CMP_TRUE;
#else
  *out_available = CMP_FALSE;
#endif
  return CMP_OK;
}

int CMP_CALL cmp_ios_backend_config_init(CMPIOSBackendConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  config->allocator = NULL;
  config->handle_capacity = CMP_IOS_DEFAULT_HANDLE_CAPACITY;
  config->clipboard_limit = (cmp_usize) ~(cmp_usize)0;
  config->enable_logging = CMP_TRUE;
  config->inline_tasks = CMP_TRUE;
  config->predictive_back = NULL;
  return CMP_OK;
}

/* GCOVR_EXCL_START */
#if defined(CMP_IOS_AVAILABLE)

struct CMPIOSBackend {
  CMPAllocator allocator;
  CMPNullBackend *null_backend;
  CMPWS ws;
  CMPGfx gfx;
  CMPEnv env;
  CMPPredictiveBack *predictive_back;
  CMPBool initialized;
};

static int cmp_ios_backend_cleanup(CMPAllocator *allocator,
                                   struct CMPIOSBackend *backend,
                                   int primary_error) {
  int rc;
  int result;

  if (allocator == NULL || backend == NULL) {
    return primary_error;
  }

  result = primary_error;

  if (backend->null_backend != NULL) {
    rc = cmp_null_backend_destroy(backend->null_backend);
    if (result == CMP_OK && rc != CMP_OK) {
      result = rc;
    }
  }

  rc = allocator->free(allocator->ctx, backend);
  if (result == CMP_OK && rc != CMP_OK) {
    result = rc;
  }

  return result;
}

int CMP_CALL cmp_ios_backend_create(const CMPIOSBackendConfig *config,
                                    CMPIOSBackend **out_backend) {
  CMPIOSBackendConfig local_config;
  CMPAllocator allocator;
  CMPNullBackendConfig null_config;
  struct CMPIOSBackend *backend;
  int rc;

  if (out_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = cmp_ios_backend_config_init(&local_config);
    CMP_IOS_RETURN_IF_ERROR(rc);
    config = &local_config;
  }

  rc = cmp_ios_backend_validate_config(config);
  CMP_IOS_RETURN_IF_ERROR(rc);

  if (config->allocator == NULL) {
    rc = cmp_get_default_allocator(&allocator);
    CMP_IOS_RETURN_IF_ERROR(rc);
  } else {
    allocator = *config->allocator;
  }

  if (allocator.alloc == NULL || allocator.realloc == NULL ||
      allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = allocator.alloc(allocator.ctx, sizeof(CMPIOSBackend), (void **)&backend);
  CMP_IOS_RETURN_IF_ERROR(rc);

  memset(backend, 0, sizeof(*backend));
  backend->allocator = allocator;

  rc = cmp_null_backend_config_init(&null_config);
  if (rc != CMP_OK) {
    return cmp_ios_backend_cleanup(&allocator, backend, rc);
  }

  null_config.allocator = &allocator;
  null_config.handle_capacity = config->handle_capacity;
  null_config.clipboard_limit = config->clipboard_limit;
  null_config.enable_logging = config->enable_logging;
  null_config.inline_tasks = config->inline_tasks;

  rc = cmp_null_backend_create(&null_config, &backend->null_backend);
  if (rc != CMP_OK) {
    return cmp_ios_backend_cleanup(&allocator, backend, rc);
  }

  rc = cmp_null_backend_get_ws(backend->null_backend, &backend->ws);
  if (rc != CMP_OK) {
    return cmp_ios_backend_cleanup(&allocator, backend, rc);
  }

  rc = cmp_null_backend_get_gfx(backend->null_backend, &backend->gfx);
  if (rc != CMP_OK) {
    return cmp_ios_backend_cleanup(&allocator, backend, rc);
  }

  rc = cmp_null_backend_get_env(backend->null_backend, &backend->env);
  if (rc != CMP_OK) {
    return cmp_ios_backend_cleanup(&allocator, backend, rc);
  }

  backend->predictive_back = config->predictive_back;
  backend->initialized = CMP_TRUE;
  *out_backend = backend;
  return CMP_OK;
}

int CMP_CALL cmp_ios_backend_destroy(CMPIOSBackend *backend) {
  int rc;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }

  rc = cmp_null_backend_destroy(backend->null_backend);
  CMP_IOS_RETURN_IF_ERROR(rc);

  backend->null_backend = NULL;
  backend->predictive_back = NULL;
  backend->initialized = CMP_FALSE;
  return backend->allocator.free(backend->allocator.ctx, backend);
}

int CMP_CALL cmp_ios_backend_get_ws(CMPIOSBackend *backend, CMPWS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }
  *out_ws = backend->ws;
  return CMP_OK;
}

int CMP_CALL cmp_ios_backend_get_gfx(CMPIOSBackend *backend, CMPGfx *out_gfx) {
  if (backend == NULL || out_gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }
  *out_gfx = backend->gfx;
  return CMP_OK;
}

int CMP_CALL cmp_ios_backend_get_env(CMPIOSBackend *backend, CMPEnv *out_env) {
  if (backend == NULL || out_env == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }
  *out_env = backend->env;
  return CMP_OK;
}

int CMP_CALL cmp_ios_backend_set_predictive_back(
    CMPIOSBackend *backend, CMPPredictiveBack *predictive) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }
  if (predictive != NULL && predictive->initialized != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  backend->predictive_back = predictive;
  return CMP_OK;
}

int CMP_CALL cmp_ios_backend_get_predictive_back(
    CMPIOSBackend *backend, CMPPredictiveBack **out_predictive) {
  if (backend == NULL || out_predictive == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }
  *out_predictive = backend->predictive_back;
  return CMP_OK;
}

int CMP_CALL cmp_ios_backend_predictive_back_start(
    CMPIOSBackend *backend, const CMPPredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized || backend->predictive_back == NULL) {
    return CMP_ERR_STATE;
  }
  return cmp_predictive_back_start(backend->predictive_back, event);
}

int CMP_CALL cmp_ios_backend_predictive_back_progress(
    CMPIOSBackend *backend, const CMPPredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized || backend->predictive_back == NULL) {
    return CMP_ERR_STATE;
  }
  return cmp_predictive_back_progress(backend->predictive_back, event);
}

int CMP_CALL cmp_ios_backend_predictive_back_commit(
    CMPIOSBackend *backend, const CMPPredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized || backend->predictive_back == NULL) {
    return CMP_ERR_STATE;
  }
  return cmp_predictive_back_commit(backend->predictive_back, event);
}

int CMP_CALL cmp_ios_backend_predictive_back_cancel(
    CMPIOSBackend *backend, const CMPPredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized || backend->predictive_back == NULL) {
    return CMP_ERR_STATE;
  }
  return cmp_predictive_back_cancel(backend->predictive_back, event);
}

/* GCOVR_EXCL_STOP */
#else

int CMP_CALL cmp_ios_backend_create(const CMPIOSBackendConfig *config,
                                    CMPIOSBackend **out_backend) {
  CMPIOSBackendConfig local_config;
  int rc;

  if (out_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = cmp_ios_backend_config_init(&local_config);
    CMP_IOS_RETURN_IF_ERROR(rc);
    config = &local_config;
  }

  rc = cmp_ios_backend_validate_config(config);
  CMP_IOS_RETURN_IF_ERROR(rc);

  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_destroy(CMPIOSBackend *backend) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_get_ws(CMPIOSBackend *backend, CMPWS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_ws, 0, sizeof(*out_ws));
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_get_gfx(CMPIOSBackend *backend, CMPGfx *out_gfx) {
  if (backend == NULL || out_gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_gfx, 0, sizeof(*out_gfx));
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_get_env(CMPIOSBackend *backend, CMPEnv *out_env) {
  if (backend == NULL || out_env == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_env, 0, sizeof(*out_env));
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_set_predictive_back(
    CMPIOSBackend *backend, CMPPredictiveBack *predictive) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  CMP_UNUSED(predictive);
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_get_predictive_back(
    CMPIOSBackend *backend, CMPPredictiveBack **out_predictive) {
  if (backend == NULL || out_predictive == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_predictive = NULL;
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_predictive_back_start(
    CMPIOSBackend *backend, const CMPPredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_predictive_back_progress(
    CMPIOSBackend *backend, const CMPPredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_predictive_back_commit(
    CMPIOSBackend *backend, const CMPPredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_predictive_back_cancel(
    CMPIOSBackend *backend, const CMPPredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_ERR_UNSUPPORTED;
}

#endif
