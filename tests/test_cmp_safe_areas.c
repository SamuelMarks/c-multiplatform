/* clang-format off */
#include <cmp.h>
#include <greatest.h>
/* clang-format on */

SUITE(cmp_safe_areas_suite);

TEST test_cmp_safe_areas_create_destroy(void) {
  cmp_safe_areas_t *safe_areas = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_safe_areas_create(&safe_areas));
  ASSERT_NEQ(NULL, safe_areas);

  ASSERT_EQ(CMP_SUCCESS, cmp_safe_areas_destroy(safe_areas));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_safe_areas_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_safe_areas_destroy(NULL));

  PASS();
}

TEST test_cmp_safe_areas_get_set(void) {
  cmp_safe_areas_t *safe_areas = NULL;
  float inset = 0.0f;

  ASSERT_EQ(CMP_SUCCESS, cmp_safe_areas_create(&safe_areas));

  ASSERT_EQ(CMP_SUCCESS, cmp_safe_areas_set_inset(safe_areas, 0, 10.5f));
  ASSERT_EQ(CMP_SUCCESS, cmp_safe_areas_set_inset(safe_areas, 1, 20.0f));
  ASSERT_EQ(CMP_SUCCESS, cmp_safe_areas_set_inset(safe_areas, 2, 30.5f));
  ASSERT_EQ(CMP_SUCCESS, cmp_safe_areas_set_inset(safe_areas, 3, 40.0f));

  ASSERT_EQ(CMP_SUCCESS, cmp_safe_areas_get_inset(safe_areas, 0, &inset));
  ASSERT_EQ_FMT(10.5f, inset, "%f");

  ASSERT_EQ(CMP_SUCCESS, cmp_safe_areas_get_inset(safe_areas, 1, &inset));
  ASSERT_EQ_FMT(20.0f, inset, "%f");

  ASSERT_EQ(CMP_SUCCESS, cmp_safe_areas_get_inset(safe_areas, 2, &inset));
  ASSERT_EQ_FMT(30.5f, inset, "%f");

  ASSERT_EQ(CMP_SUCCESS, cmp_safe_areas_get_inset(safe_areas, 3, &inset));
  ASSERT_EQ_FMT(40.0f, inset, "%f");

  /* Bounds checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_safe_areas_set_inset(safe_areas, 4, 10.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_safe_areas_set_inset(safe_areas, -1, 10.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_safe_areas_get_inset(safe_areas, 4, &inset));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_safe_areas_get_inset(safe_areas, -1, &inset));

  ASSERT_EQ(CMP_SUCCESS, cmp_safe_areas_destroy(safe_areas));

  PASS();
}

SUITE(cmp_safe_areas_suite) {
  RUN_TEST(test_cmp_safe_areas_create_destroy);
  RUN_TEST(test_cmp_safe_areas_get_set);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_safe_areas_suite);
  GREATEST_MAIN_END();
}
