void test_ui_router_oom_add2(void) {
  extern int g_malloc_fail_countdown;
  struct ui_router *router = NULL;
  ui_router_create(&router);
  if (router) {
    for (int i = 0; i < 5; i++) {
      g_malloc_fail_countdown = i;
      ui_router_add_route(router, "/my/route/pattern", mock_factory_success,
                          NULL);
    }
    g_malloc_fail_countdown = -1;
    (void)ui_router_destroy(router);
  }
}
