void test_ui_rte_errors3(void) {
  (void)ui_rich_text_editor_base_destroy(NULL);
  ui_rich_text_editor_base_get_component(NULL, NULL);
  struct ui_component *comp;
  ui_rich_text_editor_base_get_component(NULL, &comp);

  ui_rich_text_editor_base_insert_text(NULL, NULL);
  ui_rich_text_editor_base_insert_text(NULL, "a");

  ui_rich_text_editor_base_set_caret_from_point(NULL, 0, 0);
  ui_rich_text_editor_base_undo(NULL);
  ui_rich_text_editor_base_redo(NULL);
  ui_rich_text_editor_base_ime_start(NULL);
  ui_rich_text_editor_base_ime_update(NULL, NULL);
  ui_rich_text_editor_base_ime_end(NULL);
}

void test_ui_rte_cva3(void) {
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

    val.ptr_val = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    cva.write_value(rte, val);

    struct ui_component *cmp;
    ui_rich_text_editor_base_get_component(rte, &cmp);

    (void)ui_rich_text_editor_base_destroy(rte);
  }
}

void test_ui_rte_cva_nulls3(void) {
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
