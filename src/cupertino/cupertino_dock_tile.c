/**
 * @file cupertino_dock_tile.c
 * @brief macOS Dock Tile Live Sync Controller implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_dock_tile.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

ui_error_t
cupertino_dock_tile_create(struct ui_engine *engine,
                           const struct cupertino_dock_tile_descriptor *desc,
                           struct cupertino_dock_tile **out_dock_tile) {
  struct cupertino_dock_tile *tile;

  if (!engine || !desc || !out_dock_tile) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tile = (struct cupertino_dock_tile *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_dock_tile));
  if (!tile) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(tile, 0, sizeof(*tile));
  tile->progress =
      (desc->initial_progress >= 0.0f && desc->initial_progress <= 1.0f)
          ? desc->initial_progress
          : 0.0f;
  tile->show_progress = desc->show_progress ? 1 : 0;
  tile->action_count = 0;

  if (desc->initial_badge_label && desc->initial_badge_label[0] != '\0') {
#if defined(_MSC_VER)
    strncpy_s(tile->badge_label, sizeof(tile->badge_label),
              desc->initial_badge_label, _TRUNCATE);
#else
    strncpy(tile->badge_label, desc->initial_badge_label,
            sizeof(tile->badge_label) - 1);
    tile->badge_label[sizeof(tile->badge_label) - 1] = '\0';
#endif
    tile->has_badge = 1;
  } else {
    tile->has_badge = 0;
  }

  *out_dock_tile = tile;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_dock_tile_destroy(struct cupertino_dock_tile *dock_tile) {
  if (!dock_tile) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(dock_tile);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_dock_tile_set_badge_label(struct cupertino_dock_tile *dock_tile,
                                    const char *label) {
  if (!dock_tile) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (label && label[0] != '\0') {
#if defined(_MSC_VER)
    strncpy_s(dock_tile->badge_label, sizeof(dock_tile->badge_label), label,
              _TRUNCATE);
#else
    strncpy(dock_tile->badge_label, label, sizeof(dock_tile->badge_label) - 1);
    dock_tile->badge_label[sizeof(dock_tile->badge_label) - 1] = '\0';
#endif
    dock_tile->has_badge = 1;
  } else {
    dock_tile->badge_label[0] = '\0';
    dock_tile->has_badge = 0;
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_dock_tile_get_badge_label(const struct cupertino_dock_tile *dock_tile,
                                    const char **out_label,
                                    int *out_has_badge) {
  if (!dock_tile || !out_label || !out_has_badge) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_label = dock_tile->badge_label;
  *out_has_badge = dock_tile->has_badge;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_dock_tile_set_progress(struct cupertino_dock_tile *dock_tile,
                                 float progress, int is_visible) {
  if (!dock_tile || progress < 0.0f || progress > 1.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dock_tile->progress = progress;
  dock_tile->show_progress = is_visible ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_dock_tile_get_progress(const struct cupertino_dock_tile *dock_tile,
                                 float *out_progress, int *out_is_visible) {
  if (!dock_tile || !out_progress || !out_is_visible) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_progress = dock_tile->progress;
  *out_is_visible = dock_tile->show_progress;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_dock_tile_add_action(struct cupertino_dock_tile *dock_tile,
                                          const char *title, int action_id) {
  size_t idx;

  if (!dock_tile || !title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (dock_tile->action_count >= CUPERTINO_DOCK_TILE_MAX_ACTIONS) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  idx = dock_tile->action_count;
#if defined(_MSC_VER)
  strncpy_s(dock_tile->actions[idx].title,
            sizeof(dock_tile->actions[idx].title), title, _TRUNCATE);
#else
  strncpy(dock_tile->actions[idx].title, title,
          sizeof(dock_tile->actions[idx].title) - 1);
  dock_tile->actions[idx].title[sizeof(dock_tile->actions[idx].title) - 1] =
      '\0';
#endif
  dock_tile->actions[idx].action_id = action_id;
  dock_tile->actions[idx].is_disabled = 0;
  dock_tile->action_count++;

  return UI_ERROR_NONE;
}

ui_error_t cupertino_dock_tile_get_action_count(
    const struct cupertino_dock_tile *dock_tile, size_t *out_count) {
  if (!dock_tile || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = dock_tile->action_count;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_dock_tile_get_action_at(const struct cupertino_dock_tile *dock_tile,
                                  size_t index, const char **out_title,
                                  int *out_action_id, int *out_is_disabled) {
  if (!dock_tile || index >= dock_tile->action_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (out_title) {
    *out_title = dock_tile->actions[index].title;
  }
  if (out_action_id) {
    *out_action_id = dock_tile->actions[index].action_id;
  }
  if (out_is_disabled) {
    *out_is_disabled = dock_tile->actions[index].is_disabled;
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_dock_tile_trigger_action(struct cupertino_dock_tile *dock_tile,
                                   int action_id, int *out_handled) {
  size_t i;

  if (!dock_tile || !out_handled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_handled = 0;
  for (i = 0; i < dock_tile->action_count; i++) {
    if (dock_tile->actions[i].action_id == action_id) {
      if (!dock_tile->actions[i].is_disabled) {
        *out_handled = 1;
        return UI_ERROR_NONE;
      }
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_dock_tile_get_badge_bounds(
    const struct cupertino_dock_tile *dock_tile, float *out_x, float *out_y,
    float *out_w, float *out_h) {
  size_t len;
  float w;

  if (!dock_tile || !out_x || !out_y || !out_w || !out_h) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!dock_tile->has_badge) {
    *out_x = 0.0f;
    *out_y = 0.0f;
    *out_w = 0.0f;
    *out_h = 0.0f;
    return UI_ERROR_NONE;
  }

  len = strlen(dock_tile->badge_label);
  w = 24.0f;
  if (len > 1) {
    w += (float)(len - 1) * 8.0f;
  }

  *out_w = w;
  *out_h = 24.0f;
  *out_x = CUPERTINO_DOCK_TILE_ICON_SIZE - w + 4.0f;
  *out_y = -4.0f;

  return UI_ERROR_NONE;
}

ui_error_t cupertino_dock_tile_get_progress_bounds(
    const struct cupertino_dock_tile *dock_tile, float *out_x, float *out_y,
    float *out_w, float *out_h) {
  if (!dock_tile || !out_x || !out_y || !out_w || !out_h) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!dock_tile->show_progress) {
    *out_x = 0.0f;
    *out_y = 0.0f;
    *out_w = 0.0f;
    *out_h = 0.0f;
    return UI_ERROR_NONE;
  }

  *out_w = 104.0f;
  *out_h = 8.0f;
  *out_x = (CUPERTINO_DOCK_TILE_ICON_SIZE - 104.0f) * 0.5f;
  *out_y = CUPERTINO_DOCK_TILE_ICON_SIZE - 8.0f - 10.0f;

  return UI_ERROR_NONE;
}
