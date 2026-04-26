/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_android_ndk_bridge {
  int is_running;
  float total_time;
};

/**
 * @brief cmp_android_ndk_bridge_create
 *
 * @param out_bridge Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_android_ndk_bridge_create(cmp_android_ndk_bridge_t **out_bridge) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_android_ndk_bridge_t *br = NULL;

  if (out_bridge == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_android_ndk_bridge_create: Invalid argument "
                  "(out_bridge=NULL): %s\n",
                  err_str);
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_android_ndk_bridge_t), (void **)&br);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_android_ndk_bridge_create: Out of memory: %s\n",
                  err_str);
    return rc;
  }

  br->is_running = 1;
  br->total_time = 0.0f;
  *out_bridge = br;

  cmp_log_debug("cmp_android_ndk_bridge_create: Successfully created ndk "
                "bridge context\n");
  return rc;
}

/**
 * @brief cmp_android_ndk_bridge_destroy
 *
 * @param bridge Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_android_ndk_bridge_destroy(cmp_android_ndk_bridge_t *bridge) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (bridge == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_android_ndk_bridge_destroy: Invalid argument (bridge=NULL): %s\n",
        err_str);
    return rc;
  }

  CMP_FREE(bridge);
  cmp_log_debug("cmp_android_ndk_bridge_destroy: Successfully destroyed ndk "
                "bridge context\n");
  return rc;
}

/**
 * @brief cmp_android_ndk_bridge_tick
 *
 * @param bridge Parameter description.
 * @param delta_time Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_android_ndk_bridge_tick(cmp_android_ndk_bridge_t *bridge,
                                float delta_time) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (bridge == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_android_ndk_bridge_tick: Invalid argument (bridge=NULL): %s\n",
        err_str);
    return rc;
  }

  bridge->total_time += delta_time;
  cmp_log_debug("cmp_android_ndk_bridge_tick: Ticked ndk bridge (delta=%.2f, "
                "total=%.2f)\n",
                delta_time, bridge->total_time);

  /* Mock: In the real Android application, this would call ALooper_pollAll */
  return rc;
}
