void test_ui_rte_coverage_errs5(void) {
  ui_rich_text_editor_base_create(NULL, NULL);
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    ui_rich_text_editor_base_insert_text(rte, "foo");
  }
}
