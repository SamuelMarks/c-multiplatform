static ui_error_t dummy_on_change(union ui_signal_payload value,
                                  void *user_data) {
  return UI_ERROR_NONE;
}

static ui_error_t dummy_on_touched(void *user_data) { return UI_ERROR_NONE; }

void test_ui_rte_extra_more(void) {
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    cva.register_on_change(rte, dummy_on_change, NULL);
    cva.register_on_touched(rte, dummy_on_touched, NULL);
    ui_rich_text_editor_base_insert_text(rte, "triggertouchandchange");
    (void)ui_rich_text_editor_base_destroy(rte);
  }
}
