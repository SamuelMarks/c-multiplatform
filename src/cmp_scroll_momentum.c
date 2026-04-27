/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <math.h>

#include "cmp_log.h"
/* clang-format on */

/**
 * @brief Calculate scroll momentum.
 *
 * @param initial_velocity Parameter description.
 * @param elapsed_time_ms Parameter description.
 * @param platform_decay_rate Parameter description.
 * @param out_offset Parameter description.
 * @param out_current_velocity Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_momentum_calculate(float initial_velocity, float elapsed_time_ms,
                                  float platform_decay_rate, float *out_offset,
                                  float *out_current_velocity) {
  const float frame_duration = 16.666f;
  float frames_elapsed;
  float current_velocity;

  if (out_offset == NULL || out_current_velocity == NULL) {
    LOG_DEBUG("Invalid argument: out_offset or out_current_velocity is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* Standard exponential decay: v(t) = v0 * decay^(t/ms_per_frame) */
  /* Assuming standard 16.6ms frame duration for delta calculations */
  frames_elapsed = elapsed_time_ms / frame_duration;
  current_velocity =
      initial_velocity * (float)pow(platform_decay_rate, frames_elapsed);

  *out_current_velocity = current_velocity;

  /* Integration of decay to get total offset: Delta = v(t) * dt */
  *out_offset = (current_velocity / 1000.0f) * elapsed_time_ms;

  return CMP_SUCCESS;
}
