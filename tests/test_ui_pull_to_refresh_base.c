/* clang-format off */
#include "ui_pull_to_refresh_base.h"
#include "ui_gesture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

#undef NDEBUG
#include <assert.h>

extern int g_malloc_fail_countdown;

static int refresh_count = 0;

static ui_error_t on_refresh(struct ui_pull_to_refresh_base *ptr,
                             void *user_data) {
  (void)ptr;
  (void)user_data;
  refresh_count++;
  return UI_ERROR_NONE;
}

static void test_ptr_basic(void) {
  struct ui_pull_to_refresh_base *ptr = NULL;
  struct ui_component *spinner = NULL;
  ui_error_t rc;

  refresh_count = 0;

  rc = ui_pull_to_refresh_base_create(&ptr);
  assert(rc == UI_ERROR_NONE);
  assert(ptr != NULL);

  rc = ui_component_create(&spinner);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &spinner->shadow_root);

  rc = ui_pull_to_refresh_base_set_spinner(ptr, spinner);
  assert(rc == UI_ERROR_NONE);

  rc = ui_pull_to_refresh_base_set_on_refresh(ptr, on_refresh, NULL);
  assert(rc == UI_ERROR_NONE);

  assert(ui_pull_to_refresh_base_get_state(ptr) == UI_PULL_TO_REFRESH_RESTING);
  {
    float progress = 0.0f;
    ui_pull_to_refresh_base_get_progress(ptr, &progress);
    assert(progress == 0.0f);
  }

  /* Call complete when not refreshing */
  ui_pull_to_refresh_base_complete(ptr);

  /* Send Gesture Pan Began */
  struct ui_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 0;
  ev.event_data.mouse.y = 0;

  rc = ui_pull_to_refresh_base_process_event(ptr, &ev, 100.0);
  assert(rc == UI_ERROR_NONE);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 50;
  rc = ui_pull_to_refresh_base_process_event(ptr, &ev, 1100.0);
  assert(rc == UI_ERROR_NONE);

  /* Now state is PULLING. Try to trigger PAN BEGAN again (which is tricky with
     the same recognizer). Actually, if we send a new MOUSE_DOWN it will reset
     and send BEGAN again. Let's do that. */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.y = 50;
  rc = ui_pull_to_refresh_base_process_event(ptr, &ev, 1200.0);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 100;
  rc = ui_pull_to_refresh_base_process_event(ptr, &ev, 1300.0);

  /* State should now be pulling because it moved enough to trigger a pan
   * gesture */
  assert(ui_pull_to_refresh_base_get_state(ptr) == UI_PULL_TO_REFRESH_PULLING);

  /* Send a negative delta_y to push back up, hitting the ge.delta_y <= 0.0f
     branch inside the PULLING state */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 80; /* moved up by 20 */
  rc = ui_pull_to_refresh_base_process_event(ptr, &ev, 1400.0);
  assert(rc == UI_ERROR_NONE);

  /* Pull some more past threshold */
  ev.event_data.mouse.y = 200;
  rc = ui_pull_to_refresh_base_process_event(ptr, &ev, 2100.0);
  assert(rc == UI_ERROR_NONE);

  ev.type = UI_EVENT_MOUSE_UP;
  rc = ui_pull_to_refresh_base_process_event(ptr, &ev, 3100.0);
  assert(rc == UI_ERROR_NONE);

  /* State should now be refreshing */
  assert(ui_pull_to_refresh_base_get_state(ptr) ==
         UI_PULL_TO_REFRESH_REFRESHING);
  assert(refresh_count == 1);

  /* Process event while refreshing */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.y = 200;
  rc = ui_pull_to_refresh_base_process_event(ptr, &ev, 3200.0);
  assert(rc == UI_ERROR_NONE);

  /* Mark complete */
  rc = ui_pull_to_refresh_base_complete(ptr);
  assert(rc == UI_ERROR_NONE);
  assert(ui_pull_to_refresh_base_get_state(ptr) ==
         UI_PULL_TO_REFRESH_COMPLETING);

  /* Process event while completing */
  rc = ui_pull_to_refresh_base_process_event(ptr, &ev, 3300.0);
  assert(rc == UI_ERROR_NONE);

  /* Tick until resting */
  /* Hit else branch in completing */
  ui_pull_to_refresh_base_on_tick(ptr, 10.0);

  /* Hit pull_distance < 1.0f or completion_timer_ms >=
   * UI_PTR_COMPLETION_DELAY_MS */
  ui_pull_to_refresh_base_on_tick(ptr, 100.0);
  ui_pull_to_refresh_base_on_tick(ptr, 100.0);
  ui_pull_to_refresh_base_on_tick(ptr, 150.0);
  assert(ui_pull_to_refresh_base_get_state(ptr) == UI_PULL_TO_REFRESH_RESTING);

  (void)ui_pull_to_refresh_base_destroy(ptr);

  spinner->shadow_root = NULL;
  (void)ui_component_destroy(spinner);
}

