void test_ui_reactor_poll_error(void) {
  struct ui_reactor *reactor = NULL;
  ui_reactor_create(&reactor);
  if (reactor) {
    /* To get cb_rc != UI_ERROR_NONE in the task queue */
    ui_reactor_schedule(reactor, (ui_error_t(*)(void *))0x1, NULL);
    /* Actually that will crash if it tries to execute 0x1. We need a real
     * callback. */
  }
}

static ui_error_t my_failing_task(void *data) { return UI_ERROR_OUT_OF_MEMORY; }

void test_ui_reactor_poll_error2(void) {
  struct ui_reactor *reactor = NULL;
  ui_reactor_create(&reactor);
  if (reactor) {
    ui_reactor_schedule(reactor, my_failing_task, NULL);
    ui_reactor_poll(reactor, 0);
    ui_reactor_destroy(reactor);
  }
}
