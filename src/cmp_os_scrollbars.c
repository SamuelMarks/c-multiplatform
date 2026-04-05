/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_os_scrollbar {
  float current_velocity;
  float friction;    /* Configurable per OS */
  float springiness; /* For overscroll bouncing (macOS) */
};

int cmp_os_scrollbar_create(cmp_os_scrollbar_t **out_scrollbar) {
  cmp_os_scrollbar_t *scrollbar;

  if (!out_scrollbar) {
    return CMP_ERROR_INVALID_ARG;
  }

  scrollbar = (cmp_os_scrollbar_t *)malloc(sizeof(cmp_os_scrollbar_t));
  if (!scrollbar) {
    return CMP_ERROR_OOM;
  }

  scrollbar->current_velocity = 0.0f;
  scrollbar->friction = 0.85f;    /* Default Windows-like friction */
  scrollbar->springiness = 0.05f; /* Very stiff default spring */

#if defined(__APPLE__)
  /* macOS uses different default physics/inertia */
  scrollbar->friction = 0.95f;
  scrollbar->springiness = 0.2f;
#endif

  *out_scrollbar = scrollbar;
  return CMP_SUCCESS;
}

int cmp_os_scrollbar_destroy(cmp_os_scrollbar_t *scrollbar) {
  if (!scrollbar) {
    return CMP_ERROR_INVALID_ARG;
  }

  free(scrollbar);
  return CMP_SUCCESS;
}

int cmp_os_scrollbar_step(cmp_os_scrollbar_t *scrollbar, float raw_delta_y,
                          unsigned int delta_time_ms, float *out_smoothed_y) {
  float dt_seconds;

  if (!scrollbar || !out_smoothed_y) {
    return CMP_ERROR_INVALID_ARG;
  }

  dt_seconds = delta_time_ms / 1000.0f;
  if (dt_seconds <= 0.0f) {
    *out_smoothed_y = 0.0f;
    return CMP_SUCCESS;
  }

  /* Add incoming impulse */
  if (raw_delta_y != 0.0f) {
    scrollbar->current_velocity += (raw_delta_y * 10.0f);
  }

  /* Apply decay (friction) over time.
     For a true exponential decay: v_new = v_old * pow(friction, dt_seconds)
     But for simple games/UI we do a linear or cheap approximation */
  scrollbar->current_velocity *= scrollbar->friction;

  /* Cut off completely if it's very slow to avoid endless micro-scrolling */
  if (scrollbar->current_velocity > -0.1f &&
      scrollbar->current_velocity < 0.1f) {
    scrollbar->current_velocity = 0.0f;
  }

  *out_smoothed_y = scrollbar->current_velocity * dt_seconds;
  return CMP_SUCCESS;
}
