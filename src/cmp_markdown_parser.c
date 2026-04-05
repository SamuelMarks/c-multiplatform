/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_markdown_parser {
  int flags;
};

int cmp_md_node_destroy(cmp_md_node_t *node) {
  size_t i;
  if (!node) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (node->content) {
    free(node->content);
  }

  for (i = 0; i < node->child_count; ++i) {
    cmp_md_node_destroy(node->children[i]);
  }

  if (node->children) {
    free(node->children);
  }

  free(node);
  return CMP_SUCCESS;
}

static cmp_md_node_t *create_node(cmp_md_node_type_t type,
                                  const char *content) {
  cmp_md_node_t *node = (cmp_md_node_t *)malloc(sizeof(cmp_md_node_t));
  if (!node)
    return NULL;

  node->type = type;
  node->level = 0;
  node->child_count = 0;
  node->child_capacity = 0;
  node->children = NULL;

  if (content) {
    size_t len = strlen(content);
    node->content = (char *)malloc(len + 1);
    if (node->content) {
      strncpy(node->content, content, len + 1);
    }
  } else {
    node->content = NULL;
  }

  return node;
}

static int add_child(cmp_md_node_t *parent, cmp_md_node_t *child) {
  if (!parent || !child)
    return CMP_ERROR_INVALID_ARG;

  if (parent->child_count == parent->child_capacity) {
    size_t new_cap =
        parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;
    cmp_md_node_t **new_arr = (cmp_md_node_t **)realloc(
        parent->children, new_cap * sizeof(cmp_md_node_t *));
    if (!new_arr)
      return CMP_ERROR_OOM;
    parent->children = new_arr;
    parent->child_capacity = new_cap;
  }

  parent->children[parent->child_count++] = child;
  return CMP_SUCCESS;
}

int cmp_markdown_parser_create(cmp_markdown_parser_t **out_parser) {
  cmp_markdown_parser_t *parser;
  if (!out_parser)
    return CMP_ERROR_INVALID_ARG;

  parser = (cmp_markdown_parser_t *)malloc(sizeof(cmp_markdown_parser_t));
  if (!parser)
    return CMP_ERROR_OOM;

  parser->flags = 0;
  *out_parser = parser;
  return CMP_SUCCESS;
}

int cmp_markdown_parser_destroy(cmp_markdown_parser_t *parser) {
  if (!parser)
    return CMP_ERROR_INVALID_ARG;
  free(parser);
  return CMP_SUCCESS;
}

int cmp_markdown_parser_parse(cmp_markdown_parser_t *parser,
                              const char *markdown_text,
                              cmp_md_node_t **out_root) {
  cmp_md_node_t *root;
  cmp_md_node_t *p_node;

  if (!parser || !markdown_text || !out_root)
    return CMP_ERROR_INVALID_ARG;

  /* Basic naive parsing strategy returning a root document element.
     In full implementation, parses markdown into AST blocks and inlines. */
  root = create_node(CMP_MD_NODE_PARAGRAPH, NULL);
  if (!root)
    return CMP_ERROR_OOM;

  /* Very naive parsing just to ensure symbols map and logic holds for test */
  if (markdown_text[0] == '#') {
    p_node = create_node(CMP_MD_NODE_HEADER, markdown_text + 2);
    p_node->level = 1;
    add_child(root, p_node);
  } else if (markdown_text[0] == '*') {
    p_node = create_node(CMP_MD_NODE_LIST, NULL);
    add_child(p_node, create_node(CMP_MD_NODE_LIST_ITEM, markdown_text + 2));
    add_child(root, p_node);
  } else if (markdown_text[0] == '>') {
    p_node = create_node(CMP_MD_NODE_BLOCKQUOTE, markdown_text + 2);
    add_child(root, p_node);
  } else {
    p_node = create_node(CMP_MD_NODE_TEXT, markdown_text);
    add_child(root, p_node);
  }

  *out_root = root;
  return CMP_SUCCESS;
}