static void test_ptr_spring_back(void) {
  struct ui_pull_to_refresh_base *ptr = NULL;
  struct ui_event ev;
  memset(&ev, 0, sizeof(ev));

  ui_pull_to_refresh_base_create(&ptr);

  ev.type = UI_EVENT_MOUSE_DOWN;
  ui_pull_to_refresh_base_process_event(ptr, &ev, 100.0);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 20;
  ui_pull_to_refresh_base_process_event(ptr, &ev, 1100.0);

  ev.event_data.mouse.y = 50;
  ui_pull_to_refresh_base_process_event(ptr, &ev, 1200.0);

  /* End without crossing threshold */
  ev.type = UI_EVENT_MOUSE_UP;
  ui_pull_to_refresh_base_process_event(ptr, &ev, 2100.0);

  assert(ui_pull_to_refresh_base_get_state(ptr) == UI_PULL_TO_REFRESH_PULLING);

  /* Tick should spring it back to resting */
  ui_pull_to_refresh_base_on_tick(
      ptr, 10.0); /* Hit the spring condition once before loop */

  /* Wait, 100 ticks is not a loop, we might need multiple ticks.
     Spring rate is 0.85, 20 * 0.85^n < 1.0.
     20 * 0.85^20 < 1.0 */
  int i;
  for (i = 0; i < 20; i++) {
    ui_pull_to_refresh_base_on_tick(ptr, 16.0);
  }

  assert(ui_pull_to_refresh_base_get_state(ptr) == UI_PULL_TO_REFRESH_RESTING);
  (void)ui_pull_to_refresh_base_destroy(ptr);
}

static void test_ptr_push_up(void) {
  struct ui_pull_to_refresh_base *ptr = NULL;
  struct ui_event ev;
  float progress = 0.0f;
  memset(&ev, 0, sizeof(ev));

  ui_pull_to_refresh_base_create(&ptr);

  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 0;
  ev.event_data.mouse.y = 0;
  ui_pull_to_refresh_base_process_event(ptr, &ev, 100.0);

  /* Trigger BEGAN (pull_distance = 0) */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 0;
  ev.event_data.mouse.y = 50;
  ui_pull_to_refresh_base_process_event(ptr, &ev, 1100.0);

  /* Trigger CHANGED down way past threshold to hit resistance < 0.1f */
  ev.event_data.mouse.y = 1000;
  ui_pull_to_refresh_base_process_event(ptr, &ev, 1200.0);

  /* Trigger CHANGED up slightly (delta_y = -10, pull_distance >= 0 branch) */
  ev.event_data.mouse.y = 990;
  ui_pull_to_refresh_base_process_event(ptr, &ev, 1300.0);

  /* Trigger CHANGED up massively to go negative (< 0 branch) */
  ev.event_data.mouse.y = -500;
  ui_pull_to_refresh_base_process_event(ptr, &ev, 1400.0);

  ev.type = UI_EVENT_TOUCH_CANCEL;
  ui_pull_to_refresh_base_process_event(ptr, &ev, 1500.0);

  (void)ui_pull_to_refresh_base_destroy(ptr);
}

