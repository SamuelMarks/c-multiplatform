/**
 * @file cupertino_menu_bar_extra.c
 * @brief macOS Menu Bar Extras / System Status Items implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_menu_bar_extra.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_menu_bar_extra_mock_recompute_fail = 0;
#endif

ui_error_t cupertino_menu_bar_extra_recompute_bounds(
    struct cupertino_menu_bar_extra *extra) {
  size_t title_len;
  float content_w;

  if (!extra) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#ifdef UI_TEST_MOCK_ALLOC
  if (g_cupertino_menu_bar_extra_mock_recompute_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
#endif

  extra->computed_height = CUPERTINO_MENU_BAR_EXTRA_STANDARD_HEIGHT;

  if (extra->fixed_width > 0.0f) {
    extra->computed_width = extra->fixed_width;
    return UI_ERROR_NONE;
  }

  content_w = 0.0f;
  if (extra->icon_symbol[0] != '\0') {
    content_w += 18.0f; /* 18pt icon frame */
  }

  title_len = strlen(extra->title);
  if (title_len > 0) {
    if (content_w > 0.0f) {
      content_w += 4.0f; /* Gap between icon and label */
    }
    content_w += ((float)title_len * 7.5f); /* Approx 7.5pt per character */
  }

  /* Padding on left and right */
  content_w += 10.0f;
  if (content_w < 24.0f) {
    content_w = 24.0f;
  }

  extra->computed_width = content_w;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_menu_bar_extra_create(
    struct ui_engine *engine,
    const struct cupertino_menu_bar_extra_descriptor *desc,
    struct cupertino_menu_bar_extra **out_extra) {
  struct cupertino_menu_bar_extra *extra;
  ui_error_t rc;

  if (!engine || !desc || !out_extra) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  extra = (struct cupertino_menu_bar_extra *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_menu_bar_extra));
  if (!extra) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(extra, 0, sizeof(*extra));
  extra->is_animating = desc->is_animating ? 1 : 0;
  extra->animation_phase =
      (desc->animation_phase >= 0.0f && desc->animation_phase <= 1.0f)
          ? desc->animation_phase
          : 0.0f;
  extra->fixed_width = desc->fixed_width > 0.0f ? desc->fixed_width : 0.0f;

  if (desc->title) {
#if defined(_MSC_VER)
    strncpy_s(extra->title, sizeof(extra->title), desc->title, _TRUNCATE);
#else
    strncpy(extra->title, desc->title, sizeof(extra->title) - 1);
    extra->title[sizeof(extra->title) - 1] = '\0';
#endif
  }

  if (desc->icon_symbol) {
#if defined(_MSC_VER)
    strncpy_s(extra->icon_symbol, sizeof(extra->icon_symbol), desc->icon_symbol,
              _TRUNCATE);
#else
    strncpy(extra->icon_symbol, desc->icon_symbol,
            sizeof(extra->icon_symbol) - 1);
    extra->icon_symbol[sizeof(extra->icon_symbol) - 1] = '\0';
#endif
  }

  rc = cupertino_menu_bar_extra_recompute_bounds(extra);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(extra);
    *out_extra = NULL;
    return rc;
  }

  *out_extra = extra;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_menu_bar_extra_destroy(struct cupertino_menu_bar_extra *extra) {
  if (!extra) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(extra);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_menu_bar_extra_set_title(struct cupertino_menu_bar_extra *extra,
                                   const char *title) {
  if (!extra) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (title) {
#if defined(_MSC_VER)
    strncpy_s(extra->title, sizeof(extra->title), title, _TRUNCATE);
#else
    strncpy(extra->title, title, sizeof(extra->title) - 1);
    extra->title[sizeof(extra->title) - 1] = '\0';
#endif
  } else {
    extra->title[0] = '\0';
  }

  return cupertino_menu_bar_extra_recompute_bounds(extra);
}

ui_error_t
cupertino_menu_bar_extra_get_title(const struct cupertino_menu_bar_extra *extra,
                                   const char **out_title) {
  if (!extra || !out_title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_title = extra->title;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_menu_bar_extra_set_icon(struct cupertino_menu_bar_extra *extra,
                                  const char *icon_symbol) {
  if (!extra) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (icon_symbol) {
#if defined(_MSC_VER)
    strncpy_s(extra->icon_symbol, sizeof(extra->icon_symbol), icon_symbol,
              _TRUNCATE);
#else
    strncpy(extra->icon_symbol, icon_symbol, sizeof(extra->icon_symbol) - 1);
    extra->icon_symbol[sizeof(extra->icon_symbol) - 1] = '\0';
#endif
  } else {
    extra->icon_symbol[0] = '\0';
  }

  return cupertino_menu_bar_extra_recompute_bounds(extra);
}

ui_error_t
cupertino_menu_bar_extra_get_icon(const struct cupertino_menu_bar_extra *extra,
                                  const char **out_symbol) {
  if (!extra || !out_symbol) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_symbol = extra->icon_symbol;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_menu_bar_extra_set_animating(struct cupertino_menu_bar_extra *extra,
                                       int is_animating,
                                       float animation_phase) {
  if (!extra || animation_phase < 0.0f || animation_phase > 1.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  extra->is_animating = is_animating ? 1 : 0;
  extra->animation_phase = animation_phase;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_menu_bar_extra_is_animating(
    const struct cupertino_menu_bar_extra *extra, int *out_is_animating,
    float *out_phase) {
  if (!extra || !out_is_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_animating = extra->is_animating;
  if (out_phase) {
    *out_phase = extra->animation_phase;
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_menu_bar_extra_set_menu_open(struct cupertino_menu_bar_extra *extra,
                                       int is_open) {
  if (!extra) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  extra->is_menu_open = is_open ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_menu_bar_extra_is_menu_open(
    const struct cupertino_menu_bar_extra *extra, int *out_is_open) {
  if (!extra || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_open = extra->is_menu_open;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_menu_bar_extra_click(struct cupertino_menu_bar_extra *extra) {
  if (!extra) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  extra->is_menu_open = !extra->is_menu_open;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_menu_bar_extra_get_bounds(
    const struct cupertino_menu_bar_extra *extra, float *out_w, float *out_h) {
  if (!extra || !out_w || !out_h) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_w = extra->computed_width;
  *out_h = extra->computed_height;
  return UI_ERROR_NONE;
}
