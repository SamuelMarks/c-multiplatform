void test_ui_rte_cva(void) {
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    union ui_signal_payload val;
    val.ptr_val = "hello";
    cva.write_value(rte, val);
    cva.set_disabled_state(rte, 1);
    ui_rich_text_editor_base_insert_text(rte, "foo");
    cva.register_on_change(rte, NULL, NULL);
    cva.register_on_touched(rte, NULL, NULL);
    (void)ui_rich_text_editor_base_destroy(rte);
  }
}
void test_ui_rte_cva_nulls(void) {
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    union ui_signal_payload val;
    val.ptr_val = NULL;
    cva.write_value(rte, val);
    cva.write_value(NULL, val);
    cva.set_disabled_state(NULL, 1);
    cva.register_on_change(NULL, NULL, NULL);
    cva.register_on_touched(NULL, NULL, NULL);
    (void)ui_rich_text_editor_base_destroy(rte);
  }
}
