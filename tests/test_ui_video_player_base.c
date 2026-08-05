/* clang-format off */
#include <stddef.h>

#include "ui_video_player_base.h"
#include "ui_error.h"
/* clang-format on */

struct ui_component {
  int id;
};

static int test_video_player_init(void) {
  struct ui_video_player_base player;
  struct ui_component comp;
  ui_error_t err;
  int failed = 0;

  err = ui_video_player_base_init(NULL, &comp, NULL);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_video_player_base_init(&player, NULL, NULL);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_video_player_base_init(&player, &comp, NULL);
  failed |= (err != UI_ERROR_NONE);
  failed |= (player.component != &comp);
  failed |= (player.is_playing != 0);
  failed |= (player.current_time != 0.0f);
  failed |= (player.duration != 0.0f);
  failed |= (player.volume != 1.0f);
  failed |= (player.is_fullscreen != 0);
  return failed;
}

static int test_video_player_operations(void) {
  struct ui_video_player_base player;
  struct ui_component comp;
  int dummy_signal = 0;
  int failed = 0;

  ui_video_player_base_init(&player, &comp, NULL);
  player.duration = 100.0f;

  /* Null checks */
  ui_video_player_base_toggle_play(NULL);
  ui_video_player_base_seek(NULL, 50.0f);
  ui_video_player_base_set_volume(NULL, 0.5f);
  ui_video_player_base_toggle_fullscreen(NULL);

  failed |=
      (ui_video_player_base_bind_src(NULL, (struct ui_signal *)&dummy_signal) !=
       UI_ERROR_INVALID_ARGUMENT);

  failed |= (ui_video_player_base_bind_src(
                 &player, (struct ui_signal *)&dummy_signal) != UI_ERROR_NONE);
  failed |= (player.src_signal != (struct ui_signal *)&dummy_signal);

  ui_video_player_base_toggle_play(&player);
  failed |= (player.is_playing != 1);
  ui_video_player_base_toggle_play(&player);
  failed |= (player.is_playing != 0);

  ui_video_player_base_seek(&player, 50.0f);
  failed |= (player.current_time != 50.0f);

  ui_video_player_base_seek(&player, -10.0f);
  failed |= (player.current_time != 0.0f);

  ui_video_player_base_seek(&player, 200.0f);
  failed |= (player.current_time != 100.0f);

  player.duration = 0.0f;
  ui_video_player_base_seek(&player, 10.0f);
  failed |= (player.current_time != 10.0f);

  ui_video_player_base_set_volume(&player, 0.5f);
  failed |= (player.volume != 0.5f);

  ui_video_player_base_set_volume(&player, -0.5f);
  failed |= (player.volume != 0.0f);

  ui_video_player_base_set_volume(&player, 1.5f);
  failed |= (player.volume != 1.0f);

  ui_video_player_base_toggle_fullscreen(&player);
  failed |= (player.is_fullscreen != 1);
  ui_video_player_base_toggle_fullscreen(&player);
  failed |= (player.is_fullscreen != 0);
  return failed;
}

int main(void) {
  int failed = 0;
  failed |= test_video_player_init();
  failed |= test_video_player_operations();
  return failed;
}
