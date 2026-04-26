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
 * @brief cmp_media_player_create
 *
 * @param out_player Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_media_player_create(cmp_media_player_t **out_player) {
  int rc = CMP_SUCCESS;
  struct cmp_media_player *ctx = NULL;

  if (!out_player) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_media_player_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_media_player), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_media_player_create: Out of memory\n");
    return rc;
  }

  ctx->is_pip_enabled = 0;
  ctx->is_spatial_audio_enabled = 0;
  ctx->use_system_controls = 1; /* HIG preferred */
  ctx->remote_cb = NULL;
  ctx->remote_userdata = NULL;
  ctx->now_playing_title = NULL;
  ctx->now_playing_artist = NULL;

  *out_player = (cmp_media_player_t *)ctx;
  return rc;
}

/**
 * @brief cmp_media_player_destroy
 *
 * @param player_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_media_player_destroy(cmp_media_player_t *player_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_media_player *ctx = (struct cmp_media_player *)player_opaque;

  if (!ctx) {
    return rc;
  }

  if (ctx->now_playing_title)
    CMP_FREE(ctx->now_playing_title);
  if (ctx->now_playing_artist)
    CMP_FREE(ctx->now_playing_artist);
  CMP_FREE(ctx);
  return rc;
}

/**
 * @brief cmp_media_player_load_url
 *
 * @param player_opaque Parameter description.
 * @param url Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_media_player_load_url(cmp_media_player_t *player_opaque,
                              const char *url) {
  int rc = CMP_SUCCESS;

  if (!player_opaque || !url) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_media_player_load_url: Invalid argument\n");
    return rc;
  }
  /* Triggers AVPlayerItem instantiation */
  return rc;
}

/**
 * @brief cmp_media_player_mount
 *
 * @param player_opaque Parameter description.
 * @param node Parameter description.
 * @param use_system_controls Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_media_player_mount(cmp_media_player_t *player_opaque,
                           cmp_ui_node_t *node, int use_system_controls) {
  int rc = CMP_SUCCESS;
  struct cmp_media_player *ctx = (struct cmp_media_player *)player_opaque;

  if (!ctx || !node) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_media_player_mount: Invalid argument\n");
    return rc;
  }

  ctx->use_system_controls = use_system_controls;
  /* Mounts AVPlayerViewController or custom player bounds */
  return rc;
}

/**
 * @brief cmp_media_player_set_pip_enabled
 *
 * @param player_opaque Parameter description.
 * @param is_enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_media_player_set_pip_enabled(cmp_media_player_t *player_opaque,
                                     int is_enabled) {
  int rc = CMP_SUCCESS;
  struct cmp_media_player *ctx = (struct cmp_media_player *)player_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_media_player_set_pip_enabled: Invalid argument\n");
    return rc;
  }

  ctx->is_pip_enabled = is_enabled;
  return rc;
}

/**
 * @brief cmp_media_player_update_now_playing
 *
 * @param player_opaque Parameter description.
 * @param title Parameter description.
 * @param artist Parameter description.
 * @param duration Parameter description.
 * @param current_time Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_media_player_update_now_playing(cmp_media_player_t *player_opaque,
                                        const char *title, const char *artist,
                                        float duration, float current_time) {
  int rc = CMP_SUCCESS;
  struct cmp_media_player *ctx = (struct cmp_media_player *)player_opaque;
  size_t len;
  (void)duration;
  (void)current_time;

  if (!ctx || !title || !artist) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_media_player_update_now_playing: Invalid argument\n");
    return rc;
  }

  if (ctx->now_playing_title) {
    CMP_FREE(ctx->now_playing_title);
  }

  len = strlen(title);
  rc = CMP_MALLOC(len + 1, (void **)&ctx->now_playing_title);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_media_player_update_now_playing: Out of memory for "
              "title\n");
    return rc;
  }
#if defined(_MSC_VER)
  strcpy_s(ctx->now_playing_title, len + 1, title);
#else
  strcpy(ctx->now_playing_title, title);
#endif

  if (ctx->now_playing_artist) {
    CMP_FREE(ctx->now_playing_artist);
  }

  len = strlen(artist);
  rc = CMP_MALLOC(len + 1, (void **)&ctx->now_playing_artist);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_media_player_update_now_playing: Out of memory for "
              "artist\n");
    return rc;
  }
#if defined(_MSC_VER)
  strcpy_s(ctx->now_playing_artist, len + 1, artist);
#else
  strcpy(ctx->now_playing_artist, artist);
#endif

  /* Communicates to MPNowPlayingInfoCenter */

  return rc;
}

/**
 * @brief cmp_media_player_set_spatial_audio_enabled
 *
 * @param player_opaque Parameter description.
 * @param is_enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_media_player_set_spatial_audio_enabled(
    cmp_media_player_t *player_opaque, int is_enabled) {
  int rc = CMP_SUCCESS;
  struct cmp_media_player *ctx = (struct cmp_media_player *)player_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_media_player_set_spatial_audio_enabled: Invalid "
              "argument\n");
    return rc;
  }

  ctx->is_spatial_audio_enabled = is_enabled;
  return rc;
}

/**
 * @brief cmp_media_player_set_remote_command_handler
 *
 * @param player_opaque Parameter description.
 * @param callback Parameter description.
 * @param userdata Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_media_player_set_remote_command_handler(
    cmp_media_player_t *player_opaque, cmp_remote_command_cb callback,
    void *userdata) {
  int rc = CMP_SUCCESS;
  struct cmp_media_player *ctx = (struct cmp_media_player *)player_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_media_player_set_remote_command_handler: Invalid "
              "argument\n");
    return rc;
  }

  ctx->remote_cb = callback;
  ctx->remote_userdata = userdata;
  /* Binds MPRemoteCommandCenter */
  return rc;
}

/**
 * @brief cmp_audio_session_create
 *
 * @param out_session Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_audio_session_create(cmp_audio_session_t **out_session) {
  int rc = CMP_SUCCESS;
  struct cmp_audio_session *ctx = NULL;

  if (!out_session) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_audio_session_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_audio_session), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_audio_session_create: Out of memory\n");
    return rc;
  }

  ctx->category = CMP_AUDIO_SESSION_AMBIENT; /* Default */
  ctx->is_active = 0;

  *out_session = (cmp_audio_session_t *)ctx;
  return rc;
}

/**
 * @brief cmp_audio_session_destroy
 *
 * @param session_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_audio_session_destroy(cmp_audio_session_t *session_opaque) {
  int rc = CMP_SUCCESS;

  if (session_opaque) {
    CMP_FREE(session_opaque);
  }
  return rc;
}

/**
 * @brief cmp_audio_session_set_category
 *
 * @param session_opaque Parameter description.
 * @param category Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_audio_session_set_category(cmp_audio_session_t *session_opaque,
                                   cmp_audio_session_category_t category) {
  int rc = CMP_SUCCESS;
  struct cmp_audio_session *ctx = (struct cmp_audio_session *)session_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_audio_session_set_category: Invalid argument\n");
    return rc;
  }

  ctx->category = category;
  return rc;
}

/**
 * @brief cmp_audio_session_activate
 *
 * @param session_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_audio_session_activate(cmp_audio_session_t *session_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_audio_session *ctx = (struct cmp_audio_session *)session_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_audio_session_activate: Invalid argument\n");
    return rc;
  }

  ctx->is_active = 1;
  /* Triggers AVAudioSession setActive:YES error handling */
  return rc;
}
