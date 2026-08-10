void test_ui_segmented_coverage_branches(void) {
  struct ui_segmented_control_base *control = NULL;
  struct ui_segmented_button_base *btn1 = NULL;
  struct ui_control_value_accessor cva;

  /* Test destroying partially initialized control (e.g. without component or
   * buttons) */
  control = malloc(sizeof(struct ui_segmented_control_base));
  if (control) {
    memset(control, 0, sizeof(struct ui_segmented_control_base));
    ui_segmented_control_base_destroy(control);
  }

  btn1 = malloc(sizeof(struct ui_segmented_button_base));
  if (btn1) {
    memset(btn1, 0, sizeof(struct ui_segmented_button_base));
    ui_segmented_button_base_destroy(btn1);
  }

  ui_segmented_control_base_create(&control, &cva);
  ui_segmented_button_base_create(&btn1);
  ui_segmented_control_base_append_segment(control, btn1);

  /* Already selected, so button->selected != selected is false */
  ui_segmented_button_base_set_selected(btn1, 0);

  ui_segmented_control_base_set_mode(control, UI_SEGMENTED_CONTROL_MODE_SINGLE);
  ui_segmented_button_base_set_selected(btn1, 1);
  ui_segmented_button_base_set_selected(btn1,
                                        0); /* hits selected == 0 branch */
  ui_segmented_button_base_set_selected(btn1, 0); /* false path */

  /* Set cva on change to NULL to hit false branch */
  control->cva_on_change = NULL;
  control->cva_on_touched = NULL;
  ui_segmented_button_base_set_selected(btn1, 1);

  ui_segmented_button_base_destroy(btn1);
  ui_segmented_control_base_destroy(control);
}
