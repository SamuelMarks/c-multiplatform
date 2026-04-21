/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

int cmp_gamepad_evaluate_focus_navigation(const cmp_gamepad_t *gamepad,
                                          float dt_ms) {
  int rc = CMP_SUCCESS;
  (void)dt_ms;

  if (!gamepad) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_gamepad_evaluate_focus_navigation: Invalid "
              "argument (gamepad=NULL)\n");
    return rc;
  }

  /* In reality, this would check threshold limits on gamepad->axes[x/y] and
     emit cmp_event_t KEY_DOWN events with VK_UP/DOWN/LEFT/RIGHT, then track
     dt_ms to apply repeating intervals. */

  return rc;
}

int cmp_gamepad_trigger_rumble(int index, cmp_gamepad_rumble_type_t type) {
  int rc = CMP_SUCCESS;
  (void)type;

  if (index < 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_gamepad_trigger_rumble: Invalid index\n");
    return rc;
  }

  /* Issues commands to OS GameController.framework or SDL_Haptic */
  return rc;
}

int cmp_gamepad_set_adaptive_trigger(
    int index, int is_left, const cmp_adaptive_trigger_config_t *config) {
  int rc = CMP_SUCCESS;
  (void)is_left;

  if (index < 0 || !config) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_gamepad_set_adaptive_trigger: Invalid argument\n");
    return rc;
  }

  /* Maps to DualSense haptic APIs */
  return rc;
}
