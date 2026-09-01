void test_ui_section_index_remove_attr_err(void) {
  struct ui_section_index_base *index = NULL;
  const char *sections[] = {"A", "B", "C"};
  ui_section_index_base_create(&index);
  {
    ui_error_t rc_cleanup =
        ui_section_index_base_set_sections(index, sections, 3);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_section_index_base_set_active_section(index, 1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  /* corrupt node to fail remove_attribute */
  index->item_nodes[1]->type = UI_DOM_NODE_TYPE_TEXT;
  {
    ui_error_t rc_cleanup = ui_section_index_base_set_active_section(index, 2);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  index->item_nodes[1]->type = UI_DOM_NODE_TYPE_ELEMENT;
  {
    ui_error_t rc_cleanup = ui_section_index_base_destroy(index);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
}
