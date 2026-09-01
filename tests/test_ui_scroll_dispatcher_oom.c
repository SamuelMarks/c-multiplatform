void test_ui_scroll_dispatcher_oom(void) {
  extern int g_malloc_fail_countdown;
  struct ui_scroll_dispatcher *dispatcher = NULL;
  int i;
  for (i = 0; i < 5; ++i) {
    g_malloc_fail_countdown = i;
    ui_scroll_dispatcher_create(&dispatcher);
  }
  g_malloc_fail_countdown = -1;

  ui_scroll_dispatcher_create(&dispatcher);
  if (dispatcher) {
    for (i = 0; i < 5; ++i) {
      g_malloc_fail_countdown = i;
      int reg_id;
      ui_scroll_dispatcher_register(dispatcher, NULL, NULL, &reg_id);
    }
    g_malloc_fail_countdown = -1;

    /* force reallocation */
    for (i = 0; i < 20; ++i) {
      int reg_id;
      ui_scroll_dispatcher_register(dispatcher, NULL, NULL, &reg_id);
    }

    /* force OOM during realloc */
    g_malloc_fail_countdown = 0;
    int r;
    ui_scroll_dispatcher_register(dispatcher, NULL, NULL, &r);
    g_malloc_fail_countdown = -1;

    {
      ui_error_t rc_cleanup = ui_scroll_dispatcher_destroy(dispatcher);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  {
    ui_error_t rc_cleanup = ui_scroll_dispatcher_destroy(NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  ui_scroll_dispatcher_unregister(NULL, 0);
}
