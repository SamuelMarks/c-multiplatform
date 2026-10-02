/**
 * @file cupertino_haptics.h
 * @brief Apple Haptic Feedback Generators (UIFeedbackGenerator bridge).
 */

#ifndef CUPERTINO_CUPERTINO_HAPTICS_H
#define CUPERTINO_CUPERTINO_HAPTICS_H

/* clang-format off */
#include "ui_error.h"
#include "ui_haptics.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum cupertino_haptic_impact_style
 * @brief Apple UIImpactFeedbackGenerator styles.
 */
enum cupertino_haptic_impact_style {
  CUPERTINO_HAPTIC_IMPACT_LIGHT = 0, /**< Subtle detent / switch flick. */
  CUPERTINO_HAPTIC_IMPACT_MEDIUM,    /**< Standard button tap. */
  CUPERTINO_HAPTIC_IMPACT_HEAVY,     /**< Destructive / full swipe action. */
  CUPERTINO_HAPTIC_IMPACT_RIGID,     /**< Hard boundary / mechanical stop. */
  CUPERTINO_HAPTIC_IMPACT_SOFT       /**< Rubber-band spring recoil. */
};

/**
 * @enum cupertino_haptic_notification_type
 * @brief Apple UINotificationFeedbackGenerator feedback types.
 */
enum cupertino_haptic_notification_type {
  CUPERTINO_HAPTIC_NOTIFICATION_SUCCESS = 0, /**< Dual-pulse success tap. */
  CUPERTINO_HAPTIC_NOTIFICATION_WARNING,     /**< Rapid warning flutter. */
  CUPERTINO_HAPTIC_NOTIFICATION_ERROR        /**< Triple heavy burst. */
};

/**
 * @brief Triggers an impact haptic pattern with optional intensity modulation.
 *
 * @param style Collision / impact style.
 * @param intensity Normalized collision intensity in [0.0, 1.0].
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_haptic_impact(
    enum cupertino_haptic_impact_style style, float intensity);

/**
 * @brief Triggers a semantic notification haptic pattern.
 *
 * @param type Notification category (success, warning, error).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_haptic_notification(enum cupertino_haptic_notification_type type);

/**
 * @brief Triggers a selection changed haptic tick (e.g. wheel picker detent).
 *
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_haptic_selection(void);

/**
 * @brief Prepares the haptic engine, reducing latency for upcoming feedback.
 *
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_haptic_prepare(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_HAPTICS_H */
