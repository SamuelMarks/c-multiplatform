/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <stdio.h>
/* clang-format on */

struct cmp_apple_gestures {
  int pinch_enabled;
  int rotation_enabled;
  int swipe_enabled;
};

/**
 * @brief cmp_apple_gestures_create
 *
 * @param out_gestures Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_apple_gestures_create(cmp_apple_gestures_t **out_gestures) {
  int rc = CMP_SUCCESS;
  cmp_apple_gestures_t *gest = NULL;

  if (!out_gestures) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_apple_gestures_create: Invalid argument "
                    "(out_gestures=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_apple_gestures_t), (void **)&gest);
  if (rc != CMP_SUCCESS) {
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_apple_gestures_create: Out of memory: %s\n", err_str);
    }
    return rc;
  }

  gest->pinch_enabled = 0;
  gest->rotation_enabled = 0;
  gest->swipe_enabled = 0;

  *out_gestures = gest;
  return rc;
}

/**
 * @brief cmp_apple_gestures_destroy
 *
 * @param gestures Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_apple_gestures_destroy(cmp_apple_gestures_t *gestures) {
  int rc = CMP_SUCCESS;

  if (!gestures) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_apple_gestures_destroy: Invalid argument "
                    "(gestures=NULL)\n");
    return rc;
  }

  CMP_FREE(gestures);
  return rc;
}

/**
 * @brief cmp_apple_gestures_enable
 *
 * @param gestures Parameter description.
 * @param window Parameter description.
 * @param enable_pinch Parameter description.
 * @param enable_rotation Parameter description.
 * @param enable_swipe Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_apple_gestures_enable(cmp_apple_gestures_t *gestures,
                              cmp_window_t *window, int enable_pinch,
                              int enable_rotation, int enable_swipe) {
  int rc = CMP_SUCCESS;

  if (!gestures || !window) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_apple_gestures_enable: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  rc = cmp_window_apple_enable_gestures(window, enable_pinch, enable_rotation,
                                        enable_swipe);
  if (rc != CMP_SUCCESS) {
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_apple_gestures_enable: Failed to enable : %s\n", err_str);
    }
    return rc;
  }

  gestures->pinch_enabled = enable_pinch;
  gestures->rotation_enabled = enable_rotation;
  gestures->swipe_enabled = enable_swipe;

  return rc;
}
