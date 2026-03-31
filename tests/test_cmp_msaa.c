/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

TEST test_msaa_create_destroy(void) {
  cmp_msaa_t *msaa = NULL;

  /* Basic valid creation: 4x MSAA, 800x600 */
  ASSERT_EQ(CMP_SUCCESS, cmp_msaa_create(4, 800, 600, &msaa));
  ASSERT_NEQ(NULL, msaa);

  /* Destroy it */
  ASSERT_EQ(CMP_SUCCESS, cmp_msaa_destroy(msaa));
  PASS();
}

TEST test_msaa_resolve_success(void) {
  cmp_msaa_t *msaa = NULL;
  cmp_texture_t target;

  ASSERT_EQ(CMP_SUCCESS, cmp_msaa_create(8, 1024, 768, &msaa));
  ASSERT_NEQ(NULL, msaa);

  /* Mock the target texture matching dimensions */
  target.width = 1024;
  target.height = 768;
  target.format = 0;
  target.internal_handle = (void *)0x1234;

  ASSERT_EQ(CMP_SUCCESS, cmp_msaa_resolve(msaa, &target));

  ASSERT_EQ(CMP_SUCCESS, cmp_msaa_destroy(msaa));
  PASS();
}

TEST test_msaa_resolve_bounds_error(void) {
  cmp_msaa_t *msaa = NULL;
  cmp_texture_t target;

  ASSERT_EQ(CMP_SUCCESS, cmp_msaa_create(2, 500, 500, &msaa));
  ASSERT_NEQ(NULL, msaa);

  /* Mismatch dimensions */
  target.width = 400;
  target.height = 500;
  target.format = 0;
  target.internal_handle = (void *)0x1234;

  ASSERT_EQ(CMP_ERROR_BOUNDS, cmp_msaa_resolve(msaa, &target));

  target.width = 500;
  target.height = 400;
  ASSERT_EQ(CMP_ERROR_BOUNDS, cmp_msaa_resolve(msaa, &target));

  ASSERT_EQ(CMP_SUCCESS, cmp_msaa_destroy(msaa));
  PASS();
}

TEST test_msaa_edge_cases(void) {
  cmp_msaa_t *msaa = NULL;
  cmp_texture_t target;

  /* Null pointer out_msaa */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_msaa_create(4, 800, 600, NULL));

  /* Invalid samples or dimensions */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_msaa_create(0, 800, 600, &msaa));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_msaa_create(4, 0, 600, &msaa));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_msaa_create(4, 800, 0, &msaa));

  /* Null arguments in destroy/resolve */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_msaa_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_msaa_resolve(NULL, &target));

  /* Create dummy msaa for testing resolve with null target */
  ASSERT_EQ(CMP_SUCCESS, cmp_msaa_create(4, 100, 100, &msaa));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_msaa_resolve(msaa, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_msaa_destroy(msaa));
  PASS();
}

SUITE(cmp_msaa_suite) {
  RUN_TEST(test_msaa_create_destroy);
  RUN_TEST(test_msaa_resolve_success);
  RUN_TEST(test_msaa_resolve_bounds_error);
  RUN_TEST(test_msaa_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_msaa_suite);
  GREATEST_MAIN_END();
}
