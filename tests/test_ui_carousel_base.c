/* clang-format off */
#include "ui_carousel_base.h"
#include "ui_dom_node.h"
#include "ui_error.h"
#include "ui_event.h"
#include "ui_gesture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

extern int g_malloc_fail_countdown;
extern int g_carousel_mock_fail;

static ui_error_t create_mock_node(size_t index, struct ui_dom_node **out_node,
                                   void *user_data) {
  (void)index;
  (void)user_data;
  return ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, out_node);
  return UI_ERROR_NONE;
}

static ui_error_t update_mock_node(size_t index, struct ui_dom_node *node,
                                   void *user_data) {
  char buf[32];
  (void)user_data;
#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "Item %lu", (unsigned long)index);
#else
  sprintf(buf, "Item %lu", (unsigned long)index);
#endif
  return ui_dom_node_set_attribute(node, "data-index", buf);
  return UI_ERROR_NONE;
}

static ui_error_t run_mock_failures(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_carousel_base *carousel = NULL;
  struct ui_carousel_config config;
  ui_error_t rc;
  int i;

  memset(&config, 0, sizeof(config));
  config.orientation = UI_CAROUSEL_ORIENTATION_HORIZONTAL;
  config.initial_item_count = 5;
  config.item_size = 100.0f;
  config.create_node = create_mock_node;
  config.update_node = update_mock_node;
  config.user_data = NULL;

  for (i = 0; i < 4; i++) {
    carousel = NULL;
    g_carousel_mock_fail = i;
    rc = ui_carousel_base_create(&carousel, &config);
    if (rc == UI_ERROR_NONE && carousel) {
      ui_carousel_base_destroy(carousel);
    }
  }
  g_carousel_mock_fail = -1;

  /* Test ui_virtual_scroll_base_render fail in scroll_to_index */
  rc = ui_carousel_base_create(&carousel, &config);
  if (rc == UI_ERROR_NONE) {
    g_carousel_mock_fail = 0;
    {
      ui_error_t rc_cleanup = ui_carousel_base_scroll_to_index(carousel, 2, 0);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    g_carousel_mock_fail = -1;
    {
      ui_error_t rc_cleanup = ui_carousel_base_destroy(carousel);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  rc = ui_carousel_base_create(&carousel, &config);
  if (rc == UI_ERROR_NONE) {
    g_carousel_mock_fail = 1;
    {
      ui_error_t rc_cleanup = ui_carousel_base_scroll_to_index(carousel, 2, 0);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    g_carousel_mock_fail = -1;
    {
      ui_error_t rc_cleanup = ui_carousel_base_destroy(carousel);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  rc = ui_carousel_base_create(&carousel, &config);
  if (rc == UI_ERROR_NONE) {
    /* mock virtual scroll render fails inside tick */
    g_carousel_mock_fail = 0;
    {
      ui_error_t rc_cleanup = ui_carousel_base_tick(carousel, 0.0);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    g_carousel_mock_fail = -1;
    {
      ui_error_t rc_cleanup = ui_carousel_base_destroy(carousel);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  g_carousel_mock_fail = -1;
#endif
  return UI_ERROR_NONE;
}

static ui_error_t run_normal_tests(void) {

  struct ui_carousel_base *carousel = NULL;
  struct ui_carousel_config config;
  ui_error_t rc;
  struct ui_event ev;

  /* Test invalid args */
  struct ui_carousel_config invalid_config = {0};
  rc = ui_carousel_base_create(NULL, &invalid_config);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_carousel_base_create(&carousel, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  invalid_config.item_size = 0.0f;
  rc = ui_carousel_base_create(&carousel, &invalid_config);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  invalid_config.item_size = -10.0f;
  rc = ui_carousel_base_create(&carousel, &invalid_config);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  printf("Testing invalid arguments...\n");
  rc = ui_carousel_base_create(NULL, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_carousel_base_destroy(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT && rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_carousel_base_set_item_count(NULL, 10);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_carousel_base_set_viewport_size(NULL, 100, 100);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  {
    struct ui_component *tmp_comp;
    rc = ui_carousel_base_get_component(NULL, &tmp_comp);
    if (rc == UI_ERROR_NONE)
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }
  rc = ui_carousel_base_process_event(NULL, NULL, 0);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_carousel_base_tick(NULL, 0);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_carousel_base_scroll_to_index(NULL, 0, 0);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_carousel_base_bind_data(NULL, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  printf("Testing normal creation and logic...\n");
  memset(&config, 0, sizeof(config));
  config.orientation = UI_CAROUSEL_ORIENTATION_HORIZONTAL;
  config.initial_item_count = 5;
  config.item_size = 100.0f;
  config.create_node = create_mock_node;
  config.update_node = update_mock_node;
  config.user_data = NULL;

  rc = ui_carousel_base_create(&carousel, &config);
  if (rc != UI_ERROR_NONE || carousel == NULL)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  {
    struct ui_component *tmp_comp;
    rc = ui_carousel_base_get_component(carousel, &tmp_comp);
    if (rc != UI_ERROR_NONE || tmp_comp == NULL)
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
    rc = ui_carousel_base_get_component(carousel, NULL);
    if (rc != UI_ERROR_INVALID_ARGUMENT)
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
    rc = ui_carousel_base_process_event(carousel, NULL, 0);
    if (rc != UI_ERROR_INVALID_ARGUMENT)
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }

  rc = ui_carousel_base_set_viewport_size(carousel, 100.0f, 100.0f);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_carousel_base_tick(carousel, 0.0);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_carousel_base_set_item_count(carousel, 10);

  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_carousel_base_bind_data(carousel, NULL);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Simulate swipe event */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 0;
  ev.event_data.touch.points[0].y = 0;
  rc = ui_carousel_base_process_event(carousel, &ev, 0.0);
  if (rc != UI_ERROR_NONE)
    return rc;

  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.points[0].x = -50; /* Trigger BEGAN */
  rc = ui_carousel_base_process_event(carousel, &ev, 10.0);
  if (rc != UI_ERROR_NONE)
    return rc;

  ev.event_data.touch.points[0].x = -60; /* Trigger CHANGED */
  rc = ui_carousel_base_process_event(carousel, &ev, 15.0);
  if (rc != UI_ERROR_NONE)
    return rc;

  ev.type = UI_EVENT_TOUCH_END;
  rc = ui_carousel_base_process_event(carousel, &ev, 20.0);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Scroll to index out of bounds */
  rc = ui_carousel_base_scroll_to_index(carousel, 100, 0);
  if (rc != UI_ERROR_OUT_OF_BOUNDS)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Scroll to index valid */
  rc = ui_carousel_base_scroll_to_index(carousel, 2, 0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Smooth scroll to index valid */
  rc = ui_carousel_base_scroll_to_index(carousel, 3, 1);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Simulate tick for animation */
  {
    int i;
    for (i = 0; i < 100; i++) {
      rc = ui_carousel_base_tick(carousel, 30.0 + i * 16.0);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
  }

  /* Test invalid event type passing through to gesture recognizer */
  memset(&ev, 0, sizeof(ev));
  ev.type = 99999;
  rc = ui_carousel_base_process_event(carousel, &ev, 2000.0);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_carousel_base_destroy(carousel);

  if (rc != UI_ERROR_NONE)
    return rc;

  /* Test VERTICAL orientation and swipe edge cases */
  config.orientation = UI_CAROUSEL_ORIENTATION_VERTICAL;
  rc = ui_carousel_base_create(&carousel, &config);
  if (rc != UI_ERROR_NONE)
    return rc;
  {
    memset(&ev, 0, sizeof(ev));
    ev.event_data.touch.num_points = 1;
    ev.event_data.touch.points[0].x = 0;
    ev.event_data.touch.points[0].y = 0;
    ev.type = UI_EVENT_TOUCH_START;
    rc = ui_carousel_base_process_event(carousel, &ev, 0.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.type = UI_EVENT_TOUCH_MOVE;
    ev.event_data.touch.points[0].y = -60; /* BEGAN */
    rc = ui_carousel_base_process_event(carousel, &ev, 10.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.event_data.touch.points[0].y = -70; /* CHANGED */
    rc = ui_carousel_base_process_event(carousel, &ev, 15.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.type = UI_EVENT_TOUCH_END;
    rc = ui_carousel_base_process_event(carousel, &ev, 20.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    /* Test fast swipe next (< -500 velocity) */
    ev.type = UI_EVENT_TOUCH_START;
    ev.event_data.touch.points[0].y = 0;
    rc = ui_carousel_base_process_event(carousel, &ev, 30.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.type = UI_EVENT_TOUCH_MOVE;
    ev.event_data.touch.points[0].y = -100; /* BEGAN */
    rc = ui_carousel_base_process_event(carousel, &ev, 32.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.event_data.touch.points[0].y = -2000; /* Simulate high velocity y */
    rc = ui_carousel_base_process_event(carousel, &ev, 35.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.type = UI_EVENT_TOUCH_END;
    ev.event_data.touch.points[0].y = -2000; /* Must match last move */
    rc = ui_carousel_base_process_event(carousel, &ev, 40.0);
    if (rc != UI_ERROR_NONE)
      return rc;

    /* Test fast swipe prev (> 500 velocity) */
    ev.type = UI_EVENT_TOUCH_START;
    ev.event_data.touch.points[0].y = 0;
    rc = ui_carousel_base_process_event(carousel, &ev, 50.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.type = UI_EVENT_TOUCH_MOVE;
    ev.event_data.touch.points[0].y = 100; /* BEGAN */
    rc = ui_carousel_base_process_event(carousel, &ev, 52.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.event_data.touch.points[0].y = 2000; /* Simulate high velocity y */
    rc = ui_carousel_base_process_event(carousel, &ev, 55.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.type = UI_EVENT_TOUCH_END;
    ev.event_data.touch.points[0].y = 2000; /* Must match last move */
    rc = ui_carousel_base_process_event(carousel, &ev, 60.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    /* Clamp out of bounds logic check */
    ev.type = UI_EVENT_TOUCH_START;
    ev.event_data.touch.points[0].y = 0;
    rc = ui_carousel_base_process_event(carousel, &ev, 70.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.type = UI_EVENT_TOUCH_MOVE;
    ev.event_data.touch.points[0].y = 100;
    rc = ui_carousel_base_process_event(carousel, &ev, 72.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.event_data.touch.points[0].y = 50000; /* Pull way too far back */
    rc = ui_carousel_base_process_event(carousel, &ev, 75.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.type = UI_EVENT_TOUCH_END;
    rc = ui_carousel_base_process_event(carousel, &ev, 80.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    /* Swipe way past the end */
    ev.type = UI_EVENT_TOUCH_START;
    ev.event_data.touch.points[0].y = 0;
    rc = ui_carousel_base_process_event(carousel, &ev, 90.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.type = UI_EVENT_TOUCH_MOVE;
    ev.event_data.touch.points[0].y = -100;
    rc = ui_carousel_base_process_event(carousel, &ev, 92.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.event_data.touch.points[0].y = -50000; /* Pull way too far forward */
    rc = ui_carousel_base_process_event(carousel, &ev, 95.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.type = UI_EVENT_TOUCH_END;
    rc = ui_carousel_base_process_event(carousel, &ev, 100.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    /* Cancel gesture */
    ev.type = UI_EVENT_TOUCH_START;
    ev.event_data.touch.points[0].y = 0;
    rc = ui_carousel_base_process_event(carousel, &ev, 110.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.type = UI_EVENT_TOUCH_MOVE;
    ev.event_data.touch.points[0].y = -50;
    rc = ui_carousel_base_process_event(carousel, &ev, 112.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.event_data.touch.points[0].y = -60;
    rc = ui_carousel_base_process_event(carousel, &ev, 115.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.type = UI_EVENT_TOUCH_CANCEL;
    rc = ui_carousel_base_process_event(carousel, &ev, 120.0);
    if (rc != UI_ERROR_NONE)
      return rc;

    /* Test item_count == 0 to cover ternary branches */
    rc = ui_carousel_base_set_item_count(carousel, 0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.type = UI_EVENT_TOUCH_START;
    ev.event_data.touch.points[0].y = 0;
    rc = ui_carousel_base_process_event(carousel, &ev, 130.0);
    if (rc != UI_ERROR_NONE)
      return rc;

    ev.type = UI_EVENT_TOUCH_MOVE;
    ev.event_data.touch.points[0].y = -50;
    rc = ui_carousel_base_process_event(carousel, &ev, 132.0);
    if (rc != UI_ERROR_NONE)
      return rc;

    ev.event_data.touch.points[0].y = -60;
    rc = ui_carousel_base_process_event(carousel, &ev, 140.0);
    if (rc != UI_ERROR_NONE)
      return rc;

    /* velocity between -500 and 500 */
    ev.type = UI_EVENT_TOUCH_END;
    rc = ui_carousel_base_process_event(carousel, &ev, 145.0);
    if (rc != UI_ERROR_NONE)
      return rc;

    /* Test cancelled gesture with item count 0 and high velocity */
    ev.type = UI_EVENT_TOUCH_START;
    ev.event_data.touch.points[0].y = 0;
    rc = ui_carousel_base_process_event(carousel, &ev, 150.0);
    if (rc != UI_ERROR_NONE)
      return rc;

    ev.type = UI_EVENT_TOUCH_MOVE;
    ev.event_data.touch.points[0].y = -100;
    rc = ui_carousel_base_process_event(carousel, &ev, 152.0);
    if (rc != UI_ERROR_NONE)
      return rc;

    ev.event_data.touch.points[0].y = -2000;
    rc = ui_carousel_base_process_event(carousel, &ev, 155.0);
    if (rc != UI_ERROR_NONE)
      return rc;

    ev.type = UI_EVENT_TOUCH_CANCEL;
    ev.event_data.touch.points[0].y =
        -2000; /* Match last move to avoid zeroing out velocity */
    rc = ui_carousel_base_process_event(carousel, &ev, 160.0);
    if (rc != UI_ERROR_NONE)
      return rc;

    /* Slow swipe to get velocity between -500 and 500 */
    ev.type = UI_EVENT_TOUCH_START;
    ev.event_data.touch.points[0].y = 0;
    rc = ui_carousel_base_process_event(carousel, &ev, 200.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.type = UI_EVENT_TOUCH_MOVE;
    ev.event_data.touch.points[0].y = -10;
    rc = ui_carousel_base_process_event(carousel, &ev, 210.0);
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.event_data.touch.points[0].y = -20;
    rc = ui_carousel_base_process_event(carousel, &ev,
                                        300.0); /* dt=90ms, dy=-10 -> v=-111 */
    if (rc != UI_ERROR_NONE)
      return rc;
    ev.type = UI_EVENT_TOUCH_END;
    ev.event_data.touch.points[0].y = -20;
    rc = ui_carousel_base_process_event(carousel, &ev, 310.0);
    if (rc != UI_ERROR_NONE)
      return rc;

    rc = ui_carousel_base_destroy(carousel);

    if (rc != UI_ERROR_NONE)
      return rc;
  }

  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static ui_error_t
run_oom_test_create_step(int i, struct ui_carousel_base **out_carousel,
                         const struct ui_carousel_config *config,
                         int *out_continue, int *out_break) {
  ui_error_t rc;
  g_malloc_fail_countdown = i;
  rc = ui_carousel_base_create(out_carousel, config);
  if (rc == UI_ERROR_OUT_OF_MEMORY) {
    if (*out_carousel != NULL)
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
    g_malloc_fail_countdown = -1;
    *out_continue = 1;
    return UI_ERROR_NONE;
  } else if (rc == UI_ERROR_NONE) {
    g_malloc_fail_countdown = -1;
    rc = ui_carousel_base_destroy(*out_carousel);
    if (rc != UI_ERROR_NONE)
      return rc;
    *out_break = 1;
    return UI_ERROR_NONE;
  }
  g_malloc_fail_countdown = -1;
  return UI_ERROR_UNKNOWN;
}

static ui_error_t run_oom_tests(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_carousel_base *carousel = NULL;
  struct ui_carousel_config config;
  ui_error_t rc;
  int i;

  memset(&config, 0, sizeof(config));
  config.orientation = UI_CAROUSEL_ORIENTATION_HORIZONTAL;
  config.initial_item_count = 5;
  config.item_size = 100.0f;
  config.create_node = create_mock_node;
  config.update_node = update_mock_node;
  config.user_data = NULL;

  printf("Testing OOM...\n");
  for (i = 0; i < 300; i++) {
    int do_continue = 0, do_break = 0;
    carousel = NULL;
    rc = run_oom_test_create_step(i, &carousel, &config, &do_continue,
                                  &do_break);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (do_continue)
      continue;
    if (do_break)
      break;
  }
  g_malloc_fail_countdown = -1;
#endif
  return UI_ERROR_NONE;
}

static ui_error_t test_carousel_mock_alloc_bypass(void) {
  ui_error_t rc;
  int i;
  for (i = 0; i < 10; i++) {
    struct ui_carousel_base *c = NULL;
    struct ui_carousel_config cfg = {0};
    cfg.orientation = UI_CAROUSEL_ORIENTATION_HORIZONTAL;
    cfg.initial_item_count = 10;
    cfg.item_size = 100.0f;
    g_malloc_fail_countdown = i;
    rc = ui_carousel_base_create(&c, &cfg);
    if (rc == UI_ERROR_OUT_OF_MEMORY) {
      continue;
    } else if (rc == UI_ERROR_NONE) {
      rc = ui_carousel_base_destroy(c);
      if (rc != UI_ERROR_NONE)
        return rc;
    } else {
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
    }
  }
  g_malloc_fail_countdown = -1;
  return UI_ERROR_NONE;
}
int main(void) {
  ui_error_t rc;

  rc = test_carousel_mock_alloc_bypass();
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = run_mock_failures();
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = run_normal_tests();
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = run_oom_tests();
  if (rc != UI_ERROR_NONE)
    return rc;
  return UI_ERROR_NONE;
}
