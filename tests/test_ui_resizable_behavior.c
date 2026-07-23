/* clang-format off */
#include "../include/ui_resizable_behavior.h"
#include "../include/ui_error.h"
#include "../include/ui_event.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static enum ui_error mock_on_resize(int new_w, int new_h, void *user_data) {
  int *dims = (int *)user_data;
  dims[0] = new_w;
  dims[1] = new_h;
  return UI_ERROR_NONE;
}

static int test_resizable_lifecycle(void) {
  struct ui_resizable_behavior *b = NULL;
  enum ui_error rc;

  rc = ui_resizable_behavior_create(&b);
  if (rc != UI_ERROR_NONE || b == NULL)
    return 1;

  ui_resizable_behavior_destroy(b);
  return 0;
}

static int test_resizable_dragging(void) {
  struct ui_resizable_behavior *b = NULL;
  enum ui_error rc;
  struct ui_event ev;
  int dims[2] = {0, 0};

  rc = ui_resizable_behavior_create(&b);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_resizable_behavior_configure(b, UI_RESIZABLE_EDGE_ALL, 10, 10, 200,
                                       200);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_resizable_behavior_set_on_resize(b, mock_on_resize, dims);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Mouse down near right edge */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 98;
  ev.event_data.mouse.y = 50;
  rc = ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Simulate mouse down while already dragging */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);

  /* Mouse move right by 20px */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 118;
  ev.event_data.mouse.y = 50;
  rc = ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);
  if (rc != UI_ERROR_NONE)
    return 1;

  if (dims[0] != 120 || dims[1] != 100)
    return 1; /* 100 + 20 */

  /* Mouse up */
  ev.type = UI_EVENT_MOUSE_UP;
  rc = ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_resizable_behavior_destroy(b);
  return 0;
}

static int test_resizable_edges_and_bounds(void) {
  struct ui_resizable_behavior *b = NULL;
  struct ui_event ev;
  int dims[2] = {0, 0};

  ui_resizable_behavior_create(&b);
  ui_resizable_behavior_configure(b, UI_RESIZABLE_EDGE_ALL, 10, 10, 200, 200);
  ui_resizable_behavior_set_on_resize(b, mock_on_resize, dims);

  /* Left Edge */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 2;
  ev.event_data.mouse.y = 50;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = -20;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);
  if (dims[0] != 122)
    return 1; /* 100 - (-22) = 122 */

  ev.type = UI_EVENT_MOUSE_UP;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);

  /* Top Edge */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 50;
  ev.event_data.mouse.y = 2;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = -20;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);
  if (dims[1] != 122)
    return 1; /* 100 - (-22) = 122 */

  ev.type = UI_EVENT_MOUSE_UP;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);

  /* Bottom Edge */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 50;
  ev.event_data.mouse.y = 98;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 120;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);
  if (dims[1] != 122)
    return 1; /* 100 + 22 = 122 */

  ev.type = UI_EVENT_MOUSE_UP;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);

  /* Bounds */
  ui_resizable_behavior_configure(
      b, UI_RESIZABLE_EDGE_RIGHT | UI_RESIZABLE_EDGE_BOTTOM, 50, 50, 150, 150);

  /* Max width */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 98;
  ev.event_data.mouse.y = 50;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 200; /* +102 */
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);
  if (dims[0] != 150)
    return 1;
  ev.type = UI_EVENT_MOUSE_UP;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);

  /* Min width */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 98;
  ev.event_data.mouse.y = 50;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 0; /* -98 */
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);
  if (dims[0] != 50)
    return 1;
  ev.type = UI_EVENT_MOUSE_UP;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);

  /* Max height */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 50;
  ev.event_data.mouse.y = 98;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 200; /* +102 */
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);
  if (dims[1] != 150)
    return 1;
  ev.type = UI_EVENT_MOUSE_UP;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);

  /* Min height */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 50;
  ev.event_data.mouse.y = 98;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 0; /* -98 */
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);
  if (dims[1] != 50)
    return 1;
  ev.type = UI_EVENT_MOUSE_UP;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);

  /* Move without size change should not call on_resize */
  dims[0] = 0;
  dims[1] = 0;
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 98;
  ev.event_data.mouse.y = 50; /* wait, 98 is in right edge. 50,50 is middle */
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 98; /* same */
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);
  if (dims[0] != 0 || dims[1] != 0)
    return 1;
  ev.type = UI_EVENT_MOUSE_UP;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);

  /* Mouse down in center (no hit) */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 50;
  ev.event_data.mouse.y = 50;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);

  /* Missing callback */
  ui_resizable_behavior_set_on_resize(b, NULL, NULL);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 60;
  ui_resizable_behavior_process_event(b, &ev, 100, 100,
                                      5); /* shouldn't drag anyway */

  /* Re-configure with min=-1, max=-1 */
  ui_resizable_behavior_configure(b, UI_RESIZABLE_EDGE_ALL, -1, -1, -1, -1);
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 98;
  ev.event_data.mouse.y = 50;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 110;
  ui_resizable_behavior_process_event(b, &ev, 100, 100, 5);

  ui_resizable_behavior_destroy(b);
  return 0;
}

static int test_resizable_nulls_and_errors(void) {
  struct ui_resizable_behavior *b = NULL;
  struct ui_event ev;

  if (ui_resizable_behavior_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  g_malloc_fail_countdown = 0;
  if (ui_resizable_behavior_create(&b) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  ui_resizable_behavior_destroy(NULL);

  if (ui_resizable_behavior_configure(NULL, 0, 0, 0, 0, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_resizable_behavior_set_on_resize(NULL, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_resizable_behavior_process_event(NULL, &ev, 100, 100, 5) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_resizable_behavior_create(&b);

  if (ui_resizable_behavior_process_event(b, NULL, 100, 100, 5) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Unhandled event type */
  ev.type = UI_EVENT_KEY_DOWN;
  if (ui_resizable_behavior_process_event(b, &ev, 100, 100, 5) != UI_ERROR_NONE)
    return 1;

  ui_resizable_behavior_destroy(b);
  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_resizable_behavior tests...\n");

  failed |= test_resizable_lifecycle();
  failed |= test_resizable_dragging();
  failed |= test_resizable_edges_and_bounds();
  failed |= test_resizable_nulls_and_errors();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
