/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_carplay_features(void) {
  cmp_carplay_features_t *ctx = NULL;
  int dummy_node = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_carplay_features_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_carplay_apply_driving_focus(ctx, &dummy_node));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_carplay_export_template_data(ctx, "List", "{\"items\":[]}"));
  ASSERT_EQ(CMP_SUCCESS, cmp_carplay_handle_siri_intent(ctx, "play_music"));
  ASSERT_EQ(CMP_SUCCESS, cmp_carplay_handle_knob_navigation(ctx, 1.0f));
  ASSERT_EQ(CMP_SUCCESS, cmp_carplay_set_network_status(ctx, 0));

  ASSERT_EQ(CMP_SUCCESS, cmp_carplay_features_destroy(ctx));
  PASS();
}

TEST test_carplay_null_args(void) {
  cmp_carplay_features_t *feat = NULL;
  int node = 0;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_carplay_features_create(NULL));
  cmp_carplay_features_create(&feat);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_carplay_apply_driving_focus(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_carplay_apply_driving_focus(feat, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_carplay_export_template_data(NULL, "List", "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_carplay_export_template_data(feat, NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_carplay_export_template_data(feat, "List", NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_carplay_handle_siri_intent(NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_carplay_handle_siri_intent(feat, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_carplay_handle_knob_navigation(NULL, 1.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_carplay_set_network_status(NULL, 1));

  cmp_carplay_features_destroy(feat);
  PASS();
}

SUITE(carplay_specific_suite) {
  RUN_TEST(test_carplay_features);
  RUN_TEST(test_carplay_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(carplay_specific_suite);
  GREATEST_MAIN_END();
}
