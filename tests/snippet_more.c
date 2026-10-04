/* Edge case: destructor fail in ui_signal_set */
{
  struct ui_signal *sig = NULL;
  union ui_signal_payload init_val;
  union ui_signal_payload new_val;
  ui_error_t rc;

  init_val.int_val = 0;
  new_val.int_val = 1;

  extern ui_error_t mock_destructor_fail(union ui_signal_payload val);
  rc = ui_signal_create(NULL, init_val, UI_SIGNAL_TYPE_INT32, NULL,
                        mock_destructor_fail, UI_SIGNAL_MODE_SINGLE_THREADED,
                        &sig);
  assert(rc == UI_ERROR_NONE);

  rc = ui_signal_set(sig, new_val);
  assert(rc == UI_ERROR_OUT_OF_MEMORY);

  sig->destructor_fn = NULL;
  ui_signal_destroy(sig);
}

/* Edge case: multiple notify fails to test rc != UI_ERROR_NONE */
{
  struct ui_signal *sig = NULL;
  struct ui_reactive_node sub1, sub2;
  struct ui_reactive_node *prev_sub = NULL;
  union ui_signal_payload init_val;
  union ui_signal_payload new_val;
  ui_error_t rc;

  init_val.int_val = 0;
  new_val.int_val = 1;

  rc = ui_signal_create(NULL, init_val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &sig);
  assert(rc == UI_ERROR_NONE);

  sub1.notify_fn = mock_notify_fail;
  sub1.user_data = NULL;
  ui_reactive_graph_set_current_node(&sub1, &prev_sub);
  ui_signal_get(sig, &init_val); /* subscribes it */
  ui_reactive_graph_set_current_node(prev_sub, &prev_sub);

  sub2.notify_fn = mock_notify_fail;
  sub2.user_data = NULL;
  ui_reactive_graph_set_current_node(&sub2, &prev_sub);
  ui_signal_get(sig, &init_val); /* subscribes it */
  ui_reactive_graph_set_current_node(prev_sub, &prev_sub);

  rc = ui_signal_set(sig, new_val);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  ui_signal_destroy(sig);
}
