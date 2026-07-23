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

static enum ui_error on_close_handler(struct ui_bottom_sheet_base *sheet,
                                      void *user_data) {
  (void)sheet;
  (void)user_data;
  if (g_close_should_fail)
    return UI_ERROR_OUT_OF_MEMORY;
  g_closed_count++;
  return UI_ERROR_NONE;
}

static int run_normal_tests(void) {
  struct ui_bottom_sheet_base *sheet = NULL;
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root = NULL;
  struct ui_event ev;
  enum ui_error rc;

  printf("Testing invalid arguments...\n");
  fflush(stdout);
  assert(ui_bottom_sheet_base_create(NULL) == UI_ERROR_INVALID_ARGUMENT);
  ui_bottom_sheet_base_destroy(NULL); /* Should not crash */

  assert(ui_bottom_sheet_base_set_content(NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_bottom_sheet_base_set_open(NULL, 1) == UI_ERROR_INVALID_ARGUMENT);

  int is_open = 0;
  assert(ui_bottom_sheet_base_is_open(NULL, &is_open) ==
         UI_ERROR_INVALID_ARGUMENT);

  assert(ui_bottom_sheet_base_set_overlay_director(NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_bottom_sheet_base_set_on_close(NULL, NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  assert(ui_bottom_sheet_base_process_event(NULL, &ev, 0.0) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_bottom_sheet_base_update(NULL, 0.0) == UI_ERROR_INVALID_ARGUMENT);

  struct ui_component *tmp_comp = NULL;
  assert(ui_bottom_sheet_base_get_component(NULL, &tmp_comp) ==
         UI_ERROR_INVALID_ARGUMENT);

  struct ui_spring_config spring = {1.0f, 1.0f, 1.0f};
  assert(ui_bottom_sheet_base_set_spring_config(NULL, &spring) ==
         UI_ERROR_INVALID_ARGUMENT);

  assert(ui_bottom_sheet_base_bind_open(NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);

  struct ui_computed *anim_sig = NULL;
  assert(ui_bottom_sheet_base_get_animating_signal(NULL, &anim_sig) ==
         UI_ERROR_INVALID_ARGUMENT);

  printf("Creating sheet...\n");
  fflush(stdout);
  rc = ui_bottom_sheet_base_create(&sheet);
  assert(rc == UI_ERROR_NONE && sheet != NULL);

  assert(ui_bottom_sheet_base_is_open(sheet, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_bottom_sheet_base_process_event(sheet, NULL, 0.0) ==
         UI_ERROR_INVALID_ARGUMENT);

  assert(ui_bottom_sheet_base_get_component(sheet, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_bottom_sheet_base_get_component(sheet, &tmp_comp) ==
             UI_ERROR_NONE &&
         tmp_comp != NULL);

  assert(ui_bottom_sheet_base_set_spring_config(sheet, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_bottom_sheet_base_set_spring_config(sheet, &spring) ==
         UI_ERROR_NONE);

  struct ui_arena *arena = NULL;
  ui_arena_create(1024, &arena);
  struct ui_signal *signal = NULL;
  union ui_signal_payload init_payload;
  memset(&init_payload, 0, sizeof(init_payload));
  ui_signal_create(arena, init_payload, UI_SIGNAL_TYPE_BOOL, NULL, NULL,
                   UI_SIGNAL_MODE_SINGLE_THREADED, &signal);
  assert(ui_bottom_sheet_base_bind_open(sheet, signal) == UI_ERROR_NONE);

  assert(ui_bottom_sheet_base_get_animating_signal(sheet, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_bottom_sheet_base_get_animating_signal(sheet, &anim_sig) ==
         UI_ERROR_NONE);

  printf("Check DOM\n");
  fflush(stdout);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_overlay_director_create(root, &director);

  assert(ui_bottom_sheet_base_set_overlay_director(sheet, director) ==
         UI_ERROR_NONE);
  assert(ui_bottom_sheet_base_set_on_close(sheet, on_close_handler, NULL) ==
         UI_ERROR_NONE);

  printf("Check open\n");
  fflush(stdout);
  assert(ui_bottom_sheet_base_set_open(sheet, 1) == UI_ERROR_NONE);
  assert(ui_bottom_sheet_base_is_open(sheet, &is_open) == UI_ERROR_NONE);
  assert(is_open == 1);

  assert(ui_bottom_sheet_base_set_open(sheet, 1) ==
         UI_ERROR_NONE); /* Already open */
  assert(ui_bottom_sheet_base_set_open(sheet, 0) == UI_ERROR_NONE); /* Close */
  assert(ui_bottom_sheet_base_is_open(sheet, &is_open) == UI_ERROR_NONE);
  assert(is_open == 0);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  assert(ui_bottom_sheet_base_set_open(sheet, 1) == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
#endif
  /* Open again for keyboard test */
  assert(ui_bottom_sheet_base_set_open(sheet, 1) == UI_ERROR_NONE);

  printf("Check escape\n");
  fflush(stdout);
  g_closed_count = 0;
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 100.0) ==
         UI_ERROR_NONE);
  assert(g_closed_count == 1);

  /* Open again for gesture test */
  assert(ui_bottom_sheet_base_set_open(sheet, 1) == UI_ERROR_NONE);
  g_closed_count = 0;

  printf("Drag down to dismiss physics evaluated bounding velocity.\n");
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 200.0) ==
         UI_ERROR_NONE);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 200; /* Move down 100 pixels */
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 210.0) ==
         UI_ERROR_NONE);

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 220;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 220.0) ==
         UI_ERROR_NONE);
  assert(g_closed_count == 1);

  /* Open again for gesture test without on_close handler */
  assert(ui_bottom_sheet_base_set_open(sheet, 1) == UI_ERROR_NONE);
  assert(ui_bottom_sheet_base_set_on_close(sheet, NULL, NULL) == UI_ERROR_NONE);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 10;
  ev.event_data.mouse.y = 10;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 300.0) ==
         UI_ERROR_NONE);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 10;
  ev.event_data.mouse.y = 110;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 310.0) ==
         UI_ERROR_NONE);

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 10;
  ev.event_data.mouse.y = 120;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 320.0) ==
         UI_ERROR_NONE);

  ui_bottom_sheet_base_is_open(sheet, &is_open);
  assert(is_open == 0); /* Closed via fallback because no handler */

  /* Open again for backdrop click test */
  assert(ui_bottom_sheet_base_set_open(sheet, 1) == UI_ERROR_NONE);
  assert(ui_bottom_sheet_base_set_on_close(sheet, on_close_handler, NULL) ==
         UI_ERROR_NONE);
  g_closed_count = 0;

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 200; /* Outside the bounds provided to backdrop */
  ev.event_data.mouse.y = 200;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 400.0) ==
         UI_ERROR_NONE);

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.x = 200;
  ev.event_data.mouse.y = 200;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 410.0) ==
         UI_ERROR_NONE);
  assert(g_closed_count == 1);

  /* Backdrop click test without handler */
  assert(ui_bottom_sheet_base_set_open(sheet, 1) == UI_ERROR_NONE);
  assert(ui_bottom_sheet_base_set_on_close(sheet, NULL, NULL) == UI_ERROR_NONE);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 200;
  ev.event_data.mouse.y = 200;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 500.0) ==
         UI_ERROR_NONE);

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.x = 200;
  ev.event_data.mouse.y = 200;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 510.0) ==
         UI_ERROR_NONE);
  ui_bottom_sheet_base_is_open(sheet, &is_open);
  assert(is_open == 0);

  /* Test setting actual content */
  struct ui_component *content_comp = NULL;
  ui_component_create(&content_comp);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &content_comp->shadow_root);
  assert(ui_bottom_sheet_base_set_content(sheet, content_comp) ==
         UI_ERROR_NONE);
  ui_component_destroy(content_comp);

  assert(ui_bottom_sheet_base_set_content(sheet, NULL) == UI_ERROR_NONE);

  assert(ui_bottom_sheet_base_update(sheet, 250.0) == UI_ERROR_NONE);
  assert(ui_bottom_sheet_base_set_open(sheet, 0) == UI_ERROR_NONE);
  assert(ui_bottom_sheet_base_update(sheet, 260.0) == UI_ERROR_NONE);
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 265.0) ==
         UI_ERROR_NONE);

  assert(ui_bottom_sheet_base_set_open(sheet, 1) == UI_ERROR_NONE);
  assert(ui_bottom_sheet_base_update(sheet, 270.0) == UI_ERROR_NONE);
  assert(ui_bottom_sheet_base_set_on_close(sheet, on_close_handler, NULL) ==
         UI_ERROR_NONE);

  /* Error paths for on_close */
  g_close_should_fail = 1;
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 600.0) ==
         UI_ERROR_OUT_OF_MEMORY);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 200; /* Backdrop */
  ev.event_data.mouse.y = 200;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 610.0) ==
         UI_ERROR_NONE);

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.x = 200;
  ev.event_data.mouse.y = 200;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 620.0) ==
         UI_ERROR_OUT_OF_MEMORY);

  /* And for swipe */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 700.0) ==
         UI_ERROR_NONE);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 200;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 710.0) ==
         UI_ERROR_NONE);

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 220;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 720.0) ==
         UI_ERROR_OUT_OF_MEMORY);

  /* Test gesture state CHANGED (not ENDED) */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 150;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 730.0) ==
         UI_ERROR_NONE);

  /* Test gesture state ENDED but velocity/delta conditions fail (e.g. swipe up)
   */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 200;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 740.0) ==
         UI_ERROR_NONE);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100; /* Move UP */
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 750.0) ==
         UI_ERROR_NONE);

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 760.0) ==
         UI_ERROR_NONE);

  /* Test diagonal swipe to get delta_y > 0 but velocity_y <= 300 */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 800.0) ==
         UI_ERROR_NONE);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 600;
  ev.event_data.mouse.y = 120;
  /* dt = 100ms. dx = 500 -> vx = 5000. dy = 20 -> vy = 200 */
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 900.0) ==
         UI_ERROR_NONE);

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 600;
  ev.event_data.mouse.y = 121; /* delta_y = 1 on UP event, so delta_y > 0 is
                                  true, but velocity_y is 200 <= 300 */
  assert(ui_bottom_sheet_base_process_event(sheet, &ev, 910.0) ==
         UI_ERROR_NONE);

  /* Test error paths inside ui_bottom_sheet_base_process_event by poking
   * internals */
  {
    struct ui_bottom_sheet_base_internal *internal =
        (struct ui_bottom_sheet_base_internal *)sheet;
    struct ui_backdrop *old_backdrop = internal->backdrop_logic;
    struct ui_gesture_recognizer *old_gesture = internal->gesture_recognizer;
    struct ui_dom_node *old_sheet_node = internal->sheet_node;

    internal->backdrop_logic = NULL;
    assert(ui_bottom_sheet_base_process_event(sheet, &ev, 800.0) ==
           UI_ERROR_INVALID_ARGUMENT);
    internal->backdrop_logic = old_backdrop;

    internal->gesture_recognizer = NULL;
    assert(ui_bottom_sheet_base_process_event(sheet, &ev, 810.0) ==
           UI_ERROR_INVALID_ARGUMENT);
    internal->gesture_recognizer = old_gesture;
    internal->sheet_node = NULL;
    assert(ui_bottom_sheet_base_set_open(sheet, 1) == UI_ERROR_NONE);
    internal->sheet_node = old_sheet_node;
  }

  g_close_should_fail = 0;

  ui_bottom_sheet_base_destroy(sheet);
  ui_overlay_director_destroy(director);
  ui_dom_node_destroy(root);
  ui_signal_destroy(signal);
  ui_arena_destroy(arena);
  return 0;
}

