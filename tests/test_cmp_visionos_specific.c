/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_visionos_features(void) {
  cmp_visionos_features_t *ctx = NULL;
  int dummy_node = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_visionos_features_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_visionos_apply_glass_material(ctx, &dummy_node));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_visionos_handle_eye_tracking_hover(ctx, &dummy_node, 1));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_visionos_set_modal_z_depth(ctx, &dummy_node, -5.0f));
  ASSERT_EQ(CMP_SUCCESS, cmp_visionos_setup_ornament(ctx, &dummy_node));

  ASSERT_EQ(CMP_SUCCESS, cmp_visionos_request_immersion_level(ctx, 1));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_visionos_handle_touch_event(ctx, 0, 100.0f, 100.0f, 50.0f));

  ASSERT_EQ(CMP_SUCCESS, cmp_visionos_features_destroy(ctx));
  PASS();
}

TEST test_visionos_null_args(void) {
  cmp_visionos_features_t *feat = NULL;
  int node = 0;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_visionos_features_create(NULL));
  cmp_visionos_features_create(&feat);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_visionos_apply_glass_material(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_visionos_apply_glass_material(feat, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_visionos_handle_eye_tracking_hover(NULL, &node, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_visionos_handle_eye_tracking_hover(feat, NULL, 1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_visionos_set_modal_z_depth(NULL, &node, -5.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_visionos_set_modal_z_depth(feat, NULL, -5.0f));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_visionos_setup_ornament(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_visionos_setup_ornament(feat, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_visionos_request_immersion_level(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_visionos_request_immersion_level(feat, -1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_visionos_request_immersion_level(feat, 3));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_visionos_handle_touch_event(NULL, 0, 0, 0, 0));

  cmp_visionos_features_destroy(feat);
  PASS();
}

TEST test_visionos_advanced_features(void) {
  cmp_visionos_features_t *ctx = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_visionos_features_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_visionos_set_window_geometry(ctx, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_visionos_set_shared_space_behavior(ctx, 1));

  ASSERT_EQ(CMP_SUCCESS, cmp_visionos_features_destroy(ctx));
  PASS();
}

TEST test_visionos_advanced_null_args(void) {
  cmp_visionos_features_t *feat = NULL;
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_visionos_set_window_geometry(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_visionos_set_shared_space_behavior(NULL, 1));

  cmp_visionos_features_create(&feat);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_visionos_set_window_geometry(feat, -1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_visionos_set_shared_space_behavior(feat, -1));
  cmp_visionos_features_destroy(feat);
  PASS();
}

SUITE(visionos_specific_suite) {
  RUN_TEST(test_visionos_features);
  RUN_TEST(test_visionos_null_args);
  RUN_TEST(test_visionos_advanced_features);
  RUN_TEST(test_visionos_advanced_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(visionos_specific_suite);
  GREATEST_MAIN_END();
}
