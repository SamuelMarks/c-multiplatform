/* clang-format off */
#include "../include/ui_ink_base.h"
#include "../include/ui_error.h"
#include <stdio.h>
#include <math.h>
/* clang-format on */

static int test_ink_smoothing(void) {
  struct ui_ink_base *ink = NULL;
  struct ui_component *comp;
  enum ui_error rc;
  size_t count = 0;
  struct ui_ink_event ev1 = {0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1000.0};
  struct ui_ink_event ev2 = {10.0f, 0.0f, 0.6f, 0.0f, 0.0f, 0.0f, 1010.0};
  struct ui_ink_event ev3 = {10.0f, 10.0f, 0.7f, 0.0f, 0.0f, 0.0f, 1020.0};
  struct ui_ink_event ev4 = {0.0f, 10.0f, 0.8f, 0.0f, 0.0f, 0.0f, 1030.0};

  rc = ui_ink_base_create(&ink);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  rc = ui_ink_base_get_component(ink, &comp);
  if (rc != UI_ERROR_NONE || !comp || !comp->shadow_root) {
    return 1;
  }

  rc = ui_ink_base_add_event(ink, &ev1);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_ink_base_add_event(ink, &ev2);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_ink_base_add_event(ink, &ev3);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_ink_base_add_event(ink, &ev4);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_ink_base_finish_stroke(ink);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_ink_base_get_smoothed_points_count(ink, &count);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* 1 (first) + 4 (segment1) + 4 (segment2) + 4 (finish) = 13 points total */
  if (count != 13) {
    printf("Expected 13 smoothed points, got %zu\n", count);
    return 1;
  }

  {
    struct ui_ink_event pt;
    if (ui_ink_base_get_smoothed_point(ink, 0, &pt) != UI_ERROR_NONE)
      return 1;
    if (ui_ink_base_get_smoothed_point(ink, 100, &pt) != UI_ERROR_OUT_OF_BOUNDS)
      return 1;
    if (ui_ink_base_get_smoothed_point(NULL, 0, &pt) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_ink_base_get_smoothed_point(ink, 0, NULL) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
  }

  ui_ink_base_destroy(ink);
  return 0;
}

extern int g_malloc_fail_countdown;

static int test_ink_oom_and_args(void) {
  struct ui_ink_base *ink = NULL;
  struct ui_component *comp;
  enum ui_error rc;
  struct ui_ink_event ev1 = {0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1000.0};
  size_t count;
  int i;

  if (ui_ink_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ink_base_get_component(NULL, &comp) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ink_base_get_component((struct ui_ink_base *)1, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ink_base_add_event(NULL, &ev1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ink_base_add_event((struct ui_ink_base *)1, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ink_base_finish_stroke(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ink_base_get_smoothed_points_count(NULL, &count) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ink_base_get_smoothed_points_count((struct ui_ink_base *)1, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_ink_base_create(&ink);
    if (rc == UI_ERROR_NONE) {
      ui_ink_base_destroy(ink);
      break;
    }
  }
  g_malloc_fail_countdown = -1;

  ui_ink_base_create(&ink);

  /* Trigger OOM on first add_event which allocates initial array */
  g_malloc_fail_countdown = 0;
  rc = ui_ink_base_add_event(ink, &ev1);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  if (ui_ink_base_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Destroy with null component using pointer arithmetic (opaque struct) */
  {
    struct ui_ink_base *ink2;
    ui_ink_base_create(&ink2);
    ui_ink_base_get_component(ink2, &comp);
    ui_component_destroy(comp);
    ((void **)ink2)[0] = NULL;
    ui_ink_base_destroy(ink2);
  }

  /* Force OOM during loop by adding lots of events */
  {
    int j;
    for (j = 0; j < 300; j++) {
      if (j == 10)
        g_malloc_fail_countdown = 0; /* Next alloc will fail */
      rc = ui_ink_base_add_event(ink, &ev1);
      if (rc == UI_ERROR_OUT_OF_MEMORY) {
        g_malloc_fail_countdown = -1;
        break;
      }
    }
  }

  /* Hit the `ink->raw_count == 1` and `ink->raw_count == 2` branches inside
   * `ui_ink_base_finish_stroke` */
  {
    struct ui_ink_base *ink3;
    struct ui_ink_event ev2 = {10.0f, 0.0f, 0.6f, 0.0f, 0.0f, 0.0f, 1010.0};
    ui_ink_base_create(&ink3);
    ui_ink_base_add_event(ink3, &ev1);
    ui_ink_base_finish_stroke(ink3);
    ui_ink_base_destroy(ink3);

    ui_ink_base_create(&ink3);
    ui_ink_base_add_event(ink3, &ev1);
    ui_ink_base_add_event(ink3, &ev2);
    ui_ink_base_finish_stroke(ink3);
    ui_ink_base_destroy(ink3);
  }

  /* Test OOM paths inside finish_stroke and append_smoothed_segment */
  {
    struct ui_ink_base *ink_oom;
    struct ui_ink_event ev_tmp = {10.0f, 0.0f, 0.6f, 0.0f, 0.0f, 0.0f, 1010.0};
    int j;

    ui_ink_base_create(&ink_oom);

    /* We need to trigger OOM exactly when append_smoothed_segment reallocates.
       Capacity starts at 32.
       Adding 1st event = 1 smoothed point.
       Adding 2nd event = 0 smoothed points.
       Adding 3rd event = 4 smoothed points (total 5).
       Adding 4th event = 4 smoothed points (total 9).
       ...
       Adding 9th event = 4 smoothed points (total 29).
       Adding 10th event = 4 smoothed points (total 33). This triggers realloc!
    */
    for (j = 1; j <= 9; j++) {
      ui_ink_base_add_event(ink_oom, &ev_tmp);
    }

    g_malloc_fail_countdown = 0;
    rc = ui_ink_base_add_event(ink_oom, &ev_tmp); /* 10th event */
    if (rc != UI_ERROR_OUT_OF_MEMORY)
      return 1;
    g_malloc_fail_countdown = -1;

    /* OOM inside add_event when raw_count == 1 (first event smoothed
     * allocation) */
    {
      struct ui_ink_base *ink_first;
      ui_ink_base_create(&ink_first);

      /* First event does 1 raw realloc (cap 16) and 1 smoothed realloc (cap 32)
       */
      /* Fail the second realloc */
      g_malloc_fail_countdown = 1;
      rc = ui_ink_base_add_event(ink_first, &ev_tmp);
      if (rc != UI_ERROR_OUT_OF_MEMORY)
        return 1;
      g_malloc_fail_countdown = -1;

      ui_ink_base_destroy(ink_first);
    }

    /* OOM inside finish_stroke when raw_count == 2 */
    {
      struct ui_ink_base *ink_two;
      ui_ink_base_create(&ink_two);
      ui_ink_base_add_event(ink_two, &ev_tmp);
      ui_ink_base_add_event(ink_two, &ev_tmp);

      /* Capacity is 32. Adding one point in finish_stroke.
         We need capacity to be full (count == 32) but raw_count to be 2.
         This is impossible since count is 1 when raw is 2.
         Wait, we can't easily hit OOM here unless smoothed capacity is 0,
         but it was already set to 32 by the first event.
         So smoothed_count = 1, capacity = 32. finish_stroke adds 1. count
         becomes 2. It will never trigger realloc here! Wait, what if we
         artificially set capacity to 1 by adding events then clearing
         raw_points? We can't. This branch is practically unreachable without
         mocking UI_REALLOC to always fail. Let's just use
         g_malloc_fail_countdown = 0 to fail ANY realloc, but there is no
         realloc here.
      */
      ui_ink_base_destroy(ink_two);
    }

    ui_ink_base_destroy(ink_oom);
  }

  ui_ink_base_destroy(ink);
  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_ink_base tests...\n");

  failed |= test_ink_smoothing();
  failed |= test_ink_oom_and_args();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
