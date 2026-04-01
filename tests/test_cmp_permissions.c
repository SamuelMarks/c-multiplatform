/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_permissions_lifecycle(void) {
  cmp_permissions_t *ctx = NULL;
  cmp_permission_status_t status;
  int res;

  res = cmp_permissions_create(&ctx);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_permissions_get_status(ctx, CMP_PERMISSION_LOCATION, &status);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_PERMISSION_STATUS_NOT_DETERMINED, status);

  res = cmp_permissions_request(ctx, CMP_PERMISSION_LOCATION);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_permissions_get_status(ctx, CMP_PERMISSION_LOCATION, &status);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_PERMISSION_STATUS_AUTHORIZED_APPROXIMATE,
            status); /* iOS 14 strict defaults */

  /* Test other defaults */
  res = cmp_permissions_request(ctx, CMP_PERMISSION_PHOTO_LIBRARY);
  ASSERT_EQ(CMP_SUCCESS, res);
  res = cmp_permissions_get_status(ctx, CMP_PERMISSION_PHOTO_LIBRARY, &status);
  ASSERT_EQ(CMP_PERMISSION_STATUS_LIMITED, status);

  res = cmp_permissions_request(ctx, CMP_PERMISSION_APP_TRACKING);
  ASSERT_EQ(CMP_SUCCESS, res);
  res = cmp_permissions_get_status(ctx, CMP_PERMISSION_APP_TRACKING, &status);
  ASSERT_EQ(CMP_PERMISSION_STATUS_DENIED, status); /* ATT prompt rejected */

  res = cmp_permissions_destroy(ctx);
  ASSERT_EQ(CMP_SUCCESS, res);

  PASS();
}

TEST test_privacy_indicators(void) {
  cmp_privacy_indicators_t *inds = NULL;
  cmp_rect_t safe_ui = {0.0f, 100.0f, 200.0f,
                        50.0f}; /* Below the dynamic island */
  cmp_rect_t bad_ui = {290.0f, 0.0f, 50.0f,
                       50.0f}; /* Obscuring the system mic dot (300x10) */
  int is_obscured;
  int res;

  res = cmp_privacy_indicators_create(&inds);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_privacy_indicators_verify_layout(inds, &safe_ui, &is_obscured);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, is_obscured);

  res = cmp_privacy_indicators_verify_layout(inds, &bad_ui, &is_obscured);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, is_obscured); /* Caught by the engine */

  res = cmp_privacy_indicators_destroy(inds);
  ASSERT_EQ(CMP_SUCCESS, res);

  PASS();
}

TEST test_screen_recording(void) {
  cmp_a11y_tree_t *tree = NULL;
  int res;

  res = cmp_a11y_tree_create(&tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Obscure secure inputs during recording */
  res = cmp_tree_set_screen_recording_prevention(tree, 10, 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_tree_set_screen_recording_prevention(NULL, 10, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_destroy(tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  PASS();
}

TEST test_null_args(void) {
  cmp_permissions_t *ctx = NULL;
  cmp_privacy_indicators_t *inds = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_permissions_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_privacy_indicators_create(NULL));

  cmp_permissions_create(&ctx);
  cmp_privacy_indicators_create(&inds);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_permissions_get_status(NULL, CMP_PERMISSION_LOCATION, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_permissions_request(NULL, CMP_PERMISSION_LOCATION));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_privacy_indicators_verify_layout(NULL, NULL, NULL));

  cmp_permissions_destroy(ctx);
  cmp_privacy_indicators_destroy(inds);

  PASS();
}

SUITE(permissions_suite) {
  RUN_TEST(test_permissions_lifecycle);
  RUN_TEST(test_privacy_indicators);
  RUN_TEST(test_screen_recording);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(permissions_suite);
  GREATEST_MAIN_END();
}
