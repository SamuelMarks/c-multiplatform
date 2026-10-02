/**
 * @file cupertino_activity_rings.c
 * @brief Cupertino Activity & Health Rings Gauge implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_activity_rings.h"
#include "ui_internal_mem.h"
#include <math.h>
#include <string.h>
/* clang-format on */

#define CUPERTINO_PI 3.14159265358979323846f

ui_error_t cupertino_activity_rings_create(
    struct ui_engine *engine,
    const struct cupertino_activity_rings_descriptor *desc,
    struct cupertino_activity_rings **out_rings) {
  struct cupertino_activity_rings *rings;
  int i;

  if (!engine || !desc || !out_rings) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (desc->outer_radius <= 0.0f || desc->ring_thickness <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rings = (struct cupertino_activity_rings *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_activity_rings));
  if (!rings) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(rings, 0, sizeof(*rings));
  rings->outer_radius = desc->outer_radius;
  rings->ring_thickness = desc->ring_thickness;
  rings->ring_gap =
      (desc->ring_gap >= 0.0f) ? desc->ring_gap : CUPERTINO_RINGS_DEFAULT_GAP;

  for (i = 0; i < CUPERTINO_RING_COUNT; i++) {
    rings->progress[i] =
        (desc->initial_progress[i] >= 0.0f) ? desc->initial_progress[i] : 0.0f;
  }

  /* Move ring: Red / Magenta */
  rings->ring_colors_start[CUPERTINO_RING_MOVE] =
      UI_COLOR_ARGB(255, 250, 17, 79);
  rings->ring_colors_end[CUPERTINO_RING_MOVE] = UI_COLOR_ARGB(255, 255, 0, 119);
  rings->background_tracks[CUPERTINO_RING_MOVE] =
      UI_COLOR_ARGB(40, 250, 17, 79);

  /* Exercise ring: Lime / Bright Green */
  rings->ring_colors_start[CUPERTINO_RING_EXERCISE] =
      UI_COLOR_ARGB(255, 175, 255, 0);
  rings->ring_colors_end[CUPERTINO_RING_EXERCISE] =
      UI_COLOR_ARGB(255, 208, 255, 0);
  rings->background_tracks[CUPERTINO_RING_EXERCISE] =
      UI_COLOR_ARGB(40, 175, 255, 0);

  /* Stand ring: Cyan / Blue */
  rings->ring_colors_start[CUPERTINO_RING_STAND] =
      UI_COLOR_ARGB(255, 0, 240, 255);
  rings->ring_colors_end[CUPERTINO_RING_STAND] =
      UI_COLOR_ARGB(255, 0, 208, 255);
  rings->background_tracks[CUPERTINO_RING_STAND] =
      UI_COLOR_ARGB(40, 0, 240, 255);

  *out_rings = rings;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_activity_rings_destroy(struct cupertino_activity_rings *rings) {
  if (!rings) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(rings);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_activity_rings_set_progress(struct cupertino_activity_rings *rings,
                                      enum cupertino_ring_id ring,
                                      float progress) {
  if (!rings || (int)ring < 0 || (int)ring >= CUPERTINO_RING_COUNT ||
      progress < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rings->progress[ring] = progress;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_activity_rings_get_progress(
    const struct cupertino_activity_rings *rings, enum cupertino_ring_id ring,
    float *out_progress) {
  if (!rings || (int)ring < 0 || (int)ring >= CUPERTINO_RING_COUNT ||
      !out_progress) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_progress = rings->progress[ring];
  return UI_ERROR_NONE;
}

ui_error_t cupertino_activity_rings_get_ring_radius(
    const struct cupertino_activity_rings *rings, enum cupertino_ring_id ring,
    float *out_radius) {
  float step;

  if (!rings || (int)ring < 0 || (int)ring >= CUPERTINO_RING_COUNT ||
      !out_radius) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  step = rings->ring_thickness + rings->ring_gap;
  *out_radius = rings->outer_radius - ((float)ring * step);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_activity_rings_calculate_shadow(
    const struct cupertino_activity_rings *rings, enum cupertino_ring_id ring,
    int *out_has_shadow, float *out_shadow_angle) {
  float p;

  if (!rings || (int)ring < 0 || (int)ring >= CUPERTINO_RING_COUNT ||
      !out_has_shadow || !out_shadow_angle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  p = rings->progress[ring];
  if (p > 1.0f) {
    *out_has_shadow = 1;
    *out_shadow_angle =
        (-CUPERTINO_PI * 0.5f) + (2.0f * CUPERTINO_PI * (p - 1.0f));
  } else {
    *out_has_shadow = 0;
    *out_shadow_angle = 0.0f;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_activity_rings_get_cap_center(
    const struct cupertino_activity_rings *rings, enum cupertino_ring_id ring,
    float *out_x, float *out_y) {
  float radius;
  float angle;
  ui_error_t rc;

  if (!out_x || !out_y) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = cupertino_activity_rings_get_ring_radius(rings, ring, &radius);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  angle =
      (-CUPERTINO_PI * 0.5f) + (2.0f * CUPERTINO_PI * rings->progress[ring]);
  *out_x = radius * (float)cos(angle);
  *out_y = radius * (float)sin(angle);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_activity_rings_get_colors(
    const struct cupertino_activity_rings *rings, enum cupertino_ring_id ring,
    ui_color_t *out_start, ui_color_t *out_end, ui_color_t *out_track) {
  if (!rings || (int)ring < 0 || (int)ring >= CUPERTINO_RING_COUNT ||
      !out_start || !out_end || !out_track) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_start = rings->ring_colors_start[ring];
  *out_end = rings->ring_colors_end[ring];
  *out_track = rings->background_tracks[ring];
  return UI_ERROR_NONE;
}

ui_error_t cupertino_activity_rings_get_dimensions(
    const struct cupertino_activity_rings *rings, float *out_width,
    float *out_height) {
  float total_radius;

  if (!rings || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  total_radius = rings->outer_radius + (rings->ring_thickness * 0.5f);
  *out_width = total_radius * 2.0f;
  *out_height = total_radius * 2.0f;
  return UI_ERROR_NONE;
}
