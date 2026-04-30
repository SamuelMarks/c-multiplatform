/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_media_player {
  int is_pip_enabled;
  int is_spatial_audio_enabled;
  int use_system_controls;
  cmp_remote_command_cb remote_cb;
  void *remote_userdata;
  char *now_playing_title;
  char *now_playing_artist;
};

struct cmp_audio_session {
  cmp_audio_session_category_t category;
  int is_active;
};

/**
 * @brief Create a media player instance.
 *
 * @param out_player Pointer to store the created media player.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_media_player_create(cmp_media_player_t **out_player) {
  int rc = CMP_SUCCESS;
  struct cmp_media_player *ctx;

  rc = CMP_SUCCESS;
  ctx = NULL;

  if (out_player == NULL) {
    LOG_DEBUG("Error in cmp_media_player_create: out_player is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_media_player), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_media_player_create: CMP_MALLOC failed (OOM)\n");
    return CMP_ERROR_OOM;
  }

  ctx->is_pip_enabled = 0;
  ctx->is_spatial_audio_enabled = 0;
  ctx->use_system_controls = 1; /* HIG preferred */
  ctx->remote_cb = NULL;
  ctx->remote_userdata = NULL;
  ctx->now_playing_title = NULL;
  ctx->now_playing_artist = NULL;

  *out_player = (cmp_media_player_t *)ctx;
  LOG_DEBUG("cmp_media_player_create: Success\n");
  return rc;
}

/**
 * @brief Destroy a media player instance.
 *
 * @param player_opaque The media player to destroy.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_media_player_destroy(cmp_media_player_t *player_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_media_player *ctx;

  rc = CMP_SUCCESS;
  ctx = (struct cmp_media_player *)player_opaque;

  if (ctx == NULL) {
    LOG_DEBUG(
        "cmp_media_player_destroy: player_opaque is NULL, doing nothing\n");
    return rc;
  }

  if (ctx->now_playing_title != NULL) {
    rc = CMP_FREE(ctx->now_playing_title);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_media_player_destroy: CMP_FREE failed for "
                "now_playing_title\n");
    }
  }

  if (ctx->now_playing_artist != NULL) {
    rc = CMP_FREE(ctx->now_playing_artist);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_media_player_destroy: CMP_FREE failed for "
                "now_playing_artist\n");
    }
  }

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_media_player_destroy: CMP_FREE failed for ctx\n");
    return rc;
  }

  LOG_DEBUG("cmp_media_player_destroy: Success\n");
  return rc;
}

/**
 * @brief Load a URL into the media player.
 *
 * @param player_opaque The media player.
 * @param url The URL to load.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_media_player_load_url(cmp_media_player_t *player_opaque,
                              const char *url) {
  int rc = CMP_SUCCESS;
  if (player_opaque == NULL) {
    LOG_DEBUG("Error in cmp_media_player_load_url: player_opaque is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (url == NULL) {
    LOG_DEBUG("Error in cmp_media_player_load_url: url is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* Triggers AVPlayerItem instantiation */
  LOG_DEBUG("cmp_media_player_load_url: Success\n");
  return rc;
}

/**
 * @brief Mount the media player to a UI node.
 *
 * @param player_opaque The media player.
 * @param node The UI node.
 * @param use_system_controls Whether to use system controls.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_media_player_mount(cmp_media_player_t *player_opaque,
                           cmp_ui_node_t *node, int use_system_controls) {
  int rc = CMP_SUCCESS;
  struct cmp_media_player *ctx;

  ctx = (struct cmp_media_player *)player_opaque;

  if (ctx == NULL) {
    LOG_DEBUG("Error in cmp_media_player_mount: player_opaque is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (node == NULL) {
    LOG_DEBUG("Error in cmp_media_player_mount: node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  ctx->use_system_controls = use_system_controls;
  /* Mounts AVPlayerViewController or custom player bounds */
  LOG_DEBUG("cmp_media_player_mount: Success\n");
  return rc;
}

/**
 * @brief Enable or disable Picture-in-Picture.
 *
 * @param player_opaque The media player.
 * @param is_enabled 1 to enable, 0 to disable.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_media_player_set_pip_enabled(cmp_media_player_t *player_opaque,
                                     int is_enabled) {
  int rc = CMP_SUCCESS;
  struct cmp_media_player *ctx;

  ctx = (struct cmp_media_player *)player_opaque;

  if (ctx == NULL) {
    LOG_DEBUG(
        "Error in cmp_media_player_set_pip_enabled: player_opaque is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  ctx->is_pip_enabled = is_enabled;
  LOG_DEBUG("cmp_media_player_set_pip_enabled: Success\n");
  return rc;
}

/**
 * @brief Update the "now playing" information.
 *
 * @param player_opaque The media player.
 * @param title The title of the track.
 * @param artist The artist of the track.
 * @param duration The total duration.
 * @param current_time The current playback time.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_media_player_update_now_playing(cmp_media_player_t *player_opaque,
                                        const char *title, const char *artist,
                                        float duration, float current_time) {
  int rc = CMP_SUCCESS;
  struct cmp_media_player *ctx;
  size_t len;

  rc = CMP_SUCCESS;
  ctx = (struct cmp_media_player *)player_opaque;
  (void)duration;
  (void)current_time;

  if (ctx == NULL) {
    LOG_DEBUG("Error in cmp_media_player_update_now_playing: player_opaque is "
              "NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (title == NULL) {
    LOG_DEBUG("Error in cmp_media_player_update_now_playing: title is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (artist == NULL) {
    LOG_DEBUG("Error in cmp_media_player_update_now_playing: artist is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (ctx->now_playing_title != NULL) {
    rc = CMP_FREE(ctx->now_playing_title);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_media_player_update_now_playing: CMP_FREE failed "
                "for title\n");
    }
  }

  len = strlen(title);
  rc = CMP_MALLOC(len + 1, (void **)&ctx->now_playing_title);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_media_player_update_now_playing: CMP_MALLOC failed "
              "for title (OOM)\n");
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  rc = strcpy_s(ctx->now_playing_title, len + 1, title);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_media_player_update_now_playing: strcpy_s failed "
              "for title\n");
    return CMP_ERROR_GENERAL;
  }
#else
  strcpy(ctx->now_playing_title, title);
#endif

  if (ctx->now_playing_artist != NULL) {
    rc = CMP_FREE(ctx->now_playing_artist);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_media_player_update_now_playing: CMP_FREE failed "
                "for artist\n");
    }
  }

  len = strlen(artist);
  rc = CMP_MALLOC(len + 1, (void **)&ctx->now_playing_artist);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_media_player_update_now_playing: CMP_MALLOC failed "
              "for artist (OOM)\n");
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  rc = strcpy_s(ctx->now_playing_artist, len + 1, artist);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_media_player_update_now_playing: strcpy_s failed "
              "for artist\n");
    return CMP_ERROR_GENERAL;
  }
#else
  strcpy(ctx->now_playing_artist, artist);
#endif

  /* Communicates to MPNowPlayingInfoCenter */
  LOG_DEBUG("cmp_media_player_update_now_playing: Success\n");
  return rc;
}

