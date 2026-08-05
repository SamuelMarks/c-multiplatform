void test_ui_router_coverage_errs1(void) {
  /* To hit null out_path */
  ui_route_request_get_path(NULL, NULL);
  /* null out_state */
  ui_route_request_get_state(NULL, NULL);
}
