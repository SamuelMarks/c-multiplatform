void test_ui_reactor_oom(void) {
  extern int g_malloc_fail_countdown;
  struct ui_reactor *reactor = NULL;

  g_malloc_fail_countdown = 0;
  ui_reactor_create(&reactor);
  g_malloc_fail_countdown = -1;

  ui_reactor_create(&reactor);
  if (reactor) {
    g_malloc_fail_countdown = 0;
    ui_reactor_register(reactor, (void *)1, UI_REACTOR_EVENT_READ,
                        (ui_error_t(*)(void *, int, void *))0x1, NULL);
    g_malloc_fail_countdown = -1;

    /* Cover destroy while having tasks and nodes */
    ui_reactor_register(reactor, (void *)1, UI_REACTOR_EVENT_READ,
                        (ui_error_t(*)(void *, int, void *))0x1, NULL);
    ui_reactor_schedule(reactor, (ui_error_t(*)(void *))0x1, NULL);
    ui_reactor_schedule(reactor, (ui_error_t(*)(void *))0x1, NULL);
    ui_reactor_destroy(reactor);
  }
}
