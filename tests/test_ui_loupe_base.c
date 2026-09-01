#include <math.h>

#ifndef NAN
#define NAN (0.0f / 0.0f)
#endif

/* clang-format off */
#include "ui_loupe_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stdio.h>

extern int g_malloc_fail_countdown;
/* clang-format on */

int main(void) {
  struct ui_arena *arena;
  struct ui_loupe_base *loupe = NULL;
  struct ui_loupe_config config;
  ui_error_t err;
  ui_signal_t *signal = NULL;
  struct ui_dom_point focal_point;
  struct ui_dom_point result_point;

  if (ui_arena_create(1024 * 16, &arena) != UI_ERROR_NONE) {
    return 1;
  }

  config.magnification_level = 1.5f;
  config.loupe_width = 100.0f;
  config.loupe_height = 100.0f;
  config.y_offset = 20.0f;

  err = ui_loupe_base_create(arena, &config, &loupe);
  if (err != UI_ERROR_NONE || loupe == NULL) {
    return 1;
  }

  err = ui_loupe_base_get_overlay_origin_signal(loupe, &signal);
  if (err != UI_ERROR_NONE || signal == NULL) {
    return 1;
  }

  focal_point.x = 200.0;
  focal_point.y = 300.0;
  focal_point.z = 0.0;
  focal_point.w = 1.0;

  err = ui_loupe_base_set_focal_point(loupe, &focal_point);
  if (err != UI_ERROR_NONE) {
    return 1;
  }

  /* Trigger point equality check by setting same point */
  err = ui_loupe_base_set_focal_point(loupe, &focal_point);
  if (err != UI_ERROR_NONE) {
    return 1;
  }

  focal_point.x = 200.0;
  focal_point.y = 400.0; /* Same x, diff y */
  ui_loupe_base_set_focal_point(loupe, &focal_point);
  focal_point.x = 300.0; /* Diff x */
  ui_loupe_base_set_focal_point(loupe, &focal_point);

  /* NaN test to cover unordered float branch */
  focal_point.x = NAN;
  ui_loupe_base_set_focal_point(loupe, &focal_point);
  focal_point.x = 200.0f;
  focal_point.y = NAN;
  ui_loupe_base_set_focal_point(loupe, &focal_point);

  err = ui_loupe_base_get_focal_point(loupe, &result_point);
  if (err != UI_ERROR_NONE) {
    return 1;
  }

  err = ui_loupe_base_set_visible(loupe, UI_TRUE);
  if (err != UI_ERROR_NONE) {
    return 1;
  }

  err = ui_loupe_base_set_magnification_level(loupe, 2.0f);
  if (err != UI_ERROR_NONE) {
    return 1;
  }

  /* Test invalid zoom guard */
  err = ui_loupe_base_set_magnification_level(loupe, -1.0f);
  if (err != UI_ERROR_NONE) {
    return 1;
  }

  /* Test errors */
  ui_loupe_base_create(NULL, &config, &loupe);
  ui_loupe_base_create(arena, NULL, &loupe);
  ui_loupe_base_create(arena, &config, NULL);
  ui_loupe_base_destroy(NULL);
  ui_loupe_base_set_focal_point(NULL, &focal_point);
  ui_loupe_base_set_focal_point(loupe, NULL);
  ui_loupe_base_get_overlay_origin_signal(NULL, &signal);
  ui_loupe_base_get_overlay_origin_signal(loupe, NULL);
  ui_loupe_base_get_focal_point(NULL, &result_point);
  ui_loupe_base_get_focal_point(loupe, NULL);
  ui_loupe_base_set_visible(NULL, UI_TRUE);
  ui_loupe_base_set_magnification_level(NULL, 1.0f);

  /* Test point equality error cases (indirect via ui_signal_set but we can't
   * easily force NULL payloads if they are value structs, actually they are
   * ptr_val so we can test it directly if we had the function, but since it's
   * static we just test what we can) */

  /* Test OOM/alloc failure */
  {
    int i;
    for (i = 0; i < 4; i++) {
      struct ui_loupe_base *loupe_oom = NULL;
      struct ui_arena *small_arena;
      if (ui_arena_create(8, &small_arena) == UI_ERROR_NONE) {
        g_malloc_fail_countdown = i;
        ui_loupe_base_create(small_arena, &config, &loupe_oom);
        g_malloc_fail_countdown = -1;
        if (loupe_oom)
          ui_loupe_base_destroy(loupe_oom);
        {
          ui_error_t rc_cleanup = ui_arena_destroy(small_arena);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }
  }

  err = ui_loupe_base_destroy(loupe);
  if (err != UI_ERROR_NONE) {
    return 1;
  }

  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}
