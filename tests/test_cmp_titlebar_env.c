/* clang-format off */
#include <cmp.h>
#include <greatest.h>
/* clang-format on */

SUITE(cmp_titlebar_env_suite);

TEST test_cmp_titlebar_env_create_destroy(void) {
  cmp_titlebar_env_t *env = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_titlebar_env_create(&env));
  ASSERT_NEQ(NULL, env);

  ASSERT_EQ(CMP_SUCCESS, cmp_titlebar_env_destroy(env));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_titlebar_env_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_titlebar_env_destroy(NULL));

  PASS();
}

TEST test_cmp_titlebar_env_get_set(void) {
  cmp_titlebar_env_t *env = NULL;
  float x = 0.0f;
  float y = 0.0f;
  float width = 0.0f;
  float height = 0.0f;

  ASSERT_EQ(CMP_SUCCESS, cmp_titlebar_env_create(&env));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_titlebar_env_set_area(env, 10.0f, 20.0f, 100.0f, 50.0f));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_titlebar_env_get_area(env, &x, &y, &width, &height));
  ASSERT_EQ_FMT(10.0f, x, "%f");
  ASSERT_EQ_FMT(20.0f, y, "%f");
  ASSERT_EQ_FMT(100.0f, width, "%f");
  ASSERT_EQ_FMT(50.0f, height, "%f");

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_titlebar_env_set_area(NULL, 10.0f, 20.0f, 100.0f, 50.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_titlebar_env_get_area(NULL, &x, &y, &width, &height));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_titlebar_env_get_area(env, NULL, &y, &width, &height));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_titlebar_env_get_area(env, &x, NULL, &width, &height));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_titlebar_env_get_area(env, &x, &y, NULL, &height));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_titlebar_env_get_area(env, &x, &y, &width, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_titlebar_env_destroy(env));
  PASS();
}

SUITE(cmp_titlebar_env_suite) {
  RUN_TEST(test_cmp_titlebar_env_create_destroy);
  RUN_TEST(test_cmp_titlebar_env_get_set);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_titlebar_env_suite);
  GREATEST_MAIN_END();
}
