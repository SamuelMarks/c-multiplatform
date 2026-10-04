/* Edge case: destructor succeeds */
{
  struct ui_signal *sig = NULL;
  union ui_signal_payload init_val;
  union ui_signal_payload new_val;
  ui_error_t rc;

  init_val.int_val = 0;
  new_val.int_val = 1;

  extern ui_error_t mock_destructor_success(union ui_signal_payload val);
  rc = ui_signal_create(NULL, init_val, UI_SIGNAL_TYPE_INT32, NULL,
                        mock_destructor_success, UI_SIGNAL_MODE_SINGLE_THREADED,
                        &sig);
  assert(rc == UI_ERROR_NONE);

  rc = ui_signal_set(sig, new_val);
  assert(rc == UI_ERROR_NONE);

  ui_signal_destroy(sig);
}
