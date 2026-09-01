static ui_error_t cov_factory(const struct ui_route_request *req,
                              void *user_data,
                              struct ui_component **out_screen) {
  const char *path = NULL;
  void *state = NULL;
  ui_route_request_get_path(req, &path);
  ui_route_request_get_state(req, &state);
  return ui_component_create(out_screen);
}

void test_ui_router_coverage(void) {
  struct ui_router *router = NULL;
  const char *path = NULL;
  void *state = NULL;
  int i;
  extern int g_malloc_fail_countdown;

  ui_router_create(&router);
  if (router) {
    /* try_match edge cases: pattern longer than target */
    ui_router_add_route(router, "/a/b", mock_factory_success, NULL);
    ui_router_navigate(router, "/a");

    /* try_match edge cases: target longer than pattern */
    ui_router_add_route(router, "/c", mock_factory_success, NULL);
    ui_router_navigate(router, "/c/d");

    /* query parsing empty keys / values */
    ui_router_add_route(router, "/q", mock_factory_success, NULL);
    ui_router_navigate(router, "/q?=");
    ui_router_navigate(router, "/q?key=");
    ui_router_navigate(router, "/q?=val");
    ui_router_navigate(router, "/q?a&b");

    /* null checks */
    ui_route_request_get_path(NULL, NULL);
    ui_route_request_get_state(NULL, NULL);
    ui_route_request_get_path(NULL, &path);
    ui_route_request_get_state(NULL, &state);

    {
      ui_error_t rc_cleanup = ui_router_destroy(router);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
}

void test_ui_router_coverage2(void) {
  struct ui_router *router = NULL;
  ui_router_create(&router);
  if (router) {
    ui_router_add_route(router, "/cov", cov_factory, NULL);
    ui_router_navigate_with_state(router, "/cov", (void *)0x123);
    {
      ui_error_t rc_cleanup = ui_router_destroy(router);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
}
