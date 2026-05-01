/* clang-format off */
#include "cmp.h"
#include "cmp_tree_sitter.h"
#include "greatest.h"
/* clang-format on */

TEST test_tree_sitter_lifecycle(void) {
  cmp_tree_sitter_t *ts = NULL;
  int res;

  res = cmp_tree_sitter_create(&ts);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, ts);

  res = cmp_tree_sitter_destroy(ts);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_tree_sitter_null_args(void) {
  cmp_tree_sitter_t *ts = NULL;
  cmp_tree_node_t *root = NULL;
  char *node_type = NULL;
  int res;

  res = cmp_tree_sitter_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tree_sitter_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tree_sitter_create(&ts);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_tree_sitter_parse(NULL, "c", "int main() {}", &root);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tree_sitter_parse(ts, NULL, "int main() {}", &root);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tree_sitter_parse(ts, "c", NULL, &root);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tree_sitter_parse(ts, "c", "int main() {}", NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tree_sitter_node_get_type(NULL, &node_type);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  /* It is valid to pass NULL to free functions, they should return SUCCESS and
   * do nothing */
  res = cmp_tree_sitter_free_node(NULL);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_tree_sitter_free_string(NULL);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_tree_sitter_destroy(ts);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_tree_sitter_parse_and_inspect(void) {
  cmp_tree_sitter_t *ts = NULL;
  cmp_tree_node_t *root = NULL;
  char *node_type = NULL;
  int res;

  res = cmp_tree_sitter_create(&ts);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_tree_sitter_parse(ts, "c", "int main() {}", &root);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, root);

  res = cmp_tree_sitter_node_get_type(root, &node_type);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, node_type);
  ASSERT_STR_EQ("translation_unit", node_type);

  res = cmp_tree_sitter_free_string(node_type);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_tree_sitter_free_node(root);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_tree_sitter_destroy(ts);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(tree_sitter_suite) {
  RUN_TEST(test_tree_sitter_lifecycle);
  RUN_TEST(test_tree_sitter_null_args);
  RUN_TEST(test_tree_sitter_parse_and_inspect);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(tree_sitter_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
