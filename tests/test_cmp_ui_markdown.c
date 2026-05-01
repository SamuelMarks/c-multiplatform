/* clang-format off */
#include "cmp.h"
#include "cmp_ui_markdown.h"
#include "greatest.h"
/* clang-format on */

TEST test_ui_markdown_lifecycle(void) {
  cmp_ui_markdown_t *md = NULL;
  int res;

  res = cmp_ui_markdown_create(&md, "# Hello", 0xFFFFFFFF, 0xFF000000);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, md);

  res = cmp_ui_markdown_destroy(md);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_ui_markdown_null_args(void) {
  cmp_ui_markdown_t *md = NULL;
  cmp_ui_node_t *node = NULL;
  int res;

  res = cmp_ui_markdown_create(NULL, "# Hello", 0, 0);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_markdown_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_markdown_create(&md, NULL, 0, 0);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_markdown_get_node(NULL, &node);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_markdown_get_node(md, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_markdown_set_text(NULL, "test");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_markdown_set_text(md, NULL);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_markdown_destroy(md);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_ui_markdown_operations(void) {
  cmp_ui_markdown_t *md = NULL;
  cmp_ui_node_t *node = NULL;
  int res;

  res = cmp_ui_markdown_create(&md, "# Title", 0, 0);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_markdown_get_node(md, &node);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, node);

  res = cmp_ui_markdown_set_text(md, "## Subtitle");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_markdown_destroy(md);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(ui_markdown_suite) {
  RUN_TEST(test_ui_markdown_lifecycle);
  RUN_TEST(test_ui_markdown_null_args);
  RUN_TEST(test_ui_markdown_operations);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_markdown_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
