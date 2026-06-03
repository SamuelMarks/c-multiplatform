/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_markdown_parser_lifecycle(void) {
  cmp_markdown_parser_t *parser = NULL;
  cmp_md_node_t *root = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_markdown_parser_create(&parser));
  ASSERT_NEQ(NULL, parser);

  /* Parse basic text */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_markdown_parser_parse(parser, "Hello World", &root));
  ASSERT_NEQ(NULL, root);
  ASSERT_EQ(CMP_MD_NODE_PARAGRAPH, root->type);
  ASSERT_EQ(1, root->child_count);
  ASSERT_NEQ(NULL, root->children[0]);
  ASSERT_EQ(CMP_MD_NODE_TEXT, root->children[0]->type);
  ASSERT_STR_EQ("Hello World", root->children[0]->content);
  ASSERT_EQ(CMP_SUCCESS, cmp_md_node_destroy(root));

  /* Parse Header */
  ASSERT_EQ(CMP_SUCCESS, cmp_markdown_parser_parse(parser, "# Header", &root));
  ASSERT_NEQ(NULL, root);
  ASSERT_EQ(CMP_MD_NODE_PARAGRAPH, root->type);
  ASSERT_EQ(1, root->child_count);
  ASSERT_NEQ(NULL, root->children[0]);
  ASSERT_EQ(CMP_MD_NODE_HEADER, root->children[0]->type);
  ASSERT_STR_EQ("Header", root->children[0]->content);
  ASSERT_EQ(1, root->children[0]->level);
  ASSERT_EQ(CMP_SUCCESS, cmp_md_node_destroy(root));

  /* Parse List */
  ASSERT_EQ(CMP_SUCCESS, cmp_markdown_parser_parse(parser, "* Item", &root));
  ASSERT_NEQ(NULL, root);
  ASSERT_EQ(1, root->child_count);
  ASSERT_EQ(CMP_MD_NODE_LIST, root->children[0]->type);
  ASSERT_EQ(1, root->children[0]->child_count);
  ASSERT_EQ(CMP_MD_NODE_LIST_ITEM, root->children[0]->children[0]->type);
  ASSERT_STR_EQ("Item", root->children[0]->children[0]->content);
  ASSERT_EQ(CMP_SUCCESS, cmp_md_node_destroy(root));

  /* Parse Blockquote */
  ASSERT_EQ(CMP_SUCCESS, cmp_markdown_parser_parse(parser, "> Quote", &root));
  ASSERT_NEQ(NULL, root);
  ASSERT_EQ(1, root->child_count);
  ASSERT_EQ(CMP_MD_NODE_BLOCKQUOTE, root->children[0]->type);
  ASSERT_STR_EQ("Quote", root->children[0]->content);
  ASSERT_EQ(CMP_SUCCESS, cmp_md_node_destroy(root));

  ASSERT_EQ(CMP_SUCCESS, cmp_markdown_parser_destroy(parser));
  PASS();
}

TEST test_markdown_parser_null_args(void) {
  cmp_markdown_parser_t *parser = NULL;
  cmp_md_node_t *root = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_markdown_parser_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_markdown_parser_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_md_node_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_markdown_parser_create(&parser));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_markdown_parser_parse(NULL, "Text", &root));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_markdown_parser_parse(parser, NULL, &root));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_markdown_parser_parse(parser, "Text", NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_markdown_parser_destroy(parser));
  PASS();
}

SUITE(cmp_markdown_parser_suite) {
  RUN_TEST(test_markdown_parser_lifecycle);
  RUN_TEST(test_markdown_parser_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_markdown_parser_suite);
  GREATEST_MAIN_END();
}
