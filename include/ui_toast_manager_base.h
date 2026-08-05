#ifndef UI_TOAST_MANAGER_BASE_H
#define UI_TOAST_MANAGER_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_event.h"
#include <stddef.h>
/* clang-format on */

struct ui_toast_manager_base;
struct ui_dom_node;
struct ui_overlay_director;

/**
 * @brief Regions where toasts can be displayed.
 */
enum ui_toast_region {
  UI_TOAST_REGION_TOP_LEFT,
  UI_TOAST_REGION_TOP_CENTER,
  UI_TOAST_REGION_TOP_RIGHT,
  UI_TOAST_REGION_BOTTOM_LEFT,
  UI_TOAST_REGION_BOTTOM_CENTER,
  UI_TOAST_REGION_BOTTOM_RIGHT,
  UI_TOAST_REGION_COUNT
};

/**
 * @brief Animation state for a single toast.
 */
enum ui_toast_anim_state {
  UI_TOAST_ANIM_SLIDE_IN,
  UI_TOAST_ANIM_VISIBLE,
  UI_TOAST_ANIM_SLIDE_OUT
};

/**
 * @brief Configuration for a single toast notification.
 */
struct ui_toast_config {
  enum ui_toast_region region;
  double duration_secs; /**< Time to stay visible before auto-dismiss (0 for
                           infinite) */
  const char *message;
  int is_error; /**< Simple flag for styling logic */
};

/**
 * @brief Opaque handle for an active or queued toast.
 */
typedef size_t ui_toast_id;

/**
 * @brief Creates a global toast manager.
 *
 * @param out_manager Pointer to receive the allocated manager.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_toast_manager_base_create(struct ui_toast_manager_base **out_manager);

/**
 * @brief Destroys a toast manager and all its tracked toasts.
 *
 * @param manager The toast manager to destroy.
 */
ui_error_t ui_toast_manager_base_destroy(struct ui_toast_manager_base *manager);

/**
 * @brief Enqueues a new toast notification.
 *
 * @param manager The toast manager.
 * @param config The toast configuration.
 * @param current_time_secs Current monotonic time in seconds.
 * @param out_id Pointer to receive the assigned toast ID.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_toast_manager_base_show(struct ui_toast_manager_base *manager,
                                      const struct ui_toast_config *config,
                                      double current_time_secs,
                                      ui_toast_id *out_id);

/**
 * @brief Programmatically dismisses an active or queued toast.
 *
 * @param manager The toast manager.
 * @param id The ID of the toast to dismiss.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if invalid.
 */
ui_error_t ui_toast_manager_base_dismiss(struct ui_toast_manager_base *manager,
                                         ui_toast_id id);

/**
 * @brief Ticks the internal logic: processes auto-dismissal timers and
 * animation states.
 *
 * @param manager The toast manager.
 * @param current_time_secs Current monotonic time in seconds.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_toast_manager_base_tick(struct ui_toast_manager_base *manager,
                                      double current_time_secs);

/**
 * @brief Processes an input event. Used for pause-on-hover functionality.
 *
 * @param manager The toast manager.
 * @param event The input event (mouse enter/leave mapped coordinates or logic).
 * @param current_time_secs Current time.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_toast_manager_base_handle_event(struct ui_toast_manager_base *manager,
                                   const struct ui_event *event,
                                   double current_time_secs);

/**
 * @brief Renders the active toasts via the overlay director.
 *
 * @param manager The toast manager.
 * @param director The overlay director managing the highest layer.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_toast_manager_base_render(struct ui_toast_manager_base *manager,
                                        struct ui_overlay_director *director);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TOAST_MANAGER_BASE_H */
