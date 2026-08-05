void test_ui_scroll_spy_nulls2(void) {
  /* need to safely hit null check in evaluate */
  ui_scroll_spy_evaluate(NULL);
  (void)ui_scroll_spy_destroy(NULL);

  /* out of bounds */
  struct ui_scroll_spy *spy = NULL;
  ui_scroll_spy_create(&spy);
  if (spy) {
    int i;
    for (i = 0; i < 32; ++i) { /* MAX_SPY_TARGETS is 32 */
      ui_scroll_spy_add_target(spy, (struct ui_dom_node *)(size_t)(0x10 + i),
                               i);
    }
    ui_scroll_spy_add_target(spy, (struct ui_dom_node *)0x10,
                             99); /* should hit out of bounds */

    (void)ui_scroll_spy_destroy(spy);
  }
}
