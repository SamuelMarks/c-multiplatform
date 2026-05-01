/* clang-format off */
#include "cmp_tree_sitter.h"
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include "cmp_log.h"
/* clang-format on */

/**
 * @brief Internal string duplication utility using CMP memory allocators.
 *
 * @param s The string to duplicate.
 * @return Pointer to the duplicated string, or NULL on memory failure.
 */
static char *cmp_math_codex_strdup(const char *s) {
  int rc = CMP_SUCCESS;
  size_t len;
  char *d;
  if (!s)
    return NULL;
  len = strlen(s);
  rc = CMP_MALLOC(len + 1, (void **)&(d));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return NULL;
  }
#if defined(_MSC_VER)
  strcpy_s(d, len + 1, s);
#else
  strcpy(d, s);
#endif
  return d;
}
#define strdup cmp_math_codex_strdup

struct cmp_tree_sitter {
  int dummy;
};

struct cmp_tree_node {
  char *type;
  struct cmp_tree_node **children;
  size_t child_count;
};

/**
 * @brief Creates a new Tree-Sitter parser context.
 *
 * @param out_ts Pointer to store the newly created Tree-Sitter context.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tree_sitter_create(cmp_tree_sitter_t **out_ts) {
  int rc = CMP_SUCCESS;
  cmp_tree_sitter_t *ts;
  if (!out_ts) {
    return CMP_ERROR_INVALID_ARG;
  }
  rc = CMP_MALLOC(sizeof(cmp_tree_sitter_t), (void **)&(ts));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }
  ts->dummy = 0;
  *out_ts = ts;
  return rc;
}

/**
 * @brief Destroys a Tree-Sitter parser context and frees its resources.
 *
 * @param ts Pointer to the Tree-Sitter context to destroy.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tree_sitter_destroy(cmp_tree_sitter_t *ts) {
  int rc = CMP_SUCCESS;
  if (!ts) {
    return CMP_ERROR_INVALID_ARG;
  }
  rc = CMP_FREE(ts);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  return rc;
}

/**
 * @brief Parses source code using the specified language grammar.
 *
 * @param ts Pointer to the Tree-Sitter context.
 * @param language The name of the language.
 * @param source_code The source code to parse.
 * @param out_root Pointer to store the root AST node.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tree_sitter_parse(cmp_tree_sitter_t *ts, const char *language,
                          const char *source_code, cmp_tree_node_t **out_root) {
  int rc = CMP_SUCCESS;
  cmp_tree_node_t *root;
  if (!ts || !language || !source_code || !out_root) {
    return CMP_ERROR_INVALID_ARG;
  }
  rc = CMP_MALLOC(sizeof(cmp_tree_node_t), (void **)&(root));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }
  /* Mock root node representing a translation_unit */
  root->type = strdup("translation_unit");
  root->children = NULL;
  root->child_count = 0;

  *out_root = root;
  return rc;
}

/**
 * @brief Retrieves the string type name of an AST node.
 *
 * @param node Pointer to the AST node.
 * @param out_type Pointer to store the returned type string.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tree_sitter_node_get_type(cmp_tree_node_t *node, char **out_type) {
  int rc = CMP_SUCCESS;
  if (!node || !out_type) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (!node->type) {
    *out_type = NULL;
    return rc;
  }
  *out_type = strdup(node->type);
  if (!*out_type) {
    return CMP_ERROR_OOM;
  }

  return rc;
}

/**
 * @brief Recursively frees an AST node and its children.
 *
 * @param node Pointer to the AST node to free.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tree_sitter_free_node(cmp_tree_node_t *node) {
  int rc = CMP_SUCCESS;
  size_t i;
  if (!node) {
    return rc;
  }
  if (node->type) {
    rc = CMP_FREE(node->type);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
  }
  if (node->children) {
    for (i = 0; i < node->child_count; i++) {
      cmp_tree_sitter_free_node(node->children[i]);
    }
    rc = CMP_FREE(node->children);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
  }
  rc = CMP_FREE(node);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  return rc;
}

/**
 * @brief Frees a string returned by the Tree-Sitter module.
 *
 * @param str The string pointer to free.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tree_sitter_free_string(char *str) {
  int rc = CMP_SUCCESS;
  if (str) {
    rc = CMP_FREE(str);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
  }
  return rc;
}
