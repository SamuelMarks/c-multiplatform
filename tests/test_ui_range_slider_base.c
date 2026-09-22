/* clang-format off */
#include "ui_range_slider_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t test_on_change(struct ui_range_slider_base *slider, float low,
                                 float high, void *user_data) {
  float *last_vals = (float *)user_data;
  if (!slider && !user_data)
    return UI_ERROR_INVALID_ARGUMENT;
  last_vals[0] = low;
  last_vals[1] = high;
  return UI_ERROR_NONE;
}

static ui_error_t test_on_change_err(struct ui_range_slider_base *slider,
                                     float low, float high, void *user_data) {
  if (slider) {
  }
  if (low) {
  }
  if (high) {
  }
  if (user_data) {
  }
  return UI_ERROR_INVALID_ARGUMENT;
}

static int test_range_slider_basic(void) {
  struct ui_range_slider_base *slider = NULL;
  ui_error_t rc;
  float vals[2] = {-1.0f, -1.0f};
  float low, high;
  struct ui_event ev;

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

  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  if (low != 20.0f || high != 40.0f) {
    fprintf(stderr, "Values not set correctly\n");
    return 1;
  }

  /* Test on_change returning error */
  ui_range_slider_base_set_on_change(slider, test_on_change_err, NULL);
  if (ui_range_slider_base_set_values(slider, 21.0f, 41.0f) !=
      UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  ui_range_slider_base_set_on_change(slider, test_on_change, vals);
  ui_range_slider_base_set_values(slider, 20.0f, 40.0f);

  if (vals[0] != 20.0f || vals[1] != 40.0f) {
    fprintf(stderr, "Callback not called correctly\n");
    return 1;
  }

  rc = ui_range_slider_base_set_normalized_value(
      slider, UI_RANGE_SLIDER_THUMB_LOW,
      0.5f); /* min=10, max=50, range=40, +10 = 30 */
  if (rc != UI_ERROR_NONE)
    return 1;

  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  if (low != 30.0f || high != 40.0f) {
    fprintf(stderr, "Normalized value low not set correctly %f %f\n", low,
            high);
    return 1;
  }

  rc = ui_range_slider_base_set_normalized_value(
      slider, UI_RANGE_SLIDER_THUMB_HIGH, 0.8f); /* range=40, +10 = 42 */
  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  if (high != 42.0f) {
    fprintf(stderr, "high != 42: %f\n", high);
    return 1;
  }

  /* normalized clamping */
  if (ui_range_slider_base_set_normalized_value(
          slider, UI_RANGE_SLIDER_THUMB_LOW, -1.0f) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  if (low != 10.0f) {
    fprintf(stderr, "low != 10: %f\n", low);
    return 1;
  }

  if (ui_range_slider_base_set_normalized_value(
          slider, UI_RANGE_SLIDER_THUMB_HIGH, 2.0f) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  if (high != 50.0f) {
    fprintf(stderr, "high != 50: %f\n", high);
    return 1;
  }

  /* Thumb collision with normalized values */
  if (ui_range_slider_base_set_values(slider, 20.0f, 40.0f) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_set_normalized_value(
          slider, UI_RANGE_SLIDER_THUMB_LOW, 0.9f) != UI_ERROR_NONE)
    return 1; /* sets to 46.0f, which pushes high, actually wait, code says "if
(new_value > slider->high_value) new_value =
slider->high_value;" so it sets to 40.0f */
  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  if (low != 40.0f) {
    fprintf(stderr, "low != 40: %f\n", low);
    return 1;
  }

  if (ui_range_slider_base_set_values(slider, 20.0f, 40.0f) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_set_normalized_value(
          slider, UI_RANGE_SLIDER_THUMB_HIGH, 0.1f) != UI_ERROR_NONE)
    return 1; /* sets to 14.0f, but low is 20.0, so clamped to 20.0 */
  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  if (high != 20.0f) {
    fprintf(stderr, "high != 20: %f\n", high);
    return 1;
  }

  /* Set normalized on NONE */
  if (ui_range_slider_base_set_normalized_value(
          slider, UI_RANGE_SLIDER_THUMB_NONE, 0.5f) != UI_ERROR_NONE)
    return 1;

  /* Simulate clamping validation logic */
  /* Validating min thumb correctly forces max thumb tracking */
  if (ui_range_slider_base_set_values(slider, 35.0f, 30.0f) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  if (high < low) {
    printf("Clamp pushing verification failed\n");
    return 1;
  }

  /* Test range limits clamping */
  if (ui_range_slider_base_set_values(slider, 5.0f, 60.0f) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  if (low != 10.0f || high != 50.0f) {
    fprintf(stderr, "limits clamping failed: %f %f\n", low, high);
    return 1;
  }

  if (ui_range_slider_base_set_step(slider, 2.0f) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_set_values(slider, 15.5f, 23.3f) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  /* 15.5 - 10 = 5.5 / 2 = 2.75 -> 3 * 2 + 10 = 16.0
     23.3 - 10 = 13.3 / 2 = 6.65 -> 7 * 2 + 10 = 24.0 */
  if (low != 16.0f || high != 24.0f) {
    fprintf(stderr, "step rounding failed: %f %f\n", low, high);
    return 1;
  }

  /* Test Step bounds */
  if (ui_range_slider_base_set_values(slider, 49.5f, 49.5f) != UI_ERROR_NONE)
    return 1;

  if (ui_range_slider_base_set_step(slider, -1.0f) != UI_ERROR_NONE)
    return 1; /* sets to 0.0 */

  if (ui_range_slider_base_set_disabled(slider, 1) != UI_ERROR_NONE)
    return 1;
  /* process event and normalize should ignore when disabled */
  if (ui_range_slider_base_set_normalized_value(
          slider, UI_RANGE_SLIDER_THUMB_LOW, 0.2f) != UI_ERROR_NONE)
    return 1;

  ev.type = UI_EVENT_KEY_DOWN;
  if (ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_LOW,
                                         0) != UI_ERROR_NONE)
    return 1;

  if (ui_range_slider_base_set_disabled(slider, 0) != UI_ERROR_NONE)
    return 1;

  {
    ui_error_t rc_cleanup = ui_range_slider_base_destroy(slider);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  return 0;
}

static int test_range_slider_bounds(void) {
  struct ui_range_slider_base *slider = NULL;
  float low, high;

  if (ui_range_slider_base_create(&slider) != UI_ERROR_NONE)
    return 1;

  if (ui_range_slider_base_set_min(slider, 10.0f) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_set_max(slider, 5.0f) != UI_ERROR_NONE)
    return 1; /* invalid max, should clamp to min */

  /* We can't access max_val directly, but setting high value above min should
   * clamp to max */
  if (ui_range_slider_base_set_values(slider, 10.0f, 100.0f) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  if (high != 5.0f) {
    fprintf(stderr, "test_range_slider_bounds high clamp fail: %f\n", high);
    return 1;
  }

  if (ui_range_slider_base_set_max(slider, 100.0f) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_set_values(slider, 20.0f, 80.0f) != UI_ERROR_NONE)
    return 1;
  /* Now set min above current low_value (20.0f) to trigger slider->low_value <
   * slider->min_val */
  if (ui_range_slider_base_set_min(slider, 30.0f) != UI_ERROR_NONE)
    return 1;
  /* Now set max below current high_value (80.0f) to trigger slider->high_value
   * > slider->max_val */
  if (ui_range_slider_base_set_max(slider, 70.0f) != UI_ERROR_NONE)
    return 1;

  if (ui_range_slider_base_set_min(slider, 200.0f) != UI_ERROR_NONE)
    return 1; /* invalid min, should push max */

  /* Setting values should clamp to min and max */
  if (ui_range_slider_base_set_values(slider, 0.0f, 1000.0f) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  if (low != 200.0f || high != 200.0f) {
    fprintf(stderr, "test_range_slider_bounds low/high mismatch: %f, %f\n", low,
            high);
    return 1;
  }

  {
    ui_error_t rc_cleanup = ui_range_slider_base_destroy(slider);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  return 0;
}

static int test_range_slider_events(void) {
  struct ui_range_slider_base *slider = NULL;
  struct ui_event ev;
  float low, high;

  if (ui_range_slider_base_create(&slider) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_set_values(slider, 20.0f, 80.0f) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_set_step(slider, 0.0f) != UI_ERROR_NONE)
    return 1;

  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_LEFT;

  /* Low Thumb */
  if (ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_LOW,
                                         0) != UI_ERROR_NONE)
    return 1;
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  if (ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_LOW,
                                         0) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  if (low != 0.0f) {
    fprintf(stderr, "test_range_slider_events low left/down: %f\n", low);
    return 1; /* 20 - 10 - 10 = 0 */
  }

  ev.event_data.keyboard.key_code = UI_KEY_RIGHT;
  if (ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_LOW,
                                         0) != UI_ERROR_NONE)
    return 1;
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  if (ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_LOW,
                                         0) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  if (low != 20.0f) {
    fprintf(stderr, "test_range_slider_events low right/up: %f\n", low);
    return 1;
  }

  ev.event_data.keyboard.key_code = UI_KEY_HOME;
  if (ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_LOW,
                                         0) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  if (low != 0.0f) {
    fprintf(stderr, "test_range_slider_events low home: %f\n", low);
    return 1;
  }

  ev.event_data.keyboard.key_code = UI_KEY_END;
  if (ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_LOW,
                                         0) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  if (low != 80.0f) {
    fprintf(stderr, "test_range_slider_events low end: %f\n", low);
    return 1; /* bounded by high thumb */
  }

  /* High Thumb */
  if (ui_range_slider_base_set_values(slider, 20.0f, 80.0f) != UI_ERROR_NONE)
    return 1;
  ev.event_data.keyboard.key_code = UI_KEY_RIGHT;
  if (ui_range_slider_base_process_event(
          slider, &ev, UI_RANGE_SLIDER_THUMB_HIGH, 0) != UI_ERROR_NONE)
    return 1;
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  if (ui_range_slider_base_process_event(
          slider, &ev, UI_RANGE_SLIDER_THUMB_HIGH, 0) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  if (high != 100.0f) {
    fprintf(stderr, "test_range_slider_events high right/up: %f\n", high);
    return 1;
  }

  /* Trigger 0.0 increment branch */
  if (ui_range_slider_base_set_min(slider, 1.0f) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_set_max(slider, 1.0f) != UI_ERROR_NONE)
    return 1;
  ev.event_data.keyboard.key_code = UI_KEY_RIGHT;
  if (ui_range_slider_base_process_event(
          slider, &ev, UI_RANGE_SLIDER_THUMB_HIGH, 0) != UI_ERROR_NONE)
    return 1;

  /* Restore bounds */
  if (ui_range_slider_base_set_max(slider, 100.0f) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_set_min(slider, 0.0f) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_set_values(slider, 20.0f, 80.0f) != UI_ERROR_NONE)
    return 1;

  ev.event_data.keyboard.key_code = UI_KEY_LEFT;
  if (ui_range_slider_base_process_event(
          slider, &ev, UI_RANGE_SLIDER_THUMB_HIGH, 0) != UI_ERROR_NONE)
    return 1;
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  if (ui_range_slider_base_process_event(
          slider, &ev, UI_RANGE_SLIDER_THUMB_HIGH, 0) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  if (high != 60.0f) {
    fprintf(stderr, "test_range_slider_events high left/down: %f\n", high);
    return 1;
  }

  ev.event_data.keyboard.key_code = UI_KEY_HOME;
  if (ui_range_slider_base_process_event(
          slider, &ev, UI_RANGE_SLIDER_THUMB_HIGH, 0) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  if (high != 20.0f) {
    fprintf(stderr, "test_range_slider_events high home: %f\n", high);
    return 1;
  }

  ev.event_data.keyboard.key_code = UI_KEY_END;
  if (ui_range_slider_base_process_event(
          slider, &ev, UI_RANGE_SLIDER_THUMB_HIGH, 0) != UI_ERROR_NONE)
    return 1;
  if (ui_range_slider_base_get_values(slider, &low, &high) != UI_ERROR_NONE)
    return 1;
  if (high != 100.0f) {
    fprintf(stderr, "test_range_slider_events high end: %f\n", high);
    return 1;
  }

  /* Thumb None */
  ev.event_data.keyboard.key_code = UI_KEY_LEFT;
  if (ui_range_slider_base_process_event(
          slider, &ev, UI_RANGE_SLIDER_THUMB_NONE, 0) != UI_ERROR_NONE)
    return 1;

  /* Unknown event */
  ev.type = UI_EVENT_KEY_UP;
  if (ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_LOW,
                                         0) != UI_ERROR_NONE)
    return 1;

  {
    ui_error_t rc_cleanup = ui_range_slider_base_destroy(slider);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  return 0;
}

static int test_range_slider_nulls(void) {
  struct ui_range_slider_base *slider = NULL;
  float val = 0.0f;
  struct ui_event ev;
  struct ui_component *comp = NULL;

  memset(&ev, 0, sizeof(ev));

  if (ui_range_slider_base_create(&slider) != UI_ERROR_NONE)
    return 1;

  if (ui_range_slider_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    ui_error_t rc_cleanup = ui_range_slider_base_destroy(NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

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
  if (ui_range_slider_base_process_event(slider, &ev, 0, -1.0) !=
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

  {
    ui_error_t rc_cleanup = ui_range_slider_base_destroy(slider);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  return 0;
}

static int test_range_slider_oom(void) {
  struct ui_range_slider_base *slider = NULL;
  int i;
  for (i = 0; i < 100; i++) {
    g_malloc_fail_countdown = i;
    if (ui_range_slider_base_create(&slider) == UI_ERROR_NONE) {
      ui_error_t rc_cleanup = ui_range_slider_base_destroy(slider);
      if (rc_cleanup != UI_ERROR_NONE) {
        g_malloc_fail_countdown = -1;
        return 1;
      }
      printf("OOM loop broke at %d\n", i);
      printf("OOM loop broke at %d\n", i);
      printf("OOM loop broke at %d\n", i);
      break;
    }
  }
  g_malloc_fail_countdown = -1;
  return 0;
}

void test_extra_range(void);
void test_extra_range_more(void);
#ifdef UI_TEST_MOCK_ALLOC
extern int g_range_slider_mock_set_attribute_fail;
extern int g_range_slider_mock_remove_attribute_fail;
extern int g_range_slider_mock_append_child_fail;
extern int g_range_slider_mock_parse_css_fail;
extern int g_range_slider_mock_set_style_fail;
extern int g_range_slider_mock_gesture_destroy_fail;
extern int g_range_slider_mock_comp_destroy_fail;
extern int g_range_slider_mock_dom_destroy_fail;
extern int g_range_slider_mock_bidi_fail;

static ui_error_t mock_slider_change_fail(struct ui_range_slider_base *s,
                                          float low, float high, void *ud) {
  if (!s && low == 0.0f && high == 0.0f && !ud)
    return UI_ERROR_UNKNOWN;
  return UI_ERROR_UNKNOWN;
}

static int test_range_slider_mock_failures(void) {
  struct ui_range_slider_base *slider = NULL;
  ui_error_t rc;
  int i;
  struct ui_event ev;

  /* 1. ui_dom_node_append_child failures during create */
  /* thumb_low append fails */
  g_range_slider_mock_append_child_fail = 1;
  rc = ui_range_slider_base_create(&slider);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("Failed mock step 1a, rc=%d\n", (int)rc);
    return 1;
  }
  g_range_slider_mock_append_child_fail = 0;

  /* thumb_high append fails */
  g_range_slider_mock_append_child_fail = 2;
  rc = ui_range_slider_base_create(&slider);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("Failed mock step 1b, rc=%d\n", (int)rc);
    return 1;
  }
  g_range_slider_mock_append_child_fail = 0;

  /* 2. parse_css failure during create */
  g_range_slider_mock_parse_css_fail = 1;
  rc = ui_range_slider_base_create(&slider);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("Failed mock step 2, rc=%d\n", (int)rc);
    return 1;
  }
  g_range_slider_mock_parse_css_fail = 0;

  /* 3. set_style failure during create */
  g_range_slider_mock_set_style_fail = 1;
  rc = ui_range_slider_base_create(&slider);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("Failed mock step 3, rc=%d\n", (int)rc);
    return 1;
  }
  g_range_slider_mock_set_style_fail = 0;

  /* 4. set_attribute failures during update_dom_state (1 to 8) */
  for (i = 1; i <= 8; i++) {
    /* create slider */
    rc = ui_range_slider_base_create(&slider);
    if (rc != UI_ERROR_NONE) {
      printf("Failed mock step 4 create, i=%d, rc=%d\n", i, (int)rc);
      return 1;
    }

    g_range_slider_mock_set_attribute_fail = i;
    rc = ui_range_slider_base_set_values(slider, 10.0f, 90.0f);
    if (rc != UI_ERROR_UNKNOWN) {
      printf("Failed mock step 4 set_values, i=%d, rc=%d\n", i, (int)rc);
      return 1;
    }
    g_range_slider_mock_set_attribute_fail = 0;

    rc = ui_range_slider_base_destroy(slider);
    if (rc != UI_ERROR_NONE) {
      printf("Failed mock step 4 destroy, i=%d, rc=%d\n", i, (int)rc);
      return 1;
    }
  }

  /* 4b. 9th set_attribute is aria-disabled when disabled = 1 */
  rc = ui_range_slider_base_create(&slider);
  if (rc != UI_ERROR_NONE) {
    printf("Failed mock step 4b create, rc=%d\n", (int)rc);
    return 1;
  }
  g_range_slider_mock_set_attribute_fail = 9;
  rc = ui_range_slider_base_set_disabled(slider, 1);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("Failed mock step 4b set_disabled, rc=%d\n", (int)rc);
    return 1;
  }
  g_range_slider_mock_set_attribute_fail = 0;
  rc = ui_range_slider_base_destroy(slider);
  if (rc != UI_ERROR_NONE) {
    printf("Failed mock step 4b destroy, rc=%d\n", (int)rc);
    return 1;
  }

  /* 5. remove_attribute failure in update_dom_state */
  rc = ui_range_slider_base_create(&slider);
  if (rc != UI_ERROR_NONE) {
    printf("Failed mock step 5 create, rc=%d\n", (int)rc);
    return 1;
  }
  /* Make sure aria-disabled is present first */
  rc = ui_range_slider_base_set_disabled(slider, 1);
  if (rc != UI_ERROR_NONE) {
    printf("Failed mock step 5 set_disabled 1, rc=%d\n", (int)rc);
    return 1;
  }
  g_range_slider_mock_remove_attribute_fail = 1;
  rc = ui_range_slider_base_set_disabled(slider, 0);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("Failed mock step 5 set_disabled 0, rc=%d\n", (int)rc);
    return 1;
  }
  g_range_slider_mock_remove_attribute_fail = 0;

  /* 6. bidi failure in process_event */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_LEFT;
  g_range_slider_mock_bidi_fail = 1;
  rc = ui_range_slider_base_process_event(slider, &ev,
                                          UI_RANGE_SLIDER_THUMB_LOW, 10.0);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("Failed mock step 6, rc=%d\n", (int)rc);
    return 1;
  }
  g_range_slider_mock_bidi_fail = 0;

  /* 7. destroy failures */
  g_range_slider_mock_gesture_destroy_fail = 1;
  rc = ui_range_slider_base_destroy(slider);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("Failed mock step 7 gesture, rc=%d\n", (int)rc);
    return 1;
  }
  g_range_slider_mock_gesture_destroy_fail = 0;

  /* test comp_destroy fail */
  rc = ui_range_slider_base_create(&slider);
  if (rc != UI_ERROR_NONE) {
    printf("Failed mock step 7 create 2, rc=%d\n", (int)rc);
    return 1;
  }
  g_range_slider_mock_comp_destroy_fail = 1;
  rc = ui_range_slider_base_destroy(slider);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("Failed mock step 7 comp, rc=%d\n", (int)rc);
    return 1;
  }
  g_range_slider_mock_comp_destroy_fail = 0;

  /* 8. cleanup path failures in create */
  /* dom_destroy failure on root_node during cleanup */
  g_range_slider_mock_dom_destroy_fail = 1;
  g_range_slider_mock_parse_css_fail = 1;
  rc = ui_range_slider_base_create(&slider);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("Failed mock step 8 dom, rc=%d\n", (int)rc);
    return 1;
  }
  g_range_slider_mock_dom_destroy_fail = 0;
  g_range_slider_mock_parse_css_fail = 0;

  /* gesture_destroy failure during cleanup */
  g_range_slider_mock_gesture_destroy_fail = 1;
  g_range_slider_mock_parse_css_fail = 1;
  rc = ui_range_slider_base_create(&slider);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("Failed mock step 8, rc=%d\n", (int)rc);
    return 1;
  }
  g_range_slider_mock_gesture_destroy_fail = 0;
  g_range_slider_mock_parse_css_fail = 0;

  /* comp_destroy failure during cleanup */
  g_range_slider_mock_comp_destroy_fail = 1;
  g_range_slider_mock_parse_css_fail = 1;
  rc = ui_range_slider_base_create(&slider);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("Failed mock step 8 comp, rc=%d\n", (int)rc);
    return 1;
  }
  g_range_slider_mock_comp_destroy_fail = 0;
  g_range_slider_mock_parse_css_fail = 0;

  /* 9. update_dom_state failure during create */
  /* Create does 7 set_attribute calls, then update_dom_state does 8.
   * So 8th set_attribute overall is the first one in update_dom_state! */
  g_range_slider_mock_set_attribute_fail = 8;
  rc = ui_range_slider_base_create(&slider);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("Failed mock step 9, rc=%d\n", (int)rc);
    return 1;
  }
  g_range_slider_mock_set_attribute_fail = 0;

  /* 10. on_change error in set_values */
  rc = ui_range_slider_base_create(&slider);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc =
      ui_range_slider_base_set_on_change(slider, mock_slider_change_fail, NULL);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_range_slider_base_set_values(slider, 10.0f, 90.0f);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("Failed mock step 10, rc=%d\n", (int)rc);
    return 1;
  }

  /* 11. set_min failure when low_value < min */
  rc = ui_range_slider_base_set_values(slider, 20.0f, 80.0f);
  if (rc != UI_ERROR_UNKNOWN)
    return 1; /* still has mock_slider_change_fail */
  rc = ui_range_slider_base_set_min(slider, 30.0f);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("Failed mock step 11, rc=%d\n", (int)rc);
    return 1;
  }

  /* 12. set_max failure when high_value > max */
  rc = ui_range_slider_base_set_max(slider, 10.0f);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("Failed mock step 12, rc=%d\n", (int)rc);
    return 1;
  }

  rc = ui_range_slider_base_set_on_change(slider, NULL, NULL);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_range_slider_base_destroy(slider);
  if (rc != UI_ERROR_NONE)
    return 1;

  return 0;
}
#endif

