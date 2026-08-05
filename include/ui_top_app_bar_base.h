#ifndef UI_TOP_APP_BAR_BASE_H
#define UI_TOP_APP_BAR_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_signal.h"
/* clang-format on */

struct ui_top_app_bar_base;
struct ui_arena;
struct ui_scroll_dispatcher;

/**
 * @brief States of the top app bar.
 */
enum ui_top_app_bar_state {
  UI_TOP_APP_BAR_STATE_EXPANDED,
  UI_TOP_APP_BAR_STATE_COLLAPSED,
  UI_TOP_APP_BAR_STATE_PINNED,
  UI_TOP_APP_BAR_STATE_FLOATING
};

/**
 * @brief Configuration for the top app bar.
 */
struct ui_top_app_bar_config {
  enum ui_top_app_bar_state initial_state;
  float expanded_height;
  float collapsed_height;
  float scroll_threshold;
};

/**
 * @brief Creates a top app bar base component.
 *
 * @param arena The memory arena.
 * @param config The configuration.
 * @param out_bar Output pointer for the created component.
 * @return ui_error_t
 */
ui_error_t
ui_top_app_bar_base_create(struct ui_arena *arena,
                           const struct ui_top_app_bar_config *config,
                           struct ui_top_app_bar_base **out_bar);

/**
 * @brief Destroys a top app bar base component.
 *
 * @param bar The component.
 * @return ui_error_t
 */
ui_error_t ui_top_app_bar_base_destroy(struct ui_top_app_bar_base *bar);

/**
 * @brief Handles scroll events.
 *
 * @param bar The component.
 * @param scroll_y The absolute scroll position.
 * @param delta_y The change in scroll position.
 * @return ui_error_t
 */
ui_error_t ui_top_app_bar_base_handle_scroll(struct ui_top_app_bar_base *bar,
                                             float scroll_y, float delta_y);

/**
 * @brief Gets the signal for the current state.
 * Payload is an int32 containing ui_top_app_bar_state.
 *
 * @param bar The component.
 * @param out_signal Output pointer for the signal.
 * @return ui_error_t
 */
ui_error_t ui_top_app_bar_base_get_state_signal(struct ui_top_app_bar_base *bar,
                                                ui_signal_t **out_signal);

/**
 * @brief Gets the signal for the current height.
 * Payload is a float32 containing the height.
 *
 * @param bar The component.
 * @param out_signal Output pointer for the signal.
 * @return ui_error_t
 */
ui_error_t
ui_top_app_bar_base_get_height_signal(struct ui_top_app_bar_base *bar,
                                      ui_signal_t **out_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TOP_APP_BAR_BASE_H */
