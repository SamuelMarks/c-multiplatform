/* clang-format off */
#include "ui_haptics.h"
#include <stddef.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

ui_error_t ui_haptics_trigger(enum ui_haptic_feedback_type type) {
  /* Suppress unused parameter warning */
  (void)type;

  /* Currently unlinked to actual OS hardware. Return UI_ERROR_UNSUPPORTED
     as the stub implementation. */
  return UI_ERROR_UNSUPPORTED;
}
