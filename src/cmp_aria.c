/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
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

/**
 * @brief Find or add a node internally
 *
 * @param a The aria context
 * @param node_id The ID to find or add
 * @param out_node The output pointer to the found/added node
 * @return Returns 0 on success, or an error code on failure.
 */
static int _cmp_aria_find_or_add_node(struct cmp_aria *a, int node_id,
                                      cmp_aria_node_t **out_node) {
  int rc = CMP_SUCCESS;
  size_t i;
  cmp_aria_node_t *new_nodes = NULL;
  size_t new_capacity;

  if (a == NULL || out_node == NULL) {
    cmp_log_debug("_cmp_aria_find_or_add_node: NULL argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < a->count; ++i) {
    if (a->nodes[i].node_id == node_id) {
      *out_node = &a->nodes[i];
      cmp_log_debug("_cmp_aria_find_or_add_node: Found existing node %d\n",
                    node_id);
      return CMP_SUCCESS;
    }
  }

  if (a->count >= a->capacity) {
    new_capacity = a->capacity == 0 ? 16 : a->capacity * 2;
    cmp_log_debug("_cmp_aria_find_or_add_node: Growing capacity to %u\n",
                  (unsigned int)new_capacity);
    rc =
        CMP_MALLOC(new_capacity * sizeof(cmp_aria_node_t), (void **)&new_nodes);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("_cmp_aria_find_or_add_node: Out of memory\n");
      return rc;
    }

    if (a->nodes != NULL) {
      /* Can't easily check error from memcpy in C89 */
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
  *out_node = &a->nodes[a->count];
  a->count++;

  cmp_log_debug("_cmp_aria_find_or_add_node: Added new node %d\n", node_id);
  return CMP_SUCCESS;
}

/**
 * @brief cmp_aria_create
 *
 * @param tree Parameter description.
 * @param out_aria Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_aria_create(cmp_a11y_tree_t *tree, cmp_aria_t **out_aria) {
  int rc = CMP_SUCCESS;
  int err_rc;
  struct cmp_aria *aria = NULL;
  const char *err_str;

  if (tree == NULL || out_aria == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_aria_create: Invalid argument: %s\n", err_str);
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_aria), (void **)&aria);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_aria_create: Out of memory: %s\n", err_str);
    return rc;
  }

  aria->tree = tree;
  aria->nodes = NULL;
  aria->count = 0;
  aria->capacity = 0;

  *out_aria = (cmp_aria_t *)aria;
  cmp_log_debug("cmp_aria_create: Successfully created aria context\n");
  return rc;
}

/**
 * @brief cmp_aria_destroy
 *
 * @param aria Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_aria_destroy(cmp_aria_t *aria) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_aria *a = (struct cmp_aria *)aria;

  if (a == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_aria_destroy: Invalid argument (aria=NULL): %s\n",
                  err_str);
    return rc;
  }

  if (a->nodes != NULL) {
    cmp_log_debug("cmp_aria_destroy: Freeing nodes array\n");
    CMP_FREE(a->nodes);
  }

  CMP_FREE(a);
  cmp_log_debug("cmp_aria_destroy: Successfully destroyed aria context\n");
  return rc;
}

/**
 * @brief cmp_aria_set_role
 *
 * @param aria Parameter description.
 * @param node_id Parameter description.
 * @param role Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_aria_set_role(cmp_aria_t *aria, int node_id, cmp_aria_role_t role) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_aria *a = (struct cmp_aria *)aria;
  cmp_aria_node_t *node = NULL;

  if (a == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_aria_set_role: Invalid argument (aria=NULL): %s\n",
                  err_str);
    return rc;
  }

  rc = _cmp_aria_find_or_add_node(a, node_id, &node);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_aria_set_role: Failed to find or add node: %s\n",
                  err_str);
    return rc;
  }

  node->role = role;
  cmp_log_debug("cmp_aria_set_role: Set role %d for node %d\n", (int)role,
                node_id);
  return CMP_SUCCESS;
}

/**
 * @brief cmp_aria_set_state_bool
 *
 * @param aria Parameter description.
 * @param node_id Parameter description.
 * @param state_name Parameter description.
 * @param value Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_aria_set_state_bool(cmp_aria_t *aria, int node_id,
                            const char *state_name, int value) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_aria *a = (struct cmp_aria *)aria;
  cmp_aria_node_t *node = NULL;

  if (a == NULL || state_name == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_aria_set_state_bool: Invalid argument: %s\n", err_str);
    return rc;
  }

  rc = _cmp_aria_find_or_add_node(a, node_id, &node);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_aria_set_state_bool: Failed to find or add node: %s\n",
                  err_str);
    return rc;
  }

  if (strcmp(state_name, "aria-expanded") == 0) {
    node->is_expanded = value != 0 ? 1 : 0;
    cmp_log_debug("cmp_aria_set_state_bool: Set aria-expanded=%d for node %d\n",
                  node->is_expanded, node_id);
  } else if (strcmp(state_name, "aria-checked") == 0) {
    node->is_checked = value != 0 ? 1 : 0;
    cmp_log_debug("cmp_aria_set_state_bool: Set aria-checked=%d for node %d\n",
                  node->is_checked, node_id);
  } else {
    cmp_log_debug("cmp_aria_set_state_bool: Unknown state %s\n", state_name);
  }

  return CMP_SUCCESS;
}

/**
 * @brief cmp_aria_sync
 *
 * @param aria Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_aria_sync(cmp_aria_t *aria) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_aria *a = (struct cmp_aria *)aria;
  size_t i;
  const char *role_str;

  if (a == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_aria_sync: Invalid argument (aria=NULL): %s\n", err_str);
    return rc;
  }

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

    if (role_str != NULL) {
      rc = cmp_a11y_tree_add_node(a->tree, a->nodes[i].node_id, role_str, NULL);
      if (rc != CMP_SUCCESS) {
        err_rc = cmp_strerror(rc, &err_str);
        if (err_rc != CMP_SUCCESS) {
          err_str = "Unknown";
        }
        cmp_log_debug("cmp_aria_sync: Failed to add node %d to a11y tree: %s\n",
                      a->nodes[i].node_id, err_str);
        return rc;
      }
      cmp_log_debug("cmp_aria_sync: Synced node %d with role %s\n",
                    a->nodes[i].node_id, role_str);
    }
  }

  cmp_log_debug("cmp_aria_sync: Sync completed successfully\n");
  return CMP_SUCCESS;
}
