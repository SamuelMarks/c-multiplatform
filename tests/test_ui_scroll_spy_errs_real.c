void test_ui_scroll_spy_err_nulls_real(void) {
  ui_scroll_spy_create(NULL);
  (void)ui_scroll_spy_destroy(NULL);
  ui_scroll_spy_set_root(NULL, NULL, 0);
  ui_scroll_spy_add_target(NULL, NULL, 0);
  ui_scroll_spy_remove_target(NULL, NULL);
  ui_scroll_spy_bind_active_section(NULL, NULL);
  ui_scroll_spy_evaluate(NULL);
}
void test_ui_scroll_spy_err_nulls_real2(void) {
  struct ui_scroll_spy *spy = NULL;
  ui_scroll_spy_create(&spy);
  if (spy) {
    ui_scroll_spy_add_target(spy, NULL, 0);
    ui_scroll_spy_remove_target(spy, NULL);
    /* remove non existent */
    ui_scroll_spy_remove_target(spy, (struct ui_dom_node *)0x1);
    ui_scroll_spy_bind_active_section(spy, NULL);
    (void)ui_scroll_spy_destroy(spy);
  }
}
void test_ui_scroll_spy_err_nulls_real3(void) {
  (void)ui_scroll_spy_destroy(NULL);
  ui_scroll_spy_evaluate(NULL);
}
