/**
 * @file test_cupertino_refresh.c
 * @brief Unit tests for Cupertino Pull-To-Refresh component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_refresh.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_refresh_suite);

static int g_refresh_invoked = 0;

static ui_error_t mock_on_refresh_cb(struct ui_pull_to_refresh_base *ptr,
                                     void *user_data) {
  int *flag = (int *)user_data;
  if (flag) {
    *flag = 1;
  }
  return UI_ERROR_NONE;
}

TEST test_refresh_invalid_args(void) {
  struct cupertino_refresh_descriptor desc;
  struct cupertino_refresh *refresh = NULL;
  struct ui_pull_to_refresh_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum ui_pull_to_refresh_state state;
  float progress = 0.0f;
  int spoke = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Create invalid */
  rc = cupertino_refresh_create(NULL, &desc, &refresh);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_refresh_create(dummy_engine, NULL, &refresh);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_refresh_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy invalid */
  rc = cupertino_refresh_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Pull / release / complete */
  rc = cupertino_refresh_handle_pull(NULL, 10.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_refresh_release(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_refresh_complete(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Getters */
  rc = cupertino_refresh_get_progress(NULL, &progress);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_refresh_get_active_spoke(NULL, &spoke);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_refresh_get_state(NULL, &state);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_refresh_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid refresh to test null out pointers, pull distance <= 0, and base ==
   * NULL */
  rc = cupertino_refresh_create(dummy_engine, &desc, &refresh);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_refresh_get_progress(refresh, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_refresh_get_active_spoke(refresh, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_refresh_get_state(refresh, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_refresh_get_base(refresh, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Negative pull distance resets to 0 */
  rc = cupertino_refresh_handle_pull(refresh, -10.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_PULL_TO_REFRESH_RESTING, refresh->state);

  /* Destroy with refresh->base == NULL */
  struct ui_pull_to_refresh_base *saved_b = refresh->base;
  refresh->base = NULL;
  rc = cupertino_refresh_complete(refresh);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_refresh_destroy(refresh);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ui_pull_to_refresh_base_destroy(saved_b);

  PASS();
}

TEST test_refresh_lifecycle_and_gestures(void) {
  struct cupertino_refresh_descriptor desc;
  struct cupertino_refresh *refresh = NULL;
  struct ui_pull_to_refresh_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum ui_pull_to_refresh_state state;
  float progress = 0.0f;
  int spoke = 0;
  ui_error_t rc;

  g_refresh_invoked = 0;
  memset(&desc, 0, sizeof(desc));
  desc.trigger_distance = 60.0f;
  desc.resting_distance = 44.0f;
  desc.on_refresh = mock_on_refresh_cb;
  desc.user_data = &g_refresh_invoked;

  rc = cupertino_refresh_create(dummy_engine, &desc, &refresh);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(refresh != NULL);

  /* Check base */
  rc = cupertino_refresh_get_base(refresh, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Initial state: RESTING, progress 0 */
  rc = cupertino_refresh_get_state(refresh, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_PULL_TO_REFRESH_RESTING, state);

  rc = cupertino_refresh_get_progress(refresh, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(progress < 0.001f);

  /* Pull halfway: 30pt out of 60pt -> progress = 0.5, spoke = 4 */
  rc = cupertino_refresh_handle_pull(refresh, 30.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_refresh_get_state(refresh, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_PULL_TO_REFRESH_PULLING, state);

  rc = cupertino_refresh_get_progress(refresh, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(progress > 0.49f && progress < 0.51f);

  rc = cupertino_refresh_get_active_spoke(refresh, &spoke);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4, spoke);

  /* Release below threshold -> snaps back to RESTING */
  rc = cupertino_refresh_release(refresh);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_refresh_get_state(refresh, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_PULL_TO_REFRESH_RESTING, state);
  rc = cupertino_refresh_get_progress(refresh, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(progress < 0.001f);

  /* Pull past threshold: 75pt >= 60pt -> progress = 1.0, spoke = 7 */
  rc = cupertino_refresh_handle_pull(refresh, 75.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_refresh_get_progress(refresh, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(progress > 0.999f);
  rc = cupertino_refresh_get_active_spoke(refresh, &spoke);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(7, spoke);

  /* Release past threshold -> snaps to REFRESHING and holds at resting_distance
   * (44pt) */
  rc = cupertino_refresh_release(refresh);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_refresh_get_state(refresh, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_PULL_TO_REFRESH_REFRESHING, state);
  ASSERT(refresh->current_pull_distance > 43.9f &&
         refresh->current_pull_distance < 44.1f);

  /* Complete refresh -> returns to RESTING */
  rc = cupertino_refresh_complete(refresh);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_refresh_get_state(refresh, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_PULL_TO_REFRESH_RESTING, state);

  /* Clean up */
  rc = cupertino_refresh_destroy(refresh);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_refresh_oom_mock(void) {
  struct cupertino_refresh_descriptor desc;
  struct cupertino_refresh *refresh = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_refresh_mock_create_fail;
  extern int g_cupertino_refresh_mock_destroy_fail;
  extern int g_cupertino_refresh_mock_set_cb_fail;
  extern int g_cupertino_refresh_mock_complete_fail;

  /* Fail malloc for struct cupertino_refresh */
  g_malloc_fail_countdown = 0;
  rc = cupertino_refresh_create(dummy_engine, &desc, &refresh);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(refresh == NULL);

  /* Fail malloc for ui_pull_to_refresh_base */
  g_malloc_fail_countdown = 1;
  rc = cupertino_refresh_create(dummy_engine, &desc, &refresh);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(refresh == NULL);
  g_malloc_fail_countdown = -1;

  /* Fail mock create */
  g_cupertino_refresh_mock_create_fail = 1;
  rc = cupertino_refresh_create(dummy_engine, &desc, &refresh);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(refresh == NULL);
  g_cupertino_refresh_mock_create_fail = 0;

  /* Fail mock set_cb */
  desc.on_refresh = mock_on_refresh_cb;
  g_cupertino_refresh_mock_set_cb_fail = 1;
  rc = cupertino_refresh_create(dummy_engine, &desc, &refresh);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT(refresh == NULL);

  /* Fail mock set_cb with mock destroy failure */
  g_cupertino_refresh_mock_destroy_fail = 1;
  rc = cupertino_refresh_create(dummy_engine, &desc, &refresh);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT(refresh == NULL);
  g_cupertino_refresh_mock_set_cb_fail = 0;
  g_cupertino_refresh_mock_destroy_fail = 0;

  /* Fail mock destroy in cupertino_refresh_destroy */
  rc = cupertino_refresh_create(dummy_engine, &desc, &refresh);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_refresh_mock_destroy_fail = 1;
  rc = cupertino_refresh_destroy(refresh);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_refresh_mock_destroy_fail = 0;

  /* Complete with mock complete failure */
  rc = cupertino_refresh_create(dummy_engine, &desc, &refresh);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_refresh_mock_complete_fail = 1;
  rc = cupertino_refresh_complete(refresh);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_refresh_mock_complete_fail = 0;

  /* Pull while REFRESHING */
  refresh->state = UI_PULL_TO_REFRESH_REFRESHING;
  rc = cupertino_refresh_handle_pull(refresh, 50.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_refresh_destroy(refresh);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#else
  rc = cupertino_refresh_create(NULL, &desc, &refresh);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_refresh_suite) {
  RUN_TEST(test_refresh_invalid_args);
  RUN_TEST(test_refresh_lifecycle_and_gestures);
  RUN_TEST(test_refresh_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_refresh_suite);
  GREATEST_MAIN_END();
}
