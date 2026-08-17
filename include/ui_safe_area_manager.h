#ifndef UI_SAFE_AREA_MANAGER_H
#define UI_SAFE_AREA_MANAGER_H

/**
 * \file ui_safe_area_manager.h
 * \brief UI Safe Area Manager component.
 *
 * This file contains definitions for managing device safe areas
 * (e.g., notches, rounded corners, home indicators) across different
 * platforms, ensuring content is not obscured.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_signal.h"
/* clang-format on */

/**
 * @struct ui_safe_area_insets
 * @brief Represents the safe area insets of a screen/window.
 */
struct ui_safe_area_insets {
  float top;    /**< The top inset in pixels. */
  float right;  /**< The right inset in pixels. */
  float bottom; /**< The bottom inset in pixels. */
  float left;   /**< The left inset in pixels. */
};

/**
 * @brief Opaque handle for a safe area manager.
 */
struct ui_safe_area_manager;

/** \brief Forward declaration of ui_arena */
struct ui_arena;

/**
 * @brief Creates a safe area manager.
 *
 * @param arena The memory arena to use for allocation.
 * @param out_manager Output pointer for the created manager.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_safe_area_manager_create(struct ui_arena *arena,
                            struct ui_safe_area_manager **out_manager);

/**
 * @brief Destroys a safe area manager.
 *
 * @param manager The safe area manager to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_safe_area_manager_destroy(struct ui_safe_area_manager *manager);

/**
 * @brief Sets the current safe area insets (usually called by window backends).
 *
 * @param manager The safe area manager.
 * @param insets The new insets to apply.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_safe_area_manager_set_insets(struct ui_safe_area_manager *manager,
                                const struct ui_safe_area_insets *insets);

/**
 * @brief Gets the current safe area insets.
 *
 * @param manager The safe area manager.
 * @param out_insets Output pointer to receive the current insets.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_safe_area_manager_get_insets(struct ui_safe_area_manager *manager,
                                struct ui_safe_area_insets *out_insets);

/**
 * @brief Gets the signal emitted when insets change.
 * Payload is a pointer to a struct ui_safe_area_insets.
 *
 * @param manager The manager.
 * @param out_signal Output pointer for the signal.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_safe_area_manager_get_change_signal(struct ui_safe_area_manager *manager,
                                       ui_signal_t **out_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SAFE_AREA_MANAGER_H */
