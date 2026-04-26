/* clang-format off */
#include "cmp.h"
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
  cmp_android_ndk_bridge_t *br;

  int rc;
  if (!out_bridge)
    return CMP_ERROR_INVALID_ARG;

  rc = CMP_MALLOC(sizeof(cmp_android_ndk_bridge_t), (void **)&br);
  if (rc != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  br->is_running = 1;
  br->total_time = 0.0f;
  *out_bridge = br;

  return CMP_SUCCESS;
}

/**
 * @brief cmp_android_ndk_bridge_destroy
 *
 * @param bridge Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_android_ndk_bridge_destroy(cmp_android_ndk_bridge_t *bridge) {
  if (!bridge)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(bridge);
  return CMP_SUCCESS;
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
  if (!bridge)
    return CMP_ERROR_INVALID_ARG;

  bridge->total_time += delta_time;

  /* Mock: In the real Android application, this would call ALooper_pollAll */
  return CMP_SUCCESS;
}
