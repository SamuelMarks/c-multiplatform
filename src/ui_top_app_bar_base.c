/**
 * @file ui_top_app_bar_base.c
 * @brief Implementation of the top app bar base component.
 */
/* clang-format off */
#include "ui_top_app_bar_base.h"
#include "ui_arena.h"
#include "ui_signal.h"
#include <stddef.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_top_app_bar_mock_fail = 0;
static int g_bar_get_counter = 0;
static int g_bar_set_counter = 0;

static ui_error_t mock_bar_signal_create(
    struct ui_arena *arena, union ui_signal_payload initial_payload,
    enum ui_signal_type type, ui_equality_fn equality, ui_destructor_fn cleanup,
    enum ui_signal_mode mode, ui_signal_t **out_signal) {
  if (g_top_app_bar_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_top_app_bar_mock_fail == 2 && type == UI_SIGNAL_TYPE_FLOAT32) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_signal_create)(arena, initial_payload, type, equality, cleanup,
                            mode, out_signal);
}
#undef ui_signal_create
/** @cond */
#define ui_signal_create mock_bar_signal_create
/** @endcond */

static ui_error_t mock_bar_signal_get(ui_signal_t *signal,
                                      union ui_signal_payload *out_payload) {
  if (g_top_app_bar_mock_fail == 3) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_top_app_bar_mock_fail == 4) {
    g_bar_get_counter++;
    if (g_bar_get_counter == 2) {
      return UI_ERROR_UNKNOWN;
    }
  }
  return (ui_signal_get)(signal, out_payload);
}
#undef ui_signal_get
/** @cond */
#define ui_signal_get mock_bar_signal_get
/** @endcond */

static ui_error_t mock_bar_signal_set(ui_signal_t *signal,
                                      union ui_signal_payload payload) {
  if (g_top_app_bar_mock_fail == 5) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_top_app_bar_mock_fail == 6) {
    g_bar_set_counter++;
    if (g_bar_set_counter == 2) {
      return UI_ERROR_UNKNOWN;
    }
  }
  return (ui_signal_set)(signal, payload);
}
#undef ui_signal_set
/** @cond */
#define ui_signal_set mock_bar_signal_set
/** @endcond */
#endif

/**
 * @struct ui_top_app_bar_base
 * @struct ui_top_app_bar_base
 * @brief Internal implementation of the top app bar base component.
 */
struct ui_top_app_bar_base {
  /* @brief Memory arena for allocations. */
  struct ui_arena *arena; /**< arena */
  /* @brief The top app bar configuration. */
  struct ui_top_app_bar_config config; /**< config */
  /* @brief Signal for the current state. */
  ui_signal_t *state_signal; /**< state_signal */
  /* @brief Signal for the current height. */
  ui_signal_t *height_signal; /**< height_signal */
};

/**
 * @brief Equality function for state signals.
 * @param a The first payload.
 * @param b The second payload.
 * @param out_equal Pointer to store the equality result.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t state_equality(union ui_signal_payload a,
                                 union ui_signal_payload b,
                                 ui_bool_t *out_equal) {
  *out_equal = (a.int_val == b.int_val) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

/**
 * @brief Equality function for height signals.
 * @param a The first payload.
 * @param b The second payload.
 * @param out_equal Pointer to store the equality result.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t height_equality(union ui_signal_payload a,
                                  union ui_signal_payload b,
                                  ui_bool_t *out_equal) {
  *out_equal = (a.float_val == b.float_val) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

ui_error_t
ui_top_app_bar_base_create(struct ui_arena *arena,
                           const struct ui_top_app_bar_config *config,
                           struct ui_top_app_bar_base **out_bar) {
  ui_error_t err;
  void *ptr;
  union ui_signal_payload initial_state;
  union ui_signal_payload initial_height;

  if (!arena || !config || !out_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_arena_alloc(arena, sizeof(struct ui_top_app_bar_base), 8, &ptr);
  if (err != UI_ERROR_NONE)
    return err;

  *out_bar = (struct ui_top_app_bar_base *)ptr;
  (*out_bar)->arena = arena;
  (*out_bar)->config = *config;

  initial_state.int_val = (ui_int32)config->initial_state;
  err = ui_signal_create(arena, initial_state, UI_SIGNAL_TYPE_INT32,
                         state_equality, NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                         &(*out_bar)->state_signal);
  if (err != UI_ERROR_NONE)
    return err;

  initial_height.float_val =
      (config->initial_state == UI_TOP_APP_BAR_STATE_EXPANDED)
          ? config->expanded_height
          : config->collapsed_height;
  err = ui_signal_create(arena, initial_height, UI_SIGNAL_TYPE_FLOAT32,
                         height_equality, NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                         &(*out_bar)->height_signal);
  if (err != UI_ERROR_NONE) {
    ui_signal_destroy((*out_bar)->state_signal);
    return err;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_top_app_bar_base_destroy(struct ui_top_app_bar_base *bar) {
  if (!bar)
    return UI_ERROR_INVALID_ARGUMENT;

  if (bar->state_signal) {
    ui_signal_destroy(bar->state_signal);
  }
  if (bar->height_signal) {
    ui_signal_destroy(bar->height_signal);
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_top_app_bar_base_handle_scroll(struct ui_top_app_bar_base *bar,
                                             float scroll_y, float delta_y) {
  union ui_signal_payload state_payload;
  union ui_signal_payload height_payload;
  float current_height;
  float new_height;
  enum ui_top_app_bar_state new_state;
  ui_error_t rc;

  if (scroll_y > 0.0f) {
  }

  if (!bar)
    return UI_ERROR_INVALID_ARGUMENT;

#ifdef UI_TEST_MOCK_ALLOC
  g_bar_get_counter = 0;
  g_bar_set_counter = 0;
#endif

  rc = ui_signal_get(bar->height_signal, &height_payload);
  if (rc != UI_ERROR_NONE)
    return rc;
  current_height = height_payload.float_val;

  rc = ui_signal_get(bar->state_signal, &state_payload);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* If scrolling down (delta_y > 0), hide bar. If scrolling up, show bar. */
  new_height = current_height - delta_y;

  if (new_height > bar->config.expanded_height) {
    new_height = bar->config.expanded_height;
  }
  if (new_height < bar->config.collapsed_height) {
    new_height = bar->config.collapsed_height;
  }

  /* Just a basic state machine for the CDK logic. More advanced logic depends
   * on the specific widget */
  if (new_height == bar->config.expanded_height) {
    new_state = UI_TOP_APP_BAR_STATE_EXPANDED;
  } else if (new_height == bar->config.collapsed_height) {
    new_state = UI_TOP_APP_BAR_STATE_COLLAPSED;
  } else {
    new_state = UI_TOP_APP_BAR_STATE_FLOATING;
  }

  if (new_height != current_height) {
    height_payload.float_val = new_height;
    rc = ui_signal_set(bar->height_signal, height_payload);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  if ((ui_int32)new_state != state_payload.int_val) {
    state_payload.int_val = (ui_int32)new_state;
    rc = ui_signal_set(bar->state_signal, state_payload);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_top_app_bar_base_get_state_signal(struct ui_top_app_bar_base *bar,
                                                ui_signal_t **out_signal) {
  if (!bar || !out_signal)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_signal = bar->state_signal;
  return UI_ERROR_NONE;
}

ui_error_t
ui_top_app_bar_base_get_height_signal(struct ui_top_app_bar_base *bar,
                                      ui_signal_t **out_signal) {
  if (!bar || !out_signal)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_signal = bar->height_signal;
  return UI_ERROR_NONE;
}
