
void test_ui_rte_coverage_errs15(void) {
  extern int g_malloc_fail_countdown;
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    g_malloc_fail_countdown = 0;
    cva.set_disabled_state(rte, 1);
    g_malloc_fail_countdown = -1;

    g_malloc_fail_countdown = 1;
    cva.set_disabled_state(rte, 1);
    g_malloc_fail_countdown = -1;

    (void)ui_rich_text_editor_base_destroy(rte);
  }
}
