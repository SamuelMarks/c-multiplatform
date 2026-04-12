/* clang-format off */
#include "cmp_tree_sitter.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static char *codex_strdup(const char *s) {
  size_t len;
  char *d;
  if (!s)
    return NULL;
  len = strlen(s);
  d = (char *)malloc(len + 1);
  if (!d)
    return NULL;
#if defined(_MSC_VER)
  strcpy_s(d, len + 1, s);
#else
  strcpy(d, s);
#endif
  return d;
}
#define strdup codex_strdup

struct cmp_tree_sitter {
  int dummy;
};

struct cmp_tree_node {
  char *type;
  struct cmp_tree_node **children;
  size_t child_count;
};

int cmp_tree_sitter_create(cmp_tree_sitter_t **out_ts) {
  cmp_tree_sitter_t *ts;
  if (!out_ts) {
    return -1;
  }
  ts = (cmp_tree_sitter_t *)malloc(sizeof(cmp_tree_sitter_t));
  if (!ts) {
    return -2;
  }
  ts->dummy = 0;
  *out_ts = ts;
  return 0;
}

int cmp_tree_sitter_destroy(cmp_tree_sitter_t *ts) {
  if (!ts) {
    return -1;
  }
  free(ts);
  return 0;
}

int cmp_tree_sitter_parse(cmp_tree_sitter_t *ts, const char *language,
                          const char *source_code, cmp_tree_node_t **out_root) {
  cmp_tree_node_t *root;
  if (!ts || !language || !source_code || !out_root) {
    return -1;
  }
  root = (cmp_tree_node_t *)malloc(sizeof(cmp_tree_node_t));
  if (!root) {
    return -2;
  }
  /* Mock root node representing a translation_unit */
  root->type = strdup("translation_unit");
  root->children = NULL;
  root->child_count = 0;

  *out_root = root;
  return 0;
}

int cmp_tree_sitter_node_get_type(cmp_tree_node_t *node, char **out_type) {
  if (!node || !out_type) {
    return -1;
  }
  if (!node->type) {
    *out_type = NULL;
    return 0;
  }
  *out_type = strdup(node->type);
  if (!*out_type) {
    return -2;
  }
  return 0;
}

int cmp_tree_sitter_free_node(cmp_tree_node_t *node) {
  size_t i;
  if (!node) {
    return 0;
  }
  if (node->type) {
    free(node->type);
  }
  if (node->children) {
    for (i = 0; i < node->child_count; i++) {
      cmp_tree_sitter_free_node(node->children[i]);
    }
    free(node->children);
  }
  free(node);
  return 0;
}

int cmp_tree_sitter_free_string(char *str) {
  if (str) {
    free(str);
  }
  return 0;
}
