/* Edge case: destructor succeeds */
{
  struct ui_signal *sig_sub = NULL;
  union ui_signal_payload init_val;
  union ui_signal_payload new_val;
  ui_error_t rc;

  init_val.int_val = 0;
  new_val.int_val = 1;

  rc = ui_signal_create(NULL, init_val, UI_SIGNAL_TYPE_INT32, NULL,
                        mock_destructor_success, UI_SIGNAL_MODE_SINGLE_THREADED,
                        &sig_sub);
  assert(rc == UI_ERROR_NONE);

  rc = ui_signal_set(sig_sub, new_val);
  assert(rc == UI_ERROR_NONE);

  ui_signal_destroy(sig_sub);
}
