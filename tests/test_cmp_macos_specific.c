/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_macos_features(void) {
  cmp_macos_features_t *ctx = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_macos_features_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_macos_set_window_controls_visible(ctx, 0));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_macos_set_document_proxy(ctx, "file:///path/to/doc.txt"));
  ASSERT_EQ(CMP_SUCCESS, cmp_macos_set_document_proxy(ctx, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_macos_set_menu_bar_extra(ctx, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_macos_invoke_print_panel(ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_macos_features_destroy(ctx));
  PASS();
}

TEST test_macos_null_args(void) {
  cmp_macos_features_t *feat = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_macos_features_create(NULL));
  cmp_macos_features_create(&feat);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_macos_set_window_controls_visible(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_macos_set_document_proxy(NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_macos_set_menu_bar_extra(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_macos_invoke_print_panel(NULL));

  cmp_macos_features_destroy(feat);
  PASS();
}

TEST test_macos_advanced_features(void) {
  cmp_macos_features_t *ctx = NULL;
  int dummy_node = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_macos_features_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_macos_setup_global_menu_bar(ctx));
  ASSERT_EQ(CMP_SUCCESS, cmp_macos_setup_context_menu(ctx, &dummy_node));
  ASSERT_EQ(CMP_SUCCESS, cmp_macos_set_hover_state(ctx, &dummy_node, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_macos_set_window_shadow(ctx, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_macos_setup_toolbar_customization(ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_macos_features_destroy(ctx));
  PASS();
}

TEST test_macos_advanced_null_args(void) {
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_macos_setup_global_menu_bar(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_macos_setup_context_menu(NULL, (void *)1));

  cmp_macos_features_t *feat = NULL;
  cmp_macos_features_create(&feat);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_macos_setup_context_menu(feat, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_macos_set_hover_state(NULL, (void *)1, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_macos_set_hover_state(feat, NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_macos_set_window_shadow(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_macos_setup_toolbar_customization(NULL));
  cmp_macos_features_destroy(feat);
  PASS();
}

SUITE(macos_specific_suite) {
  RUN_TEST(test_macos_features);
  RUN_TEST(test_macos_null_args);
  RUN_TEST(test_macos_advanced_features);
  RUN_TEST(test_macos_advanced_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(macos_specific_suite);
  GREATEST_MAIN_END();
}
