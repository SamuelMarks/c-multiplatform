/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

typedef struct cmp_a11y_tree_node {
  int id;
  char *role;
  char *name;
} cmp_a11y_tree_node_t;

struct cmp_a11y_tree {
  cmp_a11y_tree_node_t *nodes;
  size_t count;
  size_t capacity;
};

int cmp_a11y_tree_create(cmp_a11y_tree_t **out_tree) {
  struct cmp_a11y_tree *tree;

  if (!out_tree)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_a11y_tree), (void **)&tree) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  tree->nodes = NULL;
  tree->count = 0;
  tree->capacity = 0;

  *out_tree = (cmp_a11y_tree_t *)tree;
  return CMP_SUCCESS;
}

int cmp_a11y_tree_destroy(cmp_a11y_tree_t *tree) {
  size_t i;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;

  if (!t)
    return CMP_ERROR_INVALID_ARG;

  if (t->nodes) {
    for (i = 0; i < t->count; ++i) {
      if (t->nodes[i].role) {
        CMP_FREE(t->nodes[i].role);
      }
      if (t->nodes[i].name) {
        CMP_FREE(t->nodes[i].name);
      }
    }
    CMP_FREE(t->nodes);
  }

  CMP_FREE(t);
  return CMP_SUCCESS;
}

int cmp_a11y_tree_add_node(cmp_a11y_tree_t *tree, int node_id, const char *role,
                           const char *name) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  cmp_a11y_tree_node_t *new_nodes;
  size_t new_capacity;
  size_t len;

  if (!t)
    return CMP_ERROR_INVALID_ARG;

  if (t->count >= t->capacity) {
    new_capacity = t->capacity == 0 ? 16 : t->capacity * 2;
    if (CMP_MALLOC(new_capacity * sizeof(cmp_a11y_tree_node_t),
                   (void **)&new_nodes) != CMP_SUCCESS)
      return CMP_ERROR_OOM;

    if (t->nodes) {
      memcpy(new_nodes, t->nodes, t->count * sizeof(cmp_a11y_tree_node_t));
      CMP_FREE(t->nodes);
    }
    t->nodes = new_nodes;
    t->capacity = new_capacity;
  }

  t->nodes[t->count].id = node_id;

  t->nodes[t->count].role = NULL;
  if (role) {
    len = strlen(role);
    if (CMP_MALLOC(len + 1, (void **)&t->nodes[t->count].role) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
#if defined(_MSC_VER)
    strcpy_s(t->nodes[t->count].role, len + 1, role);
#else
    strcpy(t->nodes[t->count].role, role);
#endif
  }

  t->nodes[t->count].name = NULL;
  if (name) {
    len = strlen(name);
    if (CMP_MALLOC(len + 1, (void **)&t->nodes[t->count].name) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
#if defined(_MSC_VER)
    strcpy_s(t->nodes[t->count].name, len + 1, name);
#else
    strcpy(t->nodes[t->count].name, name);
#endif
  }

  t->count++;
  return CMP_SUCCESS;
}

int cmp_a11y_tree_get_node_desc(cmp_a11y_tree_t *tree, int node_id,
                                char *out_desc, size_t out_capacity) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;
  const char *role_str;
  const char *name_str;

  if (!t || !out_desc || out_capacity == 0)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      size_t role_len;
      size_t name_len;

      role_str = t->nodes[i].role ? t->nodes[i].role : "unknown";
      name_str = t->nodes[i].name ? t->nodes[i].name : "unnamed";

      role_len = strlen(role_str);
      name_len = strlen(name_str);
      if (role_len + 2 + name_len + 1 > out_capacity) {
        return CMP_ERROR_BOUNDS;
      }

#if defined(_MSC_VER)
      sprintf_s(out_desc, out_capacity, "%s: %s", role_str, name_str);
#else
      sprintf(out_desc, "%s: %s", role_str, name_str);
#endif
      return CMP_SUCCESS;
    }
  }

  return CMP_ERROR_NOT_FOUND;
}
