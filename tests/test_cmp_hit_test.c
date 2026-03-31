/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

/* Hack for testing the mock hit test state */
struct cmp_hit_test {
  cmp_ui_node_t *tree;
  cmp_ui_node_t *mock_hit_result;
};

TEST test_hit_test_create_destroy(void) {
  cmp_hit_test_t *ht = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_hit_test_create(NULL, &ht));
  ASSERT_NEQ(NULL, ht);

  ASSERT_EQ(CMP_SUCCESS, cmp_hit_test_destroy(ht));
  PASS();
}

TEST test_hit_test_query(void) {
  cmp_hit_test_t *ht = NULL;
  cmp_ui_node_t node = {0};
  cmp_ui_node_t *result = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_hit_test_create(NULL, &ht));

  /* Before setting mock, valid positive coordinates should return NOT_FOUND
   * (simulating no elements on screen) */
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, cmp_hit_test_query(ht, 10.0f, 10.0f, &result));
  ASSERT_EQ(NULL, result);

  /* Set internal mock data to test hit success */
  ((struct cmp_hit_test *)ht)->mock_hit_result = &node;

  /* Success hit */
  ASSERT_EQ(CMP_SUCCESS, cmp_hit_test_query(ht, 10.0f, 10.0f, &result));
  ASSERT_EQ(&node, result);

  /* Negative coordinates - guaranteed miss regardless of mock state */
  result = NULL;
  ASSERT_EQ(CMP_ERROR_NOT_FOUND,
            cmp_hit_test_query(ht, -10.0f, 10.0f, &result));
  ASSERT_EQ(NULL, result);

  ASSERT_EQ(CMP_SUCCESS, cmp_hit_test_destroy(ht));
  PASS();
}

TEST test_hit_test_edge_cases(void) {
  cmp_hit_test_t *ht = NULL;
  cmp_ui_node_t *result = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_hit_test_create(NULL, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_hit_test_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_hit_test_create(NULL, &ht));

  /* Null pointer arguments */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_hit_test_query(NULL, 10.0f, 10.0f, &result));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_hit_test_query(ht, 10.0f, 10.0f, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_hit_test_destroy(ht));
  PASS();
}

SUITE(cmp_hit_test_suite) {
  RUN_TEST(test_hit_test_create_destroy);
  RUN_TEST(test_hit_test_query);
  RUN_TEST(test_hit_test_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_hit_test_suite);
  GREATEST_MAIN_END();
}
