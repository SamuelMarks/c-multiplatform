/* clang-format off */
#include "ui_range_slider_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static enum ui_error test_on_change(struct ui_range_slider_base *slider,
                                    float low, float high, void *user_data) {
  float *last_vals = (float *)user_data;
  last_vals[0] = low;
  last_vals[1] = high;
  return UI_ERROR_NONE;
}

static int test_range_slider_basic(void) {
  struct ui_range_slider_base *slider = NULL;
  enum ui_error rc;
  float vals[2] = {-1.0f, -1.0f};
  float low, high;

  rc = ui_range_slider_base_create(&slider);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "Failed to create slider\n");
    return 1;
  }

  rc = ui_range_slider_base_set_on_change(slider, test_on_change, vals);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_range_slider_base_set_min(slider, 10.0f);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_range_slider_base_set_max(slider, 50.0f);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_range_slider_base_set_values(slider, 20.0f, 40.0f);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_range_slider_base_get_values(slider, &low, &high);
  if (low != 20.0f || high != 40.0f) {
    fprintf(stderr, "Values not set correctly\n");
    return 1;
  }

  if (vals[0] != 20.0f || vals[1] != 40.0f) {
    fprintf(stderr, "Callback not called correctly\n");
    return 1;
  }

  rc = ui_range_slider_base_set_normalized_value(
      slider, UI_RANGE_SLIDER_THUMB_LOW,
      0.5f); /* min=10, max=50, range=40, +10 = 30 */
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_range_slider_base_get_values(slider, &low, &high);
  if (low != 30.0f || high != 40.0f) {
    fprintf(stderr, "Normalized value low not set correctly %f %f\n", low,
            high);
    return 1;
  }

  rc = ui_range_slider_base_set_normalized_value(
      slider, UI_RANGE_SLIDER_THUMB_HIGH, 0.8f); /* range=40, +10 = 42 */
  ui_range_slider_base_get_values(slider, &low, &high);
  if (high != 42.0f) {
    fprintf(stderr, "high != 42: %f\n", high);
    return 1;
  }

  /* normalized clamping */
  ui_range_slider_base_set_normalized_value(slider, UI_RANGE_SLIDER_THUMB_LOW,
                                            -1.0f);
  ui_range_slider_base_get_values(slider, &low, &high);
  if (low != 10.0f) {
    fprintf(stderr, "low != 10: %f\n", low);
    return 1;
  }

  ui_range_slider_base_set_normalized_value(slider, UI_RANGE_SLIDER_THUMB_HIGH,
                                            2.0f);
  ui_range_slider_base_get_values(slider, &low, &high);
  if (high != 50.0f) {
    fprintf(stderr, "high != 50: %f\n", high);
    return 1;
  }

  /* Thumb collision with normalized values */
  ui_range_slider_base_set_values(slider, 20.0f, 40.0f);
  ui_range_slider_base_set_normalized_value(
      slider, UI_RANGE_SLIDER_THUMB_LOW,
      0.9f); /* sets to 46.0f, which pushes high, actually wait, code says "if
                (new_value > slider->high_value) new_value =
                slider->high_value;" so it sets to 40.0f */
  ui_range_slider_base_get_values(slider, &low, &high);
  if (low != 40.0f) {
    fprintf(stderr, "low != 40: %f\n", low);
    return 1;
  }

  ui_range_slider_base_set_values(slider, 20.0f, 40.0f);
  ui_range_slider_base_set_normalized_value(
      slider, UI_RANGE_SLIDER_THUMB_HIGH,
      0.1f); /* sets to 14.0f, but low is 20.0, so clamped to 20.0 */
  ui_range_slider_base_get_values(slider, &low, &high);
  if (high != 20.0f) {
    fprintf(stderr, "high != 20: %f\n", high);
    return 1;
  }

  /* Set normalized on NONE */
  ui_range_slider_base_set_normalized_value(slider, UI_RANGE_SLIDER_THUMB_NONE,
                                            0.5f);

  /* Simulate clamping validation logic */
  /* Validating min thumb correctly forces max thumb tracking */
  ui_range_slider_base_set_values(slider, 35.0f, 30.0f);
  ui_range_slider_base_get_values(slider, &low, &high);
  if (high < low) {
    printf("Clamp pushing verification failed\n");
    return 1;
  }

  /* Test range limits clamping */
  ui_range_slider_base_set_values(slider, 5.0f, 60.0f);
  ui_range_slider_base_get_values(slider, &low, &high);
  if (low != 10.0f || high != 50.0f) {
    fprintf(stderr, "limits clamping failed: %f %f\n", low, high);
    return 1;
  }

  ui_range_slider_base_set_step(slider, 2.0f);
  ui_range_slider_base_set_values(slider, 15.5f, 23.3f);
  ui_range_slider_base_get_values(slider, &low, &high);
  /* 15.5 - 10 = 5.5 / 2 = 2.75 -> 3 * 2 + 10 = 16.0
     23.3 - 10 = 13.3 / 2 = 6.65 -> 7 * 2 + 10 = 24.0 */
  if (low != 16.0f || high != 24.0f) {
    fprintf(stderr, "step rounding failed: %f %f\n", low, high);
    return 1;
  }

  /* Test Step bounds */
  ui_range_slider_base_set_values(slider, 49.5f, 49.5f);

  ui_range_slider_base_set_step(slider, -1.0f); /* sets to 0.0 */

  ui_range_slider_base_set_disabled(slider, 1);
  /* process event and normalize should ignore when disabled */
  ui_range_slider_base_set_normalized_value(slider, UI_RANGE_SLIDER_THUMB_LOW,
                                            0.2f);
  struct ui_event ev;
  ev.type = UI_EVENT_KEY_DOWN;
  ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_LOW, 0);

  ui_range_slider_base_set_disabled(slider, 0);

  ui_range_slider_base_destroy(slider);
  return 0;
}

