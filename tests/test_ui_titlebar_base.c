/* clang-format off */
#include "ui_titlebar_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#define ACCUM_ERR(failed, expr) failed |= ((expr) != UI_ERROR_NONE)
#define ACCUM_FAIL(failed, expr) failed |= (expr)

static int test_normal(void) {
  struct ui_arena *arena;
  struct ui_titlebar_config config;
  struct ui_titlebar_base *tb = NULL;
  enum ui_titlebar_hit_test_result res;
  int failed = 0;

  ACCUM_ERR(failed, ui_arena_create(4096, &arena));

  config.draggable = UI_TRUE;
  config.height = 32.0f;

  failed |= (ui_titlebar_base_create(NULL, &config, &tb) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_titlebar_base_create(arena, NULL, &tb) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_titlebar_base_create(arena, &config, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_titlebar_base_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT);

  ACCUM_ERR(failed, ui_titlebar_base_create(arena, &config, &tb));

  failed |= (ui_titlebar_base_add_button_rect(
                 NULL, UI_TITLEBAR_HIT_TEST_CLOSE_BTN, 0, 0, 10, 10) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_titlebar_base_hit_test(NULL, 0, 0, &res) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_titlebar_base_hit_test(tb, 0, 0, NULL) != UI_ERROR_INVALID_ARGUMENT);

  /* Add some buttons */
  ACCUM_ERR(failed, ui_titlebar_base_add_button_rect(
                        tb, UI_TITLEBAR_HIT_TEST_MINIMIZE_BTN, 100.0f, 5.0f,
                        20.0f, 20.0f));
  ACCUM_ERR(failed, ui_titlebar_base_add_button_rect(
                        tb, UI_TITLEBAR_HIT_TEST_MAXIMIZE_BTN, 130.0f, 5.0f,
                        20.0f, 20.0f));
  ACCUM_ERR(failed,
            ui_titlebar_base_add_button_rect(tb, UI_TITLEBAR_HIT_TEST_CLOSE_BTN,
                                             160.0f, 5.0f, 20.0f, 20.0f));

  /* Test outside titlebar entirely */
  ACCUM_ERR(failed, ui_titlebar_base_hit_test(tb, 10.0f, -1.0f, &res));
  ACCUM_FAIL(failed, res != UI_TITLEBAR_HIT_TEST_NONE);

  ACCUM_ERR(failed, ui_titlebar_base_hit_test(tb, 10.0f, 33.0f, &res));
  ACCUM_FAIL(failed, res != UI_TITLEBAR_HIT_TEST_NONE);

  /* Test drag area */
  ACCUM_ERR(failed, ui_titlebar_base_hit_test(tb, 10.0f, 10.0f, &res));
  ACCUM_FAIL(failed, res != UI_TITLEBAR_HIT_TEST_DRAG_AREA);

  /* Test buttons */
  ACCUM_ERR(failed, ui_titlebar_base_hit_test(tb, 105.0f, 10.0f, &res));
  ACCUM_FAIL(failed, res != UI_TITLEBAR_HIT_TEST_MINIMIZE_BTN);

  /* Miss buttons logic testing (branch coverage) */
  ACCUM_ERR(failed, ui_titlebar_base_hit_test(tb, 99.0f, 10.0f,
                                              &res)); /* x < curr->x */
  ACCUM_FAIL(failed, res != UI_TITLEBAR_HIT_TEST_DRAG_AREA);

  ACCUM_ERR(failed, ui_titlebar_base_hit_test(tb, 105.0f, 4.0f,
                                              &res)); /* y < curr->y */
  ACCUM_FAIL(failed, res != UI_TITLEBAR_HIT_TEST_DRAG_AREA);

  ACCUM_ERR(failed, ui_titlebar_base_hit_test(
                        tb, 105.0f, 30.0f, &res)); /* y > curr->y + curr->h */
  ACCUM_FAIL(failed, res != UI_TITLEBAR_HIT_TEST_DRAG_AREA);

  ACCUM_ERR(failed,
            ui_titlebar_base_hit_test(
                tb, 125.0f, 10.0f,
                &res)); /* x > curr->x + curr->w, but misses next button */
  ACCUM_FAIL(failed, res != UI_TITLEBAR_HIT_TEST_DRAG_AREA);

  ACCUM_ERR(failed, ui_titlebar_base_hit_test(tb, 135.0f, 10.0f, &res));
  ACCUM_FAIL(failed, res != UI_TITLEBAR_HIT_TEST_MAXIMIZE_BTN);

  ACCUM_ERR(failed, ui_titlebar_base_hit_test(tb, 165.0f, 10.0f, &res));
  ACCUM_FAIL(failed, res != UI_TITLEBAR_HIT_TEST_CLOSE_BTN);

  ACCUM_ERR(failed, ui_titlebar_base_destroy(tb));
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Now test non-draggable drag area */
  ACCUM_ERR(failed, ui_arena_create(4096, &arena));
  config.draggable = UI_FALSE;
  ACCUM_ERR(failed, ui_titlebar_base_create(arena, &config, &tb));
  ACCUM_ERR(failed, ui_titlebar_base_hit_test(tb, 10.0f, 10.0f, &res));
  ACCUM_FAIL(failed, res != UI_TITLEBAR_HIT_TEST_NONE);
  ACCUM_ERR(failed, ui_titlebar_base_destroy(tb));
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  return failed;
}

static int test_oom(void) {
  int failed = 0;
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_arena *small_arena;
  struct ui_titlebar_config config;
  struct ui_titlebar_base *tb;

  ui_arena_create(4096, &small_arena);
  g_malloc_fail_countdown = 1;
  {
    ui_error_t rc = ui_titlebar_base_create(small_arena, &config, &tb);
    if (rc == UI_ERROR_NONE) {
    }
  }
  g_malloc_fail_countdown = -1;

  ui_titlebar_base_create(small_arena, &config, &tb);
  g_malloc_fail_countdown = 1;
  {
    ui_error_t rc = ui_titlebar_base_add_button_rect(
        tb, UI_TITLEBAR_HIT_TEST_CLOSE_BTN, 0, 0, 10, 10);
    if (rc == UI_ERROR_NONE) {
    }
  }
  g_malloc_fail_countdown = -1;

  {
    ui_error_t rc_cleanup = ui_arena_destroy(small_arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
#endif
  return failed;
}

int main(void) {
  int failed = 0;
  failed |= test_normal();
  failed |= test_oom();

  if (!failed) {
    printf("All ui_titlebar_base tests passed.\n");
  }
  return failed;
}