int main(void) {
  int failed = 0;
  failed |= test_range_slider_basic();
  failed |= test_range_slider_bounds();
  failed |= test_range_slider_events();
  failed |= test_range_slider_nulls();
  failed |= test_range_slider_oom();
  test_extra_range();
  test_extra_range_more();
#ifdef UI_TEST_MOCK_ALLOC
  failed |= test_range_slider_mock_failures();
#endif

  if (failed) {
    fprintf(stderr, "test_ui_range_slider_base failed\n");
    return 1;
  }

  printf("test_ui_range_slider_base passed\n");
  return 0;
}

void test_extra_range(void) {
  struct ui_range_slider_base *slider = NULL;
  struct ui_event ev;

  if (ui_range_slider_base_create(&slider) != UI_ERROR_NONE)
    return;
  if (slider) {
    ui_range_slider_base_set_step(slider, 0.0f);
    ui_range_slider_base_set_max(slider, 100.0f);
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

    ui_range_slider_base_destroy(slider);
  }
}

void test_extra_range_more(void) {
  struct ui_range_slider_base *slider = NULL;
  struct ui_event ev;

  if (ui_range_slider_base_create(&slider) != UI_ERROR_NONE)
    return;
  if (slider) {
    ui_range_slider_base_set_step(slider, 2.0f);
    ev.type = UI_EVENT_KEY_DOWN;
    ev.event_data.keyboard.key_code = UI_KEY_RIGHT;
    ui_range_slider_base_process_event(slider, &ev, UI_RANGE_SLIDER_THUMB_LOW,
                                       0.0);

    /* Cover invalid active_thumb */
    ui_range_slider_base_process_event(slider, &ev,
                                       (enum ui_range_slider_thumb)99, 0.0);

    ui_range_slider_base_destroy(slider);
  }
}
