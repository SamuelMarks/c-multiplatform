#include "test_utils.h"

#include "cmpc/cmp_backend_gtk4.h"

#include <string.h>

int main(void) {
  CMPGTK4BackendConfig config;
  CMPGTK4Backend *backend;
  CMPAllocator default_alloc;
  CMPBool available;

  CMP_TEST_EXPECT(cmp_gtk4_backend_config_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_gtk4_backend_config_init(&config));
  CMP_TEST_ASSERT(config.handle_capacity != 0);
  CMP_TEST_ASSERT(config.clipboard_limit != 0);
  CMP_TEST_ASSERT(config.enable_logging == CMP_TRUE);
  CMP_TEST_ASSERT(config.inline_tasks == CMP_TRUE);
  CMP_TEST_ASSERT(config.enable_gdk_texture == CMP_TRUE);

  CMP_TEST_EXPECT(cmp_gtk4_backend_test_validate_config(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_get_default_allocator(&default_alloc));
  CMP_TEST_OK(cmp_gtk4_backend_config_init(&config));
  config.allocator = &default_alloc;
  CMP_TEST_OK(cmp_gtk4_backend_test_validate_config(&config));
  {
    CMPAllocator bad_alloc;

    CMP_TEST_OK(cmp_gtk4_backend_config_init(&config));
    bad_alloc = default_alloc;
    bad_alloc.alloc = NULL;
    config.allocator = &bad_alloc;
    CMP_TEST_EXPECT(cmp_gtk4_backend_test_validate_config(&config),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(cmp_gtk4_backend_config_init(&config));
    bad_alloc = default_alloc;
    bad_alloc.realloc = NULL;
    config.allocator = &bad_alloc;
    CMP_TEST_EXPECT(cmp_gtk4_backend_test_validate_config(&config),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(cmp_gtk4_backend_config_init(&config));
    bad_alloc = default_alloc;
    bad_alloc.free = NULL;
    config.allocator = &bad_alloc;
    CMP_TEST_EXPECT(cmp_gtk4_backend_test_validate_config(&config),
                    CMP_ERR_INVALID_ARGUMENT);
  }

  CMP_TEST_EXPECT(cmp_gtk4_backend_is_available(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_gtk4_backend_is_available(&available));

  backend = NULL;
  CMP_TEST_EXPECT(cmp_gtk4_backend_create(NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_gtk4_backend_test_set_config_init_fail(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_gtk4_backend_create(NULL, &backend), CMP_ERR_IO);
  {
    int rc;
    rc = cmp_gtk4_backend_create(NULL, &backend);
    if (!available) {
      CMP_TEST_EXPECT(rc, CMP_ERR_UNSUPPORTED);
      CMP_TEST_ASSERT(backend == NULL);
    } else if (rc == CMP_OK && backend != NULL) {
      CMP_TEST_OK(cmp_gtk4_backend_destroy(backend));
    }
  }

  CMP_TEST_OK(cmp_gtk4_backend_config_init(&config));
  config.handle_capacity = 0;
  CMP_TEST_EXPECT(cmp_gtk4_backend_create(&config, &backend),
                  CMP_ERR_INVALID_ARGUMENT);

  {
    CMPAllocator bad_alloc;
    CMP_TEST_OK(cmp_gtk4_backend_config_init(&config));
    memset(&bad_alloc, 0, sizeof(bad_alloc));
    config.allocator = &bad_alloc;
    CMP_TEST_EXPECT(cmp_gtk4_backend_create(&config, &backend),
                    CMP_ERR_INVALID_ARGUMENT);
  }

  if (!available) {
    CMPWS ws;
    CMPGfx gfx;
    CMPEnv env;

    CMP_TEST_OK(cmp_gtk4_backend_config_init(&config));
    backend = NULL;
    CMP_TEST_EXPECT(cmp_gtk4_backend_create(&config, &backend),
                    CMP_ERR_UNSUPPORTED);
    CMP_TEST_ASSERT(backend == NULL);

    CMP_TEST_EXPECT(cmp_gtk4_backend_destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_gtk4_backend_destroy((CMPGTK4Backend *)1),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(cmp_gtk4_backend_get_ws(NULL, &ws),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_gtk4_backend_get_ws((CMPGTK4Backend *)1, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_gtk4_backend_get_ws((CMPGTK4Backend *)1, &ws),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(cmp_gtk4_backend_get_gfx(NULL, &gfx),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_gtk4_backend_get_gfx((CMPGTK4Backend *)1, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_gtk4_backend_get_gfx((CMPGTK4Backend *)1, &gfx),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(cmp_gtk4_backend_get_env(NULL, &env),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_gtk4_backend_get_env((CMPGTK4Backend *)1, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_gtk4_backend_get_env((CMPGTK4Backend *)1, &env),
                    CMP_ERR_UNSUPPORTED);

    return 0;
  }

#if defined(CMP_GTK4_AVAILABLE)
  {
    CMPWS ws;
    CMPGfx gfx;
    CMPEnv env;
    int rc;

    CMP_TEST_OK(cmp_gtk4_backend_config_init(&config));
    backend = NULL;
    rc = cmp_gtk4_backend_create(&config, &backend);
    if (rc != CMP_OK) {
      return 0;
    }

    CMP_TEST_OK(cmp_gtk4_backend_get_ws(backend, &ws));
    CMP_TEST_OK(cmp_gtk4_backend_get_gfx(backend, &gfx));
    CMP_TEST_OK(cmp_gtk4_backend_get_env(backend, &env));

    CMP_TEST_OK(cmp_gtk4_backend_destroy(backend));
  }
#endif

  return 0;
}
