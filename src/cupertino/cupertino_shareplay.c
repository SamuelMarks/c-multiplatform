/**
 * @file cupertino_shareplay.c
 * @brief Cupertino SharePlay Group Activities Control implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_shareplay.h"
#include "ui_internal_mem.h"
#include <math.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_shareplay_mock_avatar_destroy_fail = 0;
static ui_error_t
mock_avatar_group_base_destroy(struct ui_avatar_group_base *group) {
  if (g_cupertino_shareplay_mock_avatar_destroy_fail) {
    (ui_avatar_group_base_destroy)(group);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_avatar_group_base_destroy)(group);
}
#undef ui_avatar_group_base_destroy
/** @cond */
#define ui_avatar_group_base_destroy mock_avatar_group_base_destroy
/** @endcond */
#endif

ui_error_t
cupertino_shareplay_create(struct ui_engine *engine,
                           const struct cupertino_shareplay_descriptor *desc,
                           struct cupertino_shareplay_control **out_control) {
  struct cupertino_shareplay_control *ctrl;
  ui_error_t rc;

  if (!engine || !desc || !out_control) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (desc->initial_participant_count < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ctrl = (struct cupertino_shareplay_control *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_shareplay_control));
  if (!ctrl) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(ctrl, 0, sizeof(*ctrl));
  ctrl->is_session_active = desc->is_session_active ? 1 : 0;
  ctrl->participant_count = desc->initial_participant_count;
  ctrl->wave_time = 0.0f;
  ctrl->wave_heights[0] = 6.0f;
  ctrl->wave_heights[1] = 10.0f;
  ctrl->wave_heights[2] = 8.0f;
  ctrl->last_action = CUPERTINO_SHAREPLAY_ACTION_NONE;
  ctrl->width = CUPERTINO_SHAREPLAY_BASE_WIDTH;
  ctrl->height = CUPERTINO_SHAREPLAY_PILL_HEIGHT;

  rc = ui_avatar_group_base_create(&ctrl->avatar_group);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(ctrl);
    return rc;
  }

  *out_control = ctrl;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_shareplay_destroy(struct cupertino_shareplay_control *control) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!control) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (control->avatar_group) {
    rc = ui_avatar_group_base_destroy(control->avatar_group);
    control->avatar_group = NULL;
  }

  C_MULTIPLATFORM_FREE(control);
  return rc;
}

ui_error_t cupertino_shareplay_set_session_active(
    struct cupertino_shareplay_control *control, int is_active) {
  if (!control) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  control->is_session_active = is_active ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_shareplay_is_session_active(
    const struct cupertino_shareplay_control *control, int *out_active) {
  if (!control || !out_active) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_active = control->is_session_active;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_shareplay_set_participant_count(
    struct cupertino_shareplay_control *control, int count) {
  if (!control || count < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  control->participant_count = count;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_shareplay_get_participant_count(
    const struct cupertino_shareplay_control *control, int *out_count) {
  if (!control || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = control->participant_count;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_shareplay_tick(struct cupertino_shareplay_control *control,
                                    float delta_ms) {
  if (!control || delta_ms < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (control->is_session_active) {
    control->wave_time += delta_ms / 1000.0f;
    control->wave_heights[0] =
        5.0f + 6.0f * (0.5f + 0.5f * (float)sin(control->wave_time * 6.0f));
    control->wave_heights[1] =
        5.0f +
        8.0f * (0.5f + 0.5f * (float)sin(control->wave_time * 8.0f + 1.2f));
    control->wave_heights[2] =
        5.0f +
        7.0f * (0.5f + 0.5f * (float)sin(control->wave_time * 7.0f + 2.4f));
  } else {
    control->wave_heights[0] = 4.0f;
    control->wave_heights[1] = 4.0f;
    control->wave_heights[2] = 4.0f;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_shareplay_get_wave_heights(
    const struct cupertino_shareplay_control *control, float out_heights[3]) {
  if (!control || !out_heights) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_heights[0] = control->wave_heights[0];
  out_heights[1] = control->wave_heights[1];
  out_heights[2] = control->wave_heights[2];
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_shareplay_trigger_action(struct cupertino_shareplay_control *control,
                                   enum cupertino_shareplay_action action) {
  if (!control || (int)action < 0 || (int)action > 2) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  control->last_action = action;
  if (action == CUPERTINO_SHAREPLAY_ACTION_LEAVE ||
      action == CUPERTINO_SHAREPLAY_ACTION_END_FOR_EVERYONE) {
    control->is_session_active = 0;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_shareplay_get_dimensions(
    const struct cupertino_shareplay_control *control, float *out_width,
    float *out_height) {
  if (!control || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_width = control->width;
  *out_height = control->height;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_shareplay_get_avatar_group(
    struct cupertino_shareplay_control *control,
    struct ui_avatar_group_base **out_avatar_group) {
  if (!control || !out_avatar_group) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_avatar_group = control->avatar_group;
  return UI_ERROR_NONE;
}
