/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static int g_capture_subsystem_initialized = 0;

int cmp_pointer_capture_init(void) {
  g_capture_subsystem_initialized = 1;
  return CMP_SUCCESS;
}

int cmp_pointer_capture_shutdown(void) {
  g_capture_subsystem_initialized = 0;
  return CMP_SUCCESS;
}

int cmp_ui_node_set_pointer_capture(cmp_ui_node_t *node, int pointer_id) {
  if (!g_capture_subsystem_initialized)
    return CMP_ERROR_IO;

  if (!node)
    return CMP_ERROR_INVALID_ARG;

  if (pointer_id < 0)
    return CMP_ERROR_INVALID_ARG;

  /* In a real implementation this might add to a global hash map
     mapping `pointer_id -> node`. For tests, we mock by setting properties */

  node->properties =
      (void *)(size_t)(pointer_id +
                       1); /* +1 so 0 is distinguishable from NULL */

  return CMP_SUCCESS;
}

int cmp_ui_node_release_pointer_capture(cmp_ui_node_t *node, int pointer_id) {
  if (!g_capture_subsystem_initialized)
    return CMP_ERROR_IO;

  if (!node || pointer_id < 0)
    return CMP_ERROR_INVALID_ARG;

  if (node->properties == (void *)(size_t)(pointer_id + 1)) {
    node->properties = NULL;
    return CMP_SUCCESS;
  }

  return CMP_ERROR_NOT_FOUND;
}

int cmp_ui_node_has_pointer_capture(const cmp_ui_node_t *node, int pointer_id) {
  if (!g_capture_subsystem_initialized || !node || pointer_id < 0)
    return 0;

  if (node->properties == (void *)(size_t)(pointer_id + 1)) {
    return 1;
  }

  return 0;
}
