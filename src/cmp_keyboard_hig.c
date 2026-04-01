/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_pointer_region {
  cmp_pointer_interaction_style_t style;
};

struct cmp_keyboard_shortcut {
  char key;
  uint32_t modifiers;
};

struct cmp_ui_command {
  char *title;
  char *action_id;
  cmp_keyboard_shortcut_t *shortcut;
};

int cmp_pointer_region_create(cmp_pointer_region_t **out_region) {
  struct cmp_pointer_region *ctx;
  if (!out_region)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_pointer_region), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->style = CMP_POINTER_INTERACTION_AUTOMATIC;

  *out_region = (cmp_pointer_region_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_pointer_region_destroy(cmp_pointer_region_t *region_opaque) {
  if (region_opaque)
    CMP_FREE(region_opaque);
  return CMP_SUCCESS;
}

int cmp_pointer_region_set_style(cmp_pointer_region_t *region_opaque,
                                 cmp_pointer_interaction_style_t style) {
  struct cmp_pointer_region *ctx = (struct cmp_pointer_region *)region_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->style = style;
  return CMP_SUCCESS;
}

int cmp_pointer_region_get_morph_scale(cmp_pointer_region_t *region_opaque,
                                       float *out_scale_x, float *out_scale_y) {
  struct cmp_pointer_region *ctx = (struct cmp_pointer_region *)region_opaque;
  if (!ctx || !out_scale_x || !out_scale_y)
    return CMP_ERROR_INVALID_ARG;

  switch (ctx->style) {
  case CMP_POINTER_INTERACTION_LIFT:
    *out_scale_x = 1.05f; /* Element lifts and scales slightly */
    *out_scale_y = 1.05f;
    break;
  case CMP_POINTER_INTERACTION_HIGHLIGHT:
    *out_scale_x = 1.0f; /* Element doesn't scale, just background */
    *out_scale_y = 1.0f;
    break;
  case CMP_POINTER_INTERACTION_HOVER:
    *out_scale_x = 1.02f; /* Minor bump */
    *out_scale_y = 1.02f;
    break;
  default:
    *out_scale_x = 1.0f;
    *out_scale_y = 1.0f;
    break;
  }
  return CMP_SUCCESS;
}

int cmp_keyboard_shortcut_create(cmp_keyboard_shortcut_t **out_shortcut,
                                 char key, uint32_t modifier_flags) {
  struct cmp_keyboard_shortcut *ctx;
  if (!out_shortcut)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_keyboard_shortcut), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->key = key;
  ctx->modifiers = modifier_flags;

  *out_shortcut = (cmp_keyboard_shortcut_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_keyboard_shortcut_destroy(cmp_keyboard_shortcut_t *shortcut_opaque) {
  if (shortcut_opaque)
    CMP_FREE(shortcut_opaque);
  return CMP_SUCCESS;
}

int cmp_ui_command_create(cmp_ui_command_t **out_command, const char *title,
                          const char *action_id) {
  struct cmp_ui_command *ctx;
  size_t len;

  if (!out_command || !title || !action_id)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_ui_command), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->shortcut = NULL;

  len = strlen(title);
  if (CMP_MALLOC(len + 1, (void **)&ctx->title) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
#if defined(_MSC_VER)
  strcpy_s(ctx->title, len + 1, title);
#else
  strcpy(ctx->title, title);
#endif

  len = strlen(action_id);
  if (CMP_MALLOC(len + 1, (void **)&ctx->action_id) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
#if defined(_MSC_VER)
  strcpy_s(ctx->action_id, len + 1, action_id);
#else
  strcpy(ctx->action_id, action_id);
#endif

  *out_command = (cmp_ui_command_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_ui_command_destroy(cmp_ui_command_t *command_opaque) {
  struct cmp_ui_command *ctx = (struct cmp_ui_command *)command_opaque;
  if (!ctx)
    return CMP_SUCCESS;

  if (ctx->title)
    CMP_FREE(ctx->title);
  if (ctx->action_id)
    CMP_FREE(ctx->action_id);
  if (ctx->shortcut)
    cmp_keyboard_shortcut_destroy((cmp_keyboard_shortcut_t *)ctx->shortcut);

  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_ui_command_set_shortcut(cmp_ui_command_t *command_opaque,
                                cmp_keyboard_shortcut_t *shortcut_opaque) {
  struct cmp_ui_command *ctx = (struct cmp_ui_command *)command_opaque;
  if (!ctx || !shortcut_opaque)
    return CMP_ERROR_INVALID_ARG;

  if (ctx->shortcut)
    cmp_keyboard_shortcut_destroy((cmp_keyboard_shortcut_t *)ctx->shortcut);
  ctx->shortcut = (struct cmp_keyboard_shortcut *)shortcut_opaque;

  return CMP_SUCCESS;
}

int cmp_keyboard_calculate_key_repeat(float time_held_ms,
                                      float *out_repeat_interval_ms) {
  if (!out_repeat_interval_ms)
    return CMP_ERROR_INVALID_ARG;

  /* Initial delay before repeat starts (typical OS default: ~500ms) */
  if (time_held_ms < 500.0f) {
    *out_repeat_interval_ms = 0.0f; /* Do not repeat yet */
    return CMP_SUCCESS;
  }

  /* Accelerating curve based on duration held.
     Starts repeating every 100ms, scaling down to a minimum of 20ms */
  float scale =
      (time_held_ms - 500.0f) / 1000.0f; /* Progress over next 1 second */
  if (scale > 1.0f)
    scale = 1.0f;

  *out_repeat_interval_ms = 100.0f - (80.0f * scale);
  if (*out_repeat_interval_ms < 20.0f)
    *out_repeat_interval_ms = 20.0f;

  return CMP_SUCCESS;
}

int cmp_trackpad_evaluate_gesture(float delta_x, float delta_y,
                                  float *out_pan_x, float *out_pan_y) {
  if (!out_pan_x || !out_pan_y)
    return CMP_ERROR_INVALID_ARG;

  /* Two-finger swipe back/forward maps to horizontal pan.
     In a real impl, this would translate delta directly to view controllers */
  *out_pan_x = delta_x;
  *out_pan_y = delta_y;

  return CMP_SUCCESS;
}
