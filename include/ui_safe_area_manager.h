#ifndef UI_SAFE_AREA_MANAGER_H
#define UI_SAFE_AREA_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_signal.h"
/* clang-format on */

/**
 * @brief Represents the safe area insets of a screen/window.
 */
struct ui_safe_area_insets {
  float top;
  float right;
  float bottom;
  float left;
};

struct ui_safe_area_manager;
struct ui_arena;

/**
 * @brief Creates a safe area manager.
 *
 * @param arena The memory arena.
 * @param out_manager Output pointer for the created manager.
 * @return enum ui_error
 */
enum ui_error
ui_safe_area_manager_create(struct ui_arena *arena,
                            struct ui_safe_area_manager **out_manager);

/**
 * @brief Destroys a safe area manager.
 *
 * @param manager The safe area manager.
 * @return enum ui_error
 */
enum ui_error
ui_safe_area_manager_destroy(struct ui_safe_area_manager *manager);

/**
 * @brief Sets the current safe area insets (usually called by window backends).
 *
 * @param manager The safe area manager.
 * @param insets The new insets.
 * @return enum ui_error
 */
enum ui_error
ui_safe_area_manager_set_insets(struct ui_safe_area_manager *manager,
                                const struct ui_safe_area_insets *insets);

/**
 * @brief Gets the current safe area insets.
 *
 * @param manager The safe area manager.
 * @param out_insets Output pointer for the insets.
 * @return enum ui_error
 */
enum ui_error
ui_safe_area_manager_get_insets(struct ui_safe_area_manager *manager,
                                struct ui_safe_area_insets *out_insets);

/**
 * @brief Gets the signal emitted when insets change.
 * Payload is a pointer to a struct ui_safe_area_insets.
 *
 * @param manager The manager.
 * @param out_signal Output pointer for the signal.
 * @return enum ui_error
 */
enum ui_error
ui_safe_area_manager_get_change_signal(struct ui_safe_area_manager *manager,
                                       ui_signal_t **out_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SAFE_AREA_MANAGER_H */
