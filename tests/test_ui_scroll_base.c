/* clang-format off */
#include "ui_scroll_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int g_change_count = 0;
static float g_last_x = 0.0f;
static float g_last_y = 0.0f;

static enum ui_error on_scroll_change(struct ui_scroll_base *scroll, float x,
                                      float y, void *user_data) {
  (void)scroll;
  (void)user_data;
  g_change_count++;
  g_last_x = x;
  g_last_y = y;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static int run_normal_tests(void) {
  struct ui_scroll_base *scroll = NULL;
  enum ui_error err;
  struct ui_event ev;

  printf("Testing invalid arguments...\n");
  if (ui_scroll_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_scroll_base_destroy(NULL); /* Should not crash */
  if (ui_scroll_base_set_scroll_pos(NULL, 0, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    float x = 0.0f;
    if (ui_scroll_base_get_scroll_x(NULL, &x) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
  }
  {
    float y = 0.0f;
    if (ui_scroll_base_get_scroll_y(NULL, &y) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
  }
  if (ui_scroll_base_set_content_size(NULL, 0, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_scroll_base_set_viewport_size(NULL, 0, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_scroll_base_set_on_change(NULL, on_scroll_change, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  memset(&ev, 0, sizeof(ev));
  if (ui_scroll_base_process_event(NULL, &ev, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_scroll_base_process_event(scroll, NULL, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    struct ui_component *tmp_comp;
    if (ui_scroll_base_get_component(NULL, &tmp_comp) == UI_ERROR_NONE)
      return 1;
  }

  err = ui_scroll_base_create(&scroll);
  if (err != UI_ERROR_NONE) {
    printf("Failed to create scroll base\n");
    return 1;
  }

  if (ui_scroll_base_process_event(scroll, NULL, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  {
    struct ui_component *tmp_comp;
    if (ui_scroll_base_get_component(scroll, &tmp_comp) != UI_ERROR_NONE ||
        tmp_comp == NULL) {
      printf("Failed to get component\n");
      return 1;
    }
  }

  g_change_count = 0;
  err = ui_scroll_base_set_on_change(scroll, on_scroll_change, NULL);
  if (err != UI_ERROR_NONE)
    return 1;

  /* Viewport 100x100, Content 200x300. Max scroll X = 100, Y = 200 */
  ui_scroll_base_set_viewport_size(scroll, 100.0f, 100.0f);
  ui_scroll_base_set_content_size(scroll, 200.0f, 300.0f);

  /* Try to scroll past limits */
  ui_scroll_base_set_scroll_pos(scroll, 500.0f, 500.0f);

  {
    float x = 0.0f, y = 0.0f;
    ui_scroll_base_get_scroll_x(scroll, &x);
    ui_scroll_base_get_scroll_y(scroll, &y);
    if (x != 100.0f || y != 200.0f) {
      printf("Scroll limits not enforced\n");
      return 1;
    }
  }

  /* Try to scroll negative */
  ui_scroll_base_set_scroll_pos(scroll, -50.0f, -50.0f);

  {
    float x = 0.0f, y = 0.0f;
    ui_scroll_base_get_scroll_x(scroll, &x);
    ui_scroll_base_get_scroll_y(scroll, &y);
    if (x != 0.0f || y != 0.0f) {
      printf("Negative scroll limits not enforced\n");
      return 1;
    }
  }

  /* Test mouse wheel */
  ev.type = UI_EVENT_MOUSE_WHEEL;
  ev.event_data.mouse.wheel_x = 0.0f;
  ev.event_data.mouse.wheel_y = 1.0f; /* Scroll down */

  ui_scroll_base_process_event(scroll, &ev, 0.0);

  {
    float y = 0.0f;
    ui_scroll_base_get_scroll_y(scroll, &y);
    if (y <= 0.0f) {
      printf("Mouse wheel did not scroll Y\n");
      return 1;
    }
  }

  /* Test viewport size negative limits handling */
  ui_scroll_base_set_content_size(scroll, 50.0f,
                                  50.0f); /* Content smaller than viewport */
  ui_scroll_base_set_scroll_pos(scroll, 10.0f, 10.0f);
  {
    float x = 0.0f, y = 0.0f;
    ui_scroll_base_get_scroll_x(scroll, &x);
    ui_scroll_base_get_scroll_y(scroll, &y);
    if (x != 0.0f || y != 0.0f) {
      printf("Content smaller than viewport allowed scroll\n");
      return 1;
    }
  }

  /* Process unhandled event */
  ev.type = UI_EVENT_MOUSE_DOWN;
  err = ui_scroll_base_process_event(scroll, &ev, 0.0);
  if (err != UI_ERROR_NONE)
    return 1;

  ui_scroll_base_destroy(scroll);
  return 0;
}

static int run_oom_tests(void) {
  struct ui_scroll_base *scroll = NULL;
  enum ui_error err;
  int i;

  printf("Running scroll base OOM tests...\n");

  /* Creation OOM */
  for (i = 0; i < 7; i++) {
    g_malloc_fail_countdown = i;
    err = ui_scroll_base_create(&scroll);
    g_malloc_fail_countdown = -1;
    if (err == UI_ERROR_OUT_OF_MEMORY) {
      continue;
    } else if (err == UI_ERROR_NONE) {
      ui_scroll_base_destroy(scroll);
      break;
    } else {
      return 1;
    }
  }

  return 0;
}

int main() {
  int failed = 0;
  failed |= run_normal_tests();
  failed |= run_oom_tests();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All ui_scroll_base tests passed.\n");
  return 0;
}
