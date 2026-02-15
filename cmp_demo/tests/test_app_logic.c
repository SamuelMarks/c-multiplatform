/* tests/test_app_logic.c */
#include <app/demo_app.h>
#include <assert.h>
#include <cmpc/cmp_backend_null.h>
#include <stdio.h>

int main(void) {
  CMPAllocator alloc;
  cmp_get_default_allocator(&alloc);

  DemoApp *app = NULL;
  int rc = demo_app_create(&alloc, &app);
  assert(rc == 0 && "App creation failed");

  /* Setup Mock Backend */
  CMPNullBackendConfig cfg;
  cmp_null_backend_config_init(&cfg);
  CMPNullBackend *backend;
  cmp_null_backend_create(&cfg, &backend);

  CMPGfx gfx;
  CMPEnv env;
  cmp_null_backend_get_gfx(backend, &gfx);
  cmp_null_backend_get_env(backend, &env);

  rc = demo_app_init_resources(app, &gfx, &env);
  assert(rc == 0 && "Resource init failed");

  /* 1. Simulate Render to perform layout */
  demo_app_render(app, &gfx, (CMPHandle){0, 0}, 800, 600, 1.0f);

  /* 2. Update Step */
  demo_app_update(app, 0.016);

  /* 3. Cleanup */
  demo_app_destroy(app);
  cmp_null_backend_destroy(backend);

  printf("App logic tests passed.\n");
  return 0;
}