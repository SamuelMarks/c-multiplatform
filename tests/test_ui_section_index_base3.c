void test_ui_section_index_remove_attr_err(void) {
  struct ui_section_index_base *index = NULL;
  const char *sections[] = {"A", "B", "C"};
  ui_section_index_base_create(&index);
  (void)ui_section_index_base_set_sections(index, sections, 3);
  (void)ui_section_index_base_set_active_section(index, 1);
  /* corrupt node to fail remove_attribute */
  index->item_nodes[1]->type = UI_DOM_NODE_TYPE_TEXT;
  (void)ui_section_index_base_set_active_section(index, 2);
  index->item_nodes[1]->type = UI_DOM_NODE_TYPE_ELEMENT;
  (void)ui_section_index_base_destroy(index);
}
