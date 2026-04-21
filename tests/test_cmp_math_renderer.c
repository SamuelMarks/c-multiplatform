/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

SUITE(cmp_math_renderer_suite);

TEST test_math_renderer_create_destroy(void) {
  cmp_math_renderer_t *renderer = NULL;
  ASSERT_EQ(CMP_SUCCESS, cmp_math_renderer_create(&renderer));
  ASSERT_NEQ(NULL, renderer);
  ASSERT_EQ(CMP_SUCCESS, cmp_math_renderer_destroy(renderer));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_math_renderer_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_math_renderer_destroy(NULL));
  PASS();
}

TEST test_math_renderer_parse(void) {
  cmp_math_renderer_t *renderer = NULL;
  void *tree = NULL;

  cmp_math_renderer_create(&renderer);

  ASSERT_EQ(CMP_SUCCESS, cmp_math_renderer_parse(renderer, "x^2", 1, &tree));
  ASSERT_NEQ(NULL, tree);

  ASSERT_EQ(CMP_SUCCESS, cmp_math_renderer_free_tree(tree));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_math_renderer_parse(NULL, "x^2", 1, &tree));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_math_renderer_parse(renderer, NULL, 1, &tree));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_math_renderer_parse(renderer, "x^2", 1, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_math_renderer_free_tree(NULL));

  cmp_math_renderer_destroy(renderer);
  PASS();
}

SUITE(cmp_math_renderer_suite) {
  RUN_TEST(test_math_renderer_create_destroy);
  RUN_TEST(test_math_renderer_parse);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_math_renderer_suite);
  GREATEST_MAIN_END();
}
