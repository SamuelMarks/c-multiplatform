/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_screen_reader_lifecycle(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_screen_reader_t *reader = NULL;
  int res;
  cmp_a11y_tree_create(&tree);

  res = cmp_screen_reader_create(tree, &reader);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, reader);

  res = cmp_screen_reader_destroy(reader);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_screen_reader_null_args(void) {
  cmp_a11y_tree_t *tree = NULL;
  int res;
  cmp_screen_reader_t *reader = NULL;
  cmp_a11y_tree_create(&tree);

  res = cmp_screen_reader_create(NULL, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_screen_reader_create(tree, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_screen_reader_create(NULL, &reader);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_screen_reader_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_screen_reader_create(tree, &reader);

  res = cmp_screen_reader_announce(NULL, "Hello");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_screen_reader_announce(reader, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_screen_reader_map_node(NULL, 1, (void *)0x1234);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_screen_reader_destroy(reader);
  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_screen_reader_operations(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_screen_reader_t *reader = NULL;
  int res;
  void *fake_native_node_1 = (void *)0x1000;
  void *fake_native_node_2 = (void *)0x2000;
  void *fake_native_node_3 = (void *)0x3000;
  cmp_a11y_tree_create(&tree);

  cmp_screen_reader_create(tree, &reader);

  res = cmp_screen_reader_announce(reader, "Test message");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_screen_reader_map_node(reader, 1, fake_native_node_1);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_screen_reader_map_node(reader, 2, fake_native_node_2);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Test overwrite existing node map */
  res = cmp_screen_reader_map_node(reader, 1, fake_native_node_3);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_screen_reader_destroy(reader);
  cmp_a11y_tree_destroy(tree);
  PASS();
}

SUITE(screen_reader_suite) {
  RUN_TEST(test_screen_reader_lifecycle);
  RUN_TEST(test_screen_reader_null_args);
  RUN_TEST(test_screen_reader_operations);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(screen_reader_suite);
  GREATEST_MAIN_END();
}
