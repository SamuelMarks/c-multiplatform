/* clang-format off */
#include "ui_syntax_surface_base.h"
#include "ui_error.h"
#include "ui_arena.h"
#include "ui_rich_text_base.h"
#include <stdio.h>
/* clang-format on */

#define ACCUM_ERR(failed, expr)                                                \
  do {                                                                         \
    ui_error_t err__ = (expr);                                                 \
    if (err__ != UI_ERROR_NONE) {                                              \
      printf("Failed %d at %d\n", err__, __LINE__);                            \
      failed = 1;                                                              \
    }                                                                          \
  } while (0)
#define ACCUM_FAIL(failed, expr) failed |= (expr)

int run_normal_tests(void) {
  struct ui_arena *arena;
  struct ui_syntax_surface_base *surface = NULL;
  float out_w;
  ui_signal_t *sig;
  int visual_index;
  struct ui_syntax_bracket_match match;
  int failed = 0;

  ACCUM_ERR(failed, ui_arena_create(64 * 1024, &arena));

  /* Null checks */
  failed |= (ui_syntax_surface_base_create(NULL, NULL, &surface) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_syntax_surface_base_create(arena, NULL, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_syntax_surface_base_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_syntax_surface_base_calculate_gutter_width(
                 NULL, 10, 8.0f, &out_w) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_syntax_surface_base_set_fold_region(NULL, 0, 10, UI_TRUE) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_syntax_surface_base_get_fold_changed_signal(NULL, &sig) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_syntax_surface_base_get_visual_line_index(
                 NULL, 10, &visual_index) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_syntax_surface_base_set_active_line(NULL, 5) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_syntax_surface_base_get_active_line_signal(NULL, &sig) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_syntax_surface_base_set_bracket_match(NULL, &match) !=
             UI_ERROR_INVALID_ARGUMENT);

  ACCUM_ERR(failed, ui_syntax_surface_base_create(arena, NULL, &surface));

  failed |= (ui_syntax_surface_base_calculate_gutter_width(
                 surface, 10, 8.0f, NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_syntax_surface_base_calculate_gutter_width(
                 surface, -1, 8.0f, &out_w) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_syntax_surface_base_calculate_gutter_width(
                 surface, 10, -1.0f, &out_w) != UI_ERROR_INVALID_ARGUMENT);

  ACCUM_ERR(failed, ui_syntax_surface_base_calculate_gutter_width(
                        surface, 9, 10.0f, &out_w));
  failed |= (out_w != 30.0f);

  ACCUM_ERR(failed, ui_syntax_surface_base_calculate_gutter_width(
                        surface, 99, 10.0f, &out_w));
  failed |= (out_w != 40.0f);

  failed |= (ui_syntax_surface_base_set_fold_region(surface, -1, 10, UI_TRUE) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_syntax_surface_base_set_fold_region(surface, 10, 5, UI_TRUE) !=
             UI_ERROR_INVALID_ARGUMENT);

  ACCUM_ERR(failed,
            ui_syntax_surface_base_set_fold_region(surface, 5, 10, UI_TRUE));
  ACCUM_ERR(failed,
            ui_syntax_surface_base_set_fold_region(surface, 15, 20, UI_TRUE));

  /* Update existing */
  ACCUM_ERR(failed,
            ui_syntax_surface_base_set_fold_region(surface, 5, 10, UI_FALSE));

  /* Test out of bounds on folds */
  int i;
  for (i = 0; i < 256; i++) {
    ui_error_t err = ui_syntax_surface_base_set_fold_region(
        surface, i * 100, (i * 100) + 1, UI_TRUE);
    if (i < 254) {
      failed |= (err != UI_ERROR_NONE);
    } else {
      failed |= (err != UI_ERROR_OUT_OF_BOUNDS);
    }
  }

  failed |= (ui_syntax_surface_base_get_fold_changed_signal(surface, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  ACCUM_ERR(failed,
            ui_syntax_surface_base_get_fold_changed_signal(surface, &sig));

  failed |= (ui_syntax_surface_base_get_visual_line_index(surface, 10, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_syntax_surface_base_get_visual_line_index(
                 surface, -1, &visual_index) != UI_ERROR_INVALID_ARGUMENT);

  /* Set up specific folds for visual line index test: Let's reset */
  {
    ui_error_t rc_cleanup = ui_syntax_surface_base_destroy(surface);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  ui_syntax_surface_base_create(arena, NULL, &surface);

  ui_syntax_surface_base_set_fold_region(surface, 10, 20, UI_TRUE);

  /* Before fold */
  ACCUM_ERR(failed, ui_syntax_surface_base_get_visual_line_index(
                        surface, 5, &visual_index));
  failed |= (visual_index != 5);

  /* On fold header */
  ACCUM_ERR(failed, ui_syntax_surface_base_get_visual_line_index(
                        surface, 10, &visual_index));
  failed |= (visual_index != 10);

  /* Inside fold */
  ACCUM_ERR(failed, ui_syntax_surface_base_get_visual_line_index(
                        surface, 15, &visual_index));
  failed |= (visual_index != -1);

  /* Open fold to test the is_collapsed false branch inside visual_line
   * computation */
  ACCUM_ERR(failed,
            ui_syntax_surface_base_set_fold_region(surface, 10, 20, UI_FALSE));
  ACCUM_ERR(failed, ui_syntax_surface_base_get_visual_line_index(
                        surface, 25, &visual_index));
  failed |= (visual_index != 25);

  /* After fold (re-collapsed) */
  ACCUM_ERR(failed,
            ui_syntax_surface_base_set_fold_region(surface, 10, 20, UI_TRUE));
  ACCUM_ERR(failed, ui_syntax_surface_base_get_visual_line_index(
                        surface, 25, &visual_index));
  failed |= (visual_index != 15);

  ACCUM_ERR(failed, ui_syntax_surface_base_set_active_line(surface, 10));

  failed |= (ui_syntax_surface_base_get_active_line_signal(surface, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  ACCUM_ERR(failed,
            ui_syntax_surface_base_get_active_line_signal(surface, &sig));

  failed |= (ui_syntax_surface_base_set_bracket_match(surface, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  match.has_match = UI_TRUE;
  match.open_line = 1;
  match.open_col = 2;
  match.close_line = 5;
  match.close_col = 10;
  ACCUM_ERR(failed, ui_syntax_surface_base_set_bracket_match(surface, &match));

  ui_signal_t *f_sig, *a_sig;
  ui_syntax_surface_base_get_fold_changed_signal(surface, &f_sig);
  ui_syntax_surface_base_get_active_line_signal(surface, &a_sig);

  union ui_signal_payload p1, p2;
  p1.ptr_val = NULL;
  p2.ptr_val = NULL;
  ui_signal_set(f_sig, p1);

  p1.int_val = 10;
  p2.int_val = 10;
  ui_signal_set(a_sig, p1);
  ui_signal_set(a_sig, p1);

  ACCUM_ERR(failed, ui_syntax_surface_base_destroy(surface));
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Arena OOM tests */
  extern int g_malloc_fail_countdown;
  struct ui_arena *small_arena;
  struct ui_syntax_surface_base *s1;
  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    if (ui_arena_create(64, &small_arena) == UI_ERROR_NONE) {
      if (ui_syntax_surface_base_create(small_arena, NULL, &s1) ==
          UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_syntax_surface_base_destroy(s1);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
      {
        ui_error_t rc_cleanup = ui_arena_destroy(small_arena);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }
  }
  g_malloc_fail_countdown = -1;

  return failed;
}

int main(void) {
  int failed = 0;
  failed |= run_normal_tests();
  if (!failed) {
    printf("All ui_syntax_surface_base tests passed.\n");
  }
  return failed;
}
