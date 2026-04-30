/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

/**
 * @brief Show an OS notification.
 *
 * @param title The title of the notification.
 * @param body The body of the notification.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_notify(const char *title, const char *body) {
  int rc = CMP_SUCCESS;

  rc = CMP_SUCCESS;

  if (title == NULL || body == NULL) {
    LOG_DEBUG("Error in cmp_os_notify: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = cmp_window_os_notify(title, body);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_os_notify: cmp_window_os_notify failed\n");
    return rc;
  }
  return rc;
}

struct cmp_os_media_controls {
  cmp_media_player_t *player;
};

/**
 * @brief Create OS media controls.
 *
 * @param out_controls Pointer to store the created context.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_media_controls_create(cmp_os_media_controls_t **out_controls) {
  int rc = CMP_SUCCESS;
  cmp_os_media_controls_t *mc;

  rc = CMP_SUCCESS;
  mc = NULL;

  if (out_controls == NULL) {
    LOG_DEBUG("Error in cmp_os_media_controls_create: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_os_media_controls_t), (void **)&mc);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG(
        "Error in cmp_os_media_controls_create: CMP_MALLOC failed (OOM)\n");
    return CMP_ERROR_OOM;
  }

  rc = cmp_media_player_create(&mc->player);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_os_media_controls_create: cmp_media_player_create "
              "failed\n");
    rc = CMP_FREE(mc);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_os_media_controls_create: CMP_FREE failed during "
                "cleanup\n");
    }
    return CMP_ERROR_GENERAL;
  }

  *out_controls = mc;
  return rc;
}

/**
 * @brief Destroy OS media controls.
 *
 * @param controls The context to destroy.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_media_controls_destroy(cmp_os_media_controls_t *controls) {
  int rc = CMP_SUCCESS;

  rc = CMP_SUCCESS;

  if (controls == NULL) {
    LOG_DEBUG("Error in cmp_os_media_controls_destroy: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (controls->player != NULL) {
    rc = cmp_media_player_destroy(controls->player);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_os_media_controls_destroy: "
                "cmp_media_player_destroy failed\n");
    }
  }

  rc = CMP_FREE(controls);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_os_media_controls_destroy: CMP_FREE failed\n");
    return rc;
  }
  return rc;
}

/**
 * @brief Update the now playing information in OS media controls.
 *
 * @param controls The controls context.
 * @param title The track title.
 * @param artist The track artist.
 * @param duration Total duration in seconds.
 * @param current_time Current playback time in seconds.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_media_controls_update(cmp_os_media_controls_t *controls,
                                 const char *title, const char *artist,
                                 float duration, float current_time) {
  int rc = CMP_SUCCESS;

  rc = CMP_SUCCESS;

  if (controls == NULL || controls->player == NULL || title == NULL ||
      artist == NULL) {
    LOG_DEBUG("Error in cmp_os_media_controls_update: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = cmp_media_player_update_now_playing(controls->player, title, artist,
                                           duration, current_time);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_os_media_controls_update: "
              "cmp_media_player_update_now_playing failed\n");
    return rc;
  }
  return rc;
}

/**
 * @brief Set the remote command handler for OS media controls.
 *
 * @param controls The controls context.
 * @param callback The callback function.
 * @param userdata User data for the callback.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_media_controls_set_handler(cmp_os_media_controls_t *controls,
                                      cmp_media_command_cb callback,
                                      void *userdata) {
  int rc = CMP_SUCCESS;

  rc = CMP_SUCCESS;

  if (controls == NULL || controls->player == NULL || callback == NULL) {
    LOG_DEBUG("Error in cmp_os_media_controls_set_handler: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = cmp_media_player_set_remote_command_handler(
      controls->player, (cmp_remote_command_cb)callback, userdata);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_os_media_controls_set_handler: "
              "cmp_media_player_set_remote_command_handler failed\n");
    return rc;
  }
  return rc;
}
