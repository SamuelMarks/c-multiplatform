void test_ui_rte_coverage_errs10(void) {
  extern int g_malloc_fail_countdown;
  struct ui_rich_text_editor_base *rte = NULL;
  int i;
  for (i = 30; i < 40; ++i) {
    g_malloc_fail_countdown = i;
    ui_rich_text_editor_base_create(&rte, NULL);
  }
  g_malloc_fail_countdown = -1;
}
