void test_ui_rte_oom(void) {
  extern int g_malloc_fail_countdown;
  struct ui_rich_text_editor_base *rte = NULL;

  for (int i = 0; i < 10; ++i) {
    g_malloc_fail_countdown = i;
    ui_rich_text_editor_base_create(&rte, NULL);
  }
  g_malloc_fail_countdown = -1;

  struct ui_control_value_accessor cva;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    g_malloc_fail_countdown = 0;
    union ui_signal_payload val;
    val.ptr_val = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    cva.write_value(rte, val);
    g_malloc_fail_countdown = -1;

    g_malloc_fail_countdown = 0;
    ui_rich_text_editor_base_insert_text(
        rte, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
             "xxxxxxxxxxxxxxxxxxxxxx");
    g_malloc_fail_countdown = -1;

    (void)ui_rich_text_editor_base_destroy(rte);
  }
}
