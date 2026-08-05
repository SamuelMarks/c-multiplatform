void test_ui_scroll_spy_extra(void) {
  struct ui_scroll_spy *spy = NULL;
  ui_scroll_spy_create(&spy);
  if (spy) {
    ui_scroll_spy_set_root(NULL, NULL, 0);
    struct ui_dom_node *target1 = (struct ui_dom_node *)0x10;

    ui_scroll_spy_add_target(NULL, target1, 1);
    ui_scroll_spy_add_target(spy, NULL, 1);

    ui_scroll_spy_remove_target(NULL, target1);
    ui_scroll_spy_remove_target(spy, NULL);

    /* Cover remove target not found */
    ui_scroll_spy_remove_target(spy, target1);

    ui_scroll_spy_evaluate(NULL);
    (void)ui_scroll_spy_destroy(NULL);

    ui_scroll_spy_bind_active_section(NULL, NULL);
    ui_scroll_spy_bind_active_section(spy, NULL);
    ui_scroll_spy_bind_active_section(spy, (struct ui_signal *)0x1);

    int i;
    for (i = 0; i < 20; ++i) {
      ui_scroll_spy_add_target(spy, (struct ui_dom_node *)(size_t)(0x10 + i),
                               i);
    }

    /* test out of bounds */
    ui_scroll_spy_add_target(spy, target1, 99);

    /* to test UI_ERROR_OUT_OF_MEMORY we mock via countdown */
    (void)ui_scroll_spy_destroy(spy);
  }
}

void test_ui_scroll_spy_oom(void) {
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
    ui_scroll_spy_add_target(spy, (struct ui_dom_node *)0x10, 1);
    /* set_root creates intersection observer, so it can fail */
    for (i = 0; i < 5; ++i) {
      g_malloc_fail_countdown = i;
      ui_scroll_spy_set_root(spy, NULL, 0);
    }
    g_malloc_fail_countdown = -1;

    /* observer destruction is hit inside set_root */
    ui_scroll_spy_set_root(spy, NULL, 0);
    ui_scroll_spy_set_root(spy, NULL,
                           0); /* hits spy->observer != NULL branch */

    (void)ui_scroll_spy_destroy(spy);
  }
}
