/* clang-format off */
#include "ui_av_sync.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

ui_error_t ui_av_sync_init(struct ui_av_sync *sync) {
  if (!sync) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sync->master_clock_us = 0;
  sync->last_update_sys_us = 0;
  sync->has_audio = 0;

  return UI_ERROR_NONE;
}

ui_error_t ui_av_sync_update_audio_pts(struct ui_av_sync *sync,
                                       ui_int64 audio_pts_us,
                                       ui_int64 sys_time_us) {
  if (!sync) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sync->master_clock_us = audio_pts_us;
  sync->last_update_sys_us = sys_time_us;
  sync->has_audio = 1;

  return UI_ERROR_NONE;
}

ui_error_t ui_av_sync_tick_system(struct ui_av_sync *sync,
                                  ui_int64 sys_time_us) {
  ui_int64 delta;

  if (!sync) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (sync->last_update_sys_us == 0) {
    sync->last_update_sys_us = sys_time_us;
    return UI_ERROR_NONE;
  }

  delta = sys_time_us - sync->last_update_sys_us;
  if (delta > 0) {
    sync->master_clock_us += delta;
    sync->last_update_sys_us = sys_time_us;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_av_sync_get_clock(struct ui_av_sync *sync,
                                ui_int64 *out_clock_us) {
  if (!sync || !out_clock_us) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_clock_us = sync->master_clock_us;

  return UI_ERROR_NONE;
}
