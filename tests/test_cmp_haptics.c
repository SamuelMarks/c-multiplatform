/* clang-format off */
#include <cmp.h>
#include <greatest.h>
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_haptics_suite);

TEST test_cmp_haptics_create_destroy(void) {
  cmp_haptics_t *haptics = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_haptics_create(&haptics));
  ASSERT_NEQ(NULL, haptics);

  ASSERT_EQ(CMP_SUCCESS, cmp_haptics_destroy(haptics));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_haptics_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_haptics_destroy(NULL));

  PASS();
}

TEST test_cmp_haptics_trigger(void) {
  cmp_haptics_t *haptics = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_haptics_create(&haptics));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_haptics_trigger(haptics, CMP_HAPTICS_TYPE_LIGHT_IMPACT));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_haptics_trigger(haptics, CMP_HAPTICS_TYPE_MEDIUM_IMPACT));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_haptics_trigger(haptics, CMP_HAPTICS_TYPE_HEAVY_IMPACT));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_haptics_trigger(haptics, CMP_HAPTICS_TYPE_SELECTION_CHANGE));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_haptics_trigger(NULL, CMP_HAPTICS_TYPE_LIGHT_IMPACT));

  ASSERT_EQ(CMP_SUCCESS, cmp_haptics_destroy(haptics));
  PASS();
}

SUITE(cmp_haptics_suite) {
  RUN_TEST(test_cmp_haptics_create_destroy);
  RUN_TEST(test_cmp_haptics_trigger);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_haptics_suite);
  GREATEST_MAIN_END();
}
