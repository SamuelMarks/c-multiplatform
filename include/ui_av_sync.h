/**
 * @file ui_av_sync.h
 * @brief Audio/Video synchronization clock manager.
 */

#ifndef UI_AV_SYNC_H
#define UI_AV_SYNC_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"

#include "ui_types.h"
/* clang-format on */

/**
 * @brief A/V Synchronization clock manager.
 * Drives the renderer clock using Audio PTS as the master clock.
 */
struct ui_av_sync {
  ui_int64 master_clock_us; /**< Current master clock time in microseconds */
  ui_int64
      last_update_sys_us; /**< System time at last update in microseconds */
  int has_audio; /**< Non-zero if audio is present and driving the clock */
};

/**
 * @brief Initializes the A/V sync clock.
 *
 * @param sync The sync instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_av_sync_init(struct ui_av_sync *sync);

/**
 * @brief Updates the clock with the latest audio PTS.
 *
 * @param sync The sync instance.
 * @param audio_pts_us The presentation timestamp from the audio sink.
 * @param sys_time_us The current system time (to interpolate between audio
 * updates).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_av_sync_update_audio_pts(struct ui_av_sync *sync,
                                       ui_int64 audio_pts_us,
                                       ui_int64 sys_time_us);

/**
 * @brief Ticks the sync clock using system time.
 * (fallback for video-only or interpolating between audio packets).
 *
 * @param sync The sync instance.
 * @param sys_time_us The current system time.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_av_sync_tick_system(struct ui_av_sync *sync,
                                  ui_int64 sys_time_us);

/**
 * @brief Retrieves the current interpolated master clock to sync video frames
 * against.
 *
 * @param sync The sync instance.
 * @param out_clock_us Pointer to receive the master clock time.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_av_sync_get_clock(struct ui_av_sync *sync,
                                ui_int64 *out_clock_us);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_AV_SYNC_H */
