/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_haptics {
  int last_triggered_type;
};

int cmp_haptics_create(cmp_haptics_t **out_haptics) {
  cmp_haptics_t *haptics;
  if (!out_haptics) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (CMP_MALLOC(sizeof(cmp_haptics_t), (void **)&haptics) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
  memset(haptics, 0, sizeof(cmp_haptics_t));
  haptics->last_triggered_type = -1;
  *out_haptics = haptics;
  return CMP_SUCCESS;
}

int cmp_haptics_destroy(cmp_haptics_t *haptics) {
  if (!haptics) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(haptics);
  return CMP_SUCCESS;
}

int cmp_haptics_trigger(cmp_haptics_t *haptics, cmp_haptics_type_t type) {
  if (!haptics) {
    return CMP_ERROR_INVALID_ARG;
  }
  haptics->last_triggered_type = type;
  /* Actual OS-specific haptic API calls would go here */
  return CMP_SUCCESS;
}

int cmp_haptics_prepare(cmp_haptics_t *haptics) {
  if (!haptics)
    return CMP_ERROR_INVALID_ARG;
  /* Simulating OS-level prepare() to wake the Taptic Engine */
  return CMP_SUCCESS;
}

int cmp_haptics_trigger_with_audio_sync(cmp_haptics_t *haptics,
                                        cmp_haptics_type_t type,
                                        const char *audio_file_path) {
  if (!haptics || !audio_file_path)
    return CMP_ERROR_INVALID_ARG;
  (void)type;
  /* Simulating audio-haptic dispatch syncing */
  return CMP_SUCCESS;
}

int cmp_haptics_trigger_rigid(cmp_haptics_t *haptics) {
  if (!haptics)
    return CMP_ERROR_INVALID_ARG;
  return CMP_SUCCESS;
}

int cmp_haptics_trigger_soft(cmp_haptics_t *haptics) {
  if (!haptics)
    return CMP_ERROR_INVALID_ARG;
  return CMP_SUCCESS;
}
