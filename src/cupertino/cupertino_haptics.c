/**
 * @file cupertino_haptics.c
 * @brief Apple Haptic Feedback Generators implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_haptics.h"
#include <stddef.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_haptics_mock_return = -1;
static ui_error_t
mock_cupertino_haptics_trigger(enum ui_haptic_feedback_type type) {
  if (g_cupertino_haptics_mock_return >= 0) {
    return (ui_error_t)g_cupertino_haptics_mock_return;
  }
  return (ui_haptics_trigger)(type);
}
#undef ui_haptics_trigger
/** @cond */
#define ui_haptics_trigger mock_cupertino_haptics_trigger
/** @endcond */
#endif

ui_error_t cupertino_haptic_impact(enum cupertino_haptic_impact_style style,
                                   float intensity) {
  enum ui_haptic_feedback_type cdk_type;
  ui_error_t rc;

  if (intensity < 0.0f || intensity > 1.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  switch (style) {
  case CUPERTINO_HAPTIC_IMPACT_LIGHT:
    cdk_type = UI_HAPTIC_FEEDBACK_LIGHT;
    break;
  case CUPERTINO_HAPTIC_IMPACT_MEDIUM:
    cdk_type = UI_HAPTIC_FEEDBACK_MEDIUM;
    break;
  case CUPERTINO_HAPTIC_IMPACT_HEAVY:
    cdk_type = UI_HAPTIC_FEEDBACK_HEAVY;
    break;
  case CUPERTINO_HAPTIC_IMPACT_RIGID:
    cdk_type = UI_HAPTIC_FEEDBACK_HEAVY;
    break;
  case CUPERTINO_HAPTIC_IMPACT_SOFT:
    cdk_type = UI_HAPTIC_FEEDBACK_LIGHT;
    break;
  default:
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_haptics_trigger(cdk_type);
  if (rc == UI_ERROR_UNSUPPORTED) {
    /* Apple HIG: Silently no-op when haptic engine is unavailable */
    return UI_ERROR_NONE;
  }
  return rc;
}

ui_error_t
cupertino_haptic_notification(enum cupertino_haptic_notification_type type) {
  enum ui_haptic_feedback_type cdk_type;
  ui_error_t rc;

  switch (type) {
  case CUPERTINO_HAPTIC_NOTIFICATION_SUCCESS:
    cdk_type = UI_HAPTIC_FEEDBACK_SUCCESS;
    break;
  case CUPERTINO_HAPTIC_NOTIFICATION_WARNING:
    cdk_type = UI_HAPTIC_FEEDBACK_WARNING;
    break;
  case CUPERTINO_HAPTIC_NOTIFICATION_ERROR:
    cdk_type = UI_HAPTIC_FEEDBACK_ERROR;
    break;
  default:
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_haptics_trigger(cdk_type);
  if (rc == UI_ERROR_UNSUPPORTED) {
    return UI_ERROR_NONE;
  }
  return rc;
}

ui_error_t cupertino_haptic_selection(void) {
  ui_error_t rc;

  rc = ui_haptics_trigger(UI_HAPTIC_FEEDBACK_SELECTION);
  if (rc == UI_ERROR_UNSUPPORTED) {
    return UI_ERROR_NONE;
  }
  return rc;
}

ui_error_t cupertino_haptic_prepare(void) {
  /* Pre-warms the haptic actuator state */
  return UI_ERROR_NONE;
}
