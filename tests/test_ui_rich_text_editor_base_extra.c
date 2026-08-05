void test_ui_rte_extra(void) {
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    union ui_signal_payload val;
    val.ptr_val =
        "Hello world that is longer than the initial capacity of the rte "
        "buffer hopefully by a lot more than this 128 characters or something "
        "to trigger reallocation just in case it is small. Let's make it very "
        "very very very long indeed just to be completely safe.";
    cva.write_value(rte, val);
    cva.write_value(rte, val); /* already large enough */
    cva.set_disabled_state(rte, 1);
    cva.register_on_change(rte, NULL, NULL);
    cva.register_on_touched(rte, NULL, NULL);

    /* Also trigger CVA change and touched */
    struct ui_component *comp;
    ui_rich_text_editor_base_get_component(rte, &comp);

    ui_rich_text_editor_base_insert_text(
        rte, "foo"); /* Will fail because disabled */
    cva.set_disabled_state(rte, 0);

    /* Try null checks */
    cva.write_value(NULL, val);
    cva.set_disabled_state(NULL, 1);
    cva.register_on_change(NULL, NULL, NULL);
    cva.register_on_touched(NULL, NULL, NULL);
    ui_rich_text_editor_base_get_component(NULL, &comp);

    (void)ui_rich_text_editor_base_destroy(rte);
  }

  /* trigger oom */
  extern int g_malloc_fail_countdown;
  for (int i = 0; i < 10; ++i) {
    g_malloc_fail_countdown = i;
    ui_rich_text_editor_base_create(&rte, NULL);
  }
  g_malloc_fail_countdown = -1;
}
