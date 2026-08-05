void test_ui_rte_errors2(void) {
  /* To hit null rte / out_component, we will call them directly */
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
