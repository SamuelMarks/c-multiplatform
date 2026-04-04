/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_resource_manager_features(void) {
  cmp_resource_manager_t *ctx = NULL;
  void *node = (void *)1;
  void *bitmap = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_resource_manager_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_resources_set_thermal_state(ctx, 2)); /* Serious */
  ASSERT_EQ(CMP_SUCCESS, cmp_resources_set_background_state(ctx, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_resources_mark_node_opaque(ctx, node, 1));

  ASSERT_EQ(CMP_SUCCESS, cmp_resources_set_low_data_mode(ctx, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_resources_cache_remote_image(
                             ctx, "https://a.com/img.png", 100.0f, 100.0f));
  ASSERT_EQ(CMP_SUCCESS, cmp_resources_allocate_offscreen_bitmap(
                             ctx, 200.0f, 200.0f, &bitmap));

  ASSERT_EQ(CMP_SUCCESS, cmp_resource_manager_destroy(ctx));
  PASS();
}

TEST test_resource_manager_null_args(void) {
  cmp_resource_manager_t *feat = NULL;
  void *node = (void *)1;
  void *bitmap = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_resource_manager_create(NULL));
  cmp_resource_manager_create(&feat);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_resources_set_thermal_state(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_resources_set_thermal_state(feat, -1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_resources_set_thermal_state(feat, 4));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_resources_set_background_state(NULL, 1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_resources_mark_node_opaque(NULL, node, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_resources_mark_node_opaque(feat, NULL, 1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_resources_set_low_data_mode(NULL, 1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_resources_cache_remote_image(NULL, "a", 1.0f, 1.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_resources_cache_remote_image(feat, NULL, 1.0f, 1.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_resources_cache_remote_image(feat, "a", 0.0f, 1.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_resources_cache_remote_image(feat, "a", 1.0f, -1.0f));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_resources_allocate_offscreen_bitmap(NULL, 1.0f, 1.0f, &bitmap));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_resources_allocate_offscreen_bitmap(feat, 0.0f, 1.0f, &bitmap));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_resources_allocate_offscreen_bitmap(
                                       feat, 1.0f, -1.0f, &bitmap));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_resources_allocate_offscreen_bitmap(feat, 1.0f, 1.0f, NULL));

  cmp_resource_manager_destroy(feat);
  PASS();
}

SUITE(resource_manager_suite) {
  RUN_TEST(test_resource_manager_features);
  RUN_TEST(test_resource_manager_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(resource_manager_suite);
  GREATEST_MAIN_END();
}
