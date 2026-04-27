/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

typedef struct cmp_aria_live_node {
  int node_id;
  cmp_aria_live_mode_t mode;
} cmp_aria_live_node_t;

struct cmp_aria_live {
  cmp_a11y_tree_t *tree;
  cmp_aria_live_node_t *nodes;
  size_t count;
  size_t capacity;
};

/**
 * @brief cmp_aria_live_create
 *
 * @param tree Parameter description.
 * @param out_live Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_aria_live_create(cmp_a11y_tree_t *tree, cmp_aria_live_t **out_live) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_aria_live *live = NULL;

  if (tree == NULL || out_live == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_aria_live_create: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_aria_live), (void **)&live);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_aria_live_create: Out of memory: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  live->tree = tree;
  live->nodes = NULL;
  live->count = 0;
  live->capacity = 0;

  *out_live = (cmp_aria_live_t *)live;
  cmp_log_debug(
      "cmp_aria_live_create: Successfully created aria live context\n");
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_aria_live_destroy
 *
 * @param live Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_aria_live_destroy(cmp_aria_live_t *live) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_aria_live *l = (struct cmp_aria_live *)live;

  if (l == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_aria_live_destroy: Invalid argument (live=NULL): %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (l->nodes != NULL) {
    cmp_log_debug("cmp_aria_live_destroy: Freeing nodes array\n");
    CMP_FREE(l->nodes);
  }

  CMP_FREE(l);
  cmp_log_debug(
      "cmp_aria_live_destroy: Successfully destroyed aria live context\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_aria_live_set_mode
 *
 * @param live Parameter description.
 * @param node_id Parameter description.
 * @param mode Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_aria_live_set_mode(cmp_aria_live_t *live, int node_id,
                           cmp_aria_live_mode_t mode) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_aria_live *l = (struct cmp_aria_live *)live;
  cmp_aria_live_node_t *new_nodes = NULL;
  size_t new_capacity;
  size_t i;

  if (l == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_aria_live_set_mode: Invalid argument (live=NULL): %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* Check if it already exists to update it */
  for (i = 0; i < l->count; ++i) {
    if (l->nodes[i].node_id == node_id) {
      l->nodes[i].mode = mode;
      cmp_log_debug(
          "cmp_aria_live_set_mode: Updated existing node %d mode to %d\n",
          node_id, (int)mode);
      if (rc != 0) {
        return rc;
      }
      return rc;
    }
  }

  /* Add new node */
  if (l->count >= l->capacity) {
    new_capacity = l->capacity == 0 ? 16 : l->capacity * 2;
    cmp_log_debug("cmp_aria_live_set_mode: Growing capacity to %u\n",
                  (unsigned int)new_capacity);
    rc = CMP_MALLOC(new_capacity * sizeof(cmp_aria_live_node_t),
                    (void **)&new_nodes);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_aria_live_set_mode: Out of memory: %s\n", err_str);
      if (rc != 0) {
        return rc;
      }
      return rc;
    }

    if (l->nodes != NULL) {
      /* Can't easily check error from memcpy in C89 */
      memcpy(new_nodes, l->nodes, l->count * sizeof(cmp_aria_live_node_t));
      CMP_FREE(l->nodes);
    }
    l->nodes = new_nodes;
    l->capacity = new_capacity;
  }

  l->nodes[l->count].node_id = node_id;
  l->nodes[l->count].mode = mode;
  l->count++;

  cmp_log_debug("cmp_aria_live_set_mode: Added new node %d with mode %d\n",
                node_id, (int)mode);
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_aria_live_announce
 *
 * @param live Parameter description.
 * @param node_id Parameter description.
 * @param message Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_aria_live_announce(cmp_aria_live_t *live, int node_id,
                           const char *message) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_aria_live *l = (struct cmp_aria_live *)live;
  size_t i;
  cmp_aria_live_mode_t mode = CMP_ARIA_LIVE_OFF;

  if (l == NULL || message == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_aria_live_announce: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* Find the mode of the node */
  for (i = 0; i < l->count; ++i) {
    if (l->nodes[i].node_id == node_id) {
      mode = l->nodes[i].mode;
      cmp_log_debug("cmp_aria_live_announce: Found node %d with mode %d\n",
                    node_id, (int)mode);
      break;
    }
  }

  if (mode == CMP_ARIA_LIVE_OFF) {
    cmp_log_debug("cmp_aria_live_announce: Node %d mode is OFF, skipping\n",
                  node_id);
    if (rc != 0) {
      return rc;
    }
    return rc; /* No announcement needed */
  }

  if (mode == CMP_ARIA_LIVE_POLITE) {
    cmp_log_debug("cmp_aria_live_announce: Queueing polite announcement: %s\n",
                  message);
    /* Queue for polite announcement */
  } else if (mode == CMP_ARIA_LIVE_ASSERTIVE) {
    cmp_log_debug(
        "cmp_aria_live_announce: Interrupting for assertive announcement: %s\n",
        message);
    /* Interrupt screen reader for assertive announcement */
  }

  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
