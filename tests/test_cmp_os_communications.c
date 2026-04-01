/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_os_communications_intents(void) {
  cmp_os_communications_t *ctx = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_os_communications_create(&ctx));
  ASSERT_EQ(CMP_SUCCESS, cmp_os_communications_register_intent(
                             ctx, "PlayPlaylist", "Play Music Playlist"));
  ASSERT_EQ(CMP_SUCCESS, cmp_os_communications_destroy(ctx));

  PASS();
}

TEST test_os_communications_handoff_spotlight(void) {
  cmp_os_communications_t *ctx = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_os_communications_create(&ctx));

  /* Handoff test */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_os_communications_broadcast_handoff(
                ctx, "com.app.reading", "\"{\"docId\":\"doc_1\"}\""));

  /* Spotlight Index */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_os_communications_index_for_spotlight(
                ctx, "doc_1", "A Great Novel", "A very cool story."));

  ASSERT_EQ(CMP_SUCCESS, cmp_os_communications_destroy(ctx));

  PASS();
}

TEST test_os_communications_focus_shareplay(void) {
  cmp_os_communications_t *ctx = NULL;
  int is_suppressed;
  cmp_window_t *dummy_win = (cmp_window_t *)1;

  ASSERT_EQ(CMP_SUCCESS, cmp_os_communications_create(&ctx));

  /* Focus Modes */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_os_communications_evaluate_focus_mode(ctx, &is_suppressed));
  ASSERT_EQ(0, is_suppressed);

  /* Shareplay */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_os_communications_start_shareplay(ctx, "com.app.watch_video"));

  /* Share Sheet */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_os_communications_show_share_sheet(
                ctx, dummy_win, "https://example.com", "Check this out"));

  ASSERT_EQ(CMP_SUCCESS, cmp_os_communications_destroy(ctx));

  PASS();
}

TEST test_os_communications_null_args(void) {
  cmp_os_communications_t *ctx = NULL;
  cmp_window_t *dummy = (cmp_window_t *)1;
  int i;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_os_communications_create(NULL));
  cmp_os_communications_create(&ctx);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_communications_register_intent(NULL, "a", "b"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_communications_register_intent(ctx, NULL, "b"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_communications_register_intent(ctx, "a", NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_communications_show_share_sheet(NULL, dummy, "a", "b"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_communications_show_share_sheet(ctx, NULL, "a", "b"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_communications_show_share_sheet(ctx, dummy, NULL,
                                                   NULL)); /* Both null fails */

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_communications_broadcast_handoff(NULL, "a", "b"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_communications_broadcast_handoff(ctx, NULL, "b"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_communications_broadcast_handoff(ctx, "a", NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_communications_index_for_spotlight(NULL, "a", "b", "c"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_communications_index_for_spotlight(ctx, NULL, "b", "c"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_communications_index_for_spotlight(ctx, "a", NULL, "c"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_communications_index_for_spotlight(ctx, "a", "b", NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_communications_evaluate_focus_mode(NULL, &i));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_communications_evaluate_focus_mode(ctx, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_communications_start_shareplay(NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_communications_start_shareplay(ctx, NULL));

  cmp_os_communications_destroy(ctx);
  PASS();
}

SUITE(os_communications_suite) {
  RUN_TEST(test_os_communications_intents);
  RUN_TEST(test_os_communications_handoff_spotlight);
  RUN_TEST(test_os_communications_focus_shareplay);
  RUN_TEST(test_os_communications_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(os_communications_suite);
  GREATEST_MAIN_END();
}
