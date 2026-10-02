/**
 * @file cupertino_fullscreen_dialog.c
 * @brief Cupertino Fullscreen Dialog Transition
 * (CupertinoFullscreenDialogTransition & CupertinoPageRoute) implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_fullscreen_dialog.h"
#include "cupertino/cupertino_spring.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_dialog_mock_spring_preset_fail = 0;
int g_cupertino_dialog_mock_spring_eval_fail = 0;

static ui_error_t
mock_cupertino_spring_get_preset(enum cupertino_spring_preset preset,
                                 struct cupertino_spring_config *config) {
  if (g_cupertino_dialog_mock_spring_preset_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return cupertino_spring_get_preset(preset, config);
}
#undef cupertino_spring_get_preset
/** @cond */
#define cupertino_spring_get_preset mock_cupertino_spring_get_preset
/** @endcond */

static ui_error_t
mock_cupertino_spring_evaluate(const struct cupertino_spring_config *config,
                               float time_seconds, float *position,
                               float *velocity) {
  if (g_cupertino_dialog_mock_spring_eval_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return cupertino_spring_evaluate(config, time_seconds, position, velocity);
}
#undef cupertino_spring_evaluate
/** @cond */
#define cupertino_spring_evaluate mock_cupertino_spring_evaluate
/** @endcond */
#endif /* UI_TEST_MOCK_ALLOC */

static ui_error_t cupertino_fullscreen_dialog_update_parent_transform(
    struct cupertino_fullscreen_dialog *dialog) {
  float p;
  if (!dialog) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  p = dialog->current_progress;
  if (p < 0.0f) {
    p = 0.0f;
  }
  if (p > 1.0f) {
    p = 1.0f;
  }

  /* Parent scales down from 1.0 to 0.92 */
  dialog->parent_scale = 1.0f - (0.08f * p);

  /* Parent top corners round from 0 to 12pt */
  dialog->parent_corner_radius = 12.0f * p;

  /* Parent view dims up to 0.25 alpha */
  dialog->parent_dim_alpha = 0.25f * p;
  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_dialog_mock_update_transform_fail = 0;
static ui_error_t
mock_update_parent_transform(struct cupertino_fullscreen_dialog *dialog) {
  if (g_cupertino_dialog_mock_update_transform_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return cupertino_fullscreen_dialog_update_parent_transform(dialog);
}
#undef cupertino_fullscreen_dialog_update_parent_transform
/** @cond */
#define cupertino_fullscreen_dialog_update_parent_transform                    \
  mock_update_parent_transform
/** @endcond */
#endif /* UI_TEST_MOCK_ALLOC */

ui_error_t cupertino_fullscreen_dialog_create(
    struct ui_engine *engine,
    const struct cupertino_fullscreen_dialog_descriptor *desc,
    struct cupertino_fullscreen_dialog **out_dialog) {
  struct cupertino_fullscreen_dialog *dialog;
  ui_error_t rc;

  if (!engine || !desc || !out_dialog) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dialog = (struct cupertino_fullscreen_dialog *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_fullscreen_dialog));
  if (!dialog) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(dialog, 0, sizeof(*dialog));
  dialog->screen_height =
      (desc->screen_height > 0.0f) ? desc->screen_height : 844.0f;
  dialog->state = CUPERTINO_DIALOG_STATE_DISMISSED;
  dialog->action_type = desc->action_type;
  dialog->swipe_to_dismiss_enabled = desc->swipe_to_dismiss_enabled;
  dialog->dismiss_velocity_threshold = (desc->dismiss_velocity_threshold > 0.0f)
                                           ? desc->dismiss_velocity_threshold
                                           : 500.0f;
  dialog->dismiss_distance_threshold = (desc->dismiss_distance_threshold > 0.0f)
                                           ? desc->dismiss_distance_threshold
                                           : 0.35f;
  dialog->current_progress = 0.0f;
  dialog->drag_offset_y = 0.0f;
  dialog->touch_velocity_y = 0.0f;
  dialog->anim_elapsed_s = 0.0f;
  dialog->dismiss_invoked_count = 0;

  if (desc->action_type == CUPERTINO_DIALOG_NAV_ACTION_CUSTOM &&
      desc->action_title) {
#if defined(_MSC_VER)
    strncpy_s(dialog->action_title, sizeof(dialog->action_title),
              desc->action_title, sizeof(dialog->action_title) - 1);
#else
    strncpy(dialog->action_title, desc->action_title,
            sizeof(dialog->action_title) - 1);
    dialog->action_title[sizeof(dialog->action_title) - 1] = '\0';
#endif
  } else if (desc->action_type == CUPERTINO_DIALOG_NAV_ACTION_DONE) {
#if defined(_MSC_VER)
    strncpy_s(dialog->action_title, sizeof(dialog->action_title), "Done",
              sizeof(dialog->action_title) - 1);
#else
    strncpy(dialog->action_title, "Done", sizeof(dialog->action_title) - 1);
    dialog->action_title[sizeof(dialog->action_title) - 1] = '\0';
#endif
  } else {
#if defined(_MSC_VER)
    strncpy_s(dialog->action_title, sizeof(dialog->action_title), "Cancel",
              sizeof(dialog->action_title) - 1);
#else
    strncpy(dialog->action_title, "Cancel", sizeof(dialog->action_title) - 1);
    dialog->action_title[sizeof(dialog->action_title) - 1] = '\0';
#endif
  }

  /* Configure default spring physics config */
  rc = cupertino_spring_get_preset(CUPERTINO_SPRING_PRESET_SNAPPY,
                                   &dialog->spring_cfg);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(dialog);
    return rc;
  }

  rc = cupertino_fullscreen_dialog_update_parent_transform(dialog);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(dialog);
    return rc;
  }

  *out_dialog = dialog;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_fullscreen_dialog_destroy(
    struct cupertino_fullscreen_dialog *dialog) {
  if (!dialog) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(dialog);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_fullscreen_dialog_present(
    struct cupertino_fullscreen_dialog *dialog) {
  ui_error_t rc;

  if (!dialog) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dialog->state = CUPERTINO_DIALOG_STATE_PRESENTING;
  dialog->drag_offset_y = 0.0f;
  dialog->touch_velocity_y = 0.0f;
  dialog->anim_elapsed_s = 0.0f;

  rc = cupertino_spring_get_preset(CUPERTINO_SPRING_PRESET_SNAPPY,
                                   &dialog->spring_cfg);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  dialog->spring_cfg.initial_position = dialog->current_progress;
  dialog->spring_cfg.target_position = 1.0f;
  dialog->spring_cfg.initial_velocity = 0.0f;

  return UI_ERROR_NONE;
}

ui_error_t cupertino_fullscreen_dialog_dismiss(
    struct cupertino_fullscreen_dialog *dialog) {
  ui_error_t rc;

  if (!dialog) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dialog->state = CUPERTINO_DIALOG_STATE_DISMISSING;
  dialog->anim_elapsed_s = 0.0f;
  dialog->dismiss_invoked_count++;

  rc = cupertino_spring_get_preset(CUPERTINO_SPRING_PRESET_SHEET_DISMISS,
                                   &dialog->spring_cfg);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  dialog->spring_cfg.initial_position = dialog->current_progress;
  dialog->spring_cfg.target_position = 0.0f;
  dialog->spring_cfg.initial_velocity =
      -dialog->touch_velocity_y / dialog->screen_height;

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_fullscreen_dialog_tick(struct cupertino_fullscreen_dialog *dialog,
                                 float delta_sec, int *out_is_settled) {
  ui_error_t rc;
  int settled;
  float pos, vel;

  if (!dialog || delta_sec < 0.0f || !out_is_settled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  settled = 0;
  pos = 0.0f;
  vel = 0.0f;

  if (dialog->state == CUPERTINO_DIALOG_STATE_PRESENTING) {
    dialog->anim_elapsed_s += delta_sec;
    rc = cupertino_spring_evaluate(&dialog->spring_cfg, dialog->anim_elapsed_s,
                                   &pos, &vel);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    dialog->current_progress = pos;
    if (dialog->anim_elapsed_s >= 0.5f ||
        (dialog->anim_elapsed_s > 0.1f && pos >= 0.999f)) {
      dialog->current_progress = 1.0f;
      dialog->state = CUPERTINO_DIALOG_STATE_PRESENTED;
      settled = 1;
    }
  } else if (dialog->state == CUPERTINO_DIALOG_STATE_DISMISSING) {
    dialog->anim_elapsed_s += delta_sec;
    rc = cupertino_spring_evaluate(&dialog->spring_cfg, dialog->anim_elapsed_s,
                                   &pos, &vel);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    dialog->current_progress = pos;
    if (dialog->anim_elapsed_s >= 0.5f ||
        (dialog->anim_elapsed_s > 0.1f && pos <= 0.001f)) {
      dialog->current_progress = 0.0f;
      dialog->state = CUPERTINO_DIALOG_STATE_DISMISSED;
      settled = 1;
    }
  } else {
    settled = 1;
  }

  rc = cupertino_fullscreen_dialog_update_parent_transform(dialog);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  *out_is_settled = settled;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_fullscreen_dialog_drag_start(
    struct cupertino_fullscreen_dialog *dialog) {
  if (!dialog) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!dialog->swipe_to_dismiss_enabled ||
      dialog->state != CUPERTINO_DIALOG_STATE_PRESENTED) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dialog->state = CUPERTINO_DIALOG_STATE_DRAGGING;
  dialog->drag_offset_y = 0.0f;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_fullscreen_dialog_drag_update(
    struct cupertino_fullscreen_dialog *dialog, float delta_y) {
  float p;
  ui_error_t rc;

  if (!dialog) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (dialog->state != CUPERTINO_DIALOG_STATE_DRAGGING) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Downward displacement only (positive delta_y) */
  if (delta_y < 0.0f) {
    delta_y = 0.0f;
  }

  dialog->drag_offset_y = delta_y;
  p = 1.0f - (delta_y / dialog->screen_height);
  if (p < 0.0f) {
    p = 0.0f;
  }
  dialog->current_progress = p;

  rc = cupertino_fullscreen_dialog_update_parent_transform(dialog);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_fullscreen_dialog_drag_end(struct cupertino_fullscreen_dialog *dialog,
                                     float velocity_y, int *out_will_dismiss) {
  float norm_dist;
  int will_dismiss;
  ui_error_t rc;

  if (!dialog || !out_will_dismiss) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (dialog->state != CUPERTINO_DIALOG_STATE_DRAGGING) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dialog->touch_velocity_y = velocity_y;
  norm_dist = dialog->drag_offset_y / dialog->screen_height;

  will_dismiss = 0;
  if (velocity_y >= dialog->dismiss_velocity_threshold ||
      norm_dist >= dialog->dismiss_distance_threshold) {
    will_dismiss = 1;
  }

  if (will_dismiss) {
    rc = cupertino_fullscreen_dialog_dismiss(dialog);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
    rc = cupertino_fullscreen_dialog_present(dialog);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  *out_will_dismiss = will_dismiss;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_fullscreen_dialog_get_translation_y(
    const struct cupertino_fullscreen_dialog *dialog,
    float *out_translation_y) {
  if (!dialog || !out_translation_y) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (dialog->state == CUPERTINO_DIALOG_STATE_DRAGGING) {
    *out_translation_y = dialog->drag_offset_y;
  } else {
    *out_translation_y =
        dialog->screen_height * (1.0f - dialog->current_progress);
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_fullscreen_dialog_get_parent_transform(
    const struct cupertino_fullscreen_dialog *dialog, float *out_scale,
    float *out_corner_radius, float *out_dim_alpha) {
  if (!dialog || !out_scale || !out_corner_radius || !out_dim_alpha) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_scale = dialog->parent_scale;
  *out_corner_radius = dialog->parent_corner_radius;
  *out_dim_alpha = dialog->parent_dim_alpha;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_fullscreen_dialog_get_nav_action_title(
    const struct cupertino_fullscreen_dialog *dialog, char *out_title,
    size_t title_size) {
  if (!dialog || !out_title || title_size == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(out_title, title_size, dialog->action_title, title_size - 1);
#else
  strncpy(out_title, dialog->action_title, title_size - 1);
  out_title[title_size - 1] = '\0';
#endif

  return UI_ERROR_NONE;
}

ui_error_t cupertino_fullscreen_dialog_set_views(
    struct cupertino_fullscreen_dialog *dialog,
    struct ui_component *dialog_content, struct ui_component *parent_content) {
  if (!dialog) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dialog->dialog_content = dialog_content;
  dialog->parent_content = parent_content;
  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
ui_error_t test_cupertino_fullscreen_dialog_update_parent_transform(
    struct cupertino_fullscreen_dialog *dialog) {
  return (cupertino_fullscreen_dialog_update_parent_transform)(dialog);
}
#endif
