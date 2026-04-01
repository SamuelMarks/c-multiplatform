/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_tvos_features(void) {
  cmp_tvos_features_t *ctx = NULL;
  int dummy_node = 0;
  float scale, tx, ty, mt, mb, ml, mr;

  ASSERT_EQ(CMP_SUCCESS, cmp_tvos_features_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_tvos_handle_focus_engine_update(ctx, &dummy_node,
                                                             &scale, &tx, &ty));
  ASSERT_GT(scale, 1.0f);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_tvos_set_overscan_margins(ctx, &mt, &mb, &ml, &mr));
  ASSERT_GT(mt, 0.0f);
  ASSERT_GT(ml, 0.0f);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_tvos_export_top_shelf(ctx, 0, "{\"type\": \"carousel\"}"));
  ASSERT_EQ(CMP_SUCCESS, cmp_tvos_handle_hardware_play_pause(ctx));
  ASSERT_EQ(CMP_SUCCESS, cmp_tvos_handle_hardware_menu_button(ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_tvos_features_destroy(ctx));
  PASS();
}

TEST test_tvos_null_args(void) {
  cmp_tvos_features_t *feat = NULL;
  int node = 0;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_tvos_features_create(NULL));
  cmp_tvos_features_create(&feat);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_tvos_handle_focus_engine_update(NULL, &node, NULL, NULL, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_tvos_handle_focus_engine_update(feat, NULL, NULL, NULL, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_tvos_set_overscan_margins(NULL, NULL, NULL, NULL, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_tvos_export_top_shelf(NULL, 1, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_tvos_export_top_shelf(feat, 1, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_tvos_handle_hardware_play_pause(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_tvos_handle_hardware_menu_button(NULL));

  cmp_tvos_features_destroy(feat);
  PASS();
}

SUITE(tvos_specific_suite) {
  RUN_TEST(test_tvos_features);
  RUN_TEST(test_tvos_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(tvos_specific_suite);
  GREATEST_MAIN_END();
}
