/**
 * @file ui_top_app_bar_base.h
 * @brief Defines the base logic for a top app bar UI component.
 */
#ifndef UI_TOP_APP_BAR_BASE_H
#define UI_TOP_APP_BAR_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_signal.h"
/* clang-format on */

/**
 * @struct ui_top_app_bar_base
 * @brief Opaque handle for a top app bar component.
 */
struct ui_top_app_bar_base;

struct ui_arena;
struct ui_scroll_dispatcher;

/**
 * @enum ui_top_app_bar_state
 * @brief States of the top app bar.
 */
enum ui_top_app_bar_state {
  /** @brief App bar is fully expanded. */
  UI_TOP_APP_BAR_STATE_EXPANDED,
  /** @brief App bar is collapsed (typically scrolled out of view). */
  UI_TOP_APP_BAR_STATE_COLLAPSED,
  /** @brief App bar is pinned to the top. */
  UI_TOP_APP_BAR_STATE_PINNED,
  /** @brief App bar is floating. */
  UI_TOP_APP_BAR_STATE_FLOATING
};

/**
 * @struct ui_top_app_bar_config
 * @brief Configuration for the top app bar.
 */
struct ui_top_app_bar_config {
  /** @brief The initial state of the app bar. */
  enum ui_top_app_bar_state initial_state;
  /** @brief The height when fully expanded. */
  float expanded_height;
  /** @brief The height when collapsed. */
  float collapsed_height;
  /** @brief The scroll offset threshold to trigger state changes. */
  float scroll_threshold;
};

/**
 * @brief Creates a top app bar base component.
 *
 * @param arena The memory arena.
 * @param config The configuration.
 * @param out_bar Output pointer for the created component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_top_app_bar_base_create(struct ui_arena *arena,
                           const struct ui_top_app_bar_config *config,
                           struct ui_top_app_bar_base **out_bar);

/**
 * @brief Destroys a top app bar base component.
 *
 * @param bar The component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_top_app_bar_base_destroy(struct ui_top_app_bar_base *bar);

/**
 * @brief Handles scroll events.
 *
 * @param bar The component.
 * @param scroll_y The absolute scroll position.
 * @param delta_y The change in scroll position.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_top_app_bar_base_handle_scroll(struct ui_top_app_bar_base *bar,
                                             float scroll_y, float delta_y);

/**
 * @brief Gets the signal for the current state.
 * Payload is an int32 containing ui_top_app_bar_state.
 *
 * @param bar The component.
 * @param out_signal Output pointer for the signal.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_top_app_bar_base_get_state_signal(struct ui_top_app_bar_base *bar,
                                                ui_signal_t **out_signal);

/**
 * @brief Gets the signal for the current height.
 * Payload is a float32 containing the height.
 *
 * @param bar The component.
 * @param out_signal Output pointer for the signal.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_top_app_bar_base_get_height_signal(struct ui_top_app_bar_base *bar,
                                      ui_signal_t **out_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TOP_APP_BAR_BASE_H */
