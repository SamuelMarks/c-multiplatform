/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

static void dummy_remote_cb(int cmd, void *u) {
  (void)cmd;
  (void)u;
}

TEST test_media_player_features(void) {
  cmp_media_player_t *ctx = NULL;
  cmp_ui_node_t dummy_node;

  ASSERT_EQ(CMP_SUCCESS, cmp_media_player_create(&ctx));

  /* Source */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_media_player_load_url(ctx, "https://example.com/stream.m3u8"));

  /* HIG: Prefers AVPlayerViewController */
  ASSERT_EQ(CMP_SUCCESS, cmp_media_player_mount(ctx, &dummy_node, 1));

  /* PiP & Spatial Audio */
  ASSERT_EQ(CMP_SUCCESS, cmp_media_player_set_pip_enabled(ctx, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_media_player_set_spatial_audio_enabled(ctx, 1));

  /* Lock Screen metadata */
  ASSERT_EQ(CMP_SUCCESS, cmp_media_player_update_now_playing(ctx, "Episode 5",
                                                             "Podcast Name",
                                                             3600.0f, 120.0f));

  /* Remote Commands */
  ASSERT_EQ(CMP_SUCCESS, cmp_media_player_set_remote_command_handler(
                             ctx, dummy_remote_cb, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_media_player_destroy(ctx));
  PASS();
}

TEST test_audio_session_management(void) {
  cmp_audio_session_t *ctx = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_audio_session_create(&ctx));

  /* Allow playing in background while screen is off (requires PLAYBACK) */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_audio_session_set_category(ctx, CMP_AUDIO_SESSION_PLAYBACK));
  ASSERT_EQ(CMP_SUCCESS, cmp_audio_session_activate(ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_audio_session_destroy(ctx));
  PASS();
}

TEST test_null_args(void) {
  cmp_media_player_t *mp = NULL;
  cmp_audio_session_t *as = NULL;
  cmp_ui_node_t node;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_media_player_create(NULL));
  cmp_media_player_create(&mp);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_media_player_load_url(NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_media_player_load_url(mp, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_media_player_mount(NULL, &node, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_media_player_mount(mp, NULL, 1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_media_player_set_pip_enabled(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_media_player_set_spatial_audio_enabled(NULL, 1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_media_player_update_now_playing(NULL, "t", "a", 0.0f, 0.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_media_player_update_now_playing(mp, NULL, "a", 0.0f, 0.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_media_player_update_now_playing(mp, "t", NULL, 0.0f, 0.0f));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_media_player_set_remote_command_handler(
                                       NULL, dummy_remote_cb, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_audio_session_create(NULL));
  cmp_audio_session_create(&as);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_audio_session_set_category(NULL, CMP_AUDIO_SESSION_AMBIENT));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_audio_session_activate(NULL));

  cmp_media_player_destroy(mp);
  cmp_audio_session_destroy(as);
  PASS();
}

SUITE(media_avkit_suite) {
  RUN_TEST(test_media_player_features);
  RUN_TEST(test_audio_session_management);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(media_avkit_suite);
  GREATEST_MAIN_END();
}
