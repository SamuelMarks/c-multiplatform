/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
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

/**
 * @brief cmp_a11y_rotor_create
 *
 * @param tree Parameter description.
 * @param out_rotor Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_rotor_create(cmp_a11y_tree_t *tree, cmp_a11y_rotor_t **out_rotor) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_rotor *rotor = NULL;

  if (!tree || !out_rotor) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_rotor_create: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_a11y_rotor), (void **)&rotor);
  if (rc != CMP_SUCCESS) {
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_rotor_create: Out of memory allocating : %s\n",
                err_str);
    }
    return rc;
  }

  rotor->tree = tree;
  rotor->nodes = NULL;
  rotor->count = 0;
  rotor->capacity = 0;

  *out_rotor = (cmp_a11y_rotor_t *)rotor;
  return rc;
}

/**
 * @brief cmp_a11y_rotor_destroy
 *
 * @param rotor Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_rotor_destroy(cmp_a11y_rotor_t *rotor) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_rotor *r = (struct cmp_a11y_rotor *)rotor;

  if (!r) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_rotor_destroy: Invalid argument (rotor=NULL): %s\n",
                err_str);
    }
    return rc;
  }

  if (r->nodes) {
    CMP_FREE(r->nodes);
  }

  CMP_FREE(r);
  return rc;
}

/**
 * @brief cmp_a11y_rotor_register_node
 *
 * @param rotor Parameter description.
 * @param node_id Parameter description.
 * @param category Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_rotor_register_node(cmp_a11y_rotor_t *rotor, int node_id,
                                 cmp_a11y_rotor_category_t category) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_rotor *r = (struct cmp_a11y_rotor *)rotor;
  cmp_a11y_rotor_node_t *new_nodes = NULL;
  size_t new_capacity;

  if (!r) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_a11y_rotor_register_node: Invalid argument "
                    "(rotor=NULL)\n");
    return rc;
  }

  if (r->count >= r->capacity) {
    new_capacity = r->capacity == 0 ? 16 : r->capacity * 2;
    rc = CMP_MALLOC(new_capacity * sizeof(cmp_a11y_rotor_node_t),
                    (void **)&new_nodes);
    if (rc != CMP_SUCCESS) {
      {
        const char *err_str;
        cmp_strerror(rc, &err_str);
        LOG_DEBUG("cmp_a11y_rotor_register_node: Out of memory : %s\n",
                  err_str);
      }
      return rc;
    }

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

  return rc;
}

/**
 * @brief cmp_a11y_rotor_get_nodes
 *
 * @param rotor Parameter description.
 * @param category Parameter description.
 * @param out_node_ids Parameter description.
 * @param max_nodes Parameter description.
 * @param out_count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_rotor_get_nodes(cmp_a11y_rotor_t *rotor,
                             cmp_a11y_rotor_category_t category,
                             int *out_node_ids, int max_nodes, int *out_count) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_rotor *r = (struct cmp_a11y_rotor *)rotor;
  size_t i;
  int count = 0;

  if (!r || !out_node_ids || !out_count || max_nodes < 0) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_rotor_get_nodes: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  for (i = 0; i < r->count; ++i) {
    if (r->nodes[i].category == category) {
      if (count < max_nodes) {
        out_node_ids[count] = r->nodes[i].node_id;
      }
      count++;
    }
  }

  *out_count = count;
  return rc;
}
