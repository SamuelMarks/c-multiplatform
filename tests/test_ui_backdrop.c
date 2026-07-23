/* clang-format off */
#include "ui_backdrop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int s_tests_passed = 0;
static int s_tests_failed = 0;

#define ASSERT_TRUE(cond)                                                      \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond);         \
      s_tests_failed++;                                                        \
    } else {                                                                   \
      s_tests_passed++;                                                        \
    }                                                                          \
  } while (0)

#define ASSERT_EQ(expected, actual)                                            \
  do {                                                                         \
    if ((expected) != (actual)) {                                              \
      fprintf(stderr, "FAIL: %s:%d: expected %d, got %d\n", __FILE__,          \
              __LINE__, (int)(expected), (int)(actual));                       \
      s_tests_failed++;                                                        \
    } else {                                                                   \
      s_tests_passed++;                                                        \
    }                                                                          \
  } while (0)

static enum ui_error test_backdrop_escape_key(void) {
  struct ui_backdrop *bd = NULL;
  struct ui_event ev;
  int should_dismiss = 0;
  enum ui_error err;

  err = ui_backdrop_create(&bd);
  ASSERT_EQ(UI_ERROR_NONE, err);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;

  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(UI_ERROR_NONE, err);
  ASSERT_EQ(1, should_dismiss);

  /* Test another key (Space) */
  ev.event_data.keyboard.key_code = UI_KEY_SPACE;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(UI_ERROR_NONE, err);
  ASSERT_EQ(0, should_dismiss);

  ui_backdrop_destroy(bd);
  return UI_ERROR_NONE;
}

static enum ui_error test_backdrop_click_outside(void) {
  struct ui_backdrop *bd = NULL;
  struct ui_event ev;
  int should_dismiss = 0;
  enum ui_error err;

  err = ui_backdrop_create(&bd);
  ASSERT_EQ(UI_ERROR_NONE, err);

  /* Mouse Down Inside -> No Dismiss */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 150;
  ev.event_data.mouse.y = 150;

  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(0, should_dismiss);

  /* Mouse Up Inside -> No Dismiss */
  ev.type = UI_EVENT_MOUSE_UP;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(0, should_dismiss);

  /* Mouse Down Outside -> Mouse Up Outside -> Dismiss */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 50; /* outside left */
  ev.event_data.mouse.y = 50; /* outside top */
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(0, should_dismiss); /* Doesn't dismiss on down, waits for up */

  ev.type = UI_EVENT_MOUSE_UP;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(1, should_dismiss);

  /* Mouse Down Outside Right -> Mouse Up Outside Right -> Dismiss */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 350; /* outside right */
  ev.event_data.mouse.y = 150;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(0, should_dismiss);

  ev.type = UI_EVENT_MOUSE_UP;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(1, should_dismiss);

  /* Mouse Down Outside Top -> Mouse Up Outside Top -> Dismiss */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 150;
  ev.event_data.mouse.y = 50; /* outside top */
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(0, should_dismiss);

  ev.type = UI_EVENT_MOUSE_UP;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(1, should_dismiss);

  /* Mouse Down Outside Bottom -> Mouse Up Outside Bottom -> Dismiss */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 150;
  ev.event_data.mouse.y = 350; /* outside bottom */
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(0, should_dismiss);

  ev.type = UI_EVENT_MOUSE_UP;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(1, should_dismiss);

  /* False Dismissal Check: Down Inside, Up Outside (e.g. drag selection) */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 150; /* inside */
  ev.event_data.mouse.y = 150;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(0, should_dismiss);

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.x = 50; /* outside */
  ev.event_data.mouse.y = 50;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(0, should_dismiss); /* Should NOT dismiss */

  /* False Dismissal Check: Down Outside, Up Inside */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 50; /* outside */
  ev.event_data.mouse.y = 50;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(0, should_dismiss);

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.x = 150; /* inside */
  ev.event_data.mouse.y = 150;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(0, should_dismiss); /* Should NOT dismiss */

