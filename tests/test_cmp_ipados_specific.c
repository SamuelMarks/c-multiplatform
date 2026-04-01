/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_ipados_multitasking_classes(void) {
  cmp_size_class_t hc, vc;

  /* Slide Over (Narrow window) */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ipados_resolve_size_classes(320.0f, 800.0f, &hc, &vc));
  ASSERT_EQ(CMP_SIZE_CLASS_COMPACT, hc);
  ASSERT_EQ(CMP_SIZE_CLASS_REGULAR, vc);

  /* Full Screen iPad */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ipados_resolve_size_classes(1024.0f, 768.0f, &hc, &vc));
  ASSERT_EQ(CMP_SIZE_CLASS_REGULAR, hc);
  ASSERT_EQ(CMP_SIZE_CLASS_REGULAR, vc);

  PASS();
}

TEST test_ipados_sidebar_collapsing(void) {
  int collapsed;

  /* Slide Over triggers collapse to iPhone-style list */
  ASSERT_EQ(CMP_SUCCESS, cmp_ipados_resolve_sidebar_state(
                             CMP_SIZE_CLASS_COMPACT, &collapsed));
  ASSERT_EQ(1, collapsed);

  /* Full screen leaves sidebar open */
  ASSERT_EQ(CMP_SUCCESS, cmp_ipados_resolve_sidebar_state(
                             CMP_SIZE_CLASS_REGULAR, &collapsed));
  ASSERT_EQ(0, collapsed);

  PASS();
}

TEST test_ipados_multiwindow_and_camera(void) {
  cmp_ipados_features_t *ctx = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_ipados_features_create(&ctx));

  /* Multi-window */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ipados_request_scene_activation(ctx, "com.app.doc.123"));

  /* Center Stage */
  ASSERT_EQ(CMP_SUCCESS, cmp_ipados_set_center_stage_enabled(ctx, 1));

  ASSERT_EQ(CMP_SUCCESS, cmp_ipados_features_destroy(ctx));
  PASS();
}

TEST test_ipados_null_args(void) {
  cmp_ipados_features_t *feat = NULL;
  cmp_size_class_t c;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ipados_features_create(NULL));
  cmp_ipados_features_create(&feat);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ipados_resolve_size_classes(-1.0f, 100.0f, &c, &c));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ipados_resolve_size_classes(100.0f, 0.0f, &c, &c));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ipados_resolve_size_classes(100.0f, 100.0f, NULL, &c));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ipados_resolve_size_classes(100.0f, 100.0f, &c, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ipados_resolve_sidebar_state(CMP_SIZE_CLASS_COMPACT, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ipados_request_scene_activation(NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ipados_request_scene_activation(feat, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ipados_set_center_stage_enabled(NULL, 1));

  cmp_ipados_features_destroy(feat);
  PASS();
}

SUITE(ipados_specific_suite) {
  RUN_TEST(test_ipados_multitasking_classes);
  RUN_TEST(test_ipados_sidebar_collapsing);
  RUN_TEST(test_ipados_multiwindow_and_camera);
  RUN_TEST(test_ipados_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ipados_specific_suite);
  GREATEST_MAIN_END();
}
