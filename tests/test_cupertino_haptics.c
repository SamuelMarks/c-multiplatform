/**
 * @file test_cupertino_haptics.c
 * @brief Unit tests for Cupertino Haptic Feedback Generators.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_haptics.h"
/* clang-format on */

SUITE(cupertino_haptics_suite);

TEST test_haptic_impact_styles(void) {
  ui_error_t rc;

  /* Test all valid impact styles */
  rc = cupertino_haptic_impact(CUPERTINO_HAPTIC_IMPACT_LIGHT, 1.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_haptic_impact(CUPERTINO_HAPTIC_IMPACT_MEDIUM, 0.75f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_haptic_impact(CUPERTINO_HAPTIC_IMPACT_HEAVY, 0.5f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_haptic_impact(CUPERTINO_HAPTIC_IMPACT_RIGID, 0.25f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_haptic_impact(CUPERTINO_HAPTIC_IMPACT_SOFT, 0.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Invalid intensity */
  rc = cupertino_haptic_impact(CUPERTINO_HAPTIC_IMPACT_LIGHT, -0.1f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_haptic_impact(CUPERTINO_HAPTIC_IMPACT_LIGHT, 1.1f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Invalid style */
  rc = cupertino_haptic_impact((enum cupertino_haptic_impact_style)999, 0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_haptic_notifications(void) {
  ui_error_t rc;

  /* Test all notification feedback types */
  rc = cupertino_haptic_notification(CUPERTINO_HAPTIC_NOTIFICATION_SUCCESS);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_haptic_notification(CUPERTINO_HAPTIC_NOTIFICATION_WARNING);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_haptic_notification(CUPERTINO_HAPTIC_NOTIFICATION_ERROR);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Invalid notification type */
  rc = cupertino_haptic_notification(
      (enum cupertino_haptic_notification_type)999);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_haptic_selection_and_prepare(void) {
  ui_error_t rc;

  rc = cupertino_haptic_selection();
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_haptic_prepare();
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
TEST test_haptic_mock_errors(void) {
  extern int g_cupertino_haptics_mock_return;
  ui_error_t rc;

  g_cupertino_haptics_mock_return = (int)UI_ERROR_INVALID_ARGUMENT;

  rc = cupertino_haptic_impact(CUPERTINO_HAPTIC_IMPACT_LIGHT, 1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_haptic_notification(CUPERTINO_HAPTIC_NOTIFICATION_SUCCESS);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_haptic_selection();
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  g_cupertino_haptics_mock_return = -1;
  PASS();
}
#endif

SUITE(cupertino_haptics_suite) {
  RUN_TEST(test_haptic_impact_styles);
  RUN_TEST(test_haptic_notifications);
  RUN_TEST(test_haptic_selection_and_prepare);
#ifdef UI_TEST_MOCK_ALLOC
  RUN_TEST(test_haptic_mock_errors);
#endif
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_haptics_suite);
  GREATEST_MAIN_END();
}
