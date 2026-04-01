/* clang-format off */
#include "cmp.h"
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

int cmp_media_player_create(cmp_media_player_t **out_player) {
  struct cmp_media_player *ctx;
  if (!out_player)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_media_player), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->is_pip_enabled = 0;
  ctx->is_spatial_audio_enabled = 0;
  ctx->use_system_controls = 1; /* HIG preferred */
  ctx->remote_cb = NULL;
  ctx->remote_userdata = NULL;
  ctx->now_playing_title = NULL;
  ctx->now_playing_artist = NULL;

  *out_player = (cmp_media_player_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_media_player_destroy(cmp_media_player_t *player_opaque) {
  struct cmp_media_player *ctx = (struct cmp_media_player *)player_opaque;
  if (!ctx)
    return CMP_SUCCESS;

  if (ctx->now_playing_title)
    CMP_FREE(ctx->now_playing_title);
  if (ctx->now_playing_artist)
    CMP_FREE(ctx->now_playing_artist);
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_media_player_load_url(cmp_media_player_t *player_opaque,
                              const char *url) {
  if (!player_opaque || !url)
    return CMP_ERROR_INVALID_ARG;
  /* Triggers AVPlayerItem instantiation */
  return CMP_SUCCESS;
}

int cmp_media_player_mount(cmp_media_player_t *player_opaque,
                           cmp_ui_node_t *node, int use_system_controls) {
  struct cmp_media_player *ctx = (struct cmp_media_player *)player_opaque;
  if (!ctx || !node)
    return CMP_ERROR_INVALID_ARG;

  ctx->use_system_controls = use_system_controls;
  /* Mounts AVPlayerViewController or custom player bounds */
  return CMP_SUCCESS;
}

int cmp_media_player_set_pip_enabled(cmp_media_player_t *player_opaque,
                                     int is_enabled) {
  struct cmp_media_player *ctx = (struct cmp_media_player *)player_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->is_pip_enabled = is_enabled;
  return CMP_SUCCESS;
}

int cmp_media_player_update_now_playing(cmp_media_player_t *player_opaque,
                                        const char *title, const char *artist,
                                        float duration, float current_time) {
  struct cmp_media_player *ctx = (struct cmp_media_player *)player_opaque;
  size_t len;
  (void)duration;
  (void)current_time;

  if (!ctx || !title || !artist)
    return CMP_ERROR_INVALID_ARG;

  if (ctx->now_playing_title)
    CMP_FREE(ctx->now_playing_title);
  len = strlen(title);
  if (CMP_MALLOC(len + 1, (void **)&ctx->now_playing_title) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
#if defined(_MSC_VER)
  strcpy_s(ctx->now_playing_title, len + 1, title);
#else
  strcpy(ctx->now_playing_title, title);
#endif

  if (ctx->now_playing_artist)
    CMP_FREE(ctx->now_playing_artist);
  len = strlen(artist);
  if (CMP_MALLOC(len + 1, (void **)&ctx->now_playing_artist) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
#if defined(_MSC_VER)
  strcpy_s(ctx->now_playing_artist, len + 1, artist);
#else
  strcpy(ctx->now_playing_artist, artist);
#endif

  /* Communicates to MPNowPlayingInfoCenter */

  return CMP_SUCCESS;
}

int cmp_media_player_set_spatial_audio_enabled(
    cmp_media_player_t *player_opaque, int is_enabled) {
  struct cmp_media_player *ctx = (struct cmp_media_player *)player_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->is_spatial_audio_enabled = is_enabled;
  return CMP_SUCCESS;
}

int cmp_media_player_set_remote_command_handler(
    cmp_media_player_t *player_opaque, cmp_remote_command_cb callback,
    void *userdata) {
  struct cmp_media_player *ctx = (struct cmp_media_player *)player_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->remote_cb = callback;
  ctx->remote_userdata = userdata;
  /* Binds MPRemoteCommandCenter */
  return CMP_SUCCESS;
}

int cmp_audio_session_create(cmp_audio_session_t **out_session) {
  struct cmp_audio_session *ctx;
  if (!out_session)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_audio_session), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->category = CMP_AUDIO_SESSION_AMBIENT; /* Default */
  ctx->is_active = 0;

  *out_session = (cmp_audio_session_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_audio_session_destroy(cmp_audio_session_t *session_opaque) {
  if (session_opaque)
    CMP_FREE(session_opaque);
  return CMP_SUCCESS;
}

int cmp_audio_session_set_category(cmp_audio_session_t *session_opaque,
                                   cmp_audio_session_category_t category) {
  struct cmp_audio_session *ctx = (struct cmp_audio_session *)session_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->category = category;
  return CMP_SUCCESS;
}

int cmp_audio_session_activate(cmp_audio_session_t *session_opaque) {
  struct cmp_audio_session *ctx = (struct cmp_audio_session *)session_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->is_active = 1;
  /* Triggers AVAudioSession setActive:YES error handling */
  return CMP_SUCCESS;
}
