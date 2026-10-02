/**
 * @file cupertino_nav_bar.c
 * @brief Cupertino Navigation Bar component implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_nav_bar.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_nav_bar_mock_base_create_fail = 0;
int g_cupertino_nav_bar_mock_base_destroy_fail = 0;
int g_cupertino_nav_bar_mock_base_handle_scroll_fail = 0;
int g_cupertino_nav_bar_mock_arena_destroy_fail = 0;

static ui_error_t
mock_ui_top_app_bar_base_create(struct ui_arena *arena,
                                const struct ui_top_app_bar_config *config,
                                struct ui_top_app_bar_base **out_base) {
  if (g_cupertino_nav_bar_mock_base_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  return ui_top_app_bar_base_create(arena, config, out_base);
}
#undef ui_top_app_bar_base_create
/** @cond */
#define ui_top_app_bar_base_create mock_ui_top_app_bar_base_create
/** @endcond */

static ui_error_t
mock_ui_top_app_bar_base_destroy(struct ui_top_app_bar_base *base) {
  if (g_cupertino_nav_bar_mock_base_destroy_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_top_app_bar_base_destroy(base);
}
#undef ui_top_app_bar_base_destroy
/** @cond */
#define ui_top_app_bar_base_destroy mock_ui_top_app_bar_base_destroy
/** @endcond */

static ui_error_t
mock_ui_top_app_bar_base_handle_scroll(struct ui_top_app_bar_base *base,
                                       float scroll_y, float delta_y) {
  if (g_cupertino_nav_bar_mock_base_handle_scroll_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_top_app_bar_base_handle_scroll(base, scroll_y, delta_y);
}
#undef ui_top_app_bar_base_handle_scroll
/** @cond */
#define ui_top_app_bar_base_handle_scroll mock_ui_top_app_bar_base_handle_scroll
/** @endcond */

static ui_error_t mock_ui_arena_destroy(struct ui_arena *arena) {
  if (g_cupertino_nav_bar_mock_arena_destroy_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_arena_destroy(arena);
}
#undef ui_arena_destroy
/** @cond */
#define ui_arena_destroy mock_ui_arena_destroy
/** @endcond */

#endif /* UI_TEST_MOCK_ALLOC */

#define CUPERTINO_NAV_BAR_COLLAPSED_HEIGHT 44.0f
#define CUPERTINO_NAV_BAR_LARGE_TITLE_EXTRA 52.0f
#define CUPERTINO_NAV_BAR_PROMPT_HEIGHT 24.0f

static float
cupertino_nav_bar_compute_base_collapsed(const struct cupertino_nav_bar *bar) {
  float h;
  h = CUPERTINO_NAV_BAR_COLLAPSED_HEIGHT;
  if (bar->prompt[0] != '\0') {
    h += CUPERTINO_NAV_BAR_PROMPT_HEIGHT;
  }
  if (bar->has_search_controller && !bar->hides_search_when_scrolling) {
    h += 52.0f;
  }
  return h;
}

static float
cupertino_nav_bar_compute_base_expanded(const struct cupertino_nav_bar *bar) {
  float h;
  h = cupertino_nav_bar_compute_base_collapsed(bar);
  if (bar->is_large_title_enabled) {
    h += CUPERTINO_NAV_BAR_LARGE_TITLE_EXTRA;
  }
  if (bar->has_search_controller && bar->hides_search_when_scrolling) {
    h += 52.0f;
  }
  return h;
}

static void cupertino_nav_bar_update_metrics(struct cupertino_nav_bar *bar) {
  float collapsed;
  float expanded;
  float scroll;

  if (bar->has_search_controller) {
    if (!bar->hides_search_when_scrolling || bar->scroll_offset <= 0.0f) {
      bar->search_collapse_progress = 0.0f;
      bar->search_bar_opacity = 1.0f;
    } else if (bar->scroll_offset < 52.0f) {
      bar->search_collapse_progress = bar->scroll_offset / 52.0f;
      bar->search_bar_opacity = 1.0f - bar->search_collapse_progress;
    } else {
      bar->search_collapse_progress = 1.0f;
      bar->search_bar_opacity = 0.0f;
    }
  } else {
    bar->search_collapse_progress = 0.0f;
    bar->search_bar_opacity = 0.0f;
  }

  collapsed = cupertino_nav_bar_compute_base_collapsed(bar);
  expanded = cupertino_nav_bar_compute_base_expanded(bar);
  scroll = bar->scroll_offset;

  if (!bar->is_large_title_enabled) {
    bar->collapse_progress = 1.0f;
    bar->current_height = collapsed;
    if (bar->has_search_controller && bar->hides_search_when_scrolling) {
      bar->current_height += (1.0f - bar->search_collapse_progress) * 52.0f;
    }
    bar->inline_title_opacity = 1.0f;
    bar->large_title_opacity = 0.0f;
    return;
  }

  if (scroll <= 0.0f) {
    /* Rubber-band overscroll pull-down expands title region */
    bar->collapse_progress = 0.0f;
    bar->current_height = expanded - scroll;
    bar->inline_title_opacity = 0.0f;
    bar->large_title_opacity = 1.0f;
  } else if (scroll < CUPERTINO_NAV_BAR_LARGE_TITLE_EXTRA) {
    float p;
    p = scroll / CUPERTINO_NAV_BAR_LARGE_TITLE_EXTRA;
    bar->collapse_progress = p;
    bar->current_height = expanded - scroll;
    bar->large_title_opacity = 1.0f - p;
    if (p > 0.6f) {
      bar->inline_title_opacity = (p - 0.6f) / 0.4f;
    } else {
      bar->inline_title_opacity = 0.0f;
    }
  } else {
    bar->collapse_progress = 1.0f;
    bar->current_height = collapsed;
    bar->inline_title_opacity = 1.0f;
    bar->large_title_opacity = 0.0f;
  }
}

ui_error_t
cupertino_nav_bar_create(struct ui_engine *engine,
                         const struct cupertino_nav_bar_descriptor *desc,
                         struct cupertino_nav_bar **out_bar) {
  struct cupertino_nav_bar *bar;
  struct ui_top_app_bar_config config;
  ui_error_t rc;

  if (!engine || !desc || !out_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar = (struct cupertino_nav_bar *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_nav_bar));
  if (!bar) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(bar, 0, sizeof(*bar));
  bar->is_large_title_enabled = desc->is_large_title_enabled ? 1 : 0;

  if (desc->title) {
#if defined(_MSC_VER)
    strncpy_s(bar->title, sizeof(bar->title), desc->title,
              sizeof(bar->title) - 1);
#else
    strncpy(bar->title, desc->title, sizeof(bar->title) - 1);
    bar->title[sizeof(bar->title) - 1] = '\0';
#endif
  }

  if (desc->prompt) {
#if defined(_MSC_VER)
    strncpy_s(bar->prompt, sizeof(bar->prompt), desc->prompt,
              sizeof(bar->prompt) - 1);
#else
    strncpy(bar->prompt, desc->prompt, sizeof(bar->prompt) - 1);
    bar->prompt[sizeof(bar->prompt) - 1] = '\0';
#endif
  }

  if (desc->previous_title) {
#if defined(_MSC_VER)
    strncpy_s(bar->previous_title, sizeof(bar->previous_title),
              desc->previous_title, sizeof(bar->previous_title) - 1);
#else
    strncpy(bar->previous_title, desc->previous_title,
            sizeof(bar->previous_title) - 1);
    bar->previous_title[sizeof(bar->previous_title) - 1] = '\0';
#endif
  }

  rc = ui_arena_create(4096, &bar->arena);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(bar);
    return rc;
  }

  memset(&config, 0, sizeof(config));
  config.initial_state = bar->is_large_title_enabled
                             ? UI_TOP_APP_BAR_STATE_EXPANDED
                             : UI_TOP_APP_BAR_STATE_COLLAPSED;
  config.expanded_height = cupertino_nav_bar_compute_base_expanded(bar);
  config.collapsed_height = cupertino_nav_bar_compute_base_collapsed(bar);
  config.scroll_threshold = CUPERTINO_NAV_BAR_LARGE_TITLE_EXTRA;

  rc = ui_top_app_bar_base_create(bar->arena, &config, &bar->base);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_arena_destroy(bar->arena);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    C_MULTIPLATFORM_FREE(bar);
    return rc;
  }

  cupertino_nav_bar_update_metrics(bar);

  *out_bar = bar;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_nav_bar_destroy(struct cupertino_nav_bar *bar) {
  ui_error_t rc;

  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (bar->base) {
    rc = ui_top_app_bar_base_destroy(bar->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    bar->base = NULL;
  }

  if (bar->arena) {
    rc = ui_arena_destroy(bar->arena);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    bar->arena = NULL;
  }

  C_MULTIPLATFORM_FREE(bar);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_nav_bar_set_title(struct cupertino_nav_bar *bar,
                                       const char *title) {
  if (!bar || !title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(bar->title, sizeof(bar->title), title, sizeof(bar->title) - 1);
#else
  strncpy(bar->title, title, sizeof(bar->title) - 1);
  bar->title[sizeof(bar->title) - 1] = '\0';
#endif

  return UI_ERROR_NONE;
}

ui_error_t cupertino_nav_bar_get_title(const struct cupertino_nav_bar *bar,
                                       const char **out_title) {
  if (!bar || !out_title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_title = bar->title;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_nav_bar_set_prompt(struct cupertino_nav_bar *bar,
                                        const char *prompt) {
  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (prompt) {
#if defined(_MSC_VER)
    strncpy_s(bar->prompt, sizeof(bar->prompt), prompt,
              sizeof(bar->prompt) - 1);
#else
    strncpy(bar->prompt, prompt, sizeof(bar->prompt) - 1);
    bar->prompt[sizeof(bar->prompt) - 1] = '\0';
#endif
  } else {
    bar->prompt[0] = '\0';
  }

  cupertino_nav_bar_update_metrics(bar);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_nav_bar_get_prompt(const struct cupertino_nav_bar *bar,
                                        const char **out_prompt) {
  if (!bar || !out_prompt) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_prompt = (bar->prompt[0] != '\0') ? bar->prompt : NULL;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_nav_bar_set_previous_title(struct cupertino_nav_bar *bar,
                                                const char *prev_title) {
  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (prev_title) {
#if defined(_MSC_VER)
    strncpy_s(bar->previous_title, sizeof(bar->previous_title), prev_title,
              sizeof(bar->previous_title) - 1);
#else
    strncpy(bar->previous_title, prev_title, sizeof(bar->previous_title) - 1);
    bar->previous_title[sizeof(bar->previous_title) - 1] = '\0';
#endif
  } else {
    bar->previous_title[0] = '\0';
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_nav_bar_get_previous_title(const struct cupertino_nav_bar *bar,
                                     const char **out_prev_title) {
  if (!bar || !out_prev_title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_prev_title =
      (bar->previous_title[0] != '\0') ? bar->previous_title : NULL;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_nav_bar_set_large_title_enabled(struct cupertino_nav_bar *bar,
                                          int enabled) {
  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar->is_large_title_enabled = enabled ? 1 : 0;
  cupertino_nav_bar_update_metrics(bar);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_nav_bar_is_large_title_enabled(const struct cupertino_nav_bar *bar,
                                         int *out_enabled) {
  if (!bar || !out_enabled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_enabled = bar->is_large_title_enabled;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_nav_bar_handle_scroll(struct cupertino_nav_bar *bar,
                                           float scroll_y, float delta_y) {
  ui_error_t rc;

  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar->scroll_offset = scroll_y;
  cupertino_nav_bar_update_metrics(bar);

  if (bar->base) {
    rc = ui_top_app_bar_base_handle_scroll(bar->base, scroll_y, delta_y);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_nav_bar_get_height(const struct cupertino_nav_bar *bar,
                                        float *out_height) {
  if (!bar || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_height = bar->current_height;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_nav_bar_get_collapse_progress(const struct cupertino_nav_bar *bar,
                                        float *out_progress) {
  if (!bar || !out_progress) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_progress = bar->collapse_progress;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_nav_bar_get_effective_back_title(const struct cupertino_nav_bar *bar,
                                           float available_width, char *out_buf,
                                           size_t buf_len) {
  size_t title_len;
  float needed_width;

  if (!bar || !out_buf || buf_len == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_buf[0] = '\0';

  /* If width is too constrained for text, show chevron icon only */
  if (available_width < 40.0f) {
    return UI_ERROR_NONE;
  }

  if (bar->previous_title[0] != '\0') {
    title_len = strlen(bar->previous_title);
    /* 9pt per character estimate + 20pt chevron padding */
    needed_width = ((float)title_len * 9.0f) + 20.0f;
    if (needed_width <= available_width) {
#if defined(_MSC_VER)
      strncpy_s(out_buf, buf_len, bar->previous_title, buf_len - 1);
#else
      strncpy(out_buf, bar->previous_title, buf_len - 1);
      out_buf[buf_len - 1] = '\0';
#endif
      return UI_ERROR_NONE;
    }
  }

  /* Fallback to standard "Back" if space permits */
  if (available_width >= 60.0f) {
#if defined(_MSC_VER)
    strncpy_s(out_buf, buf_len, "Back", buf_len - 1);
#else
    strncpy(out_buf, "Back", buf_len - 1);
    out_buf[buf_len - 1] = '\0';
#endif
    return UI_ERROR_NONE;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_nav_bar_get_base(struct cupertino_nav_bar *bar,
                                      struct ui_top_app_bar_base **out_base) {
  if (!bar || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = bar->base;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_nav_bar_set_search_controller(struct cupertino_nav_bar *bar,
                                        struct cupertino_search_bar *search_bar,
                                        int hides_when_scrolling) {
  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar->search_bar = search_bar;
  bar->has_search_controller = search_bar ? 1 : 0;
  bar->hides_search_when_scrolling = hides_when_scrolling ? 1 : 0;
  cupertino_nav_bar_update_metrics(bar);

  return UI_ERROR_NONE;
}

ui_error_t cupertino_nav_bar_get_search_controller(
    const struct cupertino_nav_bar *bar,
    struct cupertino_search_bar **out_search_bar,
    int *out_hides_when_scrolling) {
  if (!bar || !out_search_bar || !out_hides_when_scrolling) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_search_bar = bar->search_bar;
  *out_hides_when_scrolling = bar->hides_search_when_scrolling;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_nav_bar_get_search_collapse_progress(
    const struct cupertino_nav_bar *bar, float *out_progress,
    float *out_opacity) {
  if (!bar || !out_progress || !out_opacity) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_progress = bar->search_collapse_progress;
  *out_opacity = bar->search_bar_opacity;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_nav_bar_compute_title_scale(const struct cupertino_nav_bar *bar,
                                      float available_width,
                                      float *out_scale_factor) {
  size_t len;
  float measured_w;
  float scale;

  if (!bar || available_width <= 0.0f || !out_scale_factor) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  len = strlen(bar->title);
  if (len == 0) {
    *out_scale_factor = 1.0f;
    return UI_ERROR_NONE;
  }

  /* SF Pro Display Large Title estimate ~20pt per glyph */
  measured_w = (float)len * 20.0f;
  if (measured_w > available_width) {
    scale = available_width / measured_w;
    if (scale < 0.7f) {
      scale = 0.7f; /* Minimum scale factor before truncation */
    }
  } else {
    scale = 1.0f;
  }

  *out_scale_factor = scale;
  return UI_ERROR_NONE;
}
