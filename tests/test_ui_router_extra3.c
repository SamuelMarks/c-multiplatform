void test_ui_router_errs(void) {
  /* direct calls to trigger null checks safely */
  ui_route_request_get_path(NULL, NULL);
  ui_route_request_get_state(NULL, NULL);
}
