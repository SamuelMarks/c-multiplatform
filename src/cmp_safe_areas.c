/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>

#include "cmp_log.h"
/* clang-format on */

struct cmp_safe_areas {
  float insets[4]; /* 0: top, 1: right, 2: bottom, 3: left */
};

/**
 * @brief Create a safe areas container.
 *
 * @param out_safe_areas Pointer to receive the created safe areas instance.
 * @return 0 on success, or an error code on failure.
 */
int cmp_safe_areas_create(cmp_safe_areas_t **out_safe_areas) {
  int rc = CMP_SUCCESS;
  cmp_safe_areas_t *safe_areas;

  rc = CMP_SUCCESS;

  if (out_safe_areas == NULL) {
    LOG_DEBUG("Invalid argument: out_safe_areas is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_safe_areas_t), (void **)&safe_areas);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  memset(safe_areas, 0, sizeof(cmp_safe_areas_t));

  *out_safe_areas = safe_areas;
  return rc;
}

/**
 * @brief Destroy a safe areas container.
 *
 * @param safe_areas The safe areas container.
 * @return 0 on success, or an error code on failure.
 */
int cmp_safe_areas_destroy(cmp_safe_areas_t *safe_areas) {
  int rc = CMP_SUCCESS;

  rc = CMP_SUCCESS;

  if (safe_areas == NULL) {
    LOG_DEBUG("Invalid argument: safe_areas is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(safe_areas);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
    return rc;
  }

  return rc;
}

/**
 * @brief Set a specific inset value.
 *
 * @param safe_areas The safe areas container.
 * @param edge The edge index (0: top, 1: right, 2: bottom, 3: left).
 * @param inset The inset value to set.
 * @return 0 on success, or an error code on failure.
 */
int cmp_safe_areas_set_inset(cmp_safe_areas_t *safe_areas, int edge,
                             float inset) {
  int rc = CMP_SUCCESS;
  if (safe_areas == NULL) {
    LOG_DEBUG("Invalid argument: safe_areas is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (edge < 0 || edge > 3) {
    LOG_DEBUG("Invalid argument: edge is out of bounds\n");
    return CMP_ERROR_INVALID_ARG;
  }

  safe_areas->insets[edge] = inset;
  return rc;
}

/**
 * @brief Get a specific inset value.
 *
 * @param safe_areas The safe areas container.
 * @param edge The edge index (0: top, 1: right, 2: bottom, 3: left).
 * @param out_inset Pointer to receive the inset value.
 * @return 0 on success, or an error code on failure.
 */
int cmp_safe_areas_get_inset(const cmp_safe_areas_t *safe_areas, int edge,
                             float *out_inset) {
  int rc = CMP_SUCCESS;
  if (safe_areas == NULL) {
    LOG_DEBUG("Invalid argument: safe_areas is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (out_inset == NULL) {
    LOG_DEBUG("Invalid argument: out_inset is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (edge < 0 || edge > 3) {
    LOG_DEBUG("Invalid argument: edge is out of bounds\n");
    return CMP_ERROR_INVALID_ARG;
  }

  *out_inset = safe_areas->insets[edge];
  return rc;
}
