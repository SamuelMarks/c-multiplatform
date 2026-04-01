/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_testing_automation_features(void) {
  cmp_testing_automation_t *ctx = NULL;
  int dummy_node = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_testing_automation_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_testing_set_animations_enabled(ctx, 0));
  ASSERT_EQ(CMP_SUCCESS, cmp_testing_tap_by_accessibility_label(ctx, "Submit"));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_testing_snapshot_verify(ctx, &dummy_node, "button_dark_rtl"));
  ASSERT_EQ(CMP_SUCCESS, cmp_testing_audit_accessibility(ctx, &dummy_node));

  ASSERT_EQ(CMP_SUCCESS, cmp_testing_automation_destroy(ctx));
  PASS();
}

TEST test_testing_automation_null_args(void) {
  cmp_testing_automation_t *feat = NULL;
  int node = 0;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_testing_automation_create(NULL));
  cmp_testing_automation_create(&feat);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_testing_set_animations_enabled(NULL, 1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_testing_tap_by_accessibility_label(NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_testing_tap_by_accessibility_label(feat, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_testing_snapshot_verify(NULL, &node, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_testing_snapshot_verify(feat, NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_testing_snapshot_verify(feat, &node, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_testing_audit_accessibility(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_testing_audit_accessibility(feat, NULL));

  cmp_testing_automation_destroy(feat);
  PASS();
}

SUITE(testing_automation_suite) {
  RUN_TEST(test_testing_automation_features);
  RUN_TEST(test_testing_automation_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(testing_automation_suite);
  GREATEST_MAIN_END();
}
