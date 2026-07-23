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
struct ui_av_sync; /* opaque */

/**
 * @brief Base state for a Video/Media Player UI component.
 */
struct ui_video_player_base {
  struct ui_component *component;
  struct ui_av_sync *av_sync;
  int is_playing;
  float current_time;
  float duration;
  float volume;
  int is_fullscreen;
  struct ui_signal *src_signal;
};

/**
 * @brief Initializes a base video player UI component.
 *
 * @param player Pointer to the video player base struct.
 * @param component The UI component to bind to.
 * @param av_sync Pointer to the AV sync engine.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_video_player_base_init(struct ui_video_player_base *player,
                                        struct ui_component *component,
                                        struct ui_av_sync *av_sync);

/**
 * @brief Toggles play/pause state.
 *
 * @param player Pointer to the video player base struct.
 */
enum ui_error
ui_video_player_base_toggle_play(struct ui_video_player_base *player);

/**
 * @brief Seeks to a specific time.
 *
 * @param player Pointer to the video player base struct.
 * @param time Time in seconds.
 */
enum ui_error ui_video_player_base_seek(struct ui_video_player_base *player,
                                        float time);

/**
 * @brief Sets the player volume.
 *
 * @param player Pointer to the video player base struct.
 * @param volume Volume level from 0.0 to 1.0.
 */
enum ui_error
ui_video_player_base_set_volume(struct ui_video_player_base *player,
                                float volume);

/**
 * @brief Toggles fullscreen mode.
 *
 * @param player Pointer to the video player base struct.
 */
enum ui_error
ui_video_player_base_toggle_fullscreen(struct ui_video_player_base *player);

/**
 * @brief Binds the src property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_video_player_base_bind_src(struct ui_video_player_base *widget,
                                            struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_VIDEO_PLAYER_BASE_H */
