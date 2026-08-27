/**
 * @file ui_theme_manager.c
 * @brief Implementation of the theme manager.
 */
/* clang-format off */
#include "ui_theme_manager.h"
#include "ui_web_bridge.h"
#include "ui_arena.h"
#include <stddef.h>
/* clang-format on */

/**
 * @struct ui_theme_manager
 * @struct ui_theme_manager
 * @brief Internal implementation of the theme manager.
 */
struct ui_theme_manager {
  /* @brief The arena used for allocations. */
  struct ui_arena *arena; /**< arena */
  /* @brief The current theme mode. */
  enum ui_theme_mode current_mode; /**< current_mode */
  /* @brief Signal emitted when the theme mode changes. */
  ui_signal_t *change_signal; /**< change_signal */
};

/*
 * @brief Equality function for theme mode signals.
 * @param a The first payload.
 * @param b The second payload.
 * @param out_equal Pointer to store the equality result.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t theme_mode_equality(union ui_signal_payload a,
                                      union ui_signal_payload b,
                                      ui_bool_t *out_equal) {
  *out_equal = a.int_val == b.int_val ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

ui_error_t ui_theme_manager_create(struct ui_arena *arena,
                                   struct ui_theme_manager **out_manager) {
  ui_error_t err;
  void *ptr;
  union ui_signal_payload initial_payload;

  if (!arena || !out_manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_arena_alloc(arena, sizeof(struct ui_theme_manager), 8, &ptr);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  *out_manager = (struct ui_theme_manager *)ptr;
  (*out_manager)->arena = arena;
  (*out_manager)->current_mode = UI_THEME_MODE_SYSTEM;

  initial_payload.int_val = (ui_int32)UI_THEME_MODE_SYSTEM;
  err = ui_signal_create(
      arena, initial_payload, UI_SIGNAL_TYPE_INT32, theme_mode_equality, NULL,
      UI_SIGNAL_MODE_SINGLE_THREADED, &(*out_manager)->change_signal);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_theme_manager_destroy(struct ui_theme_manager *manager) {
  ui_error_t err;

  if (!manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_signal_destroy(manager->change_signal);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  /* Note: The manager memory itself is managed by the arena. */

  return UI_ERROR_NONE;
}

ui_error_t ui_theme_manager_set_mode(struct ui_theme_manager *manager,
                                     enum ui_theme_mode mode) {
  ui_error_t err;
  union ui_signal_payload new_payload;

  if (!manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  manager->current_mode = mode;

  new_payload.int_val = (ui_int32)mode;
  err = ui_signal_set(manager->change_signal, new_payload);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_theme_manager_get_mode(struct ui_theme_manager *manager,
                                     enum ui_theme_mode *out_mode) {
  if (!manager || !out_mode) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_mode = manager->current_mode;
  return UI_ERROR_NONE;
}

ui_error_t ui_theme_manager_get_change_signal(struct ui_theme_manager *manager,
                                              ui_signal_t **out_signal) {
  if (!manager || !out_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_signal = manager->change_signal;
  return UI_ERROR_NONE;
}