static int run_oom_tests(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_bottom_sheet_base *sheet = NULL;
  enum ui_error rc;
  int i;

  printf("Testing OOM...\n");

  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_bottom_sheet_base_create(&sheet);
    if (rc == UI_ERROR_OUT_OF_MEMORY) {
      continue;
    } else if (rc == UI_ERROR_NONE) {
      ui_bottom_sheet_base_destroy(sheet);
      break;
    } else {
      printf("OOM failed at index %d with code %d\n", i, rc);
      g_malloc_fail_countdown = -1;
      return 1;
    }
  }
  g_malloc_fail_countdown = -1;
#endif
  return 0;
}

static void test_branch_coverage_destroy(void) {
  struct ui_bottom_sheet_base *sheet = NULL;

  /* Test destroy when closed */
  assert(ui_bottom_sheet_base_create(&sheet) == UI_ERROR_NONE);
  ui_bottom_sheet_base_destroy(sheet);

  /* Test destroy when open but no director */
  assert(ui_bottom_sheet_base_create(&sheet) == UI_ERROR_NONE);
  assert(ui_bottom_sheet_base_set_open(sheet, 1) == UI_ERROR_NONE);
  ui_bottom_sheet_base_destroy(sheet);

  /* Test set_open edge cases */
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root = NULL;
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_overlay_director_create(root, &director);

  /* Test open without director, then close without director (covers else branch
   * without director) */
  assert(ui_bottom_sheet_base_create(&sheet) == UI_ERROR_NONE);
  assert(ui_bottom_sheet_base_set_open(sheet, 1) == UI_ERROR_NONE);
  assert(ui_bottom_sheet_base_set_open(sheet, 0) == UI_ERROR_NONE);

  /* Now test with director but no overlay, then close */
  assert(ui_bottom_sheet_base_set_overlay_director(sheet, director) ==
         UI_ERROR_NONE);
  assert(ui_bottom_sheet_base_set_open(sheet, 1) == UI_ERROR_NONE);
  /* Force overlay to NULL to test that branch */
  ui_overlay_director_unmount(
      director, ((struct ui_bottom_sheet_base_internal *)sheet)->overlay);
  ((struct ui_bottom_sheet_base_internal *)sheet)->overlay = NULL;
  assert(ui_bottom_sheet_base_set_open(sheet, 0) == UI_ERROR_NONE);

  /* Test destroy when open, with director, but overlay is NULL */
  assert(ui_bottom_sheet_base_set_open(sheet, 1) == UI_ERROR_NONE);
  ui_overlay_director_unmount(
      director, ((struct ui_bottom_sheet_base_internal *)sheet)->overlay);
  ((struct ui_bottom_sheet_base_internal *)sheet)->overlay = NULL;
  ui_bottom_sheet_base_destroy(sheet);

  assert(ui_bottom_sheet_base_create(&sheet) == UI_ERROR_NONE);
  /* Set director but mock mount error inside set_open if possible,
     or just test open and close repeatedly */
  assert(ui_bottom_sheet_base_set_overlay_director(sheet, director) ==
         UI_ERROR_NONE);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  assert(ui_bottom_sheet_base_set_open(sheet, 1) == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
#endif

  ui_bottom_sheet_base_destroy(sheet);
  ui_overlay_director_destroy(director);
  ui_dom_node_destroy(root);
}

int main(void) {
  if (run_normal_tests() != 0) {
    printf("Normal tests failed.\n");
    return 1;
  }

  test_branch_coverage_destroy();

  if (run_oom_tests() != 0) {
    printf("OOM tests failed.\n");
    return 1;
  }

  printf("All ui_bottom_sheet_base tests passed.\n");
  return 0;
}
