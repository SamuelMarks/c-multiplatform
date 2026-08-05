void test_ui_router_extra2(void) {
  /* need to actually get the request. The test sets up a mock route factory */
  struct ui_router *router = NULL;
  ui_router_create(&router);
  ui_router_navigate(router, "/my/path?k=v");
  (void)ui_router_destroy(router);
}
