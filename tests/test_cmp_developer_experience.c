/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_dx_features(void) {
  cmp_developer_experience_t *ctx = NULL;
  void *node = NULL;
  char buf[256];

  ASSERT_EQ(CMP_SUCCESS, cmp_developer_experience_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_dx_build_declarative_node(ctx, "Button", &node));
  ASSERT_EQ(CMP_SUCCESS, cmp_dx_apply_typed_color(ctx, node, 0));
  ASSERT_EQ(CMP_SUCCESS, cmp_dx_enable_live_preview(ctx, node));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_dx_export_debug_hierarchy(ctx, node, buf, sizeof(buf)));

  ASSERT_EQ(CMP_SUCCESS, cmp_developer_experience_destroy(ctx));
  PASS();
}

TEST test_dx_null_args(void) {
  cmp_developer_experience_t *feat = NULL;
  void *node = (void *)1;
  char buf[256];

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_developer_experience_create(NULL));
  cmp_developer_experience_create(&feat);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_dx_build_declarative_node(NULL, "a", &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_dx_build_declarative_node(feat, NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_dx_build_declarative_node(feat, "a", NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dx_apply_typed_color(NULL, node, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dx_apply_typed_color(feat, NULL, 0));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dx_enable_live_preview(NULL, node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dx_enable_live_preview(feat, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_dx_export_debug_hierarchy(NULL, node, buf, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_dx_export_debug_hierarchy(feat, NULL, buf, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_dx_export_debug_hierarchy(feat, node, NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_dx_export_debug_hierarchy(feat, node, buf, 0));

  cmp_developer_experience_destroy(feat);
  PASS();
}

SUITE(developer_experience_suite) {
  RUN_TEST(test_dx_features);
  RUN_TEST(test_dx_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(developer_experience_suite);
  GREATEST_MAIN_END();
}
