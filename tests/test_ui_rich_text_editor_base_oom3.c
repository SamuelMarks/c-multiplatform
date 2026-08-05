void test_ui_rte_coverage_errs7(void) {
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    ui_rich_text_editor_base_insert_text(rte, "foo");
    (void)ui_rich_text_editor_base_destroy(rte);
  }
}

static ui_error_t my_cva_on_change(union ui_signal_payload payload,
                                   void *user_data) {
  (void)payload;
  (void)user_data;
  return UI_ERROR_NOT_FOUND;
}
static ui_error_t my_cva_on_touched(void *user_data) {
  (void)user_data;
  return UI_ERROR_NOT_FOUND;
}

void test_ui_rte_coverage_errs8(void) {
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    cva.register_on_change(rte, my_cva_on_change, NULL);
    cva.register_on_touched(rte, my_cva_on_touched, NULL);
    ui_rich_text_editor_base_insert_text(rte, "foo");
    (void)ui_rich_text_editor_base_destroy(rte);
  }
}

void test_ui_rte_coverage_errs9(void) {
  extern int g_malloc_fail_countdown;
  struct ui_rich_text_editor_base *rte = NULL;
  int i;
  for (i = 20; i < 30; ++i) {
    g_malloc_fail_countdown = i;
    ui_rich_text_editor_base_create(&rte, NULL);
  }
  g_malloc_fail_countdown = -1;
}
