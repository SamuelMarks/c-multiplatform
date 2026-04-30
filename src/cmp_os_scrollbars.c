/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_os_scrollbar {
  float current_velocity;
  float friction;    /* Configurable per OS */
  float springiness; /* For overscroll bouncing (macOS) */
};

/**
 * @brief Create an OS scrollbar context.
 *
 * @param out_scrollbar Pointer to store the created context.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_scrollbar_create(cmp_os_scrollbar_t **out_scrollbar) {
  int rc = CMP_SUCCESS;
  cmp_os_scrollbar_t *scrollbar;

  rc = CMP_SUCCESS;
  scrollbar = NULL;

  if (out_scrollbar == NULL) {
    LOG_DEBUG("Error in cmp_os_scrollbar_create: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_os_scrollbar_t), (void **)&scrollbar);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_os_scrollbar_create: CMP_MALLOC failed (OOM)\n");
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
  return rc;
}

/**
 * @brief Destroy an OS scrollbar context.
 *
 * @param scrollbar The scrollbar to destroy.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_scrollbar_destroy(cmp_os_scrollbar_t *scrollbar) {
  int rc = CMP_SUCCESS;

  rc = CMP_SUCCESS;

  if (scrollbar == NULL) {
    LOG_DEBUG("Error in cmp_os_scrollbar_destroy: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(scrollbar);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_os_scrollbar_destroy: CMP_FREE failed\n");
    return rc;
  }
  return rc;
}

/**
 * @brief Step the OS scrollbar physics simulation.
 *
 * @param scrollbar The scrollbar context.
 * @param raw_delta_y The raw scroll delta input.
 * @param delta_time_ms The delta time in milliseconds.
 * @param out_smoothed_y Pointer to store the smoothed scroll delta.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_os_scrollbar_step(cmp_os_scrollbar_t *scrollbar, float raw_delta_y,
                          unsigned int delta_time_ms, float *out_smoothed_y) {
  int rc = CMP_SUCCESS;
  float dt_seconds;

  if (scrollbar == NULL || out_smoothed_y == NULL) {
    LOG_DEBUG("Error in cmp_os_scrollbar_step: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  dt_seconds = delta_time_ms / 1000.0f;
  if (dt_seconds <= 0.0f) {
    *out_smoothed_y = 0.0f;
    return rc;
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
  return rc;
}
