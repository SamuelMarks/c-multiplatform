/* Edge case: malloc failure in ui_signal_set */
#ifdef UI_TEST_MOCK_ALLOC
{
  struct ui_signal *sig = NULL;
  struct ui_reactive_node sub;
  struct ui_reactive_node *prev_sub = NULL;
  union ui_signal_payload init_val;
  union ui_signal_payload new_val;
  ui_error_t rc;

  init_val.int_val = 0;
  new_val.int_val = 1;

  rc = ui_signal_create(NULL, init_val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &sig);
  assert(rc == UI_ERROR_NONE);

  sub.notify_fn = NULL;
  sub.user_data = NULL;
  ui_reactive_graph_set_current_node(&sub, &prev_sub);
  ui_signal_get(sig, &init_val); /* subscribes it */
  ui_reactive_graph_set_current_node(prev_sub, &prev_sub);

  g_malloc_fail_countdown = 0;
  rc = ui_signal_set(sig, new_val);
  assert(rc == UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  ui_signal_destroy(sig);
}
#endif

/* Edge case: notify fail */
{
  struct ui_signal *sig = NULL;
  struct ui_reactive_node sub;
  struct ui_reactive_node *prev_sub = NULL;
  union ui_signal_payload init_val;
  union ui_signal_payload new_val;
  ui_error_t rc;

  init_val.int_val = 0;
  new_val.int_val = 1;

  rc = ui_signal_create(NULL, init_val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &sig);
  assert(rc == UI_ERROR_NONE);

  sub.notify_fn = mock_notify_fail;
  sub.user_data = NULL;
  ui_reactive_graph_set_current_node(&sub, &prev_sub);
  ui_signal_get(sig, &init_val); /* subscribes it */
  ui_reactive_graph_set_current_node(prev_sub, &prev_sub);

  rc = ui_signal_set(sig, new_val);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_signal_update(sig, mock_update);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  ui_signal_destroy(sig);
}
