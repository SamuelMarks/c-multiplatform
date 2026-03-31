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
  struct cmp_a11y_action *action;

  if (!tree || !out_action)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_a11y_action), (void **)&action) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  action->tree = tree;

  *out_action = (cmp_a11y_action_t *)action;
  return CMP_SUCCESS;
}

int cmp_a11y_action_destroy(cmp_a11y_action_t *action) {
  struct cmp_a11y_action *a = (struct cmp_a11y_action *)action;

  if (!a)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(a);
  return CMP_SUCCESS;
}

int cmp_a11y_action_execute(cmp_a11y_action_t *action, int node_id,
                            cmp_a11y_action_type_t action_type) {
  struct cmp_a11y_action *a = (struct cmp_a11y_action *)action;
  cmp_event_t simulated_event;

  if (!a || node_id < 0)
    return CMP_ERROR_INVALID_ARG;

  memset(&simulated_event, 0, sizeof(cmp_event_t));
  simulated_event.source_id = node_id;

  /* This is a simulated adapter layer routing Screen Reader OS Actions into the
   * framework's unified event system */
  switch (action_type) {
  case CMP_A11Y_ACTION_CLICK:
    simulated_event.type = 0; /* MOUSE / TOUCH Equivalent */
    simulated_event.action = CMP_ACTION_DOWN;
    cmp_event_push(&simulated_event);
    simulated_event.action = CMP_ACTION_UP;
    cmp_event_push(&simulated_event);
    break;

  case CMP_A11Y_ACTION_SCROLL_FORWARD:
    simulated_event.type = 2; /* SCROLL Equivalent */
    simulated_event.action = CMP_ACTION_MOVE;
    simulated_event.y = -50; /* Arbitrary scroll tick */
    cmp_event_push(&simulated_event);
    break;

  case CMP_A11Y_ACTION_SCROLL_BACKWARD:
    simulated_event.type = 2; /* SCROLL Equivalent */
    simulated_event.action = CMP_ACTION_MOVE;
    simulated_event.y = 50; /* Arbitrary scroll tick */
    cmp_event_push(&simulated_event);
    break;

  case CMP_A11Y_ACTION_FOCUS:
    cmp_event_set_focus(node_id);
    break;

  case CMP_A11Y_ACTION_BLUR:
    if (cmp_event_get_focus() == node_id) {
      cmp_event_clear_focus();
    }
    break;

  default:
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}
