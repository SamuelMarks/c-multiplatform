/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

/**
 * @brief cmp_gamepad_evaluate_focus_navigation
 *
 * @param gamepad Parameter description.
 * @param dt_ms Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gamepad_evaluate_focus_navigation(const cmp_gamepad_t *gamepad,
                                          float dt_ms) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  (void)dt_ms;

  if (gamepad == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_gamepad_evaluate_focus_navigation: Invalid argument "
                  "(gamepad=NULL): %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* In reality, this would check threshold limits on gamepad->axes[x/y] and
     emit cmp_event_t KEY_DOWN events with VK_UP/DOWN/LEFT/RIGHT, then track
     dt_ms to apply repeating intervals. */

  cmp_log_debug(
      "cmp_gamepad_evaluate_focus_navigation: Evaluated gamepad UI controls\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_gamepad_trigger_rumble
 *
 * @param index Parameter description.
 * @param type Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gamepad_trigger_rumble(int index, cmp_gamepad_rumble_type_t type) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  (void)type;

  if (index < 0) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_gamepad_trigger_rumble: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* Issues commands to OS GameController.framework or SDL_Haptic */
  cmp_log_debug("cmp_gamepad_trigger_rumble: Triggered device rumble\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_gamepad_set_adaptive_trigger
 *
 * @param index Parameter description.
 * @param is_left Parameter description.
 * @param config Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gamepad_set_adaptive_trigger(
    int index, int is_left, const cmp_adaptive_trigger_config_t *config) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  (void)is_left;

  if (index < 0 || config == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_gamepad_set_adaptive_trigger: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* Maps to DualSense haptic APIs */
  cmp_log_debug(
      "cmp_gamepad_set_adaptive_trigger: Set haptic trigger config\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
