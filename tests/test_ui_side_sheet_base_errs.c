
static ui_error_t mock_on_close_fail(struct ui_side_sheet_base *sheet,
                                     void *user_data) {
  (void)sheet;
  (void)user_data;
  return UI_ERROR_UNKNOWN;
}

static void test_ui_side_sheet_errs(void) {
  struct ui_side_sheet_base *sheet = NULL;
  struct ui_event ev;
  ui_side_sheet_base_create(&sheet);

  {
    ui_error_t rc_cleanup =
        ui_side_sheet_base_set_on_close(sheet, mock_on_close_fail, NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_side_sheet_base_set_open(sheet, 1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  /* Triggers the if (rc != UI_ERROR_NONE) return rc; inside set_open */
  {
    ui_error_t rc_cleanup = ui_side_sheet_base_set_open(sheet, 0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_side_sheet_base_set_open(sheet, 1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  /* Triggers set_open failing from process_event -> returns rc */
  {
    ui_error_t rc_cleanup = ui_side_sheet_base_process_event(sheet, &ev, 0.0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_side_sheet_base_destroy(sheet);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
}
