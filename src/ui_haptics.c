/**
 * @file ui_haptics.c
 * @brief ui_haptics.c implementation.
 */
/* clang-format off */
#include "ui_haptics.h"
#include <stddef.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

ui_error_t ui_haptics_trigger(enum ui_haptic_feedback_type type) {
  if ((int)type < (int)UI_HAPTIC_FEEDBACK_LIGHT ||
      (int)type > (int)UI_HAPTIC_FEEDBACK_SELECTION) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Currently unlinked to actual OS hardware. Return UI_ERROR_UNSUPPORTED
     as the stub implementation. */
  return UI_ERROR_UNSUPPORTED;
}
