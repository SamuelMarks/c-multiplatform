/* clang-format off */
#include <cmp.h>
#include <greatest.h>
/* clang-format on */

SUITE(cmp_app_region_suite);

TEST test_cmp_app_region_create_destroy(void) {
  cmp_app_region_t *region = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_app_region_create(&region));
  ASSERT_NEQ(NULL, region);

  ASSERT_EQ(CMP_SUCCESS, cmp_app_region_destroy(region));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_app_region_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_app_region_destroy(NULL));

  PASS();
}

TEST test_cmp_app_region_hit_test(void) {
  cmp_app_region_t *region = NULL;
  cmp_app_region_type_t type = CMP_APP_REGION_NONE;

  ASSERT_EQ(CMP_SUCCESS, cmp_app_region_create(&region));

  /* Base drag region: 0,0 100x100 */
  ASSERT_EQ(CMP_SUCCESS, cmp_app_region_add_rect(region, 0.0f, 0.0f, 100.0f,
                                                 100.0f, CMP_APP_REGION_DRAG));

  /* Overlapping no-drag region (e.g. close button): 80,0 20x20 */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_app_region_add_rect(region, 80.0f, 0.0f, 20.0f, 20.0f,
                                    CMP_APP_REGION_NO_DRAG));

  /* Inside base drag region */
  ASSERT_EQ(CMP_SUCCESS, cmp_app_region_hit_test(region, 10.0f, 10.0f, &type));
  ASSERT_EQ(CMP_APP_REGION_DRAG, type);

  /* Inside no-drag region (should override base drag region) */
  ASSERT_EQ(CMP_SUCCESS, cmp_app_region_hit_test(region, 90.0f, 10.0f, &type));
  ASSERT_EQ(CMP_APP_REGION_NO_DRAG, type);

  /* Outside any region */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_app_region_hit_test(region, 150.0f, 150.0f, &type));
  ASSERT_EQ(CMP_APP_REGION_NONE, type);

  /* Clear regions */
  ASSERT_EQ(CMP_SUCCESS, cmp_app_region_clear(region));
  ASSERT_EQ(CMP_SUCCESS, cmp_app_region_hit_test(region, 10.0f, 10.0f, &type));
  ASSERT_EQ(CMP_APP_REGION_NONE, type);

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_app_region_add_rect(NULL, 0, 0, 10, 10, CMP_APP_REGION_DRAG));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_app_region_clear(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_app_region_hit_test(NULL, 10, 10, &type));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_app_region_hit_test(region, 10, 10, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_app_region_destroy(region));
  PASS();
}

SUITE(cmp_app_region_suite) {
  RUN_TEST(test_cmp_app_region_create_destroy);
  RUN_TEST(test_cmp_app_region_hit_test);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_app_region_suite);
  GREATEST_MAIN_END();
}
