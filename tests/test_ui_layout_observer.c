/* clang-format off */
#include "ui_layout_observer.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int callback_fire_count = 0;
static int last_fired_id = -1;
static int last_fired_state = -1;

static enum ui_error observer_callback(struct ui_layout_observer *observer,
                                       int breakpoint_id, int is_active,
                                       void *user_data) {
  (void)observer;

  callback_fire_count++;
  last_fired_id = breakpoint_id;
  last_fired_state = is_active;

  if (user_data) {
    int *val = (int *)user_data;
    (*val)++;
    return UI_ERROR_NONE;
  }
}

static int run_normal_tests(void) {
  struct ui_layout_observer *observer = NULL;
  enum ui_error rc;
  int bp_mobile, bp_desktop, bp_tall;
  int my_data = 0;

  printf("Testing invalid arguments...\n");
  if (ui_layout_observer_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_layout_observer_create(&observer);
  if (rc != UI_ERROR_NONE || !observer)
    return 1;

  if (ui_layout_observer_add_breakpoint(NULL, 0, 0, 0, 0, &bp_mobile) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_layout_observer_add_breakpoint(observer, 0, 0, 0, 0, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  printf("Adding breakpoints...\n");
  /* Mobile: width <= 767 */
  rc = ui_layout_observer_add_breakpoint(observer, -1, 767, -1, -1, &bp_mobile);
  if (rc != UI_ERROR_NONE || bp_mobile <= 0)
    return 1;

  /* Desktop: width >= 768 */
  rc =
      ui_layout_observer_add_breakpoint(observer, 768, -1, -1, -1, &bp_desktop);
  if (rc != UI_ERROR_NONE || bp_desktop <= 0)
    return 1;

  /* Tall: height >= 1000 */
  rc = ui_layout_observer_add_breakpoint(observer, -1, -1, 1000, -1, &bp_tall);
  if (rc != UI_ERROR_NONE || bp_tall <= 0)
    return 1;

  if (ui_layout_observer_subscribe(NULL, observer_callback, &my_data) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_layout_observer_subscribe(observer, NULL, &my_data) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_layout_observer_subscribe(observer, observer_callback, &my_data);
  if (rc != UI_ERROR_NONE)
    return 1;

  printf("Testing resize notifications...\n");
  if (ui_layout_observer_notify_resize(NULL, 800, 600) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_layout_observer_notify_resize(observer, -1, 600) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Initialize to a desktop size */
  callback_fire_count = 0;
  rc = ui_layout_observer_notify_resize(observer, 1024, 768);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* It should fire for desktop going to active, mobile staying inactive (no
   * fire), tall staying inactive */
  /* Actually since previous width/height was -1, they evaluate from 0 ->
   * active state where applicable */
  {
    int is_active = 0;
    ui_layout_observer_is_active(observer, bp_desktop, &is_active);
    if (is_active != 1)
      return 1;
  }
  {
    int is_active = 0;
    ui_layout_observer_is_active(observer, bp_mobile, &is_active);
    if (is_active != 0)
      return 1;
  }
  {
    int is_active = 0;
    ui_layout_observer_is_active(observer, bp_tall, &is_active);
    if (is_active != 0)
      return 1;
  }
  if (callback_fire_count == 0)
    return 1;

  /* Resize without changing breakpoint states should not fire callbacks */
  callback_fire_count = 0;
  ui_layout_observer_notify_resize(observer, 1200, 800);
  if (callback_fire_count != 0)
    return 1;

  /* Resize to mobile */
  callback_fire_count = 0;
  ui_layout_observer_notify_resize(observer, 400, 800);
  if (callback_fire_count != 2)
    return 1; /* Desktop off, Mobile on */
  {
    int is_active = 0;
    ui_layout_observer_is_active(observer, bp_desktop, &is_active);
    if (is_active != 0)
      return 1;
  }
  {
    int is_active = 0;
    ui_layout_observer_is_active(observer, bp_mobile, &is_active);
    if (is_active != 1)
      return 1;
  }

  /* Resize to tall */
  callback_fire_count = 0;
  ui_layout_observer_notify_resize(observer, 400, 1200);
  if (callback_fire_count != 1)
    return 1; /* Tall on */
  {
    int is_active = 0;
    ui_layout_observer_is_active(observer, bp_tall, &is_active);
    if (is_active != 1)
      return 1;
  }

  printf("Testing cleanup...\n");
  ui_layout_observer_destroy(NULL);
  ui_layout_observer_destroy(observer);

  return 0;
}

static int run_error_paths(void) {
  struct ui_layout_observer *observer = NULL;
  int bp_id;
  int i;
  enum ui_error rc;

  /* Creation OOM */
  g_malloc_fail_countdown = 0;
  rc = ui_layout_observer_create(&observer);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  ui_layout_observer_create(&observer);

  /* Trigger capacity expansion logic */
  ui_layout_observer_add_breakpoint(observer, -1, 100, -1, -1, &bp_id);
  ui_layout_observer_add_breakpoint(observer, -1, 200, -1, -1, &bp_id);
  ui_layout_observer_add_breakpoint(observer, -1, 300, -1, -1, &bp_id);
  ui_layout_observer_add_breakpoint(observer, -1, 400, -1, -1, &bp_id);

  g_malloc_fail_countdown = 0;
  rc = ui_layout_observer_add_breakpoint(observer, -1, 500, -1, -1,
                                         &bp_id); /* Exceeds initial 4 */
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;
  ui_layout_observer_add_breakpoint(observer, -1, 500, -1, -1, &bp_id);

  ui_layout_observer_subscribe(observer, observer_callback, NULL);
  ui_layout_observer_subscribe(observer, observer_callback, NULL);
  ui_layout_observer_subscribe(observer, observer_callback, NULL);
  ui_layout_observer_subscribe(observer, observer_callback, NULL);

  g_malloc_fail_countdown = 0;
  rc = ui_layout_observer_subscribe(observer, observer_callback,
                                    NULL); /* Exceeds initial 4 */
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;
  ui_layout_observer_subscribe(observer, observer_callback, NULL);

  {
    int active;
    ui_layout_observer_is_active(NULL, 1, &active);
    ui_layout_observer_is_active(observer, 1, NULL);
    ui_layout_observer_is_active(observer, -1, &active);
    ui_layout_observer_is_active(observer, 9999, &active);
  }

  ui_layout_observer_destroy(observer);

  /* Add breakpoint branch evaluation */
  {
    ui_layout_observer_create(&observer);
    ui_layout_observer_notify_resize(observer, 500, 500);

    /* Cover paths for min/max width/height */
    ui_layout_observer_add_breakpoint(observer, 600, -1, -1, -1,
                                      &bp_id); /* min_width failed */
    ui_layout_observer_add_breakpoint(observer, -1, 400, -1, -1,
                                      &bp_id); /* max_width failed */
    ui_layout_observer_add_breakpoint(observer, -1, -1, 600, -1,
                                      &bp_id); /* min_height failed */
    ui_layout_observer_add_breakpoint(observer, -1, -1, -1, 400,
                                      &bp_id); /* max_height failed */

    ui_layout_observer_notify_resize(observer, 700, 700);

    ui_layout_observer_destroy(observer);
  }

  {
    /* Cover identical notify resize */
    ui_layout_observer_create(&observer);
    ui_layout_observer_notify_resize(observer, 500, 500);
    ui_layout_observer_notify_resize(observer, 500, 500); /* Same */
    ui_layout_observer_destroy(observer);
  }

  return 0;
}

int main(void) {
  if (run_normal_tests() != 0) {
    return 1;
  }

  if (run_error_paths() != 0) {
    return 1;
  }

  printf("test_ui_layout_observer passed.\n");
  return 0;
}
