/* Edge case: ui_signal_set with equal value */
{
  struct ui_signal *sig = NULL;
  union ui_signal_payload init_val;
  ui_error_t rc;

  init_val.int_val = 0;

  rc = ui_signal_create(NULL, init_val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &sig);
  assert(rc == UI_ERROR_NONE);
  printf("EQUAL TEST RAN\n");

  rc = ui_signal_set(sig, init_val);
  assert(rc == UI_ERROR_NONE);
  printf("EQUAL TEST RAN\n");

  ui_signal_destroy(sig);
}
