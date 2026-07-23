/* clang-format off */
#include "ui_av_sync.h"
#include "ui_error.h"
#include <stdio.h>
/* clang-format on */

int main(void) {
  struct ui_av_sync sync;
  enum ui_error err;
  ui_int64 clock_us;

  /* Invalid args */
  if (ui_av_sync_init(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_av_sync_update_audio_pts(NULL, 100, 100) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_av_sync_tick_system(NULL, 100) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_av_sync_get_clock(NULL, &clock_us) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_av_sync_get_clock(&sync, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  err = ui_av_sync_init(&sync);
  if (err != UI_ERROR_NONE)
    return 1;

  if (sync.master_clock_us != 0 || sync.last_update_sys_us != 0 ||
      sync.has_audio != 0)
    return 1;

  /* Test tick system when last_update_sys_us is 0 */
  err = ui_av_sync_tick_system(&sync, 50000);
  if (err != UI_ERROR_NONE)
    return 1;
  if (sync.last_update_sys_us != 50000)
    return 1;
  if (sync.master_clock_us != 0)
    return 1; /* Should not have ticked clock */

  /* Test update audio pts */
  err = ui_av_sync_update_audio_pts(&sync, 100000, 50000);
  if (err != UI_ERROR_NONE)
    return 1;

  err = ui_av_sync_get_clock(&sync, &clock_us);
  if (err != UI_ERROR_NONE || clock_us != 100000)
    return 1;

  /* Test tick system with positive delta */
  err = ui_av_sync_tick_system(&sync, 60000);
  if (err != UI_ERROR_NONE)
    return 1;

  err = ui_av_sync_get_clock(&sync, &clock_us);
  if (err != UI_ERROR_NONE || clock_us != 110000)
    return 1;

  /* Test tick system with negative delta (e.g. clock adjusted backwards or old
   * event) */
  err = ui_av_sync_tick_system(&sync, 50000);
  if (err != UI_ERROR_NONE)
    return 1;

  err = ui_av_sync_get_clock(&sync, &clock_us);
  if (err != UI_ERROR_NONE || clock_us != 110000)
    return 1; /* Clock should not have gone backwards */

  printf("All test_ui_av_sync passed.\n");
  return 0;
}
