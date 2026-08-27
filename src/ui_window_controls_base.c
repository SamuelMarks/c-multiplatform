/**
 * @file ui_window_controls_base.c
 * @brief Implementation of the window controls base component.
 */

/* clang-format off */
#include "ui_window_controls_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @struct ui_region_rect
 * \brief ui_region_rect
 */
struct ui_region_rect {
  int x;                                    /**< x */
  int y;                                    /**< y */
  int width;                                /**< width */
  int height;                               /**< height */
  enum ui_window_control_hit_region region; /**< region */
};

/** @cond */
#define UI_MAX_WINDOW_REGIONS 16
/** @endcond */

/**
 * @struct ui_window_controls_base
 * @struct ui_window_controls_base
 * @brief Internal state for the window controls component.
 */
struct ui_window_controls_base {
  struct ui_arena *arena;     /**< arena */
  enum ui_window_state state; /**< state */
  ui_signal_t *state_signal;  /**< state_signal */

  struct ui_region_rect regions[UI_MAX_WINDOW_REGIONS]; /**< regions */
  int num_regions;                                      /**< num_regions */
};

static ui_error_t state_equality(union ui_signal_payload a,
                                 union ui_signal_payload b,
                                 ui_bool_t *out_equal) {
  *out_equal = (a.int_val == b.int_val) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

ui_error_t
ui_window_controls_base_create(struct ui_arena *arena,
                               struct ui_window_controls_base **out_controls) {
  ui_error_t err;
  void *ptr;
  union ui_signal_payload initial_payload;

  if (!arena || !out_controls) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_arena_alloc(arena, sizeof(struct ui_window_controls_base), 8, &ptr);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  *out_controls = (struct ui_window_controls_base *)ptr;
  (*out_controls)->arena = arena;
  (*out_controls)->state = UI_WINDOW_STATE_NORMAL;
  (*out_controls)->num_regions = 0;

  initial_payload.int_val = (ui_int32)UI_WINDOW_STATE_NORMAL;
  err = ui_signal_create(arena, initial_payload, UI_SIGNAL_TYPE_INT32,
                         state_equality, NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                         &(*out_controls)->state_signal);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  return UI_ERROR_NONE;
}

ui_error_t
ui_window_controls_base_destroy(struct ui_window_controls_base *controls) {
  if (!controls) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  (void)ui_signal_destroy(controls->state_signal);

  return UI_ERROR_NONE;
}

ui_error_t
ui_window_controls_base_set_state(struct ui_window_controls_base *controls,
                                  enum ui_window_state state) {
  ui_error_t err;
  union ui_signal_payload payload;

  if (!controls) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  controls->state = state;
  payload.int_val = (ui_int32)state;

  err = ui_signal_set(controls->state_signal, payload);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_window_controls_base_get_state_signal(
    struct ui_window_controls_base *controls, ui_signal_t **out_signal) {
  if (!controls || !out_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_signal = controls->state_signal;
  return UI_ERROR_NONE;
}

ui_error_t ui_window_controls_base_hit_test(
    struct ui_window_controls_base *controls, int x, int y,
    enum ui_window_control_hit_region *out_region) {
  int i;

  if (!controls || !out_region) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_region = UI_WINDOW_CONTROL_HIT_REGION_NONE;

  for (i = 0; i < controls->num_regions; ++i) {
    struct ui_region_rect *r = &controls->regions[i];
    if (x >= r->x && x < r->x + r->width && y >= r->y && y < r->y + r->height) {
      *out_region = r->region;
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_window_controls_base_set_region_rect(
    struct ui_window_controls_base *controls,
    enum ui_window_control_hit_region region, int x, int y, int width,
    int height) {
  int i;
  struct ui_region_rect *r;

  if (!controls) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Look for an existing entry for this region and update it */
  for (i = 0; i < controls->num_regions; ++i) {
    if (controls->regions[i].region == region) {
      controls->regions[i].x = x;
      controls->regions[i].y = y;
      controls->regions[i].width = width;
      controls->regions[i].height = height;
      return UI_ERROR_NONE;
    }
  }

  /* Add new region */
  if (controls->num_regions >= UI_MAX_WINDOW_REGIONS) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  r = &controls->regions[controls->num_regions++];
  r->region = region;
  r->x = x;
  r->y = y;
  r->width = width;
  r->height = height;

  return UI_ERROR_NONE;
}

ui_error_t ui_window_controls_base_trigger_hover_intent(
    struct ui_window_controls_base *controls,
    enum ui_window_control_hit_region region) {
  if (!controls) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* In a real implementation, this might emit another signal or interact
   * directly with OS APIs to show the Windows 11 Snap Layouts menu on hover.
   * For this CDK layer, we just validate inputs. */
  if (region == UI_WINDOW_CONTROL_HIT_REGION_NONE) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return UI_ERROR_NONE;
}
