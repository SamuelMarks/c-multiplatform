/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

struct cmp_a11y_action {
  cmp_a11y_tree_t *tree;
};

int cmp_a11y_action_create(cmp_a11y_tree_t *tree,
                           cmp_a11y_action_t **out_action) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_action *action = NULL;

  if (!tree || !out_action) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr,
            "Error in cmp_a11y_action_create: Invalid argument (tree=%p, "
            "out_action=%p)\n",
            (void *)tree, (void *)out_action);
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_a11y_action), (void **)&action);
  if (rc != CMP_SUCCESS) {
    fprintf(stderr,
            "Error in cmp_a11y_action_create: Out of memory allocating "
            "cmp_a11y_action (size=%u)\n",
            (unsigned int)sizeof(struct cmp_a11y_action));
    return rc;
  }

  action->tree = tree;

  *out_action = (cmp_a11y_action_t *)action;
  return rc;
}

int cmp_a11y_action_destroy(cmp_a11y_action_t *action) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_action *a = (struct cmp_a11y_action *)action;

  if (!a) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(
        stderr,
        "Error in cmp_a11y_action_destroy: Invalid argument (action=NULL)\n");
    return rc;
  }

  CMP_FREE(a);
  return rc;
}

int cmp_a11y_action_execute(cmp_a11y_action_t *action, int node_id,
                            cmp_a11y_action_type_t action_type) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_action *a = (struct cmp_a11y_action *)action;
  cmp_event_t simulated_event;

  if (!a || node_id < 0) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr,
            "Error in cmp_a11y_action_execute: Invalid argument (action=%p, "
            "node_id=%d)\n",
            (void *)a, node_id);
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
      fprintf(stderr,
              "Error in cmp_a11y_action_execute: Failed to push DOWN event "
              "(rc=%d)\n",
              rc);
      return rc;
    }
    simulated_event.action = CMP_ACTION_UP;
    rc = cmp_event_push(&simulated_event);
    if (rc != CMP_SUCCESS) {
      fprintf(
          stderr,
          "Error in cmp_a11y_action_execute: Failed to push UP event (rc=%d)\n",
          rc);
      return rc;
    }
    break;

  case CMP_A11Y_ACTION_SCROLL_FORWARD:
    simulated_event.type = 2; /* SCROLL Equivalent */
    simulated_event.action = CMP_ACTION_MOVE;
    simulated_event.y = -50; /* Arbitrary scroll tick */
    rc = cmp_event_push(&simulated_event);
    if (rc != CMP_SUCCESS) {
      fprintf(stderr,
              "Error in cmp_a11y_action_execute: Failed to push SCROLL_FORWARD "
              "event (rc=%d)\n",
              rc);
      return rc;
    }
    break;

  case CMP_A11Y_ACTION_SCROLL_BACKWARD:
    simulated_event.type = 2; /* SCROLL Equivalent */
    simulated_event.action = CMP_ACTION_MOVE;
    simulated_event.y = 50; /* Arbitrary scroll tick */
    rc = cmp_event_push(&simulated_event);
    if (rc != CMP_SUCCESS) {
      fprintf(stderr,
              "Error in cmp_a11y_action_execute: Failed to push "
              "SCROLL_BACKWARD event (rc=%d)\n",
              rc);
      return rc;
    }
    break;

  case CMP_A11Y_ACTION_FOCUS:
    rc = cmp_event_set_focus(node_id);
    if (rc != CMP_SUCCESS) {
      fprintf(stderr,
              "Error in cmp_a11y_action_execute: Failed to set focus on node "
              "%d (rc=%d)\n",
              node_id, rc);
      return rc;
    }
    break;

  case CMP_A11Y_ACTION_BLUR:
    if (cmp_event_get_focus() == node_id) {
      rc = cmp_event_clear_focus();
      if (rc != CMP_SUCCESS) {
        fprintf(
            stderr,
            "Error in cmp_a11y_action_execute: Failed to clear focus (rc=%d)\n",
            rc);
        return rc;
      }
    }
    break;

  default:
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr,
            "Error in cmp_a11y_action_execute: Unhandled action type %d\n",
            (int)action_type);
    return rc;
  }

  return rc;
}