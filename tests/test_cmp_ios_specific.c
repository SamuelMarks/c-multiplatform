/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_ios_keyboard_avoidance(void) {
  float adjust;

  /* Input at Y=800, Screen=1000, Kbd=300 (Visible bottom=700).
     Overlap is 800 - 700 + 16pt margin = 116pt adjustment */
  ASSERT_EQ(CMP_SUCCESS, cmp_ios_calculate_keyboard_avoidance(
                             300.0f, 800.0f, 1000.0f, &adjust));
  ASSERT_EQ(116.0f, adjust);

  /* Input at Y=400, Screen=1000, Kbd=300. No overlap. */
  ASSERT_EQ(CMP_SUCCESS, cmp_ios_calculate_keyboard_avoidance(
                             300.0f, 400.0f, 1000.0f, &adjust));
  ASSERT_EQ(0.0f, adjust);

  PASS();
}

TEST test_ios_pull_to_refresh(void) {
  float opacity;
  int trigger;

  /* No pull */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ios_evaluate_pull_to_refresh(50.0f, &opacity, &trigger));
  ASSERT_EQ(0.0f, opacity);
  ASSERT_EQ(0, trigger);

  /* Half pull (-50pts out of -100pts target) */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ios_evaluate_pull_to_refresh(-50.0f, &opacity, &trigger));
  ASSERT_EQ(0.5f, opacity);
  ASSERT_EQ(0, trigger);

  /* Full pull past threshold */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ios_evaluate_pull_to_refresh(-120.0f, &opacity, &trigger));
  ASSERT_EQ(1.0f, opacity);
  ASSERT_EQ(1, trigger);

  PASS();
}

TEST test_ios_sheet_detents(void) {
  cmp_sheet_detent_t detent;

  /* Swipe up near top (0) -> Large */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ios_evaluate_sheet_detent_snap(100.0f, 1000.0f, &detent));
  ASSERT_EQ(CMP_SHEET_DETENT_LARGE, detent);

  /* Swipe down near middle (500) -> Medium */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ios_evaluate_sheet_detent_snap(600.0f, 1000.0f, &detent));
  ASSERT_EQ(CMP_SHEET_DETENT_MEDIUM, detent);

  PASS();
}

TEST test_ios_reachability(void) {
  float touch_y = 600.0f; /* Physical screen touch */

  /* OS pushes view down 400pts */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ios_mitigate_reachability_offset(&touch_y, 400.0f));

  /* Logical touch hitting the UI should be 200 */
  ASSERT_EQ(200.0f, touch_y);

  PASS();
}

TEST test_ios_context_menu_peek(void) {
  float scale;

  /* Light press */
  ASSERT_EQ(CMP_SUCCESS, cmp_ios_evaluate_context_menu_peek(0.2f, &scale));
  /* Due to floating point precision */
  ASSERT_GT(scale, 0.63f);
  ASSERT_LT(scale, 0.65f); /* 0.6 + (0.2 * 0.2) */

  /* Hard press */
  ASSERT_EQ(CMP_SUCCESS, cmp_ios_evaluate_context_menu_peek(1.0f, &scale));
  ASSERT_EQ(0.8f, scale);

  PASS();
}

TEST test_ios_null_args(void) {
  float f;
  int i;
  cmp_sheet_detent_t d;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ios_calculate_keyboard_avoidance(-1.0f, 0.0f, 1.0f, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ios_calculate_keyboard_avoidance(0.0f, -1.0f, 1.0f, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ios_calculate_keyboard_avoidance(0.0f, 0.0f, 0.0f, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ios_calculate_keyboard_avoidance(0.0f, 0.0f, 1.0f, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ios_evaluate_pull_to_refresh(0.0f, NULL, &i));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ios_evaluate_pull_to_refresh(0.0f, &f, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ios_evaluate_sheet_detent_snap(0.0f, 0.0f, &d));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ios_evaluate_sheet_detent_snap(0.0f, 100.0f, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ios_mitigate_reachability_offset(NULL, 10.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ios_mitigate_reachability_offset(&f, -1.0f));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ios_evaluate_context_menu_peek(-0.1f, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ios_evaluate_context_menu_peek(1.1f, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ios_evaluate_context_menu_peek(0.5f, NULL));

  PASS();
}

SUITE(ios_specific_suite) {
  RUN_TEST(test_ios_keyboard_avoidance);
  RUN_TEST(test_ios_pull_to_refresh);
  RUN_TEST(test_ios_sheet_detents);
  RUN_TEST(test_ios_reachability);
  RUN_TEST(test_ios_context_menu_peek);
  RUN_TEST(test_ios_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ios_specific_suite);
  GREATEST_MAIN_END();
}
