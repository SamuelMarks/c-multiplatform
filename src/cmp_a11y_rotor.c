/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

typedef struct cmp_a11y_rotor_node {
  int node_id;
  cmp_a11y_rotor_category_t category;
} cmp_a11y_rotor_node_t;

struct cmp_a11y_rotor {
  cmp_a11y_tree_t *tree;
  cmp_a11y_rotor_node_t *nodes;
  size_t count;
  size_t capacity;
};

int cmp_a11y_rotor_create(cmp_a11y_tree_t *tree, cmp_a11y_rotor_t **out_rotor) {
  struct cmp_a11y_rotor *rotor;

  if (!tree || !out_rotor)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_a11y_rotor), (void **)&rotor) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  rotor->tree = tree;
  rotor->nodes = NULL;
  rotor->count = 0;
  rotor->capacity = 0;

  *out_rotor = (cmp_a11y_rotor_t *)rotor;
  return CMP_SUCCESS;
}

int cmp_a11y_rotor_destroy(cmp_a11y_rotor_t *rotor) {
  struct cmp_a11y_rotor *r = (struct cmp_a11y_rotor *)rotor;

  if (!r)
    return CMP_ERROR_INVALID_ARG;

  if (r->nodes) {
    CMP_FREE(r->nodes);
  }

  CMP_FREE(r);
  return CMP_SUCCESS;
}

int cmp_a11y_rotor_register_node(cmp_a11y_rotor_t *rotor, int node_id,
                                 cmp_a11y_rotor_category_t category) {
  struct cmp_a11y_rotor *r = (struct cmp_a11y_rotor *)rotor;
  cmp_a11y_rotor_node_t *new_nodes;
  size_t new_capacity;

  if (!r)
    return CMP_ERROR_INVALID_ARG;

  if (r->count >= r->capacity) {
    new_capacity = r->capacity == 0 ? 16 : r->capacity * 2;
    if (CMP_MALLOC(new_capacity * sizeof(cmp_a11y_rotor_node_t),
                   (void **)&new_nodes) != CMP_SUCCESS)
      return CMP_ERROR_OOM;

    if (r->nodes) {
      memcpy(new_nodes, r->nodes, r->count * sizeof(cmp_a11y_rotor_node_t));
      CMP_FREE(r->nodes);
    }
    r->nodes = new_nodes;
    r->capacity = new_capacity;
  }

  r->nodes[r->count].node_id = node_id;
  r->nodes[r->count].category = category;
  r->count++;

  return CMP_SUCCESS;
}

int cmp_a11y_rotor_get_nodes(cmp_a11y_rotor_t *rotor,
                             cmp_a11y_rotor_category_t category,
                             int *out_node_ids, int max_nodes, int *out_count) {
  struct cmp_a11y_rotor *r = (struct cmp_a11y_rotor *)rotor;
  size_t i;
  int count = 0;

  if (!r || !out_node_ids || !out_count || max_nodes < 0)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < r->count; ++i) {
    if (r->nodes[i].category == category) {
      if (count < max_nodes) {
        out_node_ids[count] = r->nodes[i].node_id;
      }
      count++;
    }
  }

  *out_count = count;
  return CMP_SUCCESS;
}
