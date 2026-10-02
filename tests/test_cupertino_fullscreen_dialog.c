/**
 * @file test_cupertino_fullscreen_dialog.c
 * @brief Unit tests for Cupertino Fullscreen Dialog Transition conforming to
 * Apple HIG.
 */

/* clang-format off */
#include "cupertino/cupertino_fullscreen_dialog.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
extern int g_cupertino_dialog_mock_spring_preset_fail;
extern int g_cupertino_dialog_mock_spring_eval_fail;
extern int g_cupertino_dialog_mock_update_transform_fail;
extern ui_error_t test_cupertino_fullscreen_dialog_update_parent_transform(
    struct cupertino_fullscreen_dialog *dialog);

TEST test_fullscreen_dialog_lifecycle_and_defaults(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_fullscreen_dialog_descriptor desc;
  struct cupertino_fullscreen_dialog *dialog = NULL;
  ui_error_t rc;
  char title_buf[32];
  float ty, scale, radius, dim;
  int is_settled = 0;

  memset(&desc, 0, sizeof(desc));
  desc.screen_height = 800.0f;
  desc.action_type = CUPERTINO_DIALOG_NAV_ACTION_CANCEL;
  desc.swipe_to_dismiss_enabled = 1;

  /* Invalid argument checks */
  rc = cupertino_fullscreen_dialog_create(NULL, &desc, &dialog);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_fullscreen_dialog_create(dummy_engine, NULL, &dialog);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_fullscreen_dialog_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = cupertino_fullscreen_dialog_create(dummy_engine, &desc, &dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, dialog);
  ASSERT_EQ(CUPERTINO_DIALOG_STATE_DISMISSED, dialog->state);

  /* Title should be "Cancel" */
  rc = cupertino_fullscreen_dialog_get_nav_action_title(dialog, title_buf,
                                                        sizeof(title_buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Cancel", title_buf);

  /* Initial translation at bottom: screen_height (800) */
  rc = cupertino_fullscreen_dialog_get_translation_y(dialog, &ty);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(800.0f, ty, 0.01f);

  /* Parent transform in dismissed state (progress 0) */
  rc = cupertino_fullscreen_dialog_get_parent_transform(dialog, &scale, &radius,
                                                        &dim);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(1.0f, scale, 0.01f);
  ASSERT_IN_RANGE(0.0f, radius, 0.01f);
  ASSERT_IN_RANGE(0.0f, dim, 0.01f);

  /* Present dialog */
  rc = cupertino_fullscreen_dialog_present(dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_DIALOG_STATE_PRESENTING, dialog->state);

  /* Advance animation frames */
  while (!is_settled) {
    rc = cupertino_fullscreen_dialog_tick(dialog, 0.016f, &is_settled);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  ASSERT_EQ(CUPERTINO_DIALOG_STATE_PRESENTED, dialog->state);
  rc = cupertino_fullscreen_dialog_get_translation_y(dialog, &ty);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.0f, ty, 0.01f);

  /* Parent should be scaled down to 0.92, corner radius 12pt, dim alpha 0.25 */
  rc = cupertino_fullscreen_dialog_get_parent_transform(dialog, &scale, &radius,
                                                        &dim);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.92f, scale, 0.01f);
  ASSERT_IN_RANGE(12.0f, radius, 0.01f);
  ASSERT_IN_RANGE(0.25f, dim, 0.01f);

  /* Dismiss dialog */
  rc = cupertino_fullscreen_dialog_dismiss(dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_DIALOG_STATE_DISMISSING, dialog->state);
  ASSERT_EQ(1, dialog->dismiss_invoked_count);

  is_settled = 0;
  while (!is_settled) {
    rc = cupertino_fullscreen_dialog_tick(dialog, 0.016f, &is_settled);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  ASSERT_EQ(CUPERTINO_DIALOG_STATE_DISMISSED, dialog->state);
  rc = cupertino_fullscreen_dialog_get_translation_y(dialog, &ty);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(800.0f, ty, 0.01f);

  /* Branch tests for error conditions on present/dismiss/tick */
  rc = cupertino_fullscreen_dialog_present(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_fullscreen_dialog_dismiss(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_fullscreen_dialog_tick(NULL, 0.016f, &is_settled);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_fullscreen_dialog_tick(dialog, -1.0f, &is_settled);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_fullscreen_dialog_tick(dialog, 0.016f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Branch tests on get_translation_y, get_parent_transform,
   * get_nav_action_title */
  rc = cupertino_fullscreen_dialog_get_translation_y(NULL, &ty);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_fullscreen_dialog_get_translation_y(dialog, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_fullscreen_dialog_get_parent_transform(NULL, &scale, &radius,
                                                        &dim);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_fullscreen_dialog_get_parent_transform(dialog, NULL, &radius,
                                                        &dim);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_fullscreen_dialog_get_parent_transform(dialog, &scale, NULL,
                                                        &dim);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_fullscreen_dialog_get_parent_transform(dialog, &scale, &radius,
                                                        NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_fullscreen_dialog_get_nav_action_title(NULL, title_buf,
                                                        sizeof(title_buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_fullscreen_dialog_get_nav_action_title(dialog, NULL,
                                                        sizeof(title_buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_fullscreen_dialog_get_nav_action_title(dialog, title_buf, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_fullscreen_dialog_destroy(dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_fullscreen_dialog_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_fullscreen_dialog_action_titles(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_fullscreen_dialog_descriptor desc;
  struct cupertino_fullscreen_dialog *dialog = NULL;
  ui_error_t rc;
  char title_buf[32];

  /* Test Done action */
  memset(&desc, 0, sizeof(desc));
  desc.action_type = CUPERTINO_DIALOG_NAV_ACTION_DONE;
  rc = cupertino_fullscreen_dialog_create(dummy_engine, &desc, &dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_fullscreen_dialog_get_nav_action_title(dialog, title_buf,
                                                        sizeof(title_buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Done", title_buf);
  rc = cupertino_fullscreen_dialog_destroy(dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test Custom action */
  memset(&desc, 0, sizeof(desc));
  desc.action_type = CUPERTINO_DIALOG_NAV_ACTION_CUSTOM;
  desc.action_title = "Close";
  rc = cupertino_fullscreen_dialog_create(dummy_engine, &desc, &dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_fullscreen_dialog_get_nav_action_title(dialog, title_buf,
                                                        sizeof(title_buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Close", title_buf);
  rc = cupertino_fullscreen_dialog_destroy(dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_fullscreen_dialog_interactive_drag(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_fullscreen_dialog_descriptor desc;
  struct cupertino_fullscreen_dialog *dialog = NULL;
  ui_error_t rc;
  int is_settled = 0;
  int will_dismiss = 0;
  float ty;

  memset(&desc, 0, sizeof(desc));
  desc.screen_height = 1000.0f;
  desc.action_type = CUPERTINO_DIALOG_NAV_ACTION_CANCEL;
  desc.swipe_to_dismiss_enabled = 1;
  desc.dismiss_distance_threshold = 0.4f; /* 400pt */
  desc.dismiss_velocity_threshold = 500.0f;

  rc = cupertino_fullscreen_dialog_create(dummy_engine, &desc, &dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks on drag APIs */
  rc = cupertino_fullscreen_dialog_drag_start(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_fullscreen_dialog_drag_update(NULL, 10.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_fullscreen_dialog_drag_end(NULL, 10.0f, &will_dismiss);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_fullscreen_dialog_drag_end(dialog, 10.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Cannot drag while dismissed */
  rc = cupertino_fullscreen_dialog_drag_start(dialog);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_fullscreen_dialog_drag_update(dialog, 10.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_fullscreen_dialog_drag_end(dialog, 10.0f, &will_dismiss);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Present and tick to fully presented */
  rc = cupertino_fullscreen_dialog_present(dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  while (!is_settled) {
    rc = cupertino_fullscreen_dialog_tick(dialog, 0.016f, &is_settled);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Set views check */
  rc = cupertino_fullscreen_dialog_set_views(dialog, NULL, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_fullscreen_dialog_set_views(NULL, NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Start drag */
  rc = cupertino_fullscreen_dialog_drag_start(dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_DIALOG_STATE_DRAGGING, dialog->state);

  /* Drag downward negative delta (should clamp to 0) */
  rc = cupertino_fullscreen_dialog_drag_update(dialog, -50.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Drag downward 100pt */
  rc = cupertino_fullscreen_dialog_drag_update(dialog, 100.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_fullscreen_dialog_get_translation_y(dialog, &ty);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(100.0f, ty, 0.01f);

  /* End drag with low velocity and below distance threshold -> cancel dismiss
   */
  rc = cupertino_fullscreen_dialog_drag_end(dialog, 50.0f, &will_dismiss);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, will_dismiss);
  ASSERT_EQ(CUPERTINO_DIALOG_STATE_PRESENTING, dialog->state);

  /* Settle back to presented */
  is_settled = 0;
  while (!is_settled) {
    rc = cupertino_fullscreen_dialog_tick(dialog, 0.016f, &is_settled);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  ASSERT_EQ(CUPERTINO_DIALOG_STATE_PRESENTED, dialog->state);

  /* Drag again past distance threshold -> dismiss */
  rc = cupertino_fullscreen_dialog_drag_start(dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_fullscreen_dialog_drag_update(dialog, 450.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_fullscreen_dialog_drag_end(dialog, 100.0f, &will_dismiss);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, will_dismiss);
  ASSERT_EQ(CUPERTINO_DIALOG_STATE_DISMISSING, dialog->state);

  is_settled = 0;
  while (!is_settled) {
    rc = cupertino_fullscreen_dialog_tick(dialog, 0.016f, &is_settled);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  ASSERT_EQ(CUPERTINO_DIALOG_STATE_DISMISSED, dialog->state);

  rc = cupertino_fullscreen_dialog_destroy(dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_fullscreen_dialog_oom(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_fullscreen_dialog_descriptor desc;
  struct cupertino_fullscreen_dialog *dialog = NULL;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  g_malloc_fail_countdown = 0;
  rc = cupertino_fullscreen_dialog_create(dummy_engine, &desc, &dialog);
  g_malloc_fail_countdown = -1;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, dialog);

  PASS();
}

TEST test_fullscreen_dialog_coverage_branches(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_fullscreen_dialog_descriptor desc;
  struct cupertino_fullscreen_dialog *dialog = NULL;
  ui_error_t rc;
  int is_settled = 0;
  int will_dismiss = 0;

  memset(&desc, 0, sizeof(desc));
  desc.action_type = CUPERTINO_DIALOG_NAV_ACTION_CUSTOM;
  desc.action_title = NULL; /* covers custom with null title branch */

  rc = cupertino_fullscreen_dialog_create(dummy_engine, &desc, &dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_fullscreen_dialog_destroy(dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* spring_get_preset failure during create */
  g_cupertino_dialog_mock_spring_preset_fail = 1;
  rc = cupertino_fullscreen_dialog_create(dummy_engine, &desc, &dialog);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_dialog_mock_spring_preset_fail = 0;

  /* update_parent_transform failure during create */
  g_cupertino_dialog_mock_update_transform_fail = 1;
  rc = cupertino_fullscreen_dialog_create(dummy_engine, &desc, &dialog);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_dialog_mock_update_transform_fail = 0;

  /* Create valid dialog */
  desc.action_type = CUPERTINO_DIALOG_NAV_ACTION_CANCEL;
  desc.swipe_to_dismiss_enabled = 1;
  rc = cupertino_fullscreen_dialog_create(dummy_engine, &desc, &dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* spring_get_preset failure during present */
  g_cupertino_dialog_mock_spring_preset_fail = 1;
  rc = cupertino_fullscreen_dialog_present(dialog);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_dialog_mock_spring_preset_fail = 0;

  /* spring_get_preset failure during dismiss */
  g_cupertino_dialog_mock_spring_preset_fail = 1;
  rc = cupertino_fullscreen_dialog_dismiss(dialog);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_dialog_mock_spring_preset_fail = 0;

  /* tick in DISMISSED state -> settled = 1 */
  dialog->state = CUPERTINO_DIALOG_STATE_DISMISSED;
  rc = cupertino_fullscreen_dialog_tick(dialog, 0.016f, &is_settled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_settled);

  /* update_parent_transform failure during tick */
  g_cupertino_dialog_mock_update_transform_fail = 1;
  rc = cupertino_fullscreen_dialog_tick(dialog, 0.016f, &is_settled);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_dialog_mock_update_transform_fail = 0;

  /* present and tick with spring evaluate failure */
  rc = cupertino_fullscreen_dialog_present(dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_dialog_mock_spring_eval_fail = 1;
  rc = cupertino_fullscreen_dialog_tick(dialog, 0.016f, &is_settled);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_dialog_mock_spring_eval_fail = 0;

  /* tick until elapsed >= 0.5s timeout path in PRESENTING */
  rc = cupertino_fullscreen_dialog_tick(dialog, 0.6f, &is_settled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_settled);
  ASSERT_EQ(CUPERTINO_DIALOG_STATE_PRESENTED, dialog->state);

  /* dismiss and tick with spring evaluate failure */
  rc = cupertino_fullscreen_dialog_dismiss(dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_dialog_mock_spring_eval_fail = 1;
  rc = cupertino_fullscreen_dialog_tick(dialog, 0.016f, &is_settled);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_dialog_mock_spring_eval_fail = 0;

  /* tick until elapsed > 0.1s and evaluate produces pos <= 0.001f in DISMISSING
   */
  rc = cupertino_fullscreen_dialog_dismiss(dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  dialog->spring_cfg.initial_position = 0.0005f;
  dialog->spring_cfg.target_position = 0.0f;
  rc = cupertino_fullscreen_dialog_tick(dialog, 0.2f, &is_settled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_settled);
  ASSERT_EQ(CUPERTINO_DIALOG_STATE_DISMISSED, dialog->state);

  /* tick until elapsed >= 0.5s timeout path in DISMISSING */
  rc = cupertino_fullscreen_dialog_tick(dialog, 0.6f, &is_settled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_settled);
  ASSERT_EQ(CUPERTINO_DIALOG_STATE_DISMISSED, dialog->state);

  /* swipe_to_dismiss_enabled == 0 check */
  dialog->swipe_to_dismiss_enabled = 0;
  dialog->state = CUPERTINO_DIALOG_STATE_PRESENTED;
  rc = cupertino_fullscreen_dialog_drag_start(dialog);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  dialog->swipe_to_dismiss_enabled = 1;

  /* drag_start with valid state */
  rc = cupertino_fullscreen_dialog_drag_start(dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* drag_update with delta_y causing p < 0 (i.e. delta_y > screen_height) */
  rc = cupertino_fullscreen_dialog_drag_update(dialog,
                                               dialog->screen_height * 2.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, dialog->current_progress);

  /* drag_update with delta_y causing p > 1 (should not happen with clamped
   * delta_y, but delta_y=0 gives p=1) */
  rc = cupertino_fullscreen_dialog_drag_update(dialog, 0.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1.0f, dialog->current_progress);

  /* drag_update when update_parent_transform fails */
  g_cupertino_dialog_mock_update_transform_fail = 1;
  rc = cupertino_fullscreen_dialog_drag_update(dialog, 10.0f);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_dialog_mock_update_transform_fail = 0;

  /* drag_end with velocity dismiss threshold met */
  rc = cupertino_fullscreen_dialog_drag_end(dialog, 1000.0f, &will_dismiss);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, will_dismiss);

  /* drag_end when dismiss fails */
  dialog->state = CUPERTINO_DIALOG_STATE_DRAGGING;
  g_cupertino_dialog_mock_spring_preset_fail = 1;
  rc = cupertino_fullscreen_dialog_drag_end(dialog, 1000.0f, &will_dismiss);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_dialog_mock_spring_preset_fail = 0;

  /* drag_end when present fails */
  dialog->state = CUPERTINO_DIALOG_STATE_DRAGGING;
  g_cupertino_dialog_mock_spring_preset_fail = 1;
  rc = cupertino_fullscreen_dialog_drag_end(dialog, 0.0f, &will_dismiss);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_dialog_mock_spring_preset_fail = 0;

  /* direct tests for update_parent_transform: null, p < 0, p > 1 */
  rc = test_cupertino_fullscreen_dialog_update_parent_transform(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  dialog->current_progress = -0.5f;
  rc = test_cupertino_fullscreen_dialog_update_parent_transform(dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1.0f, dialog->parent_scale);

  dialog->current_progress = 1.5f;
  rc = test_cupertino_fullscreen_dialog_update_parent_transform(dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.92f, dialog->parent_scale);

  dialog->current_progress = 0.5f;
  rc = test_cupertino_fullscreen_dialog_update_parent_transform(dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_fullscreen_dialog_destroy(dialog);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(cupertino_fullscreen_dialog_suite) {
  RUN_TEST(test_fullscreen_dialog_lifecycle_and_defaults);
  RUN_TEST(test_fullscreen_dialog_action_titles);
  RUN_TEST(test_fullscreen_dialog_interactive_drag);
  RUN_TEST(test_fullscreen_dialog_oom);
  RUN_TEST(test_fullscreen_dialog_coverage_branches);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_fullscreen_dialog_suite);
  GREATEST_MAIN_END();
}
