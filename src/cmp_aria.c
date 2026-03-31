/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

typedef struct cmp_aria_node {
  int node_id;
  cmp_aria_role_t role;
  /* Simulated state storage */
  int is_expanded;
  int is_checked;
} cmp_aria_node_t;

struct cmp_aria {
  cmp_a11y_tree_t *tree;
  cmp_aria_node_t *nodes;
  size_t count;
  size_t capacity;
};

int cmp_aria_create(cmp_a11y_tree_t *tree, cmp_aria_t **out_aria) {
  struct cmp_aria *aria;

  if (!tree || !out_aria)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_aria), (void **)&aria) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  aria->tree = tree;
  aria->nodes = NULL;
  aria->count = 0;
  aria->capacity = 0;

  *out_aria = (cmp_aria_t *)aria;
  return CMP_SUCCESS;
}

int cmp_aria_destroy(cmp_aria_t *aria) {
  struct cmp_aria *a = (struct cmp_aria *)aria;

  if (!a)
    return CMP_ERROR_INVALID_ARG;

  if (a->nodes) {
    CMP_FREE(a->nodes);
  }

  CMP_FREE(a);
  return CMP_SUCCESS;
}

static cmp_aria_node_t *_cmp_aria_find_or_add_node(struct cmp_aria *a,
                                                   int node_id) {
  size_t i;
  cmp_aria_node_t *new_nodes;
  size_t new_capacity;

  for (i = 0; i < a->count; ++i) {
    if (a->nodes[i].node_id == node_id) {
      return &a->nodes[i];
    }
  }

  if (a->count >= a->capacity) {
    new_capacity = a->capacity == 0 ? 16 : a->capacity * 2;
    if (CMP_MALLOC(new_capacity * sizeof(cmp_aria_node_t),
                   (void **)&new_nodes) != CMP_SUCCESS)
      return NULL;

    if (a->nodes) {
      memcpy(new_nodes, a->nodes, a->count * sizeof(cmp_aria_node_t));
      CMP_FREE(a->nodes);
    }
    a->nodes = new_nodes;
    a->capacity = new_capacity;
  }

  a->nodes[a->count].node_id = node_id;
  a->nodes[a->count].role = CMP_ARIA_ROLE_NONE;
  a->nodes[a->count].is_expanded = -1; /* -1 means unset */
  a->nodes[a->count].is_checked = -1;
  a->count++;

  return &a->nodes[a->count - 1];
}

int cmp_aria_set_role(cmp_aria_t *aria, int node_id, cmp_aria_role_t role) {
  struct cmp_aria *a = (struct cmp_aria *)aria;
  cmp_aria_node_t *node;

  if (!a)
    return CMP_ERROR_INVALID_ARG;

  node = _cmp_aria_find_or_add_node(a, node_id);
  if (!node)
    return CMP_ERROR_OOM;

  node->role = role;
  return CMP_SUCCESS;
}

int cmp_aria_set_state_bool(cmp_aria_t *aria, int node_id,
                            const char *state_name, int value) {
  struct cmp_aria *a = (struct cmp_aria *)aria;
  cmp_aria_node_t *node;

  if (!a || !state_name)
    return CMP_ERROR_INVALID_ARG;

  node = _cmp_aria_find_or_add_node(a, node_id);
  if (!node)
    return CMP_ERROR_OOM;

  if (strcmp(state_name, "aria-expanded") == 0) {
    node->is_expanded = value ? 1 : 0;
  } else if (strcmp(state_name, "aria-checked") == 0) {
    node->is_checked = value ? 1 : 0;
  }
  /* Other states would be handled similarly */

  return CMP_SUCCESS;
}

int cmp_aria_sync(cmp_aria_t *aria) {
  struct cmp_aria *a = (struct cmp_aria *)aria;
  size_t i;
  const char *role_str;

  if (!a)
    return CMP_ERROR_INVALID_ARG;

  /* Iterate through the ARIA nodes and sync them with the underlying a11y tree
   */
  for (i = 0; i < a->count; ++i) {
    switch (a->nodes[i].role) {
    case CMP_ARIA_ROLE_BUTTON:
      role_str = "button";
      break;
    case CMP_ARIA_ROLE_CHECKBOX:
      role_str = "checkbox";
      break;
    case CMP_ARIA_ROLE_DIALOG:
      role_str = "dialog";
      break;
    case CMP_ARIA_ROLE_HEADING:
      role_str = "heading";
      break;
    case CMP_ARIA_ROLE_LINK:
      role_str = "link";
      break;
    case CMP_ARIA_ROLE_TABLIST:
      role_str = "tablist";
      break;
    case CMP_ARIA_ROLE_TAB:
      role_str = "tab";
      break;
    case CMP_ARIA_ROLE_TABPANEL:
      role_str = "tabpanel";
      break;
    case CMP_ARIA_ROLE_TOOLTIP:
      role_str = "tooltip";
      break;
    default:
      role_str = NULL;
      break;
    }

    if (role_str) {
      /* Using the add_node function to update/overwrite the role if it exists.
       * A real implementation might have a specialized update method, but for
       * this framework architecture, updating via re-adding or internal mapping
       * is the simulated path.
       */
      cmp_a11y_tree_add_node(a->tree, a->nodes[i].node_id, role_str, NULL);
    }
  }

  return CMP_SUCCESS;
}
