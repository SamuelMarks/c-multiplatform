/* clang-format off */
#include <cmp.h>
#include <greatest.h>
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_pip_suite);

TEST test_cmp_pip_create_destroy(void) {
  cmp_pip_t *pip = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_pip_create(&pip));
  ASSERT_NEQ(NULL, pip);

  ASSERT_EQ(CMP_SUCCESS, cmp_pip_destroy(pip));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pip_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pip_destroy(NULL));

  PASS();
}

TEST test_cmp_pip_enable_disable(void) {
  cmp_pip_t *pip = NULL;
  void *dummy_node = (void *)0x1234;
  int is_active = -1;

  ASSERT_EQ(CMP_SUCCESS, cmp_pip_create(&pip));

  /* Initial state */
  ASSERT_EQ(CMP_SUCCESS, cmp_pip_is_active(pip, &is_active));
  ASSERT_EQ(0, is_active);

  /* Enable */
  ASSERT_EQ(CMP_SUCCESS, cmp_pip_enable(pip, dummy_node));
  ASSERT_EQ(CMP_SUCCESS, cmp_pip_is_active(pip, &is_active));
  ASSERT_EQ(1, is_active);

  /* Disable */
  ASSERT_EQ(CMP_SUCCESS, cmp_pip_disable(pip));
  ASSERT_EQ(CMP_SUCCESS, cmp_pip_is_active(pip, &is_active));
  ASSERT_EQ(0, is_active);

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pip_enable(NULL, dummy_node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pip_enable(pip, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pip_disable(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pip_is_active(NULL, &is_active));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pip_is_active(pip, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_pip_destroy(pip));
  PASS();
}

SUITE(cmp_pip_suite) {
  RUN_TEST(test_cmp_pip_create_destroy);
  RUN_TEST(test_cmp_pip_enable_disable);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_pip_suite);
  GREATEST_MAIN_END();
}
