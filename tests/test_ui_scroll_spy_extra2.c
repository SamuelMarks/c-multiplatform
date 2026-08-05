void test_ui_scroll_spy_extra3(void) {
  /* let's mock it safely using the same technique as oom */
  extern int g_malloc_fail_countdown;
  struct ui_scroll_spy *spy = NULL;
  int i;
  for (i = 0; i < 5; ++i) {
    g_malloc_fail_countdown = i;
    ui_scroll_spy_create(&spy);
  }
  g_malloc_fail_countdown = -1;

  ui_scroll_spy_create(&spy);
  if (spy) {
    for (i = 0; i < 20; ++i) {
      ui_scroll_spy_add_target(spy, (struct ui_dom_node *)(size_t)(0x10 + i),
                               i);
    }
    /* trigger out of bounds */
    ui_scroll_spy_add_target(spy, (struct ui_dom_node *)0x1, 1);

    /* observer memory fail */
    for (i = 0; i < 5; ++i) {
      g_malloc_fail_countdown = i;
      ui_scroll_spy_set_root(spy, NULL, 0);
    }
    g_malloc_fail_countdown = -1;

    ui_scroll_spy_set_root(spy, NULL, 0); /* creates observer */
    ui_scroll_spy_set_root(spy, NULL, 0); /* observer destruction path */

    (void)ui_scroll_spy_destroy(spy);
  }
}

void test_ui_scroll_spy_err_nulls(void) {
  ui_scroll_spy_create(NULL);
  (void)ui_scroll_spy_destroy(NULL);
  ui_scroll_spy_set_root(NULL, NULL, 0);
  ui_scroll_spy_add_target(NULL, NULL, 0);
  ui_scroll_spy_remove_target(NULL, NULL);
  ui_scroll_spy_bind_active_section(NULL, NULL);
  ui_scroll_spy_evaluate(NULL);

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
