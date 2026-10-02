/**
 * @file cupertino_content_unavailable.c
 * @brief Cupertino Content Unavailable View (UIContentUnavailableView)
 * implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_content_unavailable.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_content_unavailable_mock_create_fail = 0;
int g_content_unavailable_mock_set_title_fail = 0;
int g_content_unavailable_mock_set_desc_fail = 0;
int g_content_unavailable_mock_destroy_fail = 0;

static ui_error_t
mock_empty_state_base_create(struct ui_empty_state_base **out_state) {
  if (g_content_unavailable_mock_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  return ui_empty_state_base_create(out_state);
}
#undef ui_empty_state_base_create
/** @cond */
#define ui_empty_state_base_create mock_empty_state_base_create
/** @endcond */

static ui_error_t
mock_empty_state_base_set_title(struct ui_empty_state_base *state,
                                const char *text) {
  if (g_content_unavailable_mock_set_title_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_empty_state_base_set_title(state, text);
}
#undef ui_empty_state_base_set_title
/** @cond */
#define ui_empty_state_base_set_title mock_empty_state_base_set_title
/** @endcond */

static ui_error_t
mock_empty_state_base_set_description(struct ui_empty_state_base *state,
                                      const char *text) {
  if (g_content_unavailable_mock_set_desc_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_empty_state_base_set_description(state, text);
}
#undef ui_empty_state_base_set_description
/** @cond */
#define ui_empty_state_base_set_description                                    \
  mock_empty_state_base_set_description
/** @endcond */

static ui_error_t mock_ui_component_destroy(struct ui_component *comp) {
  if (g_content_unavailable_mock_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_component_destroy(comp);
}
#undef ui_component_destroy
/** @cond */
#define ui_component_destroy mock_ui_component_destroy
/** @endcond */
#endif

ui_error_t cupertino_content_unavailable_create(
    struct ui_engine *engine,
    const struct cupertino_content_unavailable_descriptor *desc,
    struct cupertino_content_unavailable_view **out_view) {
  struct cupertino_content_unavailable_view *view;
  ui_error_t rc;

  if (!engine || !desc || !out_view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if ((int)desc->style < 0 || (int)desc->style > 1) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  view = (struct cupertino_content_unavailable_view *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_content_unavailable_view));
  if (!view) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(view, 0, sizeof(*view));
  view->style = desc->style;
  view->width = CUPERTINO_CONTENT_UNAVAILABLE_DEFAULT_WIDTH;
  view->height = CUPERTINO_CONTENT_UNAVAILABLE_DEFAULT_HEIGHT;

  if (desc->symbol_name) {
#if defined(_MSC_VER)
    strncpy_s(view->symbol_name, sizeof(view->symbol_name), desc->symbol_name,
              _TRUNCATE);
#else
    strncpy(view->symbol_name, desc->symbol_name,
            sizeof(view->symbol_name) - 1);
    view->symbol_name[sizeof(view->symbol_name) - 1] = '\0';
#endif
  } else {
    if (desc->style == CUPERTINO_CONTENT_UNAVAILABLE_SEARCH) {
#if defined(_MSC_VER)
      strcpy_s(view->symbol_name, sizeof(view->symbol_name), "magnifyingglass");
#else
      strcpy(view->symbol_name, "magnifyingglass");
#endif
    } else {
#if defined(_MSC_VER)
      strcpy_s(view->symbol_name, sizeof(view->symbol_name), "tray");
#else
      strcpy(view->symbol_name, "tray");
#endif
    }
  }

  if (desc->search_query) {
#if defined(_MSC_VER)
    strncpy_s(view->search_query, sizeof(view->search_query),
              desc->search_query, _TRUNCATE);
#else
    strncpy(view->search_query, desc->search_query,
            sizeof(view->search_query) - 1);
    view->search_query[sizeof(view->search_query) - 1] = '\0';
#endif
  }

  if (desc->title) {
#if defined(_MSC_VER)
    strncpy_s(view->title, sizeof(view->title), desc->title, _TRUNCATE);
#else
    strncpy(view->title, desc->title, sizeof(view->title) - 1);
    view->title[sizeof(view->title) - 1] = '\0';
#endif
  } else if (desc->style == CUPERTINO_CONTENT_UNAVAILABLE_SEARCH) {
#if defined(_MSC_VER)
    sprintf_s(view->title, sizeof(view->title), "No Results for '%s'",
              view->search_query);
#else
    sprintf(view->title, "No Results for '%s'", view->search_query);
#endif
  } else {
#if defined(_MSC_VER)
    strcpy_s(view->title, sizeof(view->title), "No Content");
#else
    strcpy(view->title, "No Content");
#endif
  }

  if (desc->description_text) {
#if defined(_MSC_VER)
    strncpy_s(view->description_text, sizeof(view->description_text),
              desc->description_text, _TRUNCATE);
#else
    strncpy(view->description_text, desc->description_text,
            sizeof(view->description_text) - 1);
    view->description_text[sizeof(view->description_text) - 1] = '\0';
#endif
  } else {
#if defined(_MSC_VER)
    strcpy_s(view->description_text, sizeof(view->description_text),
             "Check again later.");
#else
    strcpy(view->description_text, "Check again later.");
#endif
  }

  if (desc->action_title) {
#if defined(_MSC_VER)
    strncpy_s(view->action_title, sizeof(view->action_title),
              desc->action_title, _TRUNCATE);
#else
    strncpy(view->action_title, desc->action_title,
            sizeof(view->action_title) - 1);
    view->action_title[sizeof(view->action_title) - 1] = '\0';
#endif
  }

  rc = ui_empty_state_base_create(&view->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(view);
    return rc;
  }

  rc = ui_empty_state_base_set_title(view->base, view->title);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_component_destroy((struct ui_component *)view->base);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    C_MULTIPLATFORM_FREE(view);
    return rc;
  }

  rc = ui_empty_state_base_set_description(view->base, view->description_text);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_component_destroy((struct ui_component *)view->base);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    C_MULTIPLATFORM_FREE(view);
    return rc;
  }

  *out_view = view;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_content_unavailable_destroy(
    struct cupertino_content_unavailable_view *view) {
  ui_error_t rc;

  if (!view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (view->base) {
    rc = ui_component_destroy((struct ui_component *)view->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    view->base = NULL;
  }

  C_MULTIPLATFORM_FREE(view);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_content_unavailable_set_title(
    struct cupertino_content_unavailable_view *view, const char *title) {
  ui_error_t rc;

  if (!view || !title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(view->title, sizeof(view->title), title, _TRUNCATE);
#else
  strncpy(view->title, title, sizeof(view->title) - 1);
  view->title[sizeof(view->title) - 1] = '\0';
#endif

  if (view->base) {
    rc = ui_empty_state_base_set_title(view->base, view->title);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_content_unavailable_get_title(
    const struct cupertino_content_unavailable_view *view,
    const char **out_title) {
  if (!view || !out_title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_title = view->title;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_content_unavailable_set_description(
    struct cupertino_content_unavailable_view *view,
    const char *description_text) {
  ui_error_t rc;

  if (!view || !description_text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(view->description_text, sizeof(view->description_text),
            description_text, _TRUNCATE);
#else
  strncpy(view->description_text, description_text,
          sizeof(view->description_text) - 1);
  view->description_text[sizeof(view->description_text) - 1] = '\0';
#endif

  if (view->base) {
    rc =
        ui_empty_state_base_set_description(view->base, view->description_text);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_content_unavailable_get_description(
    const struct cupertino_content_unavailable_view *view,
    const char **out_description) {
  if (!view || !out_description) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_description = view->description_text;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_content_unavailable_set_search_query(
    struct cupertino_content_unavailable_view *view, const char *search_query) {
  if (!view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (search_query) {
#if defined(_MSC_VER)
    strncpy_s(view->search_query, sizeof(view->search_query), search_query,
              _TRUNCATE);
#else
    strncpy(view->search_query, search_query, sizeof(view->search_query) - 1);
    view->search_query[sizeof(view->search_query) - 1] = '\0';
#endif
  } else {
    view->search_query[0] = '\0';
  }

  if (view->style == CUPERTINO_CONTENT_UNAVAILABLE_SEARCH) {
#if defined(_MSC_VER)
    sprintf_s(view->title, sizeof(view->title), "No Results for '%s'",
              view->search_query);
#else
    sprintf(view->title, "No Results for '%s'", view->search_query);
#endif
    if (view->base) {
      return ui_empty_state_base_set_title(view->base, view->title);
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_content_unavailable_trigger_action(
    struct cupertino_content_unavailable_view *view) {
  if (!view) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  view->action_triggered_count++;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_content_unavailable_get_dimensions(
    const struct cupertino_content_unavailable_view *view, float *out_width,
    float *out_height) {
  if (!view || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_width = view->width;
  *out_height = view->height;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_content_unavailable_get_base(
    struct cupertino_content_unavailable_view *view,
    struct ui_empty_state_base **out_base) {
  if (!view || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = view->base;
  return UI_ERROR_NONE;
}
