#include "test_utils.h"

#include "cmpc/cmp_backend_android.h"
#include "cmpc/cmp_predictive.h"

#include <string.h>

int main(void) {
  CMPAndroidBackendConfig config;
  CMPAndroidBackend *backend;
  CMPAllocator default_alloc;
  CMPBool available;

  CMP_TEST_EXPECT(cmp_android_backend_config_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_android_backend_config_init(&config));
  CMP_TEST_ASSERT(config.handle_capacity != 0);
  CMP_TEST_ASSERT(config.clipboard_limit != 0);
  CMP_TEST_ASSERT(config.enable_logging == CMP_TRUE);
  CMP_TEST_ASSERT(config.inline_tasks == CMP_TRUE);
  CMP_TEST_ASSERT(config.predictive_back == NULL);
  CMP_TEST_ASSERT(config.java_vm == NULL);
  CMP_TEST_ASSERT(config.activity == NULL);

  CMP_TEST_EXPECT(cmp_android_backend_test_validate_config(NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_get_default_allocator(&default_alloc));
  CMP_TEST_OK(cmp_android_backend_config_init(&config));
  config.allocator = &default_alloc;
  config.java_vm = (void *)1;
  config.activity = (void *)1;
  CMP_TEST_OK(cmp_android_backend_test_validate_config(&config));
  {
    CMPPredictiveBack predictive;
    memset(&predictive, 0, sizeof(predictive));
    config.predictive_back = &predictive;
    CMP_TEST_EXPECT(cmp_android_backend_test_validate_config(&config),
                   CMP_ERR_STATE);
    CMP_TEST_OK(cmp_predictive_back_init(&predictive, NULL));
    CMP_TEST_OK(cmp_android_backend_test_validate_config(&config));
    config.predictive_back = NULL;
  }

  CMP_TEST_EXPECT(cmp_android_backend_is_available(NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_android_backend_is_available(&available));

  backend = NULL;
  CMP_TEST_EXPECT(cmp_android_backend_create(NULL, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  {
    int rc;
    rc = cmp_android_backend_create(NULL, &backend);
    if (!available) {
      CMP_TEST_EXPECT(rc, CMP_ERR_UNSUPPORTED);
      CMP_TEST_ASSERT(backend == NULL);
    } else if (rc == CMP_OK && backend != NULL) {
      CMP_TEST_OK(cmp_android_backend_destroy(backend));
    }
  }

  CMP_TEST_OK(cmp_android_backend_config_init(&config));
  config.handle_capacity = 0;
  CMP_TEST_EXPECT(cmp_android_backend_create(&config, &backend),
                 CMP_ERR_INVALID_ARGUMENT);

  {
    CMPAllocator bad_alloc;
    CMP_TEST_OK(cmp_android_backend_config_init(&config));
    memset(&bad_alloc, 0, sizeof(bad_alloc));
    config.allocator = &bad_alloc;
    CMP_TEST_EXPECT(cmp_android_backend_create(&config, &backend),
                   CMP_ERR_INVALID_ARGUMENT);
  }

#if defined(CMP_ANDROID_AVAILABLE)
  CMP_TEST_OK(cmp_android_backend_config_init(&config));
  config.activity = (void *)1;
  CMP_TEST_EXPECT(cmp_android_backend_create(&config, &backend),
                 CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_android_backend_config_init(&config));
  config.java_vm = (void *)1;
  CMP_TEST_EXPECT(cmp_android_backend_create(&config, &backend),
                 CMP_ERR_INVALID_ARGUMENT);
#endif

  if (!available) {
    CMPWS ws;
    CMPGfx gfx;
    CMPEnv env;
    CMPPredictiveBack *predictive;
    CMPPredictiveBackEvent event;

    CMP_TEST_OK(cmp_android_backend_config_init(&config));
    backend = NULL;
    CMP_TEST_EXPECT(cmp_android_backend_create(&config, &backend),
                   CMP_ERR_UNSUPPORTED);
    CMP_TEST_ASSERT(backend == NULL);

    CMP_TEST_EXPECT(cmp_android_backend_destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_android_backend_destroy((CMPAndroidBackend *)1),
                   CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(cmp_android_backend_get_ws(NULL, &ws),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_android_backend_get_ws((CMPAndroidBackend *)1, NULL),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_android_backend_get_ws((CMPAndroidBackend *)1, &ws),
                   CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(cmp_android_backend_get_gfx(NULL, &gfx),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_android_backend_get_gfx((CMPAndroidBackend *)1, NULL),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_android_backend_get_gfx((CMPAndroidBackend *)1, &gfx),
                   CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(cmp_android_backend_get_env(NULL, &env),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_android_backend_get_env((CMPAndroidBackend *)1, NULL),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_android_backend_get_env((CMPAndroidBackend *)1, &env),
                   CMP_ERR_UNSUPPORTED);

    predictive = (CMPPredictiveBack *)1;
    CMP_TEST_EXPECT(cmp_android_backend_set_predictive_back(NULL, NULL),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_android_backend_get_predictive_back((CMPAndroidBackend *)1, NULL),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_android_backend_get_predictive_back((CMPAndroidBackend *)1,
                                                          &predictive),
                   CMP_ERR_UNSUPPORTED);
    CMP_TEST_ASSERT(predictive == NULL);
    CMP_TEST_EXPECT(
        cmp_android_backend_set_predictive_back((CMPAndroidBackend *)1, NULL),
        CMP_ERR_UNSUPPORTED);
    CMP_TEST_OK(cmp_predictive_back_event_init(&event));
    CMP_TEST_EXPECT(cmp_android_backend_predictive_back_start(NULL, &event),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_android_backend_predictive_back_start((CMPAndroidBackend *)1, NULL),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_android_backend_predictive_back_start((CMPAndroidBackend *)1, &event),
        CMP_ERR_UNSUPPORTED);
    CMP_TEST_EXPECT(cmp_android_backend_predictive_back_progress(NULL, &event),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_android_backend_predictive_back_progress(
                       (CMPAndroidBackend *)1, NULL),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_android_backend_predictive_back_progress(
                       (CMPAndroidBackend *)1, &event),
                   CMP_ERR_UNSUPPORTED);
    CMP_TEST_EXPECT(cmp_android_backend_predictive_back_commit(NULL, &event),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_android_backend_predictive_back_commit((CMPAndroidBackend *)1, NULL),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_android_backend_predictive_back_commit(
                       (CMPAndroidBackend *)1, &event),
                   CMP_ERR_UNSUPPORTED);
    CMP_TEST_EXPECT(cmp_android_backend_predictive_back_cancel(NULL, &event),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_android_backend_predictive_back_cancel((CMPAndroidBackend *)1, NULL),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_android_backend_predictive_back_cancel(
                       (CMPAndroidBackend *)1, &event),
                   CMP_ERR_UNSUPPORTED);

    return 0;
  }

#if defined(CMP_ANDROID_AVAILABLE)
  {
    CMPWS ws;
    CMPGfx gfx;
    CMPEnv env;
    CMPPredictiveBack predictive;
    CMPPredictiveBackEvent event;
    CMPPredictiveBack *out_predictive;
    int rc;

    CMP_TEST_OK(cmp_android_backend_config_init(&config));
    config.java_vm = (void *)1;
    config.activity = (void *)1;
    memset(&predictive, 0, sizeof(predictive));
    CMP_TEST_OK(cmp_predictive_back_init(&predictive, NULL));
    config.predictive_back = &predictive;
    backend = NULL;
    rc = cmp_android_backend_create(&config, &backend);
    if (rc != CMP_OK) {
      return 0;
    }

    CMP_TEST_OK(cmp_android_backend_get_ws(backend, &ws));
    CMP_TEST_OK(cmp_android_backend_get_gfx(backend, &gfx));
    CMP_TEST_OK(cmp_android_backend_get_env(backend, &env));
    out_predictive = NULL;
    CMP_TEST_OK(
        cmp_android_backend_get_predictive_back(backend, &out_predictive));
    CMP_TEST_ASSERT(out_predictive == &predictive);

    CMP_TEST_OK(cmp_predictive_back_event_init(&event));
    event.edge = CMP_PREDICTIVE_BACK_EDGE_LEFT;
    event.progress = 0.0f;
    CMP_TEST_EXPECT(cmp_android_backend_set_predictive_back(NULL, NULL),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_android_backend_predictive_back_start(NULL, &event),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_android_backend_predictive_back_progress(NULL, &event),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_android_backend_predictive_back_commit(NULL, &event),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_android_backend_predictive_back_cancel(NULL, &event),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_android_backend_predictive_back_start(backend, &event));
    event.progress = 0.5f;
    CMP_TEST_OK(cmp_android_backend_predictive_back_progress(backend, &event));
    event.progress = 1.0f;
    CMP_TEST_OK(cmp_android_backend_predictive_back_commit(backend, &event));
    CMP_TEST_OK(cmp_android_backend_predictive_back_start(backend, &event));
    CMP_TEST_OK(cmp_android_backend_predictive_back_cancel(backend, &event));

    CMP_TEST_OK(cmp_android_backend_destroy(backend));
  }
#endif

  return 0;
}
