/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
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

/**
 * @brief cmp_pointer_region_create
 *
 * @param out_region Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pointer_region_create(cmp_pointer_region_t **out_region) {
  int rc = CMP_SUCCESS;
  struct cmp_pointer_region *ctx = NULL;

  if (!out_region) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pointer_region_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_pointer_region), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_pointer_region_create: Out of memory\n");
    return rc;
  }

  ctx->style = CMP_POINTER_INTERACTION_AUTOMATIC;

  *out_region = (cmp_pointer_region_t *)ctx;
  return rc;
}

/**
 * @brief cmp_pointer_region_destroy
 *
 * @param region_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pointer_region_destroy(cmp_pointer_region_t *region_opaque) {
  int rc = CMP_SUCCESS;

  if (region_opaque) {
    CMP_FREE(region_opaque);
  }
  return rc;
}

/**
 * @brief cmp_pointer_region_set_style
 *
 * @param region_opaque Parameter description.
 * @param style Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pointer_region_set_style(cmp_pointer_region_t *region_opaque,
                                 cmp_pointer_interaction_style_t style) {
  int rc = CMP_SUCCESS;
  struct cmp_pointer_region *ctx = (struct cmp_pointer_region *)region_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pointer_region_set_style: Invalid argument\n");
    return rc;
  }
  ctx->style = style;
  return rc;
}

/**
 * @brief cmp_pointer_region_get_morph_scale
 *
 * @param region_opaque Parameter description.
 * @param out_scale_x Parameter description.
 * @param out_scale_y Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pointer_region_get_morph_scale(cmp_pointer_region_t *region_opaque,
                                       float *out_scale_x, float *out_scale_y) {
  int rc = CMP_SUCCESS;
  struct cmp_pointer_region *ctx = (struct cmp_pointer_region *)region_opaque;

  if (!ctx || !out_scale_x || !out_scale_y) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_pointer_region_get_morph_scale: Invalid argument\n");
    return rc;
  }

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
  return rc;
}

/**
 * @brief cmp_keyboard_shortcut_create
 *
 * @param out_shortcut Parameter description.
 * @param key Parameter description.
 * @param modifier_flags Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_keyboard_shortcut_create(cmp_keyboard_shortcut_t **out_shortcut,
                                 char key, uint32_t modifier_flags) {
  int rc = CMP_SUCCESS;
  struct cmp_keyboard_shortcut *ctx = NULL;

  if (!out_shortcut) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_keyboard_shortcut_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_keyboard_shortcut), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_keyboard_shortcut_create: Out of memory\n");
    return rc;
  }

  ctx->key = key;
  ctx->modifiers = modifier_flags;

  *out_shortcut = (cmp_keyboard_shortcut_t *)ctx;
  return rc;
}

/**
 * @brief cmp_keyboard_shortcut_destroy
 *
 * @param shortcut_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_keyboard_shortcut_destroy(cmp_keyboard_shortcut_t *shortcut_opaque) {
  int rc = CMP_SUCCESS;

  if (shortcut_opaque) {
    CMP_FREE(shortcut_opaque);
  }
  return rc;
}

/**
 * @brief cmp_ui_command_create
 *
 * @param out_command Parameter description.
 * @param title Parameter description.
 * @param action_id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_command_create(cmp_ui_command_t **out_command, const char *title,
                          const char *action_id) {
  int rc = CMP_SUCCESS;
  struct cmp_ui_command *ctx = NULL;
  size_t len;

  if (!out_command || !title || !action_id) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ui_command_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_ui_command), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_ui_command_create: Out of memory\n");
    return rc;
  }

  ctx->shortcut = NULL;

  len = strlen(title);
  rc = CMP_MALLOC(len + 1, (void **)&ctx->title);
  if (rc != CMP_SUCCESS) {
    CMP_FREE(ctx);
    LOG_DEBUG(
        "Error in cmp_ui_command_create: Out of memory allocating title\n");
    return rc;
  }
#if defined(_MSC_VER)
  strcpy_s(ctx->title, len + 1, title);
#else
  strcpy(ctx->title, title);
#endif

  len = strlen(action_id);
  rc = CMP_MALLOC(len + 1, (void **)&ctx->action_id);
  if (rc != CMP_SUCCESS) {
    CMP_FREE(ctx->title);
    CMP_FREE(ctx);
    LOG_DEBUG(
        "Error in cmp_ui_command_create: Out of memory allocating action_id\n");
    return rc;
  }
#if defined(_MSC_VER)
  strcpy_s(ctx->action_id, len + 1, action_id);
#else
  strcpy(ctx->action_id, action_id);
#endif

  *out_command = (cmp_ui_command_t *)ctx;
  return rc;
}

/**
 * @brief cmp_ui_command_destroy
 *
 * @param command_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_command_destroy(cmp_ui_command_t *command_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_ui_command *ctx = (struct cmp_ui_command *)command_opaque;

  if (!ctx) {
    return rc;
  }

  if (ctx->title)
    CMP_FREE(ctx->title);
  if (ctx->action_id)
    CMP_FREE(ctx->action_id);
  if (ctx->shortcut)
    cmp_keyboard_shortcut_destroy((cmp_keyboard_shortcut_t *)ctx->shortcut);

  CMP_FREE(ctx);
  return rc;
}

/**
 * @brief cmp_ui_command_set_shortcut
 *
 * @param command_opaque Parameter description.
 * @param shortcut_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_command_set_shortcut(cmp_ui_command_t *command_opaque,
                                cmp_keyboard_shortcut_t *shortcut_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_ui_command *ctx = (struct cmp_ui_command *)command_opaque;

  if (!ctx || !shortcut_opaque) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ui_command_set_shortcut: Invalid argument\n");
    return rc;
  }

  if (ctx->shortcut) {
    cmp_keyboard_shortcut_destroy((cmp_keyboard_shortcut_t *)ctx->shortcut);
  }
  ctx->shortcut = (struct cmp_keyboard_shortcut *)shortcut_opaque;

  return rc;
}

/**
 * @brief cmp_keyboard_calculate_key_repeat
 *
 * @param time_held_ms Parameter description.
 * @param out_repeat_interval_ms Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_keyboard_calculate_key_repeat(float time_held_ms,
                                      float *out_repeat_interval_ms) {
  int rc = CMP_SUCCESS;
  float scale;

  if (!out_repeat_interval_ms) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_keyboard_calculate_key_repeat: Invalid argument\n");
    return rc;
  }

  /* Initial delay before repeat starts (typical OS default: ~500ms) */
  if (time_held_ms < 500.0f) {
    *out_repeat_interval_ms = 0.0f; /* Do not repeat yet */
    return rc;
  }

  /* Accelerating curve based on duration held.
     Starts repeating every 100ms, scaling down to a minimum of 20ms */
  scale = (time_held_ms - 500.0f) / 1000.0f; /* Progress over next 1 second */
  if (scale > 1.0f)
    scale = 1.0f;

  *out_repeat_interval_ms = 100.0f - (80.0f * scale);
  if (*out_repeat_interval_ms < 20.0f)
    *out_repeat_interval_ms = 20.0f;

  return rc;
}

/**
 * @brief cmp_trackpad_evaluate_gesture
 *
 * @param delta_x Parameter description.
 * @param delta_y Parameter description.
 * @param out_pan_x Parameter description.
 * @param out_pan_y Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_trackpad_evaluate_gesture(float delta_x, float delta_y,
                                  float *out_pan_x, float *out_pan_y) {
  int rc = CMP_SUCCESS;

  if (!out_pan_x || !out_pan_y) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_trackpad_evaluate_gesture: Invalid argument\n");
    return rc;
  }

  /* Two-finger swipe back/forward maps to horizontal pan.
     In a real impl, this would translate delta directly to view controllers */
  *out_pan_x = delta_x;
  *out_pan_y = delta_y;

  return rc;
}