  ui_backdrop_destroy(bd);
  return UI_ERROR_NONE;
}

static enum ui_error test_backdrop_other_events(void) {
  struct ui_backdrop *bd = NULL;
  struct ui_event ev;
  int should_dismiss = 0;
  enum ui_error err;

  err = ui_backdrop_create(&bd);
  ASSERT_EQ(UI_ERROR_NONE, err);

  /* Inactive backdrop shouldn't dismiss */
  err = ui_backdrop_set_active(bd, 0);
  ASSERT_EQ(UI_ERROR_NONE, err);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(UI_ERROR_NONE, err);
  ASSERT_EQ(0, should_dismiss);

  err = ui_backdrop_set_active(bd, 1);
  ASSERT_EQ(UI_ERROR_NONE, err);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_backdrop_set_active(NULL, 1));

  /* Touch Outside */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 50;
  ev.event_data.touch.points[0].y = 50;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(UI_ERROR_NONE, err);

  ev.type = UI_EVENT_TOUCH_END;
  ev.event_data.touch.points[0].x = 50;
  ev.event_data.touch.points[0].y = 50;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(1, should_dismiss);

  /* Touch without points (relying on pointer_down_was_outside) */
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 50;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);

  ev.type = UI_EVENT_TOUCH_END;
  ev.event_data.touch.num_points = 0;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(1, should_dismiss);

  /* Pen Outside */
  ev.type = UI_EVENT_PEN_DOWN;
  ev.event_data.pen.x = 50;
  ev.event_data.pen.y = 50;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(UI_ERROR_NONE, err);

  ev.type = UI_EVENT_PEN_UP;
  ev.event_data.pen.x = 50;
  ev.event_data.pen.y = 50;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(1, should_dismiss);

  /* Right click doesn't trigger dismissal logic in mouse */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 1; /* Right click */
  ev.event_data.mouse.x = 50;
  ev.event_data.mouse.y = 50;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(0, should_dismiss);

  ev.type = UI_EVENT_MOUSE_UP;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(0, should_dismiss);

  /* Unhandled event */
  ev.type = UI_EVENT_MOUSE_MOVE;
  err = ui_backdrop_process_event(bd, &ev, 100.0f, 100.0f, 200.0f, 200.0f,
                                  &should_dismiss);
  ASSERT_EQ(0, should_dismiss);

  ui_backdrop_destroy(bd);
  return UI_ERROR_NONE;
}

static enum ui_error test_invalid_args_and_oom(void) {
  struct ui_backdrop *bd = NULL;
  struct ui_event ev;
  int should_dismiss = 0;

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_backdrop_create(NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_backdrop_destroy(NULL));

  ui_backdrop_create(&bd);

  ASSERT_EQ(
      UI_ERROR_INVALID_ARGUMENT,
      ui_backdrop_process_event(NULL, &ev, 0, 0, 100, 100, &should_dismiss));
  ASSERT_EQ(
      UI_ERROR_INVALID_ARGUMENT,
      ui_backdrop_process_event(bd, NULL, 0, 0, 100, 100, &should_dismiss));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_backdrop_process_event(bd, &ev, 0, 0, 100, 100, NULL));

  ui_backdrop_destroy(bd);
  bd = NULL;

  g_malloc_fail_countdown = 0;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, ui_backdrop_create(&bd));
  g_malloc_fail_countdown = -1;
  return UI_ERROR_NONE;
}

int main(void) {
  test_backdrop_escape_key();
  test_backdrop_click_outside();
  test_backdrop_other_events();
  test_invalid_args_and_oom();

  printf("Tests passed: %d\n", s_tests_passed);
  printf("Tests failed: %d\n", s_tests_failed);

  if (s_tests_failed > 0) {
    return 1;
  }
  return 0;
}
