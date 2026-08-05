void test_ui_rte_cva2(void) {
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    union ui_signal_payload val;
    val.ptr_val = "hello";
    if (cva.write_value)
      cva.write_value(rte, val);
    if (cva.set_disabled_state)
      cva.set_disabled_state(rte, 1);
    if (cva.register_on_change)
      cva.register_on_change(rte, NULL, NULL);
    if (cva.register_on_touched)
      cva.register_on_touched(rte, NULL, NULL);

    val.ptr_val = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    if (cva.write_value)
      cva.write_value(rte, val);

    struct ui_component *comp;
    ui_rich_text_editor_base_get_component(rte, &comp);
    (void)ui_rich_text_editor_base_destroy(rte);
  }
}
