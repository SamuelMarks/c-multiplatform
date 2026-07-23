#ifndef UI_HAPTICS_H
#define UI_HAPTICS_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Represents different types of haptic feedback for OS integration.
 */
enum ui_haptic_feedback_type {
  UI_HAPTIC_FEEDBACK_LIGHT,
  UI_HAPTIC_FEEDBACK_MEDIUM,
  UI_HAPTIC_FEEDBACK_HEAVY,
  UI_HAPTIC_FEEDBACK_SUCCESS,
  UI_HAPTIC_FEEDBACK_WARNING,
  UI_HAPTIC_FEEDBACK_ERROR,
  UI_HAPTIC_FEEDBACK_SELECTION
};

/**
 * @brief Triggers a haptic feedback event.
 *
 * Dispatches to OS-level hardware feedback (Android/iOS) or falls back
 * gracefully on platforms without haptic engines.
 *
 * @param type The type of haptic feedback to trigger.
 * @return UI_ERROR_NONE on success, UI_ERROR_UNSUPPORTED if the platform does
 * not support haptics, or another appropriate error code.
 */
enum ui_error ui_haptics_trigger(enum ui_haptic_feedback_type type);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_HAPTICS_H */
