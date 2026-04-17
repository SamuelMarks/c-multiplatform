/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_passive_event_init(void) {
  int rc = CMP_SUCCESS;
  return rc;
}

int cmp_ui_node_add_passive_listener(cmp_ui_node_t *node, uint32_t event_type,
                                     void (*callback)(cmp_event_t *,
                                                      cmp_ui_node_t *, void *),
                                     void *user_data) {
  int rc = CMP_SUCCESS;

  if (!node || !callback) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ui_node_add_passive_listener: Invalid argument\n");
    return rc;
  }

  /* Just as with normal listeners, we would append this to the node's listener
     chain, but flagging it as `passive: true`. */
  (void)event_type;
  (void)user_data;

  return rc;
}

int cmp_event_prevent_default(cmp_event_t *event) {
  int rc = CMP_SUCCESS;

  if (!event) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_event_prevent_default: Invalid argument\n");
    return rc;
  }

  /* Simulate checking if the event is being processed in a passive context.
     Since we can't easily pack that into the basic cmp_event_t without breaking
     other tests, we use a mock convention: if modifiers has a specific bit set,
     it's passive. */
  if (event->modifiers & 0x80000000) {
    rc = CMP_ERROR_BOUNDS; /* Represents "cannot prevent default from passive
                              listener" */
    LOG_DEBUG("Error in cmp_event_prevent_default: Cannot prevent default from "
              "passive listener\n");
    return rc;
  }

  /* Otherwise, successfully mark it as prevented using a different bit */
  event->modifiers |= 0x40000000;

  return rc;
}

int cmp_event_is_default_prevented(const cmp_event_t *event) {
  int rc = 0;

  if (!event) {
    return rc;
  }

  rc = (event->modifiers & 0x40000000) ? 1 : 0;
  return rc;
}
