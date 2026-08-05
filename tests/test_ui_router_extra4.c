void test_ui_router_errs2(void) {
  struct ui_router *router = NULL;
  ui_router_create(&router);
  ui_router_navigate(router, "/my/path?k=v");
  (void)ui_router_destroy(router);
}
