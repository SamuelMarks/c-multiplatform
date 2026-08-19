/* clang-format off */
#include "ui_video_player_base.h"
/* clang-format on */

/**
 * @brief ui_video_player_base_init.
 * @param player Parameter player.
 * @param component Parameter component.
 * @param av_sync Parameter av_sync.
 * @return Return value.
 */
ui_error_t ui_video_player_base_init(struct ui_video_player_base *player,
                                     struct ui_component *component,
                                     struct ui_av_sync *av_sync) {
  if (!player || !component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  player->component = component;
  player->av_sync = av_sync;
  player->is_playing = 0;
  player->current_time = 0.0f;
  player->duration = 0.0f;
  player->volume = 1.0f;
  player->is_fullscreen = 0;

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_video_player_base_toggle_play(struct ui_video_player_base *player) {
  if (player) {
    player->is_playing = !player->is_playing;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_video_player_base_seek.
 * @param player Parameter player.
 * @param time Parameter time.
 * @return Return value.
 */
ui_error_t ui_video_player_base_seek(struct ui_video_player_base *player,
                                     float time) {
  if (player) {
    if (time < 0.0f) {
      time = 0.0f;
    } else if (time > player->duration && player->duration > 0.0f) {
      time = player->duration;
    }
    player->current_time = time;
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_video_player_base_set_volume(struct ui_video_player_base *player,
                                           float volume) {
  if (player) {
    if (volume < 0.0f) {
      volume = 0.0f;
    } else if (volume > 1.0f) {
      volume = 1.0f;
    }
    player->volume = volume;
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_video_player_base_toggle_fullscreen(struct ui_video_player_base *player) {
  if (player) {
    player->is_fullscreen = !player->is_fullscreen;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_video_player_base_bind_src.
 * @param widget Parameter widget.
 * @param signal Parameter signal.
 * @return Return value.
 */
ui_error_t ui_video_player_base_bind_src(struct ui_video_player_base *widget,
                                         struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->src_signal = signal;
  return UI_ERROR_NONE;
}
