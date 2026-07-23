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

static enum ui_error
create_mock_node(size_t index, struct ui_dom_node **out_node, void *user_data) {
  (void)index;
  (void)user_data;
  return ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, out_node);
}

static enum ui_error update_mock_node(size_t index, struct ui_dom_node *node,
                                      void *user_data) {
  char buf[32];
  (void)user_data;
#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "Item %lu", (unsigned long)index);
#else
  sprintf(buf, "Item %lu", (unsigned long)index);
#endif
  return ui_dom_node_set_attribute(node, "data-index", buf);
}

static int run_normal_tests(void) {
  struct ui_carousel_base *carousel = NULL;
  struct ui_carousel_config config;
  enum ui_error rc;
  struct ui_event ev;

  /* Test invalid args */
  struct ui_carousel_config invalid_config = {0};
  if (ui_carousel_base_create(NULL, &invalid_config) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_carousel_base_create(&carousel, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  invalid_config.item_size = 0.0f;
  if (ui_carousel_base_create(&carousel, &invalid_config) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  invalid_config.item_size = -10.0f;
  if (ui_carousel_base_create(&carousel, &invalid_config) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  printf("Testing invalid arguments...\n");
  if (ui_carousel_base_create(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_carousel_base_destroy(NULL); /* Should not crash */
  if (ui_carousel_base_set_item_count(NULL, 10) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_carousel_base_set_viewport_size(NULL, 100, 100) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    struct ui_component *tmp_comp;
    if (ui_carousel_base_get_component(NULL, &tmp_comp) == UI_ERROR_NONE)
      return 1;
  }
  if (ui_carousel_base_process_event(NULL, NULL, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_carousel_base_tick(NULL, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_carousel_base_scroll_to_index(NULL, 0, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_carousel_base_bind_data(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

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
    return 1;

  {
    struct ui_component *tmp_comp;
    if (ui_carousel_base_get_component(carousel, &tmp_comp) != UI_ERROR_NONE ||
        tmp_comp == NULL)
      return 1;
    if (ui_carousel_base_get_component(carousel, NULL) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_carousel_base_process_event(carousel, NULL, 0) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
  }

  rc = ui_carousel_base_set_viewport_size(carousel, 100.0f, 100.0f);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_carousel_base_tick(carousel, 0.0);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_carousel_base_set_item_count(carousel, 10);
  ui_carousel_base_bind_data(carousel, NULL);

  /* Simulate swipe event */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 0;
  ev.event_data.touch.points[0].y = 0;
  rc = ui_carousel_base_process_event(carousel, &ev, 0.0);
  if (rc != UI_ERROR_NONE)
    return 1;

  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.points[0].x = -50; /* Trigger BEGAN */
  rc = ui_carousel_base_process_event(carousel, &ev, 10.0);
  if (rc != UI_ERROR_NONE)
    return 1;

  ev.event_data.touch.points[0].x = -60; /* Trigger CHANGED */
  rc = ui_carousel_base_process_event(carousel, &ev, 15.0);
  if (rc != UI_ERROR_NONE)
    return 1;

  ev.type = UI_EVENT_TOUCH_END;
  rc = ui_carousel_base_process_event(carousel, &ev, 20.0);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Scroll to index out of bounds */
  if (ui_carousel_base_scroll_to_index(carousel, 100, 0) !=
      UI_ERROR_OUT_OF_BOUNDS)
    return 1;

  /* Scroll to index valid */
  if (ui_carousel_base_scroll_to_index(carousel, 2, 0) != UI_ERROR_NONE)
    return 1;

  /* Smooth scroll to index valid */
  if (ui_carousel_base_scroll_to_index(carousel, 3, 1) != UI_ERROR_NONE)
    return 1;

  /* Simulate tick for animation */
  {
    int i;
    for (i = 0; i < 100; i++) {
      ui_carousel_base_tick(carousel, 30.0 + i * 16.0);
    }
  }

  /* Test invalid event type passing through to gesture recognizer */
  memset(&ev, 0, sizeof(ev));
  ev.type = 99999;
  ui_carousel_base_process_event(carousel, &ev, 2000.0);

  ui_carousel_base_destroy(carousel);

  /* Test VERTICAL orientation and swipe edge cases */
  config.orientation = UI_CAROUSEL_ORIENTATION_VERTICAL;
  rc = ui_carousel_base_create(&carousel, &config);
  if (rc == UI_ERROR_NONE) {
    memset(&ev, 0, sizeof(ev));
    ev.event_data.touch.num_points = 1;
    ev.event_data.touch.points[0].x = 0;
    ev.event_data.touch.points[0].y = 0;
    ev.type = UI_EVENT_TOUCH_START;
    ui_carousel_base_process_event(carousel, &ev, 0.0);
    ev.type = UI_EVENT_TOUCH_MOVE;
    ev.event_data.touch.points[0].y = -60; /* BEGAN */
    ui_carousel_base_process_event(carousel, &ev, 10.0);
    ev.event_data.touch.points[0].y = -70; /* CHANGED */
    ui_carousel_base_process_event(carousel, &ev, 15.0);
    ev.type = UI_EVENT_TOUCH_END;
    ui_carousel_base_process_event(carousel, &ev, 20.0);
    /* Test fast swipe next (< -500 velocity) */
    ev.type = UI_EVENT_TOUCH_START;
    ev.event_data.touch.points[0].y = 0;
    ui_carousel_base_process_event(carousel, &ev, 30.0);
    ev.type = UI_EVENT_TOUCH_MOVE;
    ev.event_data.touch.points[0].y = -100; /* BEGAN */
    ui_carousel_base_process_event(carousel, &ev, 32.0);
    ev.event_data.touch.points[0].y = -2000; /* Simulate high velocity y */
    ui_carousel_base_process_event(carousel, &ev, 35.0);
    ev.type = UI_EVENT_TOUCH_END;
    ev.event_data.touch.points[0].y = -2000; /* Must match last move */
    ui_carousel_base_process_event(carousel, &ev, 40.0);

    /* Test fast swipe prev (> 500 velocity) */
    ev.type = UI_EVENT_TOUCH_START;
    ev.event_data.touch.points[0].y = 0;
    ui_carousel_base_process_event(carousel, &ev, 50.0);
    ev.type = UI_EVENT_TOUCH_MOVE;
    ev.event_data.touch.points[0].y = 100; /* BEGAN */
    ui_carousel_base_process_event(carousel, &ev, 52.0);
    ev.event_data.touch.points[0].y = 2000; /* Simulate high velocity y */
    ui_carousel_base_process_event(carousel, &ev, 55.0);
    ev.type = UI_EVENT_TOUCH_END;
    ev.event_data.touch.points[0].y = 2000; /* Must match last move */
    ui_carousel_base_process_event(carousel, &ev, 60.0);
    /* Clamp out of bounds logic check */
    ev.type = UI_EVENT_TOUCH_START;
    ev.event_data.touch.points[0].y = 0;
    ui_carousel_base_process_event(carousel, &ev, 70.0);
    ev.type = UI_EVENT_TOUCH_MOVE;
    ev.event_data.touch.points[0].y = 100;
    ui_carousel_base_process_event(carousel, &ev, 72.0);
    ev.event_data.touch.points[0].y = 50000; /* Pull way too far back */
    ui_carousel_base_process_event(carousel, &ev, 75.0);
    ev.type = UI_EVENT_TOUCH_END;
    ui_carousel_base_process_event(carousel, &ev, 80.0);
    /* Swipe way past the end */
    ev.type = UI_EVENT_TOUCH_START;
    ev.event_data.touch.points[0].y = 0;
    ui_carousel_base_process_event(carousel, &ev, 90.0);
    ev.type = UI_EVENT_TOUCH_MOVE;
    ev.event_data.touch.points[0].y = -100;
    ui_carousel_base_process_event(carousel, &ev, 92.0);
    ev.event_data.touch.points[0].y = -50000; /* Pull way too far forward */
    ui_carousel_base_process_event(carousel, &ev, 95.0);
    ev.type = UI_EVENT_TOUCH_END;
    ui_carousel_base_process_event(carousel, &ev, 100.0);
    /* Cancel gesture */
    ev.type = UI_EVENT_TOUCH_START;
    ev.event_data.touch.points[0].y = 0;
    ui_carousel_base_process_event(carousel, &ev, 110.0);
    ev.type = UI_EVENT_TOUCH_MOVE;
    ev.event_data.touch.points[0].y = -50;
    ui_carousel_base_process_event(carousel, &ev, 112.0);
    ev.event_data.touch.points[0].y = -60;
    ui_carousel_base_process_event(carousel, &ev, 115.0);
    ev.type = UI_EVENT_TOUCH_CANCEL;
    ui_carousel_base_process_event(carousel, &ev, 120.0);

    /* Test item_count == 0 to cover ternary branches */
    ui_carousel_base_set_item_count(carousel, 0);
    ev.type = UI_EVENT_TOUCH_START;
    ev.event_data.touch.points[0].y = 0;
    ui_carousel_base_process_event(carousel, &ev, 130.0);

    ev.type = UI_EVENT_TOUCH_MOVE;
    ev.event_data.touch.points[0].y = -50;
    ui_carousel_base_process_event(carousel, &ev, 132.0);

    ev.event_data.touch.points[0].y = -60;
    ui_carousel_base_process_event(carousel, &ev, 140.0);

    /* velocity between -500 and 500 */
    ev.type = UI_EVENT_TOUCH_END;
    ui_carousel_base_process_event(carousel, &ev, 145.0);

    /* Test cancelled gesture with item count 0 and high velocity */
    ev.type = UI_EVENT_TOUCH_START;
    ev.event_data.touch.points[0].y = 0;
    ui_carousel_base_process_event(carousel, &ev, 150.0);

    ev.type = UI_EVENT_TOUCH_MOVE;
    ev.event_data.touch.points[0].y = -100;
    ui_carousel_base_process_event(carousel, &ev, 152.0);

    ev.event_data.touch.points[0].y = -2000;
    ui_carousel_base_process_event(carousel, &ev, 155.0);

    ev.type = UI_EVENT_TOUCH_CANCEL;
    ev.event_data.touch.points[0].y =
        -2000; /* Match last move to avoid zeroing out velocity */
    ui_carousel_base_process_event(carousel, &ev, 160.0);

    /* Slow swipe to get velocity between -500 and 500 */
    ev.type = UI_EVENT_TOUCH_START;
    ev.event_data.touch.points[0].y = 0;
    ui_carousel_base_process_event(carousel, &ev, 200.0);
    ev.type = UI_EVENT_TOUCH_MOVE;
    ev.event_data.touch.points[0].y = -10;
    ui_carousel_base_process_event(carousel, &ev, 210.0);
    ev.event_data.touch.points[0].y = -20;
    ui_carousel_base_process_event(carousel, &ev,
                                   300.0); /* dt=90ms, dy=-10 -> v=-111 */
    ev.type = UI_EVENT_TOUCH_END;
    ev.event_data.touch.points[0].y = -20;
    ui_carousel_base_process_event(carousel, &ev, 310.0);

    ui_carousel_base_destroy(carousel);
  }

  return 0;
}

static int run_oom_tests(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_carousel_base *carousel = NULL;
  struct ui_carousel_config config;
  enum ui_error rc;
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
    g_malloc_fail_countdown = i;
    carousel = NULL;
    rc = ui_carousel_base_create(&carousel, &config);
    if (rc == UI_ERROR_OUT_OF_MEMORY) {
      if (carousel != NULL) {
        printf("Leaked carousel struct pointer on OOM.\n");
        return 1;
      }
    } else if (rc == UI_ERROR_NONE) {
      ui_carousel_base_destroy(carousel);
      break; /* We hit enough allocations to succeed */
    } else {
      printf("Unexpected error code %d\n", rc);
      return 1;
    }
  }
  g_malloc_fail_countdown = -1;
#endif
  return 0;
}

static void test_carousel_mock_alloc_bypass(void) {
  int i;
  for (i = 0; i < 10; i++) {
    struct ui_carousel_base *c = NULL;
    struct ui_carousel_config cfg = {0};
    cfg.orientation = UI_CAROUSEL_ORIENTATION_HORIZONTAL;
    cfg.initial_item_count = 10;
    cfg.item_size = 100.0f;
    g_malloc_fail_countdown = i;
    ui_carousel_base_create(&c, &cfg);
    if (c)
      ui_carousel_base_destroy(c);
  }
  g_malloc_fail_countdown = -1;
}
int main(void) {
  test_carousel_mock_alloc_bypass();
  if (run_normal_tests() != 0) {
    printf("Normal tests failed.\n");
    return 1;
  }
  if (run_oom_tests() != 0) {
    printf("OOM tests failed.\n");
    return 1;
  }
  printf("All ui_carousel_base tests passed.\n");
  return 0;
}
