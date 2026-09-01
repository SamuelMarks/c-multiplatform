/* clang-format off */
#include "ui_bottom_sheet_base.h"
#include "ui_overlay_director.h"
#include "ui_dom_node.h"
#include "ui_event.h"
#include "ui_keyboard_responder.h"
#include "ui_signal.h"
#include "ui_arena.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

struct ui_bottom_sheet_base_internal {
  struct ui_component *component;
  struct ui_dom_node *root_node;
  struct ui_dom_node *sheet_node;
  struct ui_dom_node *drag_handle_node;
  struct ui_dom_node *content_node;
  struct ui_component *content_component;
  int is_open;
  struct ui_overlay_director *director;
  struct ui_backdrop *backdrop_logic;
  struct ui_overlay *overlay;
  struct ui_gesture_recognizer *gesture_recognizer;
  struct ui_spring_config spring_config;
  struct ui_spring_state spring_state;
  ui_bottom_sheet_on_close_t on_close;
  void *user_data;
  struct ui_signal *open_signal;
  struct ui_computed *animating_signal;
};

static int g_closed_count = 0;
static int g_close_should_fail = 0;

static ui_error_t on_close_handler(struct ui_bottom_sheet_base *sheet,
                                   void *user_data) {
  (void)sheet;
  (void)user_data;
  if (g_close_should_fail)
    return UI_ERROR_OUT_OF_MEMORY;
  g_closed_count++;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static ui_error_t run_normal_tests(void) {
  struct ui_bottom_sheet_base *sheet = NULL;
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root = NULL;
  struct ui_event ev;
  ui_error_t rc;

  printf("Testing invalid arguments...\n");
  fflush(stdout);
  rc = ui_bottom_sheet_base_create(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_destroy(NULL); /* Should not crash */
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_bottom_sheet_base_set_content(NULL, NULL);

  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_set_open(NULL, 1);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  int is_open = 0;
  rc = ui_bottom_sheet_base_is_open(NULL, &is_open);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_bottom_sheet_base_set_overlay_director(NULL, NULL);

  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_set_on_close(NULL, NULL, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  rc = ui_bottom_sheet_base_process_event(NULL, &ev, 0.0);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_update(NULL, 0.0);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  struct ui_component *tmp_comp = NULL;
  rc = ui_bottom_sheet_base_get_component(NULL, &tmp_comp);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  struct ui_spring_config spring = {1.0f, 1.0f, 1.0f};
  rc = ui_bottom_sheet_base_set_spring_config(NULL, &spring);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_bottom_sheet_base_bind_open(NULL, NULL);

  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  struct ui_computed *anim_sig = NULL;
  rc = ui_bottom_sheet_base_get_animating_signal(NULL, &anim_sig);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  printf("Creating sheet...\n");
  fflush(stdout);
  rc = ui_bottom_sheet_base_create(&sheet);
  assert(rc == UI_ERROR_NONE && sheet != NULL);

  rc = ui_bottom_sheet_base_is_open(sheet, NULL);

  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_process_event(sheet, NULL, 0.0);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_bottom_sheet_base_get_component(sheet, NULL);

  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  assert(ui_bottom_sheet_base_get_component(sheet, &tmp_comp) ==
             UI_ERROR_NONE &&
         tmp_comp != NULL);

  rc = ui_bottom_sheet_base_set_spring_config(sheet, NULL);

  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_set_spring_config(sheet, &spring);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  struct ui_arena *arena = NULL;
  rc = ui_arena_create(1024, &arena);
  if (rc != UI_ERROR_NONE)
    return rc;
  struct ui_signal *signal = NULL;
  union ui_signal_payload init_payload;
  memset(&init_payload, 0, sizeof(init_payload));
  rc = ui_signal_create(arena, init_payload, UI_SIGNAL_TYPE_BOOL, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &signal);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_bottom_sheet_base_bind_open(sheet, signal);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_bottom_sheet_base_get_animating_signal(sheet, NULL);

  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_get_animating_signal(sheet, &anim_sig);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  printf("Check DOM\n");
  fflush(stdout);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_overlay_director_create(root, &director);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_bottom_sheet_base_set_overlay_director(sheet, director);

  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_set_on_close(sheet, on_close_handler, NULL);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  printf("Check open\n");
  fflush(stdout);
  rc = ui_bottom_sheet_base_set_open(sheet, 1);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_is_open(sheet, &is_open);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  assert(is_open == 1);

  rc = ui_bottom_sheet_base_set_open(sheet, 1);

  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc; /* Already open */
  rc = ui_bottom_sheet_base_set_open(sheet, 0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc; /* Close */
  rc = ui_bottom_sheet_base_is_open(sheet, &is_open);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  assert(is_open == 0);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = ui_bottom_sheet_base_set_open(sheet, 1);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  g_malloc_fail_countdown = -1;
#endif
  /* Open again for keyboard test */
  rc = ui_bottom_sheet_base_set_open(sheet, 1);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  printf("Check escape\n");
  fflush(stdout);
  g_closed_count = 0;
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 100.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  assert(g_closed_count == 1);

  /* Open again for gesture test */
  rc = ui_bottom_sheet_base_set_open(sheet, 1);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  g_closed_count = 0;

  printf("Drag down to dismiss physics evaluated bounding velocity.\n");
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 200.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 200; /* Move down 100 pixels */
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 210.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 220;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 220.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  assert(g_closed_count == 1);

  /* Open again for gesture test without on_close handler */
  rc = ui_bottom_sheet_base_set_open(sheet, 1);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_set_on_close(sheet, NULL, NULL);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 10;
  ev.event_data.mouse.y = 10;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 300.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 10;
  ev.event_data.mouse.y = 110;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 310.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 10;
  ev.event_data.mouse.y = 120;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 320.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_bottom_sheet_base_is_open(sheet, &is_open);

  if (rc != UI_ERROR_NONE)
    return rc;
  assert(is_open == 0); /* Closed via fallback because no handler */

  /* Open again for backdrop click test */
  rc = ui_bottom_sheet_base_set_open(sheet, 1);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_set_on_close(sheet, on_close_handler, NULL);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  g_closed_count = 0;

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 200; /* Outside the bounds provided to backdrop */
  ev.event_data.mouse.y = 200;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 400.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.x = 200;
  ev.event_data.mouse.y = 200;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 410.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  assert(g_closed_count == 1);

  /* Backdrop click test without handler */
  rc = ui_bottom_sheet_base_set_open(sheet, 1);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_set_on_close(sheet, NULL, NULL);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 200;
  ev.event_data.mouse.y = 200;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 500.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.x = 200;
  ev.event_data.mouse.y = 200;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 510.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_is_open(sheet, &is_open);
  if (rc != UI_ERROR_NONE)
    return rc;
  assert(is_open == 0);

  /* Test setting actual content */
  struct ui_component *content_comp = NULL;
  rc = ui_component_create(&content_comp);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &content_comp->shadow_root);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_bottom_sheet_base_set_content(sheet, content_comp);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_component_destroy(content_comp);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_bottom_sheet_base_set_content(sheet, NULL);

  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_bottom_sheet_base_update(sheet, 250.0);

  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_set_open(sheet, 0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_update(sheet, 260.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 265.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_bottom_sheet_base_set_open(sheet, 1);

  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_update(sheet, 270.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_set_on_close(sheet, on_close_handler, NULL);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Error paths for on_close */
  g_close_should_fail = 1;
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 600.0);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 200; /* Backdrop */
  ev.event_data.mouse.y = 200;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 610.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.x = 200;
  ev.event_data.mouse.y = 200;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 620.0);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* And for swipe */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 700.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 200;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 710.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 220;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 720.0);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Test gesture state CHANGED (not ENDED) */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 150;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 730.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Test gesture state ENDED but velocity/delta conditions fail (e.g. swipe up)
   */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 200;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 740.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100; /* Move UP */
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 750.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 760.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Test diagonal swipe to get delta_y > 0 but velocity_y <= 300 */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 800.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 600;
  ev.event_data.mouse.y = 120;
  /* dt = 100ms. dx = 500 -> vx = 5000. dy = 20 -> vy = 200 */
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 900.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 600;
  ev.event_data.mouse.y = 121; /* delta_y = 1 on UP event, so delta_y > 0 is
                                  true, but velocity_y is 200 <= 300 */
  rc = ui_bottom_sheet_base_process_event(sheet, &ev, 910.0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Test error paths inside ui_bottom_sheet_base_process_event by poking
   * internals */
  {
    struct ui_bottom_sheet_base_internal *internal =
        (struct ui_bottom_sheet_base_internal *)sheet;
    struct ui_backdrop *old_backdrop = internal->backdrop_logic;
    struct ui_gesture_recognizer *old_gesture = internal->gesture_recognizer;
    struct ui_dom_node *old_sheet_node = internal->sheet_node;

    internal->backdrop_logic = NULL;
    rc = ui_bottom_sheet_base_process_event(sheet, &ev, 800.0);
    if (rc != UI_ERROR_INVALID_ARGUMENT)
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
    internal->backdrop_logic = old_backdrop;

    internal->gesture_recognizer = NULL;
    rc = ui_bottom_sheet_base_process_event(sheet, &ev, 810.0);
    if (rc != UI_ERROR_INVALID_ARGUMENT)
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
    internal->gesture_recognizer = old_gesture;
    internal->sheet_node = NULL;
    rc = ui_bottom_sheet_base_set_open(sheet, 1);
    if (rc != UI_ERROR_NONE)
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
    internal->sheet_node = old_sheet_node;
  }

  g_close_should_fail = 0;

  rc = ui_bottom_sheet_base_destroy(sheet);

  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_overlay_director_destroy(director);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_dom_node_destroy(root);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_signal_destroy(signal);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_arena_destroy(arena);
  if (rc != UI_ERROR_NONE)
    return rc;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static ui_error_t run_oom_tests(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_bottom_sheet_base *sheet = NULL;
  ui_error_t rc;
  int i;

  printf("Testing OOM...\n");

  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_bottom_sheet_base_create(&sheet);
    if (rc == UI_ERROR_OUT_OF_MEMORY) {
      continue;
    } else if (rc == UI_ERROR_NONE) {
      rc = ui_bottom_sheet_base_destroy(sheet);
      if (rc != UI_ERROR_NONE)
        return rc;
      break;
    } else {
      printf("OOM failed at index %d with code %d\n", i, rc);
      g_malloc_fail_countdown = -1;
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
    }
  }
  g_malloc_fail_countdown = -1;
#endif
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static ui_error_t test_branch_coverage_destroy(void) {
  ui_error_t rc;
  struct ui_bottom_sheet_base *sheet = NULL;

  /* Test destroy when closed */
  rc = ui_bottom_sheet_base_create(&sheet);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_destroy(sheet);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Test destroy when open but no director */
  rc = ui_bottom_sheet_base_create(&sheet);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_set_open(sheet, 1);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_destroy(sheet);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Test set_open edge cases */
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root = NULL;
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_overlay_director_create(root, &director);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Test open without director, then close without director (covers else branch
   * without director) */
  rc = ui_bottom_sheet_base_create(&sheet);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_set_open(sheet, 1);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_set_open(sheet, 0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Now test with director but no overlay, then close */
  rc = ui_bottom_sheet_base_set_overlay_director(sheet, director);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_bottom_sheet_base_set_open(sheet, 1);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  /* Force overlay to NULL to test that branch */
  rc = ui_overlay_director_unmount(
      director, ((struct ui_bottom_sheet_base_internal *)sheet)->overlay);
  if (rc != UI_ERROR_NONE)
    return rc;
  ((struct ui_bottom_sheet_base_internal *)sheet)->overlay = NULL;
  rc = ui_bottom_sheet_base_set_open(sheet, 0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Test destroy when open, with director, but overlay is NULL */
  rc = ui_bottom_sheet_base_set_open(sheet, 1);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_overlay_director_unmount(
      director, ((struct ui_bottom_sheet_base_internal *)sheet)->overlay);
  if (rc != UI_ERROR_NONE)
    return rc;
  ((struct ui_bottom_sheet_base_internal *)sheet)->overlay = NULL;
  rc = ui_bottom_sheet_base_destroy(sheet);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_bottom_sheet_base_create(&sheet);

  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  /* Set director but mock mount error inside set_open if possible,
     or just test open and close repeatedly */
  rc = ui_bottom_sheet_base_set_overlay_director(sheet, director);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = ui_bottom_sheet_base_set_open(sheet, 1);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  g_malloc_fail_countdown = -1;
#endif

  rc = ui_bottom_sheet_base_destroy(sheet);

  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_overlay_director_destroy(director);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_dom_node_destroy(root);
  if (rc != UI_ERROR_NONE)
    return rc;
  return UI_ERROR_NONE;
}

static ui_error_t run_mock_fail_tests(void) {
#ifdef UI_TEST_MOCK_ALLOC
  int i;
  extern int g_bottom_sheet_mock_fail;

  for (i = 1; i <= 16; i++) {
    struct ui_bottom_sheet_base *sheet = NULL;
    ui_error_t rc;
    g_bottom_sheet_mock_fail = i;
    rc = ui_bottom_sheet_base_create(&sheet);
    if (rc == UI_ERROR_NONE && sheet) {
      g_bottom_sheet_mock_fail = 0;
      ui_bottom_sheet_base_destroy(sheet);
    }
  }

  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root = NULL;

  struct ui_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;

/* Helper macro to test a specific mock failure safely */
#define TEST_MOCK(fail_id, action)                                             \
  do {                                                                         \
    struct ui_bottom_sheet_base *sheet = NULL;                                 \
    ui_bottom_sheet_base_create(&sheet);                                       \
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);                       \
    ui_overlay_director_create(root, &director);                               \
    ui_bottom_sheet_base_set_overlay_director(sheet, director);                \
    g_bottom_sheet_mock_fail = (fail_id);                                      \
    action;                                                                    \
    g_bottom_sheet_mock_fail = 0;                                              \
    if ((fail_id) != 8 && (fail_id) != 9 && (fail_id) != 10)                   \
      ui_bottom_sheet_base_destroy(sheet);                                     \
    ui_overlay_director_destroy(director);                                     \
    ui_dom_node_destroy(root);                                                 \
  } while (0)

  TEST_MOCK(4, ui_bottom_sheet_base_set_open(sheet, 1));
  TEST_MOCK(5, {
    ui_bottom_sheet_base_set_open(sheet, 1);
    g_bottom_sheet_mock_fail = 5;
    ui_bottom_sheet_base_set_open(sheet, 0);
  });

  /* Set open signal and test it */
  TEST_MOCK(11, {
    ui_bottom_sheet_base_bind_open(sheet, (struct ui_signal *)1);
    ui_bottom_sheet_base_set_open(sheet, 1);
    g_bottom_sheet_mock_fail = 11;
    ui_bottom_sheet_base_set_open(sheet, 0);
  });

  TEST_MOCK(7, {
    ui_bottom_sheet_base_set_open(sheet, 1);
    g_bottom_sheet_mock_fail = 7;
    ui_bottom_sheet_base_set_open(sheet, 0);
  });
  TEST_MOCK(7, {
    ui_bottom_sheet_base_set_open(sheet, 1);
    g_bottom_sheet_mock_fail = 7;
    ui_bottom_sheet_base_destroy(sheet);
    g_bottom_sheet_mock_fail = 0;
  });
  TEST_MOCK(12, ui_bottom_sheet_base_update(sheet, 100.0));
  TEST_MOCK(16, ui_bottom_sheet_base_process_event(sheet, &ev, 100.0));
  TEST_MOCK(17, {
    ui_bottom_sheet_base_set_open(sheet, 1);
    g_bottom_sheet_mock_fail = 17;
    ui_bottom_sheet_base_process_event(sheet, &ev, 100.0);
    g_bottom_sheet_mock_fail = 0;
  });
  TEST_MOCK(18, {
    ui_bottom_sheet_base_set_open(sheet, 1);
    g_bottom_sheet_mock_fail = 18;
    ui_bottom_sheet_base_process_event(sheet, &ev, 100.0);
    g_bottom_sheet_mock_fail = 0;
  });

  TEST_MOCK(8, ui_bottom_sheet_base_destroy(sheet));
  TEST_MOCK(9, ui_bottom_sheet_base_destroy(sheet));
  TEST_MOCK(10, ui_bottom_sheet_base_destroy(sheet));

#undef TEST_MOCK
#endif
  return UI_ERROR_NONE;
}

int main(void) {
  ui_error_t rc;

#ifdef UI_TEST_MOCK_ALLOC
  extern ui_error_t run_bottom_sheet_coverage(void);
  rc = run_bottom_sheet_coverage();
  if (rc != UI_ERROR_NONE)
    return rc;
#endif

  if (run_normal_tests() != UI_ERROR_NONE) {
    printf("Normal tests failed.\n");
    return 1;
  }

  rc = test_branch_coverage_destroy();

  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  if (run_oom_tests() != UI_ERROR_NONE) {
    printf("OOM tests failed.\n");
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }

  run_mock_fail_tests();
  printf("All ui_bottom_sheet_base tests passed.\n");
  return 0;
}
