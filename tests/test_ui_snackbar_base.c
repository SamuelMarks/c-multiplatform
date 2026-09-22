/* clang-format off */
#include "greatest.h"
#include "ui_snackbar_base.h"
#include "ui_timer.h"
#include "ui_overlay_director.h"
#include "ui_ring_buffer.h"
#include "ui_component.h"
#include "ui_dom_node.h"
#include "ui_signal.h"
#include "ui_computed.h"
#include "ui_test_mock_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct internal_snackbar {
  char *message;
  char *action_label;
  ui_error_t (*action_callback)(struct ui_snackbar_base *, void *);
  void *action_user_data;
  double duration_secs;
};

struct ui_snackbar_base {
  struct ui_timer *timer;
  struct ui_overlay_director *director;
  struct ui_ring_buffer *queue;
  struct ui_component *component;
  struct ui_dom_node *root_node;
  struct ui_dom_node *wrapper_node;
  struct ui_dom_node *message_node;
  struct ui_dom_node *message_text_node;
  struct ui_dom_node *action_node;
  struct ui_dom_node *action_text_node;
  struct ui_overlay *overlay_handle;
  int is_active;
  struct internal_snackbar current;
  double show_time;
  struct ui_signal *open_signal;
  struct ui_computed *animating_signal;
};

extern int g_malloc_fail_countdown;
extern int g_snackbar_mock_fail;
extern int g_snackbar_append_child_fail_target;

static double g_mock_time = 0.0;

static ui_error_t mock_time_source(void *user_data, double *out_time_secs) {
  if (user_data) {
  }
  *out_time_secs = g_mock_time;
  return UI_ERROR_NONE;
}

static int g_action_called = 0;

static ui_error_t mock_action_cb(struct ui_snackbar_base *snackbar,
                                 void *user_data) {
  if (snackbar) {
  }
  if (user_data) {
  }
  g_action_called = 1;
  return UI_ERROR_NONE;
}

static ui_error_t mock_action_cb_fail(struct ui_snackbar_base *snackbar,
                                      void *user_data) {
  if (snackbar) {
  }
  if (user_data) {
  }
  return UI_ERROR_UNKNOWN;
}

