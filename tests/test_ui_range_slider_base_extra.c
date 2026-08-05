#include "ui_range_slider_base.h"

void test_extra_range(void) {
  struct ui_range_slider_base *slider = NULL;
  ui_range_slider_base_create(&slider);
  if (slider) {
    ui_range_slider_base_set_step(slider, 0.0f);
    ui_range_slider_base_set_max(slider, 100.0f);
    struct ui_event ev;
    ev.type = UI_EVENT_KEY_DOWN;
    ev.event_data.keyboard.key_code = UI_KEY_RIGHT;
    ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_LOW,
                                       0.0);

    /* Cover key_code not handled */
    ev.event_data.keyboard.key_code = UI_KEY_UNKNOWN;
    ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_LOW,
                                       0.0);

    /* Also try active_thumb high */
    ev.event_data.keyboard.key_code = UI_KEY_UNKNOWN;
    ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_HIGH,
                                       0.0);

    (void)ui_range_slider_base_destroy(slider);
  }
}
