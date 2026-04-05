/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

int cmp_os_notify(const char *title, const char *body) {
  if (!title || !body)
    return CMP_ERROR_INVALID_ARG;
  return cmp_window_os_notify(title, body);
}

struct cmp_os_media_controls {
  cmp_media_player_t *player;
};

int cmp_os_media_controls_create(cmp_os_media_controls_t **out_controls) {
  cmp_os_media_controls_t *mc;
  int err;

  if (!out_controls)
    return CMP_ERROR_INVALID_ARG;

  mc = (cmp_os_media_controls_t *)malloc(sizeof(cmp_os_media_controls_t));
  if (!mc)
    return CMP_ERROR_OOM;

  err = cmp_media_player_create(&mc->player);
  if (err != CMP_SUCCESS) {
    free(mc);
    return err;
  }

  *out_controls = mc;
  return CMP_SUCCESS;
}

int cmp_os_media_controls_destroy(cmp_os_media_controls_t *controls) {
  int err = CMP_SUCCESS;
  if (!controls)
    return CMP_ERROR_INVALID_ARG;

  if (controls->player) {
    err = cmp_media_player_destroy(controls->player);
  }
  free(controls);
  return err;
}

int cmp_os_media_controls_update(cmp_os_media_controls_t *controls,
                                 const char *title, const char *artist,
                                 float duration, float current_time) {
  if (!controls || !controls->player)
    return CMP_ERROR_INVALID_ARG;

  return cmp_media_player_update_now_playing(controls->player, title, artist,
                                             duration, current_time);
}

int cmp_os_media_controls_set_handler(cmp_os_media_controls_t *controls,
                                      cmp_media_command_cb callback,
                                      void *userdata) {
  if (!controls || !controls->player)
    return CMP_ERROR_INVALID_ARG;

  /* Assuming cmp_media_player_set_remote_command_handler takes a generic
     callback. If the type signature is exactly the same, we can cast it safely.
   */
  return cmp_media_player_set_remote_command_handler(
      controls->player, (cmp_remote_command_cb)callback, userdata);
}
