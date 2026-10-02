/**
 * @file cupertino_search_bar.c
 * @brief Cupertino Search Bar component implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_search_bar.h"
#include "ui_component.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_search_bar_mock_init_fail = 0;
int g_cupertino_search_bar_mock_cleanup_fail = 0;
int g_cupertino_search_bar_mock_set_query_fail = 0;

static ui_error_t
mock_search_bar_base_init(struct ui_search_bar_base *base,
                          struct ui_component *comp,
                          struct ui_control_value_accessor *cva) {
  if (g_cupertino_search_bar_mock_init_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_search_bar_base_init(base, comp, cva);
}
#undef ui_search_bar_base_init
/** @cond */
#define ui_search_bar_base_init mock_search_bar_base_init
/** @endcond */

static ui_error_t
mock_search_bar_base_cleanup(struct ui_search_bar_base *base) {
  if (g_cupertino_search_bar_mock_cleanup_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_search_bar_base_cleanup(base);
}
#undef ui_search_bar_base_cleanup
/** @cond */
#define ui_search_bar_base_cleanup mock_search_bar_base_cleanup
/** @endcond */

static ui_error_t
mock_search_bar_base_set_query(struct ui_search_bar_base *base,
                               const char *query) {
  if (g_cupertino_search_bar_mock_set_query_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_search_bar_base_set_query(base, query);
}
#undef ui_search_bar_base_set_query
/** @cond */
#define ui_search_bar_base_set_query mock_search_bar_base_set_query
/** @endcond */
#endif

static ui_error_t
cupertino_search_bar_cva_write_value(void *component,
                                     union ui_signal_payload value) {
  struct cupertino_search_bar *bar;
  const char *str;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar = (struct cupertino_search_bar *)component;
  str = (const char *)value.ptr_val;
  if (str) {
    return cupertino_search_bar_set_query(bar, str);
  }
  return cupertino_search_bar_clear(bar);
}

static ui_error_t cupertino_search_bar_cva_set_disabled_state(void *component,
                                                              int is_disabled) {
  struct cupertino_search_bar *bar;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar = (struct cupertino_search_bar *)component;
  bar->base.is_disabled = is_disabled ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_search_bar_create(struct ui_engine *engine,
                            const struct cupertino_search_bar_descriptor *desc,
                            struct cupertino_search_bar **out_search_bar) {
  struct cupertino_search_bar *bar;
  ui_error_t rc;
  size_t i;

  if (!engine || !desc || !out_search_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (desc->scope_count > CUPERTINO_SEARCH_BAR_MAX_SCOPES) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar = (struct cupertino_search_bar *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_search_bar));
  if (!bar) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(bar, 0, sizeof(*bar));

  bar->component = (struct ui_component *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_component));
  if (!bar->component) {
    C_MULTIPLATFORM_FREE(bar);
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(bar->component, 0, sizeof(*bar->component));

  rc = ui_search_bar_base_init(&bar->base, bar->component, &bar->cva);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(bar->component);
    C_MULTIPLATFORM_FREE(bar);
    return rc;
  }

  bar->cva.write_value = cupertino_search_bar_cva_write_value;
  bar->cva.set_disabled_state = cupertino_search_bar_cva_set_disabled_state;

  if (desc->placeholder) {
#if defined(_MSC_VER)
    strncpy_s(bar->placeholder, sizeof(bar->placeholder), desc->placeholder,
              sizeof(bar->placeholder) - 1);
#else
    strncpy(bar->placeholder, desc->placeholder, sizeof(bar->placeholder) - 1);
    bar->placeholder[sizeof(bar->placeholder) - 1] = '\0';
#endif
  }

  bar->scope_count = desc->scope_count;
  bar->selected_scope_index = desc->initial_scope_index;

  for (i = 0; i < desc->scope_count; i++) {
    if (desc->scope_titles && desc->scope_titles[i]) {
#if defined(_MSC_VER)
      strncpy_s(bar->scopes[i], sizeof(bar->scopes[i]), desc->scope_titles[i],
                sizeof(bar->scopes[i]) - 1);
#else
      strncpy(bar->scopes[i], desc->scope_titles[i],
              sizeof(bar->scopes[i]) - 1);
      bar->scopes[i][sizeof(bar->scopes[i]) - 1] = '\0';
#endif
    }
  }

  *out_search_bar = bar;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_search_bar_destroy(struct cupertino_search_bar *search_bar) {
  ui_error_t rc;

  if (!search_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_search_bar_base_cleanup(&search_bar->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (search_bar->component) {
    C_MULTIPLATFORM_FREE(search_bar->component);
    search_bar->component = NULL;
  }

  C_MULTIPLATFORM_FREE(search_bar);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_search_bar_set_query(struct cupertino_search_bar *search_bar,
                               const char *query) {
  ui_error_t rc;

  if (!search_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (query) {
#if defined(_MSC_VER)
    strncpy_s(search_bar->query_buf, sizeof(search_bar->query_buf), query,
              sizeof(search_bar->query_buf) - 1);
#else
    strncpy(search_bar->query_buf, query, sizeof(search_bar->query_buf) - 1);
    search_bar->query_buf[sizeof(search_bar->query_buf) - 1] = '\0';
#endif
  } else {
    search_bar->query_buf[0] = '\0';
  }

  rc = ui_search_bar_base_set_query(&search_bar->base, search_bar->query_buf);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_search_bar_get_query(const struct cupertino_search_bar *search_bar,
                               const char **out_query) {
  if (!search_bar || !out_query) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_query = search_bar->query_buf;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_search_bar_clear(struct cupertino_search_bar *search_bar) {
  return cupertino_search_bar_set_query(search_bar, "");
}

ui_error_t
cupertino_search_bar_set_focused(struct cupertino_search_bar *search_bar,
                                 int focused) {
  if (!search_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  search_bar->is_focused = focused ? 1 : 0;
  search_bar->cancel_button_progress = focused ? 1.0f : 0.0f;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_search_bar_is_focused(const struct cupertino_search_bar *search_bar,
                                int *out_focused) {
  if (!search_bar || !out_focused) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_focused = search_bar->is_focused;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_search_bar_get_cancel_progress(
    const struct cupertino_search_bar *search_bar, float *out_progress) {
  if (!search_bar || !out_progress) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_progress = search_bar->cancel_button_progress;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_search_bar_set_scope_selected(struct cupertino_search_bar *search_bar,
                                        size_t index) {
  if (!search_bar || index >= search_bar->scope_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  search_bar->selected_scope_index = index;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_search_bar_get_scope_selected(
    const struct cupertino_search_bar *search_bar, size_t *out_index) {
  if (!search_bar || !out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_index = search_bar->selected_scope_index;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_search_bar_add_token(struct cupertino_search_bar *search_bar,
                               const char *token, size_t *out_index) {
  size_t idx;

  if (!search_bar || !token) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (search_bar->token_count >= CUPERTINO_SEARCH_BAR_MAX_TOKENS) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  idx = search_bar->token_count;
#if defined(_MSC_VER)
  strncpy_s(search_bar->tokens[idx], sizeof(search_bar->tokens[idx]), token,
            sizeof(search_bar->tokens[idx]) - 1);
#else
  strncpy(search_bar->tokens[idx], token, sizeof(search_bar->tokens[idx]) - 1);
  search_bar->tokens[idx][sizeof(search_bar->tokens[idx]) - 1] = '\0';
#endif

  search_bar->token_count++;
  if (out_index) {
    *out_index = idx;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_search_bar_get_token_count(
    const struct cupertino_search_bar *search_bar, size_t *out_count) {
  if (!search_bar || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = search_bar->token_count;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_search_bar_get_cva(struct cupertino_search_bar *search_bar,
                             struct ui_control_value_accessor **out_cva) {
  if (!search_bar || !out_cva) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_cva = &search_bar->cva;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_search_bar_get_base(struct cupertino_search_bar *search_bar,
                              struct ui_search_bar_base **out_base) {
  if (!search_bar || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = &search_bar->base;
  return UI_ERROR_NONE;
}
