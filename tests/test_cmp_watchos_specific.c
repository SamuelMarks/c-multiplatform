/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_watchos_features(void) {
  cmp_watchos_features_t *ctx = NULL;
  int dummy_node = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_watchos_features_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_watchos_handle_digital_crown(ctx, 1.5f));
  ASSERT_EQ(CMP_SUCCESS, cmp_watchos_handle_double_tap(ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_watchos_set_aod_state(ctx, 1));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_watchos_export_complication_data(ctx, "{\"test\": 1}"));
  ASSERT_EQ(CMP_SUCCESS, cmp_watchos_export_smart_stack(ctx, "{\"data\": 2}"));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_watchos_apply_edge_to_edge_styling(ctx, &dummy_node));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_watchos_apply_pill_button_styling(ctx, &dummy_node));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_watchos_apply_hierarchical_pagination(ctx, &dummy_node));

  ASSERT_EQ(CMP_SUCCESS, cmp_watchos_features_destroy(ctx));
  PASS();
}

TEST test_watchos_null_args(void) {
  cmp_watchos_features_t *feat = NULL;
  int node = 0;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_watchos_features_create(NULL));
  cmp_watchos_features_create(&feat);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_watchos_handle_digital_crown(NULL, 1.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_watchos_handle_double_tap(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_watchos_set_aod_state(NULL, 1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_watchos_export_complication_data(NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_watchos_export_complication_data(feat, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_watchos_export_smart_stack(NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_watchos_export_smart_stack(feat, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_watchos_apply_edge_to_edge_styling(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_watchos_apply_edge_to_edge_styling(feat, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_watchos_apply_pill_button_styling(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_watchos_apply_pill_button_styling(feat, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_watchos_apply_hierarchical_pagination(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_watchos_apply_hierarchical_pagination(feat, NULL));

  cmp_watchos_features_destroy(feat);
  PASS();
}

SUITE(watchos_specific_suite) {
  RUN_TEST(test_watchos_features);
  RUN_TEST(test_watchos_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(watchos_specific_suite);
  GREATEST_MAIN_END();
}
