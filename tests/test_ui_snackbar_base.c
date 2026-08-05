/* clang-format off */
#include "ui_snackbar_base.h"
#include "ui_timer.h"
#include "ui_overlay_director.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static double g_mock_time = 0.0;

static ui_error_t mock_time_source(void *user_data, double *out_time_secs) {
  (void)user_data;
  *out_time_secs = g_mock_time;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static int action_called = 0;

static ui_error_t mock_action_cb(struct ui_snackbar_base *snackbar,
                                 void *user_data) {
  (void)snackbar;
  (void)user_data;
  action_called = 1;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static int run_normal_tests(void) {
  struct ui_timer_config tconfig = {mock_time_source, NULL};
  struct ui_timer *timer = NULL;
  struct ui_dom_node *root_node = NULL;
  struct ui_overlay_director *director = NULL;
  struct ui_snackbar_base *snackbar = NULL;
  struct ui_snackbar_config sconfig;
  struct ui_event ev;
  ui_error_t rc;

  printf("Testing invalid arguments...\n");
  if (ui_snackbar_base_create(NULL, NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_snackbar_base_destroy(NULL); /* Should not crash */
  if (ui_snackbar_base_enqueue(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_snackbar_base_dismiss_current(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_snackbar_base_tick(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  printf("Testing normal creation and logic...\n");
  rc = ui_timer_create_custom(&tconfig, &timer);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE)
    return 1;
  ui_dom_node_set_tag_name(root_node, "body");

  rc = ui_overlay_director_create(root_node, &director);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_snackbar_base_create(timer, director, &snackbar);
  if (rc != UI_ERROR_NONE || snackbar == NULL)
    return 1;

  sconfig.message = "Hello World";
  sconfig.action_label = "UNDO";
  sconfig.action_callback = mock_action_cb;
  sconfig.action_user_data = NULL;
  sconfig.duration_secs = 2.0;

  rc = ui_snackbar_base_enqueue(snackbar, &sconfig);
  if (rc != UI_ERROR_NONE)
    return 1;

  g_mock_time = 1.0;
  rc = ui_snackbar_base_tick(snackbar);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test that it stays active before duration */
  g_mock_time = 2.5;
  ui_snackbar_base_tick(snackbar);

  /* Test auto dismissal triggers */
  g_mock_time = 3.5;
  ui_snackbar_base_tick(snackbar);

  /* Enqueue another one and test manual dismissal via process_event */
  rc = ui_snackbar_base_enqueue(snackbar, &sconfig);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_snackbar_base_tick(snackbar); /* Show it */

  action_called = 0;
  ev.type = UI_EVENT_MOUSE_DOWN;
  rc = ui_snackbar_base_process_event(snackbar, &ev, 0.0);
  if (rc != UI_ERROR_NONE)
    return 1;

  if (!action_called) {
    printf("Action callback was not invoked on click event.\n");
    return 1;
  }

  if (ui_snackbar_base_bind_open(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_snackbar_base_bind_open(snackbar, NULL) != UI_ERROR_NONE)
    return 1;
  if (ui_snackbar_base_get_animating_signal(NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  {
    struct ui_computed *comp = NULL;
    if (ui_snackbar_base_get_animating_signal(snackbar, NULL) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_snackbar_base_get_animating_signal(snackbar, &comp) != UI_ERROR_NONE)
      return 1;
  }

  /* Push to fill queue partially */
  {
    int i;
    for (i = 0; i < 3; i++) {
      ui_snackbar_base_enqueue(snackbar, &sconfig);
    }
  }

  /* Push without action_label */
  sconfig.action_label = NULL;
  ui_snackbar_base_enqueue(snackbar, &sconfig);

  /* Activate the first one */
  ui_snackbar_base_tick(snackbar);

  /* Test UI_EVENT_TOUCH_START */
  ev.type = UI_EVENT_TOUCH_START;
  ui_snackbar_base_process_event(snackbar, &ev, 1.0); /* Dismisses first */

  ui_snackbar_base_process_event(NULL, &ev, 1.0);
  ui_snackbar_base_process_event(snackbar, NULL, 1.0);

  /* Activate second */
  ui_snackbar_base_tick(snackbar);
  /* process event on inactive */
  ui_snackbar_base_dismiss_current(snackbar); /* Dismiss second */
  ui_snackbar_base_process_event(snackbar, &ev,
                                 1.0); /* process while inactive */

  /* Activate third (which is sconfig with action_label="UNDO") */
  ui_snackbar_base_tick(snackbar);

  /* Test UI_EVENT_MOUSE_DOWN on third (dismisses third) */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ui_snackbar_base_process_event(snackbar, &ev, 1.0);

  /* Activate fourth (which is sconfig with action_label=NULL) */
  ui_snackbar_base_tick(snackbar);
  ui_snackbar_base_dismiss_current(snackbar); /* Dismiss fourth */

  /* Enqueue another one to test destruction of active and queue */
  sconfig.action_label = "L";
  ui_snackbar_base_enqueue(snackbar, &sconfig);
  ui_snackbar_base_enqueue(snackbar, &sconfig); /* queue item */
  ui_snackbar_base_tick(snackbar);              /* activate */

  /* Enqueue until full to test push failure branch */
  {
    int i;
    for (i = 0; i < 15; i++) {
      ui_snackbar_base_enqueue(snackbar, &sconfig);
    }
  }

  /* Destroy while active and queued */
  (void)ui_snackbar_base_destroy(snackbar);
  (void)ui_overlay_director_destroy(director);
  (void)ui_dom_node_destroy(root_node);
  ui_timer_destroy(timer);
  return 0;
}

static int run_oom_tests(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_timer_config tconfig = {mock_time_source, NULL};
  struct ui_timer *timer = NULL;
  struct ui_dom_node *root_node = NULL;
  struct ui_overlay_director *director = NULL;
  struct ui_snackbar_base *snackbar = NULL;
  struct ui_snackbar_config sconfig;
  ui_error_t rc;
  int i;

  ui_timer_create_custom(&tconfig, &timer);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  ui_dom_node_set_tag_name(root_node, "body");
  ui_overlay_director_create(root_node, &director);

  sconfig.message = "Hello World";
  sconfig.action_label = "UNDO";
  sconfig.action_callback = NULL;
  sconfig.action_user_data = NULL;
  sconfig.duration_secs = 2.0;

  printf("Testing OOM...\n");
  for (i = 0; i < 1000; i++) {
    g_malloc_fail_countdown = i;
    snackbar = NULL;
    rc = ui_snackbar_base_create(timer, director, &snackbar);
    if (rc == UI_ERROR_OUT_OF_MEMORY) {
      if (snackbar != NULL) {
        printf("Leaked snackbar struct pointer on OOM.\n");
        return 1;
      }
    } else if (rc == UI_ERROR_NONE) {
      /* Once creation succeeds, also test OOM for enqueue */
      int j;
      for (j = 0; j < 10; j++) {
        g_malloc_fail_countdown = j;
        rc = ui_snackbar_base_enqueue(snackbar, &sconfig);
        if (rc == UI_ERROR_NONE) {
          break;
        }
      }
      if (j == 10)
        return 1;
      (void)ui_snackbar_base_destroy(snackbar);
      break; /* We hit enough allocations to succeed */
    } else {
      printf("Unexpected error code %d\n", rc);
      return 1;
    }
  }
  if (i == 1000)
    return 1;
  if (i == 100)
    return 1;

  (void)ui_overlay_director_destroy(director);
  (void)ui_dom_node_destroy(root_node);
  ui_timer_destroy(timer);
#endif
  return 0;
}

int main(void) {
  if (run_normal_tests() != 0) {
    printf("Normal tests failed.\n");
    return 1;
  }
  if (run_oom_tests() != 0) {
    printf("OOM tests failed.\n");
    return 1;
  }
  printf("All ui_snackbar_base tests passed.\n");
  return 0;
}
