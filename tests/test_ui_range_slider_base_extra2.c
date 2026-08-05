void test_extra_range_more(void) {
  struct ui_range_slider_base *slider = NULL;
  ui_range_slider_base_create(&slider);
  if (slider) {
    ui_range_slider_base_set_step(slider, 2.0f);
    struct ui_event ev;
    ev.type = UI_EVENT_KEY_DOWN;
    ev.event_data.keyboard.key_code = UI_KEY_RIGHT;
    ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_LOW,
                                       0.0);

    /* Cover invalid active_thumb */
    ui_range_slider_base_process_event(slider, &ev,
                                       (enum ui_range_slider_thumb)99, 0.0);

    (void)ui_range_slider_base_destroy(slider);
  }
}