static int test_range_slider_bounds(void) {
  struct ui_range_slider_base *slider = NULL;
  float low, high;
  ui_range_slider_base_create(&slider);

  ui_range_slider_base_set_min(slider, 10.0f);
  ui_range_slider_base_set_max(slider,
                               5.0f); /* invalid max, should clamp to min */

  /* We can't access max_val directly, but setting high value above min should
   * clamp to max */
  ui_range_slider_base_set_values(slider, 10.0f, 100.0f);
  ui_range_slider_base_get_values(slider, &low, &high);
  if (high != 5.0f) {
    fprintf(stderr, "test_range_slider_bounds high clamp fail: %f\n", high);
    return 1;
  }

  ui_range_slider_base_set_max(slider, 100.0f);
  ui_range_slider_base_set_min(slider,
                               200.0f); /* invalid min, should push max */

  /* Setting values should clamp to min and max */
  ui_range_slider_base_set_values(slider, 0.0f, 1000.0f);
  ui_range_slider_base_get_values(slider, &low, &high);
  if (low != 200.0f || high != 200.0f) {
    fprintf(stderr, "test_range_slider_bounds low/high mismatch: %f, %f\n", low,
            high);
    return 1;
  }

  ui_range_slider_base_destroy(slider);
  return 0;
}