TEST test_snackbar_invalid_args(void) {
  struct ui_timer_config tconfig;
  struct ui_timer *timer = NULL;
  struct ui_dom_node *root_node = NULL;
  struct ui_overlay_director *director = NULL;
  struct ui_snackbar_base *snackbar = NULL;
  struct ui_snackbar_config conf;
  struct ui_event ev;
  struct ui_computed *comp = NULL;
  ui_error_t rc;

  tconfig.time_source = mock_time_source;
  tconfig.user_data = NULL;

  rc = ui_timer_create_custom(&tconfig, &timer);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_create(root_node, &director);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_snackbar_base_create(NULL, NULL, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_snackbar_base_create(timer, NULL, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_snackbar_base_create(timer, director, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_snackbar_base_create(NULL, director, &snackbar));

  ASSERT_EQ(UI_ERROR_NONE, ui_snackbar_base_destroy(NULL));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_snackbar_base_enqueue(NULL, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_snackbar_base_dismiss_current(NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_snackbar_base_tick(NULL));

  memset(&ev, 0, sizeof(ev));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_snackbar_base_process_event(NULL, &ev, 0.0));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_snackbar_base_bind_open(NULL, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_snackbar_base_get_animating_signal(NULL, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_snackbar_base_get_animating_signal(NULL, &comp));

  rc = ui_snackbar_base_create(timer, director, &snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_snackbar_base_enqueue(snackbar, NULL));
  memset(&conf, 0, sizeof(conf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_snackbar_base_enqueue(snackbar, &conf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_snackbar_base_process_event(snackbar, NULL, 0.0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_snackbar_base_get_animating_signal(snackbar, NULL));

  rc = ui_snackbar_base_destroy(snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_destroy(director);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(root_node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_timer_destroy(timer);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_snackbar_lifecycle_and_events(void) {
  struct ui_timer_config tconfig;
  struct ui_timer *timer = NULL;
  struct ui_dom_node *root_node = NULL;
  struct ui_overlay_director *director = NULL;
  struct ui_snackbar_base *snackbar = NULL;
  struct ui_snackbar_config sconfig;
  struct ui_event ev;
  struct ui_computed *comp = NULL;
  ui_error_t rc;

  tconfig.time_source = mock_time_source;
  tconfig.user_data = NULL;

  rc = ui_timer_create_custom(&tconfig, &timer);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_set_tag_name(root_node, "body");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_create(root_node, &director);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_snackbar_base_create(timer, director, &snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(snackbar != NULL);

  rc = ui_snackbar_base_bind_open(snackbar, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_snackbar_base_get_animating_signal(snackbar, &comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Tick when queue is empty and inactive */
  rc = ui_snackbar_base_tick(snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Enqueue with action label */
  memset(&sconfig, 0, sizeof(sconfig));
  sconfig.message = "Hello";
  sconfig.action_label = "UNDO";
  sconfig.action_callback = mock_action_cb;
  sconfig.duration_secs = 2.0;
  rc = ui_snackbar_base_enqueue(snackbar, &sconfig);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Enqueue without action label and duration 0 */
  sconfig.message = "No Action";
  sconfig.action_label = NULL;
  sconfig.action_callback = NULL;
  sconfig.duration_secs = 0.0;
  rc = ui_snackbar_base_enqueue(snackbar, &sconfig);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Enqueue with empty action label */
  sconfig.message = "Empty Action";
  sconfig.action_label = "";
  sconfig.duration_secs = 1.0;
  rc = ui_snackbar_base_enqueue(snackbar, &sconfig);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* First tick: activates "Hello" */
  g_mock_time = 1.0;
  rc = ui_snackbar_base_tick(snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, snackbar->is_active);

  /* Process unhandled event (e.g. key down) -> does not dismiss */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  rc = ui_snackbar_base_process_event(snackbar, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, snackbar->is_active);

  /* Tick before expiration -> stays active */
  g_mock_time = 2.0;
  rc = ui_snackbar_base_tick(snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, snackbar->is_active);

  /* Process TOUCH_START while active -> dismisses and calls callback */
  g_action_called = 0;
  ev.type = UI_EVENT_TOUCH_START;
  rc = ui_snackbar_base_process_event(snackbar, &ev, 2.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, g_action_called);
  ASSERT_EQ(0, snackbar->is_active);

  /* Process event while inactive -> returns NONE */
  rc = ui_snackbar_base_process_event(snackbar, &ev, 2.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Tick activates "No Action" (duration 0) */
  rc = ui_snackbar_base_tick(snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, snackbar->is_active);

  /* Tick does not auto-dismiss duration 0 */
  g_mock_time = 100.0;
  rc = ui_snackbar_base_tick(snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, snackbar->is_active);

  /* Mouse down dismisses "No Action" */
  ev.type = UI_EVENT_MOUSE_DOWN;
  rc = ui_snackbar_base_process_event(snackbar, &ev, 100.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, snackbar->is_active);

  /* Tick activates "Empty Action" (empty action string) */
  rc = ui_snackbar_base_tick(snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, snackbar->is_active);

  /* Auto-dismiss "Empty Action" on timeout */
  g_mock_time = 105.0;
  rc = ui_snackbar_base_tick(snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, snackbar->is_active);

  /* Push an item with NULL message directly into queue to cover ternary */
  {
    struct internal_snackbar null_msg_item;
    memset(&null_msg_item, 0, sizeof(null_msg_item));
    rc = ui_ring_buffer_push(snackbar->queue, &null_msg_item);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_snackbar_base_tick(snackbar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(1, snackbar->is_active);
    rc = ui_snackbar_base_dismiss_current(snackbar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Dismiss when already inactive */
  rc = ui_snackbar_base_dismiss_current(snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_snackbar_base_destroy(snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test destroy with active item having NULL message and NULL action_label */
  {
    rc = ui_snackbar_base_create(timer, director, &snackbar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    snackbar->is_active = 1;
    snackbar->current.message = NULL;
    snackbar->current.action_label = NULL;
    rc = ui_snackbar_base_destroy(snackbar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Test destroy with queued items having NULL message and NULL action_label */
  {
    struct internal_snackbar item1;
    struct internal_snackbar item2;
    rc = ui_snackbar_base_create(timer, director, &snackbar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    memset(&item1, 0, sizeof(item1));
    item1.message = NULL;
    item1.action_label = NULL;
    rc = ui_ring_buffer_push(snackbar->queue, &item1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    memset(&item2, 0, sizeof(item2));
    item2.message = ui_mock_strdup("m");
    item2.action_label = NULL;
    rc = ui_ring_buffer_push(snackbar->queue, &item2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_snackbar_base_destroy(snackbar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Test destroy with NULL queue and NULL component */
  {
    rc = ui_snackbar_base_create(timer, director, &snackbar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_ring_buffer_destroy(snackbar->queue);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    snackbar->queue = NULL;
    rc = ui_component_destroy(snackbar->component);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    snackbar->component = NULL;
    rc = ui_snackbar_base_destroy(snackbar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Test tick with pre-existing overlay_handle */
  {
    rc = ui_snackbar_base_create(timer, director, &snackbar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    sconfig.message = "pre-mounted";
    sconfig.action_label = NULL;
    rc = ui_snackbar_base_enqueue(snackbar, &sconfig);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    snackbar->overlay_handle = (struct ui_overlay *)1;
    rc = ui_snackbar_base_tick(snackbar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    snackbar->overlay_handle = NULL;
    rc = ui_snackbar_base_destroy(snackbar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = ui_overlay_director_destroy(director);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(root_node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_timer_destroy(timer);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_snackbar_error_branches(void) {
  struct ui_timer_config tconfig;
  struct ui_timer *timer = NULL;
  struct ui_dom_node *root_node = NULL;
  struct ui_overlay_director *director = NULL;
  struct ui_snackbar_base *snackbar = NULL;
  struct ui_snackbar_config sconfig;
  struct ui_event ev;
  ui_error_t rc;
  int i;

  tconfig.time_source = mock_time_source;
  tconfig.user_data = NULL;

  rc = ui_timer_create_custom(&tconfig, &timer);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_set_tag_name(root_node, "body");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_create(root_node, &director);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 1. append_child mock failures in create */
  for (i = 1; i <= 5; i++) {
    g_snackbar_append_child_fail_target = i;
    snackbar = NULL;
    rc = ui_snackbar_base_create(timer, director, &snackbar);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    ASSERT(snackbar == NULL);
  }
  g_snackbar_append_child_fail_target = 0;

  /* 2. component_set_default_style mock failure in create */
  g_snackbar_mock_fail = 1;
  snackbar = NULL;
  rc = ui_snackbar_base_create(timer, director, &snackbar);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_snackbar_mock_fail = 0;

  /* 3. cleanup failures in create */
  for (i = 2; i <= 4; i++) {
    g_snackbar_append_child_fail_target = 1;
    g_snackbar_mock_fail = i;
    snackbar = NULL;
    rc = ui_snackbar_base_create(timer, director, &snackbar);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_snackbar_mock_fail = 0;
    g_snackbar_append_child_fail_target = 0;
  }

  /* Create valid snackbar for subsequent tests */
  rc = ui_snackbar_base_create(timer, director, &snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 4. Action callback failure during process_event */
  memset(&sconfig, 0, sizeof(sconfig));
  sconfig.message = "Cb Fail";
  sconfig.action_label = "FAIL";
  sconfig.action_callback = mock_action_cb_fail;
  rc = ui_snackbar_base_enqueue(snackbar, &sconfig);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_snackbar_base_tick(snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  rc = ui_snackbar_base_process_event(snackbar, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  rc = ui_snackbar_base_dismiss_current(snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 5. Unmount failure during process_event */
  sconfig.action_callback = mock_action_cb;
  rc = ui_snackbar_base_enqueue(snackbar, &sconfig);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_snackbar_base_tick(snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  {
    struct ui_overlay_director *orig = snackbar->director;
    snackbar->director = NULL;
    rc = ui_snackbar_base_process_event(snackbar, &ev, 1.0);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    snackbar->director = orig;
    rc = ui_snackbar_base_dismiss_current(snackbar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* 6. Unmount failure during auto-dismiss timeout */
  sconfig.duration_secs = 2.0;
  rc = ui_snackbar_base_enqueue(snackbar, &sconfig);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_snackbar_base_tick(snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_mock_time = 1000.0;
  {
    struct ui_overlay_director *orig = snackbar->director;
    snackbar->director = NULL;
    rc = ui_snackbar_base_tick(snackbar);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    snackbar->director = orig;
    rc = ui_snackbar_base_dismiss_current(snackbar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* 7. Unmount failure in destroy */
  rc = ui_snackbar_base_enqueue(snackbar, &sconfig);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_snackbar_base_tick(snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  {
    struct ui_overlay_director *orig = snackbar->director;
    snackbar->director = NULL;
    rc = ui_snackbar_base_destroy(snackbar);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    snackbar->director = orig;
  }

  /* 8. Queue and component destroy failures in destroy */
  g_snackbar_mock_fail = 4;
  rc = ui_snackbar_base_destroy(snackbar);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_snackbar_mock_fail = 0;

  rc = ui_snackbar_base_create(timer, director, &snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_snackbar_mock_fail = 3;
  rc = ui_snackbar_base_destroy(snackbar);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_snackbar_mock_fail = 0;

  /* 9. DOM and mount failure mocks during tick */
  for (i = 5; i <= 9; i++) {
    rc = ui_snackbar_base_create(timer, director, &snackbar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_snackbar_base_enqueue(snackbar, &sconfig);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_snackbar_mock_fail = i;
    rc = ui_snackbar_base_tick(snackbar);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_snackbar_mock_fail = 0;
    rc = ui_snackbar_base_destroy(snackbar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Timer now failure in tick */
  {
    struct ui_timer *orig_timer;
    rc = ui_snackbar_base_create(timer, director, &snackbar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    orig_timer = snackbar->timer;
    snackbar->timer = NULL;
    rc = ui_snackbar_base_tick(snackbar);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    snackbar->timer = orig_timer;
    rc = ui_snackbar_base_destroy(snackbar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Also test tick when action_label is NULL and mock fail 7 occurs */
  {
    sconfig.action_label = NULL;
    rc = ui_snackbar_base_create(timer, director, &snackbar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_snackbar_base_enqueue(snackbar, &sconfig);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_snackbar_mock_fail = 7;
    rc = ui_snackbar_base_tick(snackbar);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_snackbar_mock_fail = 0;
    rc = ui_snackbar_base_destroy(snackbar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* 10. Push failure when queue is full */
  rc = ui_snackbar_base_create(timer, director, &snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  sconfig.action_label = "FULL";
  for (i = 0; i < 10; i++) {
    rc = ui_snackbar_base_enqueue(snackbar, &sconfig);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = ui_snackbar_base_enqueue(snackbar, &sconfig);
  ASSERT_EQ(UI_ERROR_QUEUE_FULL, rc);

  sconfig.action_label = NULL;
  rc = ui_snackbar_base_enqueue(snackbar, &sconfig);
  ASSERT_EQ(UI_ERROR_QUEUE_FULL, rc);

  rc = ui_snackbar_base_destroy(snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_destroy(director);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(root_node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_timer_destroy(timer);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_snackbar_oom(void) {
  struct ui_timer_config tconfig;
  struct ui_timer *timer = NULL;
  struct ui_dom_node *root_node = NULL;
  struct ui_overlay_director *director = NULL;
  struct ui_snackbar_base *snackbar = NULL;
  struct ui_snackbar_config sconfig;
  ui_error_t rc;
  int i;
  int j;

  tconfig.time_source = mock_time_source;
  tconfig.user_data = NULL;

  rc = ui_timer_create_custom(&tconfig, &timer);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_set_tag_name(root_node, "body");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_create(root_node, &director);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&sconfig, 0, sizeof(sconfig));
  sconfig.message = "OOM msg";
  sconfig.action_label = "OOM act";
  sconfig.duration_secs = 2.0;

  for (i = 0; i < 100; i++) {
    g_malloc_fail_countdown = i;
    snackbar = NULL;
    rc = ui_snackbar_base_create(timer, director, &snackbar);
    if (rc == UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      rc = ui_snackbar_base_destroy(snackbar);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    ASSERT(snackbar == NULL);
  }
  g_malloc_fail_countdown = -1;

  /* Direct enqueue OOM tests */
  rc = ui_snackbar_base_create(timer, director, &snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_malloc_fail_countdown = 0;
  rc = ui_snackbar_base_enqueue(snackbar, &sconfig);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  rc = ui_snackbar_base_enqueue(snackbar, &sconfig);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
  rc = ui_snackbar_base_destroy(snackbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_overlay_director_destroy(director);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(root_node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_timer_destroy(timer);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(ui_snackbar_base_suite) {
  RUN_TEST(test_snackbar_invalid_args);
  RUN_TEST(test_snackbar_lifecycle_and_events);
  RUN_TEST(test_snackbar_error_branches);
  RUN_TEST(test_snackbar_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_snackbar_base_suite);
  GREATEST_MAIN_END();
}
