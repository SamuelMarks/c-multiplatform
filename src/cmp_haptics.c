/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_haptics {
  int last_triggered_type;
};

/**
 * @brief cmp_haptics_create
 *
 * @param out_haptics Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_haptics_create(cmp_haptics_t **out_haptics) {
  int rc = CMP_SUCCESS;
  cmp_haptics_t *haptics = NULL;

  if (!out_haptics) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_haptics_create: Invalid argument (out_haptics=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_haptics_t), (void **)&haptics);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_haptics_create: Out of memory\n");
    return rc;
  }

  memset(haptics, 0, sizeof(cmp_haptics_t));
  haptics->last_triggered_type = -1;
  *out_haptics = haptics;
  return rc;
}

/**
 * @brief cmp_haptics_destroy
 *
 * @param haptics Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_haptics_destroy(cmp_haptics_t *haptics) {
  int rc = CMP_SUCCESS;

  if (!haptics) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_haptics_destroy: Invalid argument\n");
    return rc;
  }
  CMP_FREE(haptics);
  return rc;
}

/**
 * @brief cmp_haptics_trigger
 *
 * @param haptics Parameter description.
 * @param type Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_haptics_trigger(cmp_haptics_t *haptics, cmp_haptics_type_t type) {
  int rc = CMP_SUCCESS;

  if (!haptics) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_haptics_trigger: Invalid argument\n");
    return rc;
  }

  haptics->last_triggered_type = type;
  /* Actual OS-specific haptic API calls would go here */
  return rc;
}

/**
 * @brief cmp_haptics_prepare
 *
 * @param haptics Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_haptics_prepare(cmp_haptics_t *haptics) {
  int rc = CMP_SUCCESS;

  if (!haptics) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_haptics_prepare: Invalid argument\n");
    return rc;
  }

  /* Simulating OS-level prepare() to wake the Taptic Engine */
  return rc;
}

/**
 * @brief cmp_haptics_trigger_with_audio_sync
 *
 * @param haptics Parameter description.
 * @param type Parameter description.
 * @param audio_file_path Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_haptics_trigger_with_audio_sync(cmp_haptics_t *haptics,
                                        cmp_haptics_type_t type,
                                        const char *audio_file_path) {
  int rc = CMP_SUCCESS;

  if (!haptics || !audio_file_path) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_haptics_trigger_with_audio_sync: Invalid argument\n");
    return rc;
  }

  (void)type;
  /* Simulating audio-haptic dispatch syncing */
  return rc;
}

/**
 * @brief cmp_haptics_trigger_rigid
 *
 * @param haptics Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_haptics_trigger_rigid(cmp_haptics_t *haptics) {
  int rc = CMP_SUCCESS;

  if (!haptics) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_haptics_trigger_rigid: Invalid argument\n");
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_haptics_trigger_soft
 *
 * @param haptics Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_haptics_trigger_soft(cmp_haptics_t *haptics) {
  int rc = CMP_SUCCESS;

  if (!haptics) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_haptics_trigger_soft: Invalid argument\n");
    return rc;
  }
  return rc;
}
