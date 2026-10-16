/* Edge case: ui_signal_update with equal value */
{
  struct ui_signal *sig_sub = NULL;
  union ui_signal_payload init_val;
  ui_error_t rc;

  init_val.int_val = 0;

  rc = ui_signal_create(NULL, init_val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &sig_sub);
  assert(rc == UI_ERROR_NONE);

  rc = ui_signal_update(sig_sub, mock_update_same);
  assert(rc == UI_ERROR_NONE);

  ui_signal_destroy(sig_sub);
}
