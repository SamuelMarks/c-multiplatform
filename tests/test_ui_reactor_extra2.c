void test_ui_reactor_oom_loop(void) {
  extern int g_malloc_fail_countdown;
  struct ui_reactor *reactor = NULL;

  ui_reactor_create(&reactor);
  if (reactor) {
    ui_reactor_register(reactor, (void *)1, UI_REACTOR_EVENT_READ,
                        (ui_error_t(*)(void *, int, void *))0x1, NULL);
    ui_reactor_schedule(reactor, (ui_error_t(*)(void *))0x1, NULL);
    ui_reactor_schedule(reactor, (ui_error_t(*)(void *))0x1, NULL);
    ui_reactor_destroy(reactor);
  }
}
