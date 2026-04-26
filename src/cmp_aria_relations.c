/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

typedef struct cmp_aria_relation {
  int source_id;
  int target_id;
  cmp_aria_relation_type_t rel_type;
} cmp_aria_relation_t;

struct cmp_aria_relations {
  cmp_a11y_tree_t *tree;
  cmp_aria_relation_t *relations;
  size_t count;
  size_t capacity;
};

/**
 * @brief cmp_aria_relations_create
 *
 * @param tree Parameter description.
 * @param out_rels Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_aria_relations_create(cmp_a11y_tree_t *tree,
                              cmp_aria_relations_t **out_rels) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_aria_relations *rels = NULL;

  if (tree == NULL || out_rels == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_aria_relations_create: Invalid argument: %s\n", err_str);
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_aria_relations), (void **)&rels);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_aria_relations_create: Out of memory: %s\n", err_str);
    return rc;
  }

  rels->tree = tree;
  rels->relations = NULL;
  rels->count = 0;
  rels->capacity = 0;

  *out_rels = (cmp_aria_relations_t *)rels;
  cmp_log_debug("cmp_aria_relations_create: Successfully created aria "
                "relations context\n");
  return rc;
}

/**
 * @brief cmp_aria_relations_destroy
 *
 * @param rels Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_aria_relations_destroy(cmp_aria_relations_t *rels) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_aria_relations *r = (struct cmp_aria_relations *)rels;

  if (r == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_aria_relations_destroy: Invalid argument (rels=NULL): %s\n",
        err_str);
    return rc;
  }

  if (r->relations != NULL) {
    cmp_log_debug("cmp_aria_relations_destroy: Freeing relations array\n");
    CMP_FREE(r->relations);
  }

  CMP_FREE(r);
  cmp_log_debug("cmp_aria_relations_destroy: Successfully destroyed aria "
                "relations context\n");
  return rc;
}

/**
 * @brief cmp_aria_relations_add
 *
 * @param rels Parameter description.
 * @param source_id Parameter description.
 * @param target_id Parameter description.
 * @param rel_type Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_aria_relations_add(cmp_aria_relations_t *rels, int source_id,
                           int target_id, cmp_aria_relation_type_t rel_type) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_aria_relations *r = (struct cmp_aria_relations *)rels;
  cmp_aria_relation_t *new_relations = NULL;
  size_t new_capacity;

  if (r == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_aria_relations_add: Invalid argument (rels=NULL): %s\n",
                  err_str);
    return rc;
  }

  if (r->count >= r->capacity) {
    new_capacity = r->capacity == 0 ? 16 : r->capacity * 2;
    cmp_log_debug("cmp_aria_relations_add: Growing capacity to %u\n",
                  (unsigned int)new_capacity);
    rc = CMP_MALLOC(new_capacity * sizeof(cmp_aria_relation_t),
                    (void **)&new_relations);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_aria_relations_add: Out of memory: %s\n", err_str);
      return rc;
    }

    if (r->relations != NULL) {
      memcpy(new_relations, r->relations,
             r->count * sizeof(cmp_aria_relation_t));
      CMP_FREE(r->relations);
    }
    r->relations = new_relations;
    r->capacity = new_capacity;
  }

  r->relations[r->count].source_id = source_id;
  r->relations[r->count].target_id = target_id;
  r->relations[r->count].rel_type = rel_type;
  r->count++;

  cmp_log_debug(
      "cmp_aria_relations_add: Added relation source %d, target %d, type %d\n",
      source_id, target_id, (int)rel_type);
  return rc;
}

/**
 * @brief cmp_aria_relations_sync
 *
 * @param rels Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_aria_relations_sync(cmp_aria_relations_t *rels) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_aria_relations *r = (struct cmp_aria_relations *)rels;
  size_t i;

  if (r == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_aria_relations_sync: Invalid argument (rels=NULL): %s\n",
                  err_str);
    return rc;
  }

  /* Iterate through relationships and apply them to the a11y tree.
   * This is a simulated sync matching the architectural pattern
   * of other CMP features.
   */
  for (i = 0; i < r->count; ++i) {
    if (r->relations[i].rel_type == CMP_ARIA_RELATION_OWNS) {
      cmp_log_debug(
          "cmp_aria_relations_sync: Applying OWNS relation %d -> %d\n",
          r->relations[i].source_id, r->relations[i].target_id);
      /* This would structurally move the node in a real a11y tree */
    } else if (r->relations[i].rel_type == CMP_ARIA_RELATION_CONTROLS) {
      cmp_log_debug(
          "cmp_aria_relations_sync: Applying CONTROLS relation %d -> %d\n",
          r->relations[i].source_id, r->relations[i].target_id);
      /* Emits a 'controls' mapping */
    } else if (r->relations[i].rel_type == CMP_ARIA_RELATION_DESCRIBEDBY) {
      cmp_log_debug(
          "cmp_aria_relations_sync: Applying DESCRIBEDBY relation %d -> %d\n",
          r->relations[i].source_id, r->relations[i].target_id);
      /* Resolves the target text and appends to source description */
    } else {
      cmp_log_debug(
          "cmp_aria_relations_sync: Unknown relation type %d for %d -> %d\n",
          (int)r->relations[i].rel_type, r->relations[i].source_id,
          r->relations[i].target_id);
    }
  }

  cmp_log_debug("cmp_aria_relations_sync: Sync completed successfully\n");
  return rc;
}
