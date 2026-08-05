static ui_error_t cov_factory3(const struct ui_route_request *req,
                               void *user_data,
                               struct ui_component **out_screen) {
  const char *val;
  ui_route_request_get_param(req, "nonexistent", &val);
  return ui_component_create(out_screen);
}

void test_ui_router_missing_param(void) {
  struct ui_router *router = NULL;
  ui_router_create(&router);
  if (router) {
    ui_router_add_route(router, "/missing/:id", cov_factory3, NULL);
    ui_router_navigate(router, "/missing/123");
    (void)ui_router_destroy(router);
  }
}
