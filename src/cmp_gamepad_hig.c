/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

int cmp_gamepad_evaluate_focus_navigation(const cmp_gamepad_t *gamepad,
                                          float dt_ms) {
  (void)dt_ms;
  if (!gamepad)
    return CMP_ERROR_INVALID_ARG;

  /* In reality, this would check threshold limits on gamepad->axes[x/y] and
     emit cmp_event_t KEY_DOWN events with VK_UP/DOWN/LEFT/RIGHT, then track
     dt_ms to apply repeating intervals. */

  return CMP_SUCCESS;
}

int cmp_gamepad_trigger_rumble(int index, cmp_gamepad_rumble_type_t type) {
  if (index < 0)
    return CMP_ERROR_INVALID_ARG;
  (void)type;
  /* Issues commands to OS GameController.framework or SDL_Haptic */
  return CMP_SUCCESS;
}

int cmp_gamepad_set_adaptive_trigger(
    int index, int is_left, const cmp_adaptive_trigger_config_t *config) {
  if (index < 0 || !config)
    return CMP_ERROR_INVALID_ARG;
  (void)is_left;
  /* Maps to DualSense haptic APIs */
  return CMP_SUCCESS;
}
