
void test_ui_section_index_errs(void) {
  extern int g_malloc_fail_countdown;
  struct ui_section_index_base *index = NULL;
  const char *sections[] = {"A", "B", "C"};
  ui_error_t rc;
  int i;

  rc = ui_section_index_base_create(&index);
  ui_section_index_base_set_sections(index, sections, 3);
  ui_section_index_base_set_active_section(index, 1);
  g_malloc_fail_countdown = 0;
  ui_section_index_base_set_active_section(index, 2);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  ui_section_index_base_set_active_section(index, 1);
  g_malloc_fail_countdown = -1;
  {
    ui_error_t rc_cleanup = ui_section_index_base_destroy(index);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
}
