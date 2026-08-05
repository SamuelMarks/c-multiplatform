void test_ui_rte_errors(void) {
  (void)ui_rich_text_editor_base_destroy(NULL);
  ui_rich_text_editor_base_get_component(NULL, NULL);
  ui_rich_text_editor_base_insert_text(NULL, NULL);
  ui_rich_text_editor_base_undo(NULL);
  ui_rich_text_editor_base_redo(NULL);
  ui_rich_text_editor_base_ime_start(NULL);
  ui_rich_text_editor_base_ime_update(NULL, NULL);
  ui_rich_text_editor_base_ime_end(NULL);
}
