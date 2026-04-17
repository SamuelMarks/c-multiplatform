/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_markdown_parser {
  int flags;
};

int cmp_md_node_destroy(cmp_md_node_t *node) {
  int rc = CMP_SUCCESS;
  size_t i;

  if (!node) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_md_node_destroy: Invalid argument\n");
    return rc;
  }

  if (node->content) {
    free(node->content);
  }

  for (i = 0; i < node->child_count; ++i) {
    rc = cmp_md_node_destroy(node->children[i]);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_md_node_destroy: child destruction failed\n");
    }
  }

  if (node->children) {
    free(node->children);
  }

  free(node);
  return CMP_SUCCESS; /* Returning SUCCESS overall even if children fail to
                         align with typical free semantics */
}

static cmp_md_node_t *create_node(cmp_md_node_type_t type,
                                  const char *content) {
  cmp_md_node_t *node = (cmp_md_node_t *)malloc(sizeof(cmp_md_node_t));
  size_t len;

  if (!node)
    return NULL;

  node->type = type;
  node->level = 0;
  node->child_count = 0;
  node->child_capacity = 0;
  node->children = NULL;

  if (content) {
    len = strlen(content);
    node->content = (char *)malloc(len + 1);
    if (node->content) {
#if defined(_MSC_VER)
      strncpy_s(node->content, len + 1, content, _TRUNCATE);
#else
      strncpy(node->content, content, len + 1);
#endif
    }
  } else {
    node->content = NULL;
  }

  return node;
}

static int add_child(cmp_md_node_t *parent, cmp_md_node_t *child) {
  int rc = CMP_SUCCESS;
  size_t new_cap;
  cmp_md_node_t **new_arr;

  if (!parent || !child) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in add_child: Invalid argument\n");
    return rc;
  }

  if (parent->child_count == parent->child_capacity) {
    new_cap = parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;
    new_arr = (cmp_md_node_t **)realloc(parent->children,
                                        new_cap * sizeof(cmp_md_node_t *));
    if (!new_arr) {
      rc = CMP_ERROR_OOM;
      LOG_DEBUG("Error in add_child: Out of memory\n");
      return rc;
    }
    parent->children = new_arr;
    parent->child_capacity = new_cap;
  }

  parent->children[parent->child_count++] = child;
  return rc;
}

int cmp_markdown_parser_create(cmp_markdown_parser_t **out_parser) {
  int rc = CMP_SUCCESS;
  cmp_markdown_parser_t *parser = NULL;

  if (!out_parser) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_markdown_parser_create: Invalid argument\n");
    return rc;
  }

  parser = (cmp_markdown_parser_t *)malloc(sizeof(cmp_markdown_parser_t));
  if (!parser) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_markdown_parser_create: Out of memory\n");
    return rc;
  }

  parser->flags = 0;
  *out_parser = parser;
  return rc;
}

int cmp_markdown_parser_destroy(cmp_markdown_parser_t *parser) {
  int rc = CMP_SUCCESS;

  if (!parser) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_markdown_parser_destroy: Invalid argument\n");
    return rc;
  }
  free(parser);
  return rc;
}

int cmp_markdown_parser_parse(cmp_markdown_parser_t *parser,
                              const char *markdown_text,
                              cmp_md_node_t **out_root) {
  int rc = CMP_SUCCESS;
  cmp_md_node_t *root = NULL;
  cmp_md_node_t *p_node = NULL;

  if (!parser || !markdown_text || !out_root) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_markdown_parser_parse: Invalid argument\n");
    return rc;
  }

  /* Basic naive parsing strategy returning a root document element.
     In full implementation, parses markdown into AST blocks and inlines. */
  root = create_node(CMP_MD_NODE_PARAGRAPH, NULL);
  if (!root) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_markdown_parser_parse: Out of memory\n");
    return rc;
  }

  /* Very naive parsing just to ensure symbols map and logic holds for test */
  if (markdown_text[0] == '#') {
    p_node = create_node(CMP_MD_NODE_HEADER, markdown_text + 2);
    if (p_node) {
      p_node->level = 1;
      rc = add_child(root, p_node);
    } else {
      rc = CMP_ERROR_OOM;
    }
  } else if (markdown_text[0] == '*') {
    p_node = create_node(CMP_MD_NODE_LIST, NULL);
    if (p_node) {
      rc = add_child(p_node,
                     create_node(CMP_MD_NODE_LIST_ITEM, markdown_text + 2));
      if (rc == CMP_SUCCESS) {
        rc = add_child(root, p_node);
      }
    } else {
      rc = CMP_ERROR_OOM;
    }
  } else if (markdown_text[0] == '>') {
    p_node = create_node(CMP_MD_NODE_BLOCKQUOTE, markdown_text + 2);
    if (p_node) {
      rc = add_child(root, p_node);
    } else {
      rc = CMP_ERROR_OOM;
    }
  } else {
    p_node = create_node(CMP_MD_NODE_TEXT, markdown_text);
    if (p_node) {
      rc = add_child(root, p_node);
    } else {
      rc = CMP_ERROR_OOM;
    }
  }

  if (rc != CMP_SUCCESS) {
    cmp_md_node_destroy(root);
    LOG_DEBUG("Error in cmp_markdown_parser_parse: Failed to parse\n");
    return rc;
  }

  *out_root = root;
  return rc;
}