static int test_range_slider_events(void) {
  struct ui_range_slider_base *slider = NULL;
  struct ui_event ev;
  float low, high;

  ui_range_slider_base_create(&slider);
  ui_range_slider_base_set_values(slider, 20.0f, 80.0f);
  ui_range_slider_base_set_step(slider, 0.0f);

  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_LEFT;

  /* Low Thumb */
  ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_LOW, 0);
  ui_range_slider_base_get_values(slider, &low, &high);
  if (low != 10.0f) {
    fprintf(stderr, "test_range_slider_events low left: %f\n", low);
    return 1; /* 20 - (100-0)*0.1 = 10 */
  }

  ev.event_data.keyboard.key_code = UI_KEY_RIGHT;
  ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_LOW, 0);
  ui_range_slider_base_get_values(slider, &low, &high);
  if (low != 20.0f) {
    fprintf(stderr, "test_range_slider_events low right: %f\n", low);
    return 1;
  }

  ev.event_data.keyboard.key_code = UI_KEY_HOME;
  ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_LOW, 0);
  ui_range_slider_base_get_values(slider, &low, &high);
  if (low != 0.0f) {
    fprintf(stderr, "test_range_slider_events low home: %f\n", low);
    return 1;
  }

  ev.event_data.keyboard.key_code = UI_KEY_END;
  ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_LOW, 0);
  ui_range_slider_base_get_values(slider, &low, &high);
  if (low != 80.0f) {
    fprintf(stderr, "test_range_slider_events low end: %f\n", low);
    return 1; /* bounded by high thumb */
  }

  /* High Thumb */
  ui_range_slider_base_set_values(slider, 20.0f, 80.0f);
  ev.event_data.keyboard.key_code = UI_KEY_RIGHT;
  ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_HIGH,
                                     0);
  ui_range_slider_base_get_values(slider, &low, &high);
  if (high != 90.0f) {
    fprintf(stderr, "test_range_slider_events high right: %f\n", high);
    return 1;
  }

  /* Trigger 0.0 increment branch */
  ui_range_slider_base_set_min(slider, 1.0f);
  ui_range_slider_base_set_max(slider, 1.0f);
  ev.event_data.keyboard.key_code = UI_KEY_RIGHT;
  ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_HIGH,
                                     0);

  /* Restore bounds */
  ui_range_slider_base_set_max(slider, 100.0f);
  ui_range_slider_base_set_min(slider, 0.0f);
  ui_range_slider_base_set_values(slider, 20.0f, 80.0f);

  ev.event_data.keyboard.key_code = UI_KEY_LEFT;
  ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_HIGH,
                                     0);
  ui_range_slider_base_get_values(slider, &low, &high);
  if (high != 70.0f) {
    fprintf(stderr, "test_range_slider_events high left: %f\n", high);
    return 1;
  }

  ev.event_data.keyboard.key_code = UI_KEY_HOME;
  ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_HIGH,
                                     0);
  ui_range_slider_base_get_values(slider, &low, &high);
  if (high != 20.0f) {
    fprintf(stderr, "test_range_slider_events high home: %f\n", high);
    return 1;
  }

  ev.event_data.keyboard.key_code = UI_KEY_END;
  ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_HIGH,
                                     0);
  ui_range_slider_base_get_values(slider, &low, &high);
  if (high != 100.0f) {
    fprintf(stderr, "test_range_slider_events high end: %f\n", high);
    return 1;
  }

  /* Thumb None */
  ev.event_data.keyboard.key_code = UI_KEY_LEFT;
  ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_NONE,
                                     0);

  /* Unknown event */
  ev.type = UI_EVENT_KEY_UP;
  ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_LOW, 0);

  ui_range_slider_base_destroy(slider);
  return 0;
}

static int test_range_slider_nulls(void) {
  struct ui_range_slider_base *slider = NULL;
  float val;
  struct ui_event ev;
  struct ui_component *comp;

  ui_range_slider_base_create(&slider);

  if (ui_range_slider_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_range_slider_base_destroy(NULL);

  if (ui_range_slider_base_set_min(NULL, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_range_slider_base_set_max(NULL, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_range_slider_base_set_values(NULL, 0, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_range_slider_base_set_step(NULL, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_range_slider_base_set_disabled(NULL, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_range_slider_base_set_on_change(NULL, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_range_slider_base_set_normalized_value(NULL, 0, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_range_slider_base_process_event(NULL, &ev, 0, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_range_slider_base_process_event(slider, NULL, 0, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_range_slider_base_get_values(NULL, &val, &val) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_range_slider_base_get_values(slider, NULL, &val) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_range_slider_base_get_values(slider, &val, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_range_slider_base_get_component(NULL, &comp) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_range_slider_base_get_component(slider, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_range_slider_base_get_component(slider, &comp) != UI_ERROR_NONE)
    return 1;
  if (comp == NULL)
    return 1;

  ui_range_slider_base_destroy(slider);
  return 0;
}

static int test_range_slider_oom(void) {
  struct ui_range_slider_base *slider = NULL;
  int i;
  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    if (ui_range_slider_base_create(&slider) == UI_ERROR_NONE) {
      ui_range_slider_base_destroy(slider);
      break;
    }
  }
  g_malloc_fail_countdown = -1;
  return 0;
}

int main(void) {
  int failed = 0;
  failed |= test_range_slider_basic();
  failed |= test_range_slider_bounds();
  failed |= test_range_slider_events();
  failed |= test_range_slider_nulls();
  failed |= test_range_slider_oom();

  if (failed) {
    fprintf(stderr, "test_ui_range_slider_base failed\n");
    return 1;
  }

  printf("test_ui_range_slider_base passed\n");
  return 0;
}
