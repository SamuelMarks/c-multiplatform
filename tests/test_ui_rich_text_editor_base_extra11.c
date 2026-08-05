void test_ui_rte_coverage_errs4(void) {
  /* To hit null new_buf we mock malloc via countdown */
  extern int g_malloc_fail_countdown;
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;

  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    union ui_signal_payload val;
    val.ptr_val = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    g_malloc_fail_countdown =
        0; /* this should trigger the REALLOC failure if countdown hits it */
    cva.write_value(rte, val);
    g_malloc_fail_countdown = -1;

    g_malloc_fail_countdown = 0;
    ui_rich_text_editor_base_insert_text(
        rte, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    g_malloc_fail_countdown = -1;

    (void)ui_rich_text_editor_base_destroy(rte);
  }
}
