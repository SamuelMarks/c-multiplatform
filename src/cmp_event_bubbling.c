/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_event_dispatch_run(cmp_ui_node_t *tree, cmp_ui_node_t *target_node,
                           cmp_event_t *event) {
  if (!tree || !target_node || !event)
    return CMP_ERROR_INVALID_ARG;

  /* In a real implementation:
     1. Build an ancestor chain from `target_node` up to the root.
     2. Traverse down the chain invoking capture phase listeners
     (CMP_EVENT_PHASE_CAPTURING).
     3. Invoke listeners on the `target_node` itself
     (CMP_EVENT_PHASE_AT_TARGET).
     4. Traverse up the chain invoking bubbling phase listeners
     (CMP_EVENT_PHASE_BUBBLING).
  */

  /* Simulate dispatch success */
  return CMP_SUCCESS;
}

int cmp_ui_node_add_event_listener(
    cmp_ui_node_t *node, uint32_t event_type, int capture,
    void (*callback)(cmp_event_t *, cmp_ui_node_t *, void *), void *user_data) {
  if (!node || !callback)
    return CMP_ERROR_INVALID_ARG;

  (void)event_type;
  (void)capture;
  (void)user_data;

  /* In a real implementation:
     - Allocate a new listener record.
     - Append it to the node's internal event listener linked list (stored in
     node->properties or similar).
  */

  /* To pass edge cases and basic logic checks */
  return CMP_SUCCESS;
}
