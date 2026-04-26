/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

struct cmp_a11y_action {
  cmp_a11y_tree_t *tree;
};

/**
 * @brief Creates an accessibility action context.
 *
 * @param tree Pointer to the accessibility tree.
 * @param out_action Pointer to store the new action context.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_action_create(cmp_a11y_tree_t *tree,
                           cmp_a11y_action_t **out_action) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_a11y_action *action = NULL;

  if (tree == NULL || out_action == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_action_create: %s\n", err_str);
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_a11y_action), (void **)&action);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_action_create CMP_MALLOC: %s\n", err_str);
    return rc;
  }

  action->tree = tree;

  *out_action = (cmp_a11y_action_t *)action;
  cmp_log_debug(
      "cmp_a11y_action_create: Successfully created a11y action context\n");
  return rc;
}

/**
 * @brief Destroys an accessibility action context.
 *
 * @param action Pointer to the action context.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_action_destroy(cmp_a11y_action_t *action) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_a11y_action *a = (struct cmp_a11y_action *)action;

  if (a == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_action_destroy: %s\n", err_str);
    return rc;
  }

  CMP_FREE(a);
  cmp_log_debug(
      "cmp_a11y_action_destroy: Successfully destroyed a11y action context\n");
  return rc;
}

/**
 * @brief Executes an accessibility action on a given node.
 *
 * @param action Pointer to the action context.
 * @param node_id The ID of the node to perform the action on.
 * @param action_type The type of action to perform.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_action_execute(cmp_a11y_action_t *action, int node_id,
                            cmp_a11y_action_type_t action_type) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_a11y_action *a = (struct cmp_a11y_action *)action;
  cmp_event_t simulated_event;

  if (a == NULL || node_id < 0) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_action_execute: %s\n", err_str);
    return rc;
  }

  memset(&simulated_event, 0, sizeof(cmp_event_t));
  simulated_event.source_id = node_id;

  /* This is a simulated adapter layer routing Screen Reader OS Actions into the
   * framework's unified event system */
  switch (action_type) {
  case CMP_A11Y_ACTION_CLICK:
    simulated_event.type = 0; /* MOUSE / TOUCH Equivalent */
    simulated_event.action = CMP_ACTION_DOWN;
    rc = cmp_event_push(&simulated_event);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_a11y_action_execute click push down: %s\n", err_str);
      return rc;
    }
    simulated_event.action = CMP_ACTION_UP;
    rc = cmp_event_push(&simulated_event);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_a11y_action_execute click push up: %s\n", err_str);
      return rc;
    }
    cmp_log_debug("cmp_a11y_action_execute: Pushed CLICK events for node %d\n",
                  node_id);
    break;

  case CMP_A11Y_ACTION_SCROLL_FORWARD:
    simulated_event.type = 2; /* SCROLL Equivalent */
    simulated_event.action = CMP_ACTION_MOVE;
    simulated_event.y = -50; /* Arbitrary scroll tick */
    rc = cmp_event_push(&simulated_event);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_a11y_action_execute scroll fwd: %s\n", err_str);
      return rc;
    }
    cmp_log_debug(
        "cmp_a11y_action_execute: Pushed SCROLL_FORWARD event for node %d\n",
        node_id);
    break;

  case CMP_A11Y_ACTION_SCROLL_BACKWARD:
    simulated_event.type = 2; /* SCROLL Equivalent */
    simulated_event.action = CMP_ACTION_MOVE;
    simulated_event.y = 50; /* Arbitrary scroll tick */
    rc = cmp_event_push(&simulated_event);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_a11y_action_execute scroll bwd: %s\n", err_str);
      return rc;
    }
    cmp_log_debug(
        "cmp_a11y_action_execute: Pushed SCROLL_BACKWARD event for node %d\n",
        node_id);
    break;

  case CMP_A11Y_ACTION_FOCUS:
    rc = cmp_event_set_focus(node_id);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_a11y_action_execute focus: %s\n", err_str);
      return rc;
    }
    cmp_log_debug("cmp_a11y_action_execute: Set focus to node %d\n", node_id);
    break;

  case CMP_A11Y_ACTION_BLUR:
    if (cmp_event_get_focus() == node_id) {
      rc = cmp_event_clear_focus();
      if (rc != CMP_SUCCESS) {
        err_rc = cmp_strerror(rc, &err_str);
        if (err_rc != CMP_SUCCESS) {
          err_str = "Unknown";
        }
        cmp_log_debug("cmp_a11y_action_execute blur: %s\n", err_str);
        return rc;
      }
      cmp_log_debug("cmp_a11y_action_execute: Cleared focus from node %d\n",
                    node_id);
    } else {
      cmp_log_debug(
          "cmp_a11y_action_execute: Node %d was not focused, ignoring BLUR\n",
          node_id);
    }
    break;

  default:
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_a11y_action_execute unhandled: %s\n", err_str);
    return rc;
  }

  return rc;
}
