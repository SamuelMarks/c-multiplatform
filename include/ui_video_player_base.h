#ifndef UI_VIDEO_PLAYER_BASE_H
#define UI_VIDEO_PLAYER_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

struct ui_component;

/**
 * @struct ui_av_sync
 * @brief Opaque handle for AV synchronization engine.
 */
struct ui_av_sync;

/**
 * @struct ui_video_player_base
 * @brief Base state for a Video/Media Player UI component.
 */
struct ui_video_player_base {
  /** @brief Pointer to the associated UI component. */
  struct ui_component *component;
  /** @brief Pointer to the AV sync engine. */
  struct ui_av_sync *av_sync;
  /** @brief Non-zero if the player is currently playing. */
  int is_playing;
  /** @brief The current playback time in seconds. */
  float current_time;
  /** @brief The total duration of the media in seconds. */
  float duration;
  /** @brief The current volume level (0.0 to 1.0). */
  float volume;
  /** @brief Non-zero if the player is in fullscreen mode. */
  int is_fullscreen;
  /** @brief The signal containing the source URI. */
  struct ui_signal *src_signal;
};

/**
 * @brief Initializes a base video player UI component.
 *
 * @param player Pointer to the video player base struct.
 * @param component The UI component to bind to.
 * @param av_sync Pointer to the AV sync engine.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_video_player_base_init(struct ui_video_player_base *player,
                                     struct ui_component *component,
                                     struct ui_av_sync *av_sync);

/**
 * @brief Toggles play/pause state.
 *
 * @param player Pointer to the video player base struct.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_video_player_base_toggle_play(struct ui_video_player_base *player);

/**
 * @brief Seeks to a specific time.
 *
 * @param player Pointer to the video player base struct.
 * @param time Time in seconds.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_video_player_base_seek(struct ui_video_player_base *player,
                                     float time);

/**
 * @brief Sets the player volume.
 *
 * @param player Pointer to the video player base struct.
 * @param volume Volume level from 0.0 to 1.0.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_video_player_base_set_volume(struct ui_video_player_base *player,
                                           float volume);

/**
 * @brief Toggles fullscreen mode.
 *
 * @param player Pointer to the video player base struct.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_video_player_base_toggle_fullscreen(struct ui_video_player_base *player);

/**
 * @brief Binds the src property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_video_player_base_bind_src(struct ui_video_player_base *widget,
                                         struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_VIDEO_PLAYER_BASE_H */
