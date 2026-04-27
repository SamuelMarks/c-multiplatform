/* clang-format off */
#include "cmp.h"

#include <stdlib.h>

#include "cmp_log.h"
/* clang-format on */

struct cmp_safe_area_handler {
  cmp_safe_areas_t *areas;
};

/**
 * @brief Create a safe area handler context.
 *
 * @param out_handler Pointer to receive the handler.
 * @return 0 on success, or an error code on failure.
 */
int cmp_safe_area_handler_create(cmp_safe_area_handler_t **out_handler) {
  int rc;
  cmp_safe_area_handler_t *handler;

  rc = CMP_SUCCESS;

  if (out_handler == NULL) {
    LOG_DEBUG("Invalid argument: out_handler is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_safe_area_handler_t), (void **)&handler);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  rc = cmp_safe_areas_create(&handler->areas);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Failed to create safe areas\n");
    if (CMP_FREE(handler) != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    return rc;
  }

  *out_handler = handler;
  return CMP_SUCCESS;
}

/**
 * @brief Destroy the safe area handler.
 *
 * @param handler The handler context.
 * @return 0 on success, or an error code on failure.
 */
int cmp_safe_area_handler_destroy(cmp_safe_area_handler_t *handler) {
  int rc;

  rc = CMP_SUCCESS;

  if (handler == NULL) {
    LOG_DEBUG("Invalid argument: handler is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (handler->areas != NULL) {
    rc = cmp_safe_areas_destroy(handler->areas);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Failed to destroy safe areas\n");
      return rc;
    }
  }

  rc = CMP_FREE(handler);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
    return rc;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Query the OS for safe area insets.
 *
 * @param handler The handler context.
 * @param window The window context.
 * @param out_top Pointer to receive top inset.
 * @param out_bottom Pointer to receive bottom inset.
 * @param out_left Pointer to receive left inset.
 * @param out_right Pointer to receive right inset.
 * @return 0 on success, or an error code on failure.
 */
int cmp_safe_area_handler_query_insets(cmp_safe_area_handler_t *handler,
                                       cmp_window_t *window, int *out_top,
                                       int *out_bottom, int *out_left,
                                       int *out_right) {
  int rc;
  float top;
  float bottom;
  float left;
  float right;

  rc = CMP_SUCCESS;
  top = 0.0f;
  bottom = 0.0f;
  left = 0.0f;
  right = 0.0f;

  if (handler == NULL || window == NULL) {
    LOG_DEBUG("Invalid argument: handler or window is NULL\n");
    if (out_top != NULL)
      *out_top = 0;
    if (out_bottom != NULL)
      *out_bottom = 0;
    if (out_left != NULL)
      *out_left = 0;
    if (out_right != NULL)
      *out_right = 0;
    return CMP_ERROR_INVALID_ARG;
  }

  rc = cmp_safe_areas_get_inset(handler->areas, 0, &top);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Failed to get inset 0\n");
    return rc;
  }

  rc = cmp_safe_areas_get_inset(handler->areas, 2, &bottom);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Failed to get inset 2\n");
    return rc;
  }

  rc = cmp_safe_areas_get_inset(handler->areas, 3, &left);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Failed to get inset 3\n");
    return rc;
  }

  rc = cmp_safe_areas_get_inset(handler->areas, 1, &right);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Failed to get inset 1\n");
    return rc;
  }

  if (out_top != NULL)
    *out_top = (int)top;
  if (out_bottom != NULL)
    *out_bottom = (int)bottom;
  if (out_left != NULL)
    *out_left = (int)left;
  if (out_right != NULL)
    *out_right = (int)right;

  return CMP_SUCCESS;
}
