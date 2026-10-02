/**
 * @file cupertino_scrollbar.c
 * @brief Cupertino Scrollbar component implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_scrollbar.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

static void cupertino_scrollbar_recalculate_geometry(
    struct cupertino_scrollbar *scrollbar) {
  float track_h;
  float max_scroll;
  float max_travel;
  float ratio;
  float p;

  track_h = scrollbar->viewport_length -
            (scrollbar->safe_area_top + scrollbar->safe_area_bottom);
  if (track_h <= 0.0f) {
    track_h = scrollbar->viewport_length;
  }

  if (scrollbar->content_length <= scrollbar->viewport_length) {
    scrollbar->thumb_length = track_h;
    scrollbar->thumb_offset = scrollbar->safe_area_top;
    return;
  }

  ratio = scrollbar->viewport_length / scrollbar->content_length;
  scrollbar->thumb_length = track_h * ratio;
  if (scrollbar->thumb_length < CUPERTINO_SCROLLBAR_MIN_THUMB_LENGTH) {
    scrollbar->thumb_length = CUPERTINO_SCROLLBAR_MIN_THUMB_LENGTH;
  }

  max_scroll = scrollbar->content_length - scrollbar->viewport_length;
  max_travel = track_h - scrollbar->thumb_length;
  if (max_travel < 0.0f) {
    max_travel = 0.0f;
  }

  p = scrollbar->content_offset / max_scroll;
  if (p < 0.0f) {
    p = 0.0f;
  } else if (p > 1.0f) {
    p = 1.0f;
  }

  scrollbar->thumb_offset = scrollbar->safe_area_top + (p * max_travel);
}

ui_error_t
cupertino_scrollbar_create(struct ui_engine *engine,
                           const struct cupertino_scrollbar_descriptor *desc,
                           struct cupertino_scrollbar **out_scrollbar) {
  struct cupertino_scrollbar *scrollbar;

  if (!engine || !desc || !out_scrollbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scrollbar = (struct cupertino_scrollbar *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_scrollbar));
  if (!scrollbar) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(scrollbar, 0, sizeof(*scrollbar));
  scrollbar->safe_area_top = desc->safe_area_top;
  scrollbar->safe_area_bottom = desc->safe_area_bottom;
  scrollbar->current_thickness = CUPERTINO_SCROLLBAR_THICKNESS_RESTING;
  scrollbar->current_opacity = 0.0f;
  scrollbar->inactivity_timer_ms = 0.0f;

  *out_scrollbar = scrollbar;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_scrollbar_destroy(struct cupertino_scrollbar *scrollbar) {
  if (!scrollbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(scrollbar);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_scrollbar_handle_scroll(struct cupertino_scrollbar *scrollbar,
                                  float content_offset, float content_length,
                                  float viewport_length) {
  if (!scrollbar || viewport_length <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scrollbar->content_offset = content_offset;
  scrollbar->content_length = content_length;
  scrollbar->viewport_length = viewport_length;
  scrollbar->current_opacity = 1.0f;
  scrollbar->inactivity_timer_ms = 0.0f;

  cupertino_scrollbar_recalculate_geometry(scrollbar);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_scrollbar_tick(struct cupertino_scrollbar *scrollbar,
                                    float delta_ms) {
  float fade_elapsed;

  if (!scrollbar || delta_ms < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (scrollbar->is_dragging) {
    scrollbar->inactivity_timer_ms = 0.0f;
    scrollbar->current_opacity = 1.0f;
    return UI_ERROR_NONE;
  }

  scrollbar->inactivity_timer_ms += delta_ms;

  if (scrollbar->inactivity_timer_ms > CUPERTINO_SCROLLBAR_FADE_TIMEOUT_MS) {
    fade_elapsed =
        scrollbar->inactivity_timer_ms - CUPERTINO_SCROLLBAR_FADE_TIMEOUT_MS;
    if (fade_elapsed >= CUPERTINO_SCROLLBAR_FADE_ANIMATION_MS) {
      scrollbar->current_opacity = 0.0f;
    } else {
      scrollbar->current_opacity =
          1.0f - (fade_elapsed / CUPERTINO_SCROLLBAR_FADE_ANIMATION_MS);
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_scrollbar_set_dragging(struct cupertino_scrollbar *scrollbar,
                                 int dragging) {
  if (!scrollbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scrollbar->is_dragging = dragging ? 1 : 0;
  scrollbar->inactivity_timer_ms = 0.0f;
  scrollbar->current_opacity = 1.0f;
  scrollbar->current_thickness = scrollbar->is_dragging
                                     ? CUPERTINO_SCROLLBAR_THICKNESS_ACTIVE
                                     : CUPERTINO_SCROLLBAR_THICKNESS_RESTING;

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_scrollbar_is_dragging(const struct cupertino_scrollbar *scrollbar,
                                int *out_dragging) {
  if (!scrollbar || !out_dragging) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_dragging = scrollbar->is_dragging;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_scrollbar_get_thickness(const struct cupertino_scrollbar *scrollbar,
                                  float *out_thickness) {
  if (!scrollbar || !out_thickness) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_thickness = scrollbar->current_thickness;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_scrollbar_get_opacity(const struct cupertino_scrollbar *scrollbar,
                                float *out_opacity) {
  if (!scrollbar || !out_opacity) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_opacity = scrollbar->current_opacity;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_scrollbar_get_thumb_geometry(
    const struct cupertino_scrollbar *scrollbar, float *out_offset,
    float *out_size) {
  if (!scrollbar || !out_offset || !out_size) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_offset = scrollbar->thumb_offset;
  *out_size = scrollbar->thumb_length;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_scrollbar_set_safe_area_insets(struct cupertino_scrollbar *scrollbar,
                                         float top, float bottom) {
  if (!scrollbar || top < 0.0f || bottom < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scrollbar->safe_area_top = top;
  scrollbar->safe_area_bottom = bottom;
  cupertino_scrollbar_recalculate_geometry(scrollbar);
  return UI_ERROR_NONE;
}
