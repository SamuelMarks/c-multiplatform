/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_ios_background_refresh {
  int is_active;
  int current_task_id;
};

/**
 * @brief cmp_ios_background_refresh_create
 *
 * @param out_refresh Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ios_background_refresh_create(
    cmp_ios_background_refresh_t **out_refresh) {
  int rc;
  rc = CMP_SUCCESS;
  cmp_ios_background_refresh_t *r = NULL;

  if (!out_refresh) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ios_background_refresh_create: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ios_background_refresh_t), (void **)&r);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_ios_background_refresh_create: Out of memory\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  r->is_active = 0;
  r->current_task_id = 0;
  *out_refresh = r;

  if (rc != 0) {

    return rc;
  }

  return rc;
}

/**
 * @brief cmp_ios_background_refresh_destroy
 *
 * @param refresh Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ios_background_refresh_destroy(cmp_ios_background_refresh_t *refresh) {
  int rc;
  rc = CMP_SUCCESS;

  if (!refresh) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_ios_background_refresh_destroy: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  rc = CMP_FREE(refresh);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_ios_background_refresh_destroy: CMP_FREE failed\n");
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_ios_background_refresh_begin_task
 *
 * @param refresh Parameter description.
 * @param out_task_id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ios_background_refresh_begin_task(cmp_ios_background_refresh_t *refresh,
                                          int *out_task_id) {
  int rc;
  rc = CMP_SUCCESS;

  if (!refresh || !out_task_id) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_ios_background_refresh_begin_task: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* Mock: In reality this calls [[UIApplication sharedApplication]
   * beginBackgroundTaskWithExpirationHandler] */
  refresh->current_task_id++;
  refresh->is_active = 1;
  *out_task_id = refresh->current_task_id;

  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_ios_background_refresh_end_task
 *
 * @param refresh Parameter description.
 * @param task_id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ios_background_refresh_end_task(cmp_ios_background_refresh_t *refresh,
                                        int task_id) {
  int rc;
  rc = CMP_SUCCESS;

  if (!refresh) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_ios_background_refresh_end_task: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* Mock: In reality this calls [[UIApplication sharedApplication]
   * endBackgroundTask] */
  if (refresh->current_task_id == task_id) {
    refresh->is_active = 0;
  }

  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
