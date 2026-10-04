/* Edge case: ui_signal_update with succeeding subscriber */
{
  struct ui_signal *sig = NULL;
  struct ui_reactive_node sub;
  struct ui_reactive_node *prev_sub = NULL;
  union ui_signal_payload init_val;
  ui_error_t rc;

  init_val.int_val = 0;

  extern ui_error_t mock_notify_success(void *user_data);

  rc = ui_signal_create(NULL, init_val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &sig);
  assert(rc == UI_ERROR_NONE);

  sub.notify_fn = mock_notify_success;
  sub.user_data = NULL;
  ui_reactive_graph_set_current_node(&sub, &prev_sub);
  ui_signal_get(sig, &init_val); /* subscribes it */
  ui_reactive_graph_set_current_node(prev_sub, &prev_sub);

  rc = ui_signal_update(sig, mock_update);
  assert(rc == UI_ERROR_NONE);

  ui_signal_destroy(sig);
}
