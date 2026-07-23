/* clang-format off */
#include "../include/ui_predictive_back.h"
#include "../include/ui_error.h"
#include "../include/ui_event.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_predictive_back_lifecycle(void) {
  struct ui_predictive_back *t = NULL;
  enum ui_error rc;

  rc = ui_predictive_back_create(&t);
  if (rc != UI_ERROR_NONE || t == NULL)
    return 1;

  ui_predictive_back_destroy(t);
  return 0;
}

static int test_predictive_back_gesture(void) {
  struct ui_predictive_back *t = NULL;
  struct ui_event ev;
  enum ui_error rc;
  struct ui_signal *sig1 = (struct ui_signal *)0x123;
  struct ui_signal *sig2 = (struct ui_signal *)0x456;

  rc = ui_predictive_back_create(&t);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_predictive_back_configure(t, 40, 1000);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_predictive_back_bind_progress(t, sig1);
  ui_predictive_back_bind_commit(t, sig2);

  /* Start outside edge */
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 50;
  rc = ui_predictive_back_process_event(t, &ev);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Start inside edge */
  ev.event_data.touch.points[0].x = 10;
  rc = ui_predictive_back_process_event(t, &ev);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Swipe right past 1.0 */
  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.points[0].x = 1510;
  rc = ui_predictive_back_process_event(t, &ev);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Swipe left past 0.0 */
  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.points[0].x = -100;
  rc = ui_predictive_back_process_event(t, &ev);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Swipe to valid mid point (e.g. 510 = 0.5) */
  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.points[0].x = 510;
  rc = ui_predictive_back_process_event(t, &ev);

  /* Touch move with 0 points (should ignore) */
  ev.event_data.touch.num_points = 0;
  ui_predictive_back_process_event(t, &ev);

  /* End past threshold */
  ev.type = UI_EVENT_TOUCH_END;
  rc = ui_predictive_back_process_event(t, &ev);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* TOUCH_END while not tracking */
  ev.type = UI_EVENT_TOUCH_END;
  rc = ui_predictive_back_process_event(t, &ev);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* TOUCH_MOVE while not tracking */
  ev.type = UI_EVENT_TOUCH_MOVE;
  rc = ui_predictive_back_process_event(t, &ev);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Try cancel event inside edge */
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 10;
  rc = ui_predictive_back_process_event(t, &ev);
  if (rc != UI_ERROR_NONE)
    return 1;

  ev.type = UI_EVENT_TOUCH_CANCEL;
  rc = ui_predictive_back_process_event(t, &ev);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Start inside edge but end before threshold */
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 10;
  ui_predictive_back_process_event(t, &ev);

  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.points[0].x = 50; /* 0.04 */
  ui_predictive_back_process_event(t, &ev);

  ev.type = UI_EVENT_TOUCH_END;
  ui_predictive_back_process_event(t, &ev);

  /* Touch start with 0 points */
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 0;
  ui_predictive_back_process_event(t, &ev);

  /* Configure with 0 screen width (div by zero check) */
  ui_predictive_back_configure(t, 40, 0);
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 10;
  ui_predictive_back_process_event(t, &ev);
  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.points[0].x = 50;
  ui_predictive_back_process_event(t, &ev);

  ui_predictive_back_destroy(t);
  return 0;
}

static int test_predictive_back_nulls_and_oom(void) {
  struct ui_predictive_back *t = NULL;
  struct ui_event ev;
  struct ui_signal *sig = (struct ui_signal *)0x123;

  if (ui_predictive_back_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_predictive_back_destroy(NULL);

  if (ui_predictive_back_configure(NULL, 10, 10) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_predictive_back_bind_progress(NULL, sig) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_predictive_back_bind_progress(t, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_predictive_back_bind_commit(NULL, sig) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_predictive_back_bind_commit(t, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_predictive_back_process_event(NULL, &ev) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_predictive_back_create(&t);
  if (ui_predictive_back_bind_progress(t, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_predictive_back_bind_commit(t, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_predictive_back_process_event(t, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_predictive_back_destroy(t);

  g_malloc_fail_countdown = 0;
  if (ui_predictive_back_create(&t) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_predictive_back tests...\n");

  failed |= test_predictive_back_lifecycle();
  failed |= test_predictive_back_gesture();
  failed |= test_predictive_back_nulls_and_oom();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
