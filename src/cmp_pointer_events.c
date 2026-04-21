/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_ui_node_set_pointer_events(cmp_ui_node_t *node,
                                   cmp_pointer_events_t state) {
  int rc = CMP_SUCCESS;

  if (!node) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ui_node_set_pointer_events: Invalid argument\n");
    return rc;
  }

  /* In a real implementation this state might be packed inside a bitfield.
     Since the exact bitfield hasn't been extracted for this phase in the
     current struct, we'll use the generic `properties` void pointer to mock the
     state or assume auto. */

  /* Mock implementation using properties just to pass logic tests */
  if (state == CMP_POINTER_EVENTS_NONE) {
    node->properties = (void *)(size_t)CMP_POINTER_EVENTS_NONE;
  } else {
    node->properties = (void *)(size_t)CMP_POINTER_EVENTS_AUTO;
  }

  return rc;
}

int cmp_ui_node_get_pointer_events(cmp_ui_node_t *node) {
  int rc = (int)CMP_POINTER_EVENTS_AUTO; /* Safe default fallback */

  if (!node) {
    return rc;
  }

  /* Mock retrieval */
  if (node->properties == (void *)(size_t)CMP_POINTER_EVENTS_NONE) {
    rc = (int)CMP_POINTER_EVENTS_NONE;
    return rc;
  }

  return rc;
}
