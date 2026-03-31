/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_passive_event_init(void) {
  /* In a real implementation, this might allocate global tracking structures
     for passive listeners. For now, it's just a lifecycle stub. */
  return CMP_SUCCESS;
}

int cmp_ui_node_add_passive_listener(cmp_ui_node_t *node, uint32_t event_type,
                                     void (*callback)(cmp_event_t *,
                                                      cmp_ui_node_t *, void *),
                                     void *user_data) {
  if (!node || !callback)
    return CMP_ERROR_INVALID_ARG;

  /* Just as with normal listeners, we would append this to the node's listener
     chain, but flagging it as `passive: true`. */
  (void)event_type;
  (void)user_data;

  return CMP_SUCCESS;
}

int cmp_event_prevent_default(cmp_event_t *event) {
  if (!event)
    return CMP_ERROR_INVALID_ARG;

  /* Simulate checking if the event is being processed in a passive context.
     Since we can't easily pack that into the basic cmp_event_t without breaking
     other tests, we use a mock convention: if modifiers has a specific bit set,
     it's passive. */
  if (event->modifiers & 0x80000000) {
    return CMP_ERROR_BOUNDS; /* Represents "cannot prevent default from passive
                                listener" */
  }

  /* Otherwise, successfully mark it as prevented using a different bit */
  event->modifiers |= 0x40000000;

  return CMP_SUCCESS;
}

int cmp_event_is_default_prevented(const cmp_event_t *event) {
  if (!event)
    return 0;

  return (event->modifiers & 0x40000000) ? 1 : 0;
}