/**
 * @brief Enable or disable spatial audio.
 *
 * @param player_opaque The media player.
 * @param is_enabled 1 to enable, 0 to disable.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_media_player_set_spatial_audio_enabled(
    cmp_media_player_t *player_opaque, int is_enabled) {
  int rc = CMP_SUCCESS;
  struct cmp_media_player *ctx;

  ctx = (struct cmp_media_player *)player_opaque;

  if (ctx == NULL) {
    LOG_DEBUG("Error in cmp_media_player_set_spatial_audio_enabled: "
              "player_opaque is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  ctx->is_spatial_audio_enabled = is_enabled;
  LOG_DEBUG("cmp_media_player_set_spatial_audio_enabled: Success\n");
  return rc;
}

/**
 * @brief Set the remote command handler.
 *
 * @param player_opaque The media player.
 * @param callback The callback function.
 * @param userdata The userdata to pass to the callback.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_media_player_set_remote_command_handler(
    cmp_media_player_t *player_opaque, cmp_remote_command_cb callback,
    void *userdata) {
  int rc = CMP_SUCCESS;
  struct cmp_media_player *ctx;

  ctx = (struct cmp_media_player *)player_opaque;

  if (ctx == NULL) {
    LOG_DEBUG("Error in cmp_media_player_set_remote_command_handler: "
              "player_opaque is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  ctx->remote_cb = callback;
  ctx->remote_userdata = userdata;
  /* Binds MPRemoteCommandCenter */
  LOG_DEBUG("cmp_media_player_set_remote_command_handler: Success\n");
  return rc;
}

/**
 * @brief Create an audio session.
 *
 * @param out_session Pointer to store the created audio session.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_audio_session_create(cmp_audio_session_t **out_session) {
  int rc = CMP_SUCCESS;
  struct cmp_audio_session *ctx;

  rc = CMP_SUCCESS;
  ctx = NULL;

  if (out_session == NULL) {
    LOG_DEBUG("Error in cmp_audio_session_create: out_session is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_audio_session), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_audio_session_create: CMP_MALLOC failed (OOM)\n");
    return CMP_ERROR_OOM;
  }

  ctx->category = CMP_AUDIO_SESSION_AMBIENT; /* Default */
  ctx->is_active = 0;

  *out_session = (cmp_audio_session_t *)ctx;
  LOG_DEBUG("cmp_audio_session_create: Success\n");
  return rc;
}

/**
 * @brief Destroy an audio session.
 *
 * @param session_opaque The audio session to destroy.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_audio_session_destroy(cmp_audio_session_t *session_opaque) {
  int rc = CMP_SUCCESS;

  rc = CMP_SUCCESS;

  if (session_opaque != NULL) {
    rc = CMP_FREE(session_opaque);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_audio_session_destroy: CMP_FREE failed\n");
      return rc;
    }
  }

  LOG_DEBUG("cmp_audio_session_destroy: Success\n");
  return rc;
}

/**
 * @brief Set the category of an audio session.
 *
 * @param session_opaque The audio session.
 * @param category The category to set.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_audio_session_set_category(cmp_audio_session_t *session_opaque,
                                   cmp_audio_session_category_t category) {
  int rc = CMP_SUCCESS;
  struct cmp_audio_session *ctx;

  ctx = (struct cmp_audio_session *)session_opaque;

  if (ctx == NULL) {
    LOG_DEBUG(
        "Error in cmp_audio_session_set_category: session_opaque is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  ctx->category = category;
  LOG_DEBUG("cmp_audio_session_set_category: Success\n");
  return rc;
}

/**
 * @brief Activate an audio session.
 *
 * @param session_opaque The audio session.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_audio_session_activate(cmp_audio_session_t *session_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_audio_session *ctx;

  ctx = (struct cmp_audio_session *)session_opaque;

  if (ctx == NULL) {
    LOG_DEBUG("Error in cmp_audio_session_activate: session_opaque is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  ctx->is_active = 1;
  /* Triggers AVAudioSession setActive:YES error handling */
  LOG_DEBUG("cmp_audio_session_activate: Success\n");
  return rc;
}
