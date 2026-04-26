/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

/**
 * @brief cmp_os_notify
 *
 * @param title Parameter description.
 * @param body Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_os_notify(const char *title, const char *body) {
  int rc = CMP_SUCCESS;
  if (!title || !body) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_os_notify: Invalid argument\n");
    return rc;
  }
  rc = cmp_window_os_notify(title, body);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_os_notify: cmp_window_os_notify failed\n");
  }
  return rc;
}

struct cmp_os_media_controls {
  cmp_media_player_t *player;
};

/**
 * @brief cmp_os_media_controls_create
 *
 * @param out_controls Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_os_media_controls_create(cmp_os_media_controls_t **out_controls) {
  int rc = CMP_SUCCESS;
  cmp_os_media_controls_t *mc = NULL;

  if (!out_controls) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_os_media_controls_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_os_media_controls_t), (void **)&(mc));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  rc = cmp_media_player_create(&mc->player);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_os_media_controls_create: cmp_media_player_create "
              "failed\n");
    rc = CMP_FREE(mc);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    return rc;
  }

  *out_controls = mc;
  return rc;
}

/**
 * @brief cmp_os_media_controls_destroy
 *
 * @param controls Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_os_media_controls_destroy(cmp_os_media_controls_t *controls) {
  int rc = CMP_SUCCESS;

  if (!controls) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_os_media_controls_destroy: Invalid argument\n");
    return rc;
  }

  if (controls->player) {
    rc = cmp_media_player_destroy(controls->player);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_os_media_controls_destroy: "
                "cmp_media_player_destroy failed\n");
    }
  }
  rc = CMP_FREE(controls);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  return rc;
}

/**
 * @brief cmp_os_media_controls_update
 *
 * @param controls Parameter description.
 * @param title Parameter description.
 * @param artist Parameter description.
 * @param duration Parameter description.
 * @param current_time Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_os_media_controls_update(cmp_os_media_controls_t *controls,
                                 const char *title, const char *artist,
                                 float duration, float current_time) {
  int rc = CMP_SUCCESS;

  if (!controls || !controls->player) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_os_media_controls_update: Invalid argument\n");
    return rc;
  }

  rc = cmp_media_player_update_now_playing(controls->player, title, artist,
                                           duration, current_time);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_os_media_controls_update: "
              "cmp_media_player_update_now_playing failed\n");
  }
  return rc;
}

/**
 * @brief cmp_os_media_controls_set_handler
 *
 * @param controls Parameter description.
 * @param callback Parameter description.
 * @param userdata Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_os_media_controls_set_handler(cmp_os_media_controls_t *controls,
                                      cmp_media_command_cb callback,
                                      void *userdata) {
  int rc = CMP_SUCCESS;

  if (!controls || !controls->player) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_os_media_controls_set_handler: Invalid argument\n");
    return rc;
  }

  /* Assuming cmp_media_player_set_remote_command_handler takes a generic
     callback. If the type signature is exactly the same, we can cast it safely.
   */
  rc = cmp_media_player_set_remote_command_handler(
      controls->player, (cmp_remote_command_cb)callback, userdata);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_os_media_controls_set_handler: "
              "cmp_media_player_set_remote_command_handler failed\n");
  }
  return rc;
}
