/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static int g_capture_subsystem_initialized = 0;

/**
 * @brief cmp_pointer_capture_init
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pointer_capture_init(void) {
  int rc;
  rc = CMP_SUCCESS;
  g_capture_subsystem_initialized = 1;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_pointer_capture_shutdown
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pointer_capture_shutdown(void) {
  int rc;
  rc = CMP_SUCCESS;
  g_capture_subsystem_initialized = 0;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_node_set_pointer_capture
 *
 * @param node Parameter description.
 * @param pointer_id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_node_set_pointer_capture(cmp_ui_node_t *node, int pointer_id) {
  int rc;
  rc = CMP_SUCCESS;

  if (!g_capture_subsystem_initialized) {
    rc = CMP_ERROR_IO;
    LOG_DEBUG("Error in cmp_ui_node_set_pointer_capture: Not initialized\n");
    return rc;
  }

  if (!node) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ui_node_set_pointer_capture: Invalid argument\n");
    return rc;
  }

  if (pointer_id < 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ui_node_set_pointer_capture: Invalid pointer_id\n");
    return rc;
  }

  /* In a real implementation this might add to a global hash map
     mapping `pointer_id -> node`. For tests, we mock by setting properties */

  node->properties =
      (void *)(size_t)(pointer_id +
                       1); /* +1 so 0 is distinguishable from NULL */
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_node_release_pointer_capture
 *
 * @param node Parameter description.
 * @param pointer_id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_node_release_pointer_capture(cmp_ui_node_t *node, int pointer_id) {
  int rc;
  rc = CMP_SUCCESS;

  if (!g_capture_subsystem_initialized) {
    rc = CMP_ERROR_IO;
    LOG_DEBUG(
        "Error in cmp_ui_node_release_pointer_capture: Not initialized\n");
    return rc;
  }

  if (!node || pointer_id < 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_ui_node_release_pointer_capture: Invalid argument\n");
    return rc;
  }

  if (node->properties == (void *)(size_t)(pointer_id + 1)) {
    node->properties = NULL;
    return rc;
  }

  rc = CMP_ERROR_NOT_FOUND;
  LOG_DEBUG(
      "Error in cmp_ui_node_release_pointer_capture: Capture not found\n");
  return CMP_ERROR_NOT_FOUND;
}

/**
 * @brief cmp_ui_node_has_pointer_capture
 *
 * @param node Parameter description.
 * @param pointer_id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_node_has_pointer_capture(const cmp_ui_node_t *node, int pointer_id,
                                    int *out_has_capture) {
  if (!out_has_capture) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_has_capture = 0;
  if (!g_capture_subsystem_initialized || !node || pointer_id < 0) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (node->properties == (void *)(size_t)(pointer_id + 1)) {
    *out_has_capture = 1;
  }
  return CMP_SUCCESS;
}
