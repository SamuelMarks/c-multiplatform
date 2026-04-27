/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>

#include "cmp_log.h"
/* clang-format on */

struct cmp_selection {
  int start_index;
  int end_index;
};

/**
 * @brief Create a selection.
 *
 * @param out_selection Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_selection_create(cmp_selection_t **out_selection) {
  int rc;
  struct cmp_selection *selection;

  rc = CMP_SUCCESS;

  if (out_selection == NULL) {
    LOG_DEBUG("Invalid argument: out_selection is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_selection), (void **)&selection);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  memset(selection, 0, sizeof(struct cmp_selection));

  *out_selection = (cmp_selection_t *)selection;
  return CMP_SUCCESS;
}

/**
 * @brief Destroy a selection.
 *
 * @param selection Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_selection_destroy(cmp_selection_t *selection) {
  int rc;
  struct cmp_selection *internal_selection;

  rc = CMP_SUCCESS;
  internal_selection = (struct cmp_selection *)selection;

  if (internal_selection == NULL) {
    LOG_DEBUG("Invalid argument: selection is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(internal_selection);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
    return rc;
  }

  return CMP_SUCCESS;
}
