/**
 * @file cupertino_wheel.c
 * @brief Cupertino 3D Cylinder Wheel Picker implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_wheel.h"
#include "ui_internal_mem.h"
#include <math.h>
#include <string.h>
/* clang-format on */

#define CUPERTINO_HALF_PI 1.57079632679489661923f

ui_error_t cupertino_wheel_create(struct ui_engine *engine,
                                  const struct cupertino_wheel_descriptor *desc,
                                  struct cupertino_wheel **out_wheel) {
  struct cupertino_wheel *wheel;

  if (!engine || !desc || !out_wheel) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (desc->item_count < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (desc->item_count > 0 &&
      (desc->selected_index < 0 || desc->selected_index >= desc->item_count)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  wheel = (struct cupertino_wheel *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_wheel));
  if (!wheel) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(wheel, 0, sizeof(*wheel));
  wheel->item_count = desc->item_count;
  wheel->selected_index = desc->selected_index;
  wheel->viewport_height = (desc->viewport_height > 0.0f)
                               ? desc->viewport_height
                               : CUPERTINO_WHEEL_DEFAULT_HEIGHT;
  wheel->item_height = (desc->item_height > 0.0f)
                           ? desc->item_height
                           : CUPERTINO_WHEEL_DEFAULT_ITEM_HEIGHT;

  wheel->scroll_offset_y = (float)wheel->selected_index * wheel->item_height;
  wheel->target_offset_y = wheel->scroll_offset_y;
  wheel->last_ticked_index = wheel->selected_index;

  *out_wheel = wheel;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_wheel_destroy(struct cupertino_wheel *wheel) {
  if (!wheel) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(wheel);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_wheel_set_item_count(struct cupertino_wheel *wheel,
                                          int count) {
  if (!wheel || count < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  wheel->item_count = count;
  if (count == 0) {
    wheel->selected_index = 0;
    wheel->scroll_offset_y = 0.0f;
    wheel->target_offset_y = 0.0f;
  } else if (wheel->selected_index >= count) {
    wheel->selected_index = count - 1;
    wheel->target_offset_y = (float)wheel->selected_index * wheel->item_height;
    wheel->scroll_offset_y = wheel->target_offset_y;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_wheel_get_item_count(const struct cupertino_wheel *wheel,
                                          int *out_count) {
  if (!wheel || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = wheel->item_count;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_wheel_set_selected_index(struct cupertino_wheel *wheel,
                                              int index, int animated) {
  if (!wheel || index < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (wheel->item_count > 0 && index >= wheel->item_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  wheel->selected_index = index;
  wheel->target_offset_y = (float)index * wheel->item_height;

  if (!animated) {
    wheel->scroll_offset_y = wheel->target_offset_y;
    wheel->is_settling = 0;
    wheel->last_ticked_index = index;
  } else {
    wheel->is_settling = 1;
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_wheel_get_selected_index(const struct cupertino_wheel *wheel,
                                   int *out_index) {
  if (!wheel || !out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_index = wheel->selected_index;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_wheel_drag_start(struct cupertino_wheel *wheel) {
  if (!wheel) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  wheel->is_dragging = 1;
  wheel->is_settling = 0;
  wheel->velocity_y = 0.0f;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_wheel_drag_update(struct cupertino_wheel *wheel,
                                       float delta_y) {
  float max_offset;

  if (!wheel) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  wheel->scroll_offset_y -= delta_y;
  max_offset = (wheel->item_count > 1)
                   ? (float)(wheel->item_count - 1) * wheel->item_height
                   : 0.0f;

  /* Rubber-band edge damping */
  if (wheel->scroll_offset_y < 0.0f) {
    wheel->scroll_offset_y = wheel->scroll_offset_y * 0.75f;
  } else if (wheel->scroll_offset_y > max_offset) {
    wheel->scroll_offset_y =
        max_offset + ((wheel->scroll_offset_y - max_offset) * 0.75f);
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_wheel_drag_end(struct cupertino_wheel *wheel,
                                    float release_velocity_y) {
  float projected_y;
  int target_idx;
  int max_idx;

  if (!wheel) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  wheel->is_dragging = 0;
  wheel->is_settling = 1;

  projected_y = wheel->scroll_offset_y - (release_velocity_y * 0.20f);
  target_idx = (int)floorf((projected_y / wheel->item_height) + 0.5f);

  max_idx = (wheel->item_count > 0) ? wheel->item_count - 1 : 0;
  if (target_idx < 0) {
    target_idx = 0;
  } else if (target_idx > max_idx) {
    target_idx = max_idx;
  }

  wheel->target_offset_y = (float)target_idx * wheel->item_height;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_wheel_tick(struct cupertino_wheel *wheel, float delta_ms,
                                int *out_haptic_tick) {
  float diff;
  int current_idx;
  int max_idx;

  if (!wheel || delta_ms < 0.0f || !out_haptic_tick) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_haptic_tick = 0;

  if (wheel->is_settling) {
    diff = wheel->target_offset_y - wheel->scroll_offset_y;
    wheel->scroll_offset_y += diff * (1.0f - (float)exp(-delta_ms / 80.0f));

    if ((float)fabs(diff) < 0.2f) {
      wheel->scroll_offset_y = wheel->target_offset_y;
      wheel->is_settling = 0;
    }
  }

  current_idx =
      (int)floorf((wheel->scroll_offset_y / wheel->item_height) + 0.5f);
  max_idx = (wheel->item_count > 0) ? wheel->item_count - 1 : 0;
  if (current_idx < 0) {
    current_idx = 0;
  } else if (current_idx > max_idx) {
    current_idx = max_idx;
  }

  if (current_idx != wheel->last_ticked_index) {
    *out_haptic_tick = 1;
    wheel->last_ticked_index = current_idx;
    wheel->selected_index = current_idx;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_wheel_calculate_item_transform(
    const struct cupertino_wheel *wheel, int item_index, float *out_y,
    float *out_scale, float *out_opacity, float *out_angle_rad) {
  float cylinder_radius;
  float item_y_world;
  float y_rel;
  float theta;
  float Z;
  float Y;

  if (!wheel || item_index < 0 || item_index >= wheel->item_count || !out_y ||
      !out_scale || !out_opacity || !out_angle_rad) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  cylinder_radius = wheel->viewport_height * 0.5f;
  item_y_world = (float)item_index * wheel->item_height;
  y_rel = item_y_world - wheel->scroll_offset_y;
  theta = y_rel / cylinder_radius;

  if ((float)fabs(theta) >= CUPERTINO_HALF_PI) {
    *out_y = -9999.0f;
    *out_scale = 0.0f;
    *out_opacity = 0.0f;
    *out_angle_rad = theta;
    return UI_ERROR_NONE;
  }

  Z = cylinder_radius * (1.0f - (float)cos(theta));
  Y = cylinder_radius * (float)sin(theta);

  *out_y = (wheel->viewport_height * 0.5f) + Y;
  *out_scale = CUPERTINO_WHEEL_PERSPECTIVE_DISTANCE /
               (CUPERTINO_WHEEL_PERSPECTIVE_DISTANCE + Z);
  *out_opacity = 1.0f - (0.85f * (float)fabs(sin(theta)));
  *out_angle_rad = theta;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_wheel_get_dimensions(const struct cupertino_wheel *wheel,
                                          float *out_width, float *out_height) {
  if (!wheel || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_width = 300.0f; /* Standard width container */
  *out_height = wheel->viewport_height;
  return UI_ERROR_NONE;
}
