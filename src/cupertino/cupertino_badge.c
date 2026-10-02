/**
 * @file cupertino_badge.c
 * @brief Cupertino Notification Badge implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_badge.h"
#include "cupertino/cupertino_tokens.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_badge_mock_get_system_color_fail = 0;
int g_cupertino_badge_mock_create_fail = 0;
int g_cupertino_badge_mock_set_text_fail = 0;
int g_cupertino_badge_mock_set_value_fail = 0;
int g_cupertino_badge_mock_set_hidden_fail = 0;
int g_cupertino_badge_mock_destroy_fail = 0;

static ui_error_t
mock_cupertino_get_system_color(enum cupertino_system_color color, int is_dark,
                                int high_contrast, ui_color_t *out_color) {
  if (g_cupertino_badge_mock_get_system_color_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return cupertino_get_system_color(color, is_dark, high_contrast, out_color);
}
#undef cupertino_get_system_color
/** @cond */
#define cupertino_get_system_color mock_cupertino_get_system_color
/** @endcond */

static ui_error_t mock_badge_base_create(struct ui_badge_base **out_badge) {
  if (g_cupertino_badge_mock_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  return ui_badge_base_create(out_badge);
}
#undef ui_badge_base_create
/** @cond */
#define ui_badge_base_create mock_badge_base_create
/** @endcond */

static ui_error_t mock_badge_base_set_text(struct ui_badge_base *badge,
                                           const char *text) {
  if (g_cupertino_badge_mock_set_text_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_badge_base_set_text(badge, text);
}
#undef ui_badge_base_set_text
/** @cond */
#define ui_badge_base_set_text mock_badge_base_set_text
/** @endcond */

static ui_error_t mock_badge_base_set_value(struct ui_badge_base *badge,
                                            int value, int max_val) {
  if (g_cupertino_badge_mock_set_value_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_badge_base_set_value(badge, value, max_val);
}
#undef ui_badge_base_set_value
/** @cond */
#define ui_badge_base_set_value mock_badge_base_set_value
/** @endcond */

static ui_error_t mock_badge_base_set_hidden(struct ui_badge_base *badge,
                                             int is_hidden) {
  if (g_cupertino_badge_mock_set_hidden_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_badge_base_set_hidden(badge, is_hidden);
}
#undef ui_badge_base_set_hidden
/** @cond */
#define ui_badge_base_set_hidden mock_badge_base_set_hidden
/** @endcond */

static ui_error_t mock_badge_base_destroy(struct ui_badge_base *badge) {
  if (g_cupertino_badge_mock_destroy_fail) {
    (ui_badge_base_destroy)(badge);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_badge_base_destroy)(badge);
}
#undef ui_badge_base_destroy
/** @cond */
#define ui_badge_base_destroy mock_badge_base_destroy
/** @endcond */
#endif

static void cupertino_badge_calculate_layout(struct cupertino_badge *badge) {
  size_t len;

  if (badge->style == CUPERTINO_BADGE_STYLE_DOT) {
    badge->width = CUPERTINO_BADGE_DOT_SIZE;
    badge->height = CUPERTINO_BADGE_DOT_SIZE;
    return;
  }

  len = strlen(badge->text);
  if (len == 0) {
    badge->width = 0.0f;
    badge->height = 0.0f;
  } else if (len == 1) {
    badge->width = CUPERTINO_BADGE_STANDARD_HEIGHT;
    badge->height = CUPERTINO_BADGE_STANDARD_HEIGHT;
  } else if (len == 2) {
    badge->width = 24.0f;
    badge->height = CUPERTINO_BADGE_STANDARD_HEIGHT;
  } else {
    badge->width = 20.0f + ((float)(len - 1) * 7.0f);
    badge->height = CUPERTINO_BADGE_STANDARD_HEIGHT;
  }
}

ui_error_t cupertino_badge_create(struct ui_engine *engine,
                                  const struct cupertino_badge_descriptor *desc,
                                  struct cupertino_badge **out_badge) {
  struct cupertino_badge *badge;
  ui_error_t rc;
  ui_color_t red_col;

  if (!engine || !desc || !out_badge) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_badge = NULL;

  badge = (struct cupertino_badge *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_badge));
  if (!badge) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(badge, 0, sizeof(*badge));
  badge->style = desc->style;
  badge->count = desc->count;
  badge->max_count = (desc->max_count > 0) ? desc->max_count
                                           : CUPERTINO_BADGE_DEFAULT_MAX_COUNT;
  badge->is_hidden = desc->is_hidden ? 1 : 0;

  rc = cupertino_get_system_color(CUPERTINO_COLOR_RED, 0, 0, &red_col);
  if (rc != UI_ERROR_NONE) {
    red_col = UI_COLOR_ARGB(255, 255, 59, 48);
  }
  badge->background_color = red_col;
  badge->text_color = UI_COLOR_ARGB(255, 255, 255, 255);

  rc = ui_badge_base_create(&badge->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(badge);
    return rc;
  }

  if (desc->custom_text) {
#if defined(_MSC_VER)
    strncpy_s(badge->text, sizeof(badge->text), desc->custom_text, _TRUNCATE);
#else
    strncpy(badge->text, desc->custom_text, sizeof(badge->text) - 1);
    badge->text[sizeof(badge->text) - 1] = '\0';
#endif
    rc = ui_badge_base_set_text(badge->base, badge->text);
    if (rc != UI_ERROR_NONE) {
      ui_error_t destroy_rc;
      destroy_rc = ui_badge_base_destroy(badge->base);
      C_MULTIPLATFORM_FREE(badge);
      if (destroy_rc != UI_ERROR_NONE) {
        return destroy_rc;
      }
      return rc;
    }
  } else if (desc->count > 0) {
    if (desc->count > badge->max_count) {
#if defined(_MSC_VER)
      sprintf_s(badge->text, sizeof(badge->text), "%d+", badge->max_count);
#else
      sprintf(badge->text, "%d+", badge->max_count);
#endif
    } else {
#if defined(_MSC_VER)
      sprintf_s(badge->text, sizeof(badge->text), "%d", desc->count);
#else
      sprintf(badge->text, "%d", desc->count);
#endif
    }
    rc = ui_badge_base_set_value(badge->base, desc->count, badge->max_count);
    if (rc != UI_ERROR_NONE) {
      ui_error_t destroy_rc;
      destroy_rc = ui_badge_base_destroy(badge->base);
      C_MULTIPLATFORM_FREE(badge);
      if (destroy_rc != UI_ERROR_NONE) {
        return destroy_rc;
      }
      return rc;
    }
  } else {
    badge->text[0] = '\0';
  }

  rc = ui_badge_base_set_hidden(badge->base, badge->is_hidden);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_badge_base_destroy(badge->base);
    C_MULTIPLATFORM_FREE(badge);
    if (destroy_rc != UI_ERROR_NONE) {
      return destroy_rc;
    }
    return rc;
  }

  cupertino_badge_calculate_layout(badge);

  *out_badge = badge;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_badge_destroy(struct cupertino_badge *badge) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!badge) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (badge->base) {
    rc = ui_badge_base_destroy(badge->base);
    badge->base = NULL;
  }

  C_MULTIPLATFORM_FREE(badge);
  return rc;
}

ui_error_t cupertino_badge_set_count(struct cupertino_badge *badge, int count) {
  ui_error_t rc;

  if (!badge || count < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  badge->count = count;
  if (count > badge->max_count) {
#if defined(_MSC_VER)
    sprintf_s(badge->text, sizeof(badge->text), "%d+", badge->max_count);
#else
    sprintf(badge->text, "%d+", badge->max_count);
#endif
  } else if (count == 0) {
    badge->text[0] = '\0';
  } else {
#if defined(_MSC_VER)
    sprintf_s(badge->text, sizeof(badge->text), "%d", count);
#else
    sprintf(badge->text, "%d", count);
#endif
  }

  if (badge->base) {
    rc = ui_badge_base_set_value(badge->base, count, badge->max_count);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  cupertino_badge_calculate_layout(badge);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_badge_get_count(const struct cupertino_badge *badge,
                                     int *out_count) {
  if (!badge || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = badge->count;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_badge_set_text(struct cupertino_badge *badge,
                                    const char *text) {
  ui_error_t rc;

  if (!badge) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (text) {
#if defined(_MSC_VER)
    strncpy_s(badge->text, sizeof(badge->text), text, _TRUNCATE);
#else
    strncpy(badge->text, text, sizeof(badge->text) - 1);
    badge->text[sizeof(badge->text) - 1] = '\0';
#endif
  } else {
    badge->text[0] = '\0';
  }

  if (badge->base) {
    rc = ui_badge_base_set_text(badge->base, badge->text);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  cupertino_badge_calculate_layout(badge);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_badge_get_text(const struct cupertino_badge *badge,
                                    const char **out_text) {
  if (!badge || !out_text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_text = badge->text;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_badge_set_hidden(struct cupertino_badge *badge,
                                      int is_hidden) {
  ui_error_t rc;

  if (!badge) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  badge->is_hidden = is_hidden ? 1 : 0;
  if (badge->base) {
    rc = ui_badge_base_set_hidden(badge->base, badge->is_hidden);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_badge_is_hidden(const struct cupertino_badge *badge,
                                     int *out_hidden) {
  if (!badge || !out_hidden) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_hidden = badge->is_hidden;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_badge_get_dimensions(const struct cupertino_badge *badge,
                                          float *out_width, float *out_height) {
  if (!badge || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_width = badge->width;
  *out_height = badge->height;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_badge_get_colors(const struct cupertino_badge *badge,
                                      ui_color_t *out_bg, ui_color_t *out_fg) {
  if (!badge || !out_bg || !out_fg) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_bg = badge->background_color;
  *out_fg = badge->text_color;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_badge_get_base(struct cupertino_badge *badge,
                                    struct ui_badge_base **out_base) {
  if (!badge || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = badge->base;
  return UI_ERROR_NONE;
}