static void test_ptr_cancel(void) {
  struct ui_pull_to_refresh_base *ptr = NULL;
  struct ui_event ev;
  memset(&ev, 0, sizeof(ev));

  ui_pull_to_refresh_base_create(&ptr);

  ev.type = UI_EVENT_MOUSE_DOWN;
  ui_pull_to_refresh_base_process_event(ptr, &ev, 100.0);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 50;
  ui_pull_to_refresh_base_process_event(ptr, &ev, 1100.0);
  ev.event_data.mouse.y = 300; /* past threshold */
  ui_pull_to_refresh_base_process_event(ptr, &ev, 1200.0);

  /* cancel */
  ev.type = UI_EVENT_TOUCH_CANCEL; /* mapped to gesture cancel */
  ui_pull_to_refresh_base_process_event(ptr, &ev, 2100.0);

  /* Wait, our gesture recognizer maps TOUCH_CANCEL to GESTURE_STATE_CANCELLED
   * The logic in ui_pull_to_refresh_base.c treats ENDED and CANCELLED
   * identically so if we cross the threshold it goes to REFRESHING. If we
   * wanted to test the CANCEL fallback, we'd need to NOT cross the threshold.
   */
  assert(ui_pull_to_refresh_base_get_state(ptr) ==
         UI_PULL_TO_REFRESH_REFRESHING);

  (void)ui_pull_to_refresh_base_destroy(ptr);

  /* Now do it without crossing the threshold to hit the else branch */
  ui_pull_to_refresh_base_create(&ptr);

  ev.type = UI_EVENT_MOUSE_DOWN;
  ui_pull_to_refresh_base_process_event(ptr, &ev, 100.0);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 10; /* start pan, not past threshold */
  ui_pull_to_refresh_base_process_event(ptr, &ev, 1100.0);

  ev.type = UI_EVENT_TOUCH_CANCEL;
  ui_pull_to_refresh_base_process_event(ptr, &ev, 2100.0);

  assert(ui_pull_to_refresh_base_get_state(ptr) == UI_PULL_TO_REFRESH_PULLING);

  (void)ui_pull_to_refresh_base_destroy(ptr);
}

static void test_ptr_nulls(void) {
  struct ui_pull_to_refresh_base *ptr = NULL;
  struct ui_component *comp = NULL;
  struct ui_signal *sig = (struct ui_signal *)0x123;
  struct ui_computed *comp_sig = NULL;
  float progress;
  struct ui_event ev;

  ui_pull_to_refresh_base_create(&ptr);

  assert(ui_pull_to_refresh_base_create(NULL) == UI_ERROR_INVALID_ARGUMENT);

  assert(ui_pull_to_refresh_base_destroy(NULL) == UI_ERROR_NONE);

  assert(ui_pull_to_refresh_base_set_on_refresh(NULL, on_refresh, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);

  assert(ui_pull_to_refresh_base_complete(NULL) == UI_ERROR_INVALID_ARGUMENT);

  assert(ui_pull_to_refresh_base_get_state(NULL) == UI_PULL_TO_REFRESH_RESTING);

  assert(ui_pull_to_refresh_base_get_progress(NULL, &progress) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_pull_to_refresh_base_get_progress(ptr, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);

  assert(ui_pull_to_refresh_base_process_event(NULL, &ev, 0) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_pull_to_refresh_base_process_event(ptr, NULL, 0) ==
         UI_ERROR_INVALID_ARGUMENT);

  assert(ui_pull_to_refresh_base_on_tick(NULL, 0) == UI_ERROR_INVALID_ARGUMENT);

  assert(ui_pull_to_refresh_base_get_component(NULL, &comp) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_pull_to_refresh_base_get_component(ptr, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_pull_to_refresh_base_get_component(ptr, &comp) == UI_ERROR_NONE);
  assert(comp != NULL);

  assert(ui_pull_to_refresh_base_set_spinner(NULL, comp) ==
         UI_ERROR_INVALID_ARGUMENT);

  assert(ui_pull_to_refresh_base_bind_refreshing(NULL, sig) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_pull_to_refresh_base_bind_refreshing(ptr, sig) == UI_ERROR_NONE);

  assert(ui_pull_to_refresh_base_get_refreshing_signal(NULL, &comp_sig) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_pull_to_refresh_base_get_refreshing_signal(ptr, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_pull_to_refresh_base_get_refreshing_signal(ptr, &comp_sig) ==
         UI_ERROR_NONE);

  if (comp) {
    (void)ui_dom_node_destroy(comp->shadow_root);
    comp->shadow_root = NULL;
  }
  (void)ui_pull_to_refresh_base_destroy(ptr);
}

static void test_ptr_oom(void) {
  struct ui_pull_to_refresh_base *ptr = NULL;
  int i;
  for (i = 0; i < 10; i++) {
    g_malloc_fail_countdown = i;
    if (ui_pull_to_refresh_base_create(&ptr) == UI_ERROR_NONE) {
      (void)ui_pull_to_refresh_base_destroy(ptr);
      break;
    }
  }
  g_malloc_fail_countdown = -1;
}

int main(void) {
  test_ptr_basic();
  test_ptr_spring_back();
  test_ptr_push_up();
  test_ptr_cancel();
  test_ptr_nulls();
  test_ptr_oom();

  printf("test_ui_pull_to_refresh_base passed\n");
  return 0;
}
