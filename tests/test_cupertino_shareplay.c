/**
 * @file test_cupertino_shareplay.c
 * @brief Unit tests for Cupertino SharePlay Group Activities Control.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_shareplay.h"
#include "ui_avatar_group_base.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_shareplay_suite);

TEST test_shareplay_invalid_arguments(void) {
  struct cupertino_shareplay_descriptor desc;
  struct cupertino_shareplay_control *ctrl = NULL;
  struct ui_avatar_group_base *grp = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  float heights[3];
  float w, h;
  int val = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Creation invalid */
  rc = cupertino_shareplay_create(NULL, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_shareplay_create(dummy_engine, NULL, &ctrl);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_shareplay_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Negative participant count */
  desc.initial_participant_count = -1;
  rc = cupertino_shareplay_create(dummy_engine, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destruction invalid */
  rc = cupertino_shareplay_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Session / participants / tick / heights / action / dimensions / base
   * invalid */
  rc = cupertino_shareplay_set_session_active(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_shareplay_is_session_active(NULL, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_shareplay_set_participant_count(NULL, 5);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_shareplay_get_participant_count(NULL, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_shareplay_tick(NULL, 16.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_shareplay_get_wave_heights(NULL, heights);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_shareplay_trigger_action(NULL,
                                          CUPERTINO_SHAREPLAY_ACTION_LEAVE);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_shareplay_get_dimensions(NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_shareplay_get_avatar_group(NULL, &grp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Create valid control to test secondary null checks */
  desc.initial_participant_count = 1;
  rc = cupertino_shareplay_create(dummy_engine, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, ctrl);

  rc = cupertino_shareplay_is_session_active(ctrl, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_shareplay_set_participant_count(ctrl, -1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_shareplay_get_participant_count(ctrl, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_shareplay_tick(ctrl, -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_shareplay_get_wave_heights(ctrl, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Invalid action values (negative and out of range) */
  rc = cupertino_shareplay_trigger_action(
      ctrl, (enum cupertino_shareplay_action) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_shareplay_trigger_action(ctrl,
                                          (enum cupertino_shareplay_action)99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_shareplay_get_dimensions(ctrl, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_shareplay_get_dimensions(ctrl, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_shareplay_get_avatar_group(ctrl, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_shareplay_destroy(ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_shareplay_lifecycle_and_waves(void) {
  struct cupertino_shareplay_descriptor desc;
  struct cupertino_shareplay_control *ctrl = NULL;
  struct ui_avatar_group_base *grp = NULL;
  struct ui_avatar_group_base *saved_grp = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  float heights[3];
  float w, h;
  int is_active = 0;
  int count = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.initial_participant_count = 3;
  desc.is_session_active = 1;

  rc = cupertino_shareplay_create(dummy_engine, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(ctrl != NULL);

  rc = cupertino_shareplay_get_avatar_group(ctrl, &grp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(grp != NULL);

  rc = cupertino_shareplay_is_session_active(ctrl, &is_active);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_active);

  /* Explicit set_session_active test */
  rc = cupertino_shareplay_set_session_active(ctrl, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_shareplay_set_session_active(ctrl, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_shareplay_is_session_active(ctrl, &is_active);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_active);
  rc = cupertino_shareplay_set_session_active(ctrl, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_shareplay_get_participant_count(ctrl, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, count);

  rc = cupertino_shareplay_get_dimensions(ctrl, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_SHAREPLAY_BASE_WIDTH, w);
  ASSERT_EQ(CUPERTINO_SHAREPLAY_PILL_HEIGHT, h);

  /* Advance tick while active -> wave heights change */
  rc = cupertino_shareplay_tick(ctrl, 200.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_shareplay_get_wave_heights(ctrl, heights);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(heights[0] >= 4.0f);
  ASSERT(heights[1] >= 4.0f);
  ASSERT(heights[2] >= 4.0f);

  /* Set participant count */
  rc = cupertino_shareplay_set_participant_count(ctrl, 5);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_shareplay_get_participant_count(ctrl, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(5, count);

  /* Trigger NONE action (doesn't end session) */
  rc =
      cupertino_shareplay_trigger_action(ctrl, CUPERTINO_SHAREPLAY_ACTION_NONE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_shareplay_is_session_active(ctrl, &is_active);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_active);

  /* Trigger End for Everyone action -> terminates session */
  rc = cupertino_shareplay_trigger_action(
      ctrl, CUPERTINO_SHAREPLAY_ACTION_END_FOR_EVERYONE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_shareplay_is_session_active(ctrl, &is_active);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_active);

  /* Trigger Leave action */
  rc = cupertino_shareplay_set_session_active(ctrl, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_shareplay_trigger_action(ctrl,
                                          CUPERTINO_SHAREPLAY_ACTION_LEAVE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_shareplay_is_session_active(ctrl, &is_active);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_active);

  /* Advance tick while inactive -> waves go flat (4.0) */
  rc = cupertino_shareplay_tick(ctrl, 100.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_shareplay_get_wave_heights(ctrl, heights);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4.0f, heights[0]);
  ASSERT_EQ(4.0f, heights[1]);
  ASSERT_EQ(4.0f, heights[2]);

  /* Test destruction when avatar_group is NULL */
  saved_grp = ctrl->avatar_group;
  ctrl->avatar_group = NULL;
  rc = cupertino_shareplay_destroy(ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ui_avatar_group_base_destroy(saved_grp);

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_cupertino_shareplay_mock_avatar_destroy_fail;
    rc = cupertino_shareplay_create(dummy_engine, &desc, &ctrl);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    g_cupertino_shareplay_mock_avatar_destroy_fail = 1;
    rc = cupertino_shareplay_destroy(ctrl);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_cupertino_shareplay_mock_avatar_destroy_fail = 0;
  }
#endif

  PASS();
}

TEST test_shareplay_oom_mock(void) {
  struct cupertino_shareplay_descriptor desc;
  struct cupertino_shareplay_control *ctrl = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  /* Fail malloc for control */
  g_malloc_fail_countdown = 0;
  rc = cupertino_shareplay_create(dummy_engine, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, ctrl);

  /* Fail malloc for avatar group */
  g_malloc_fail_countdown = 1;
  rc = cupertino_shareplay_create(dummy_engine, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, ctrl);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_shareplay_create(NULL, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_shareplay_suite) {
  RUN_TEST(test_shareplay_invalid_arguments);
  RUN_TEST(test_shareplay_lifecycle_and_waves);
  RUN_TEST(test_shareplay_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_shareplay_suite);
  GREATEST_MAIN_END();
}
