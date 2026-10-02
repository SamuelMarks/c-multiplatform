/**
 * @file cupertino_spell_check_toolbar.c
 * @brief iOS Spell Check Suggestions Popover Toolbar implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_spell_check_toolbar.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_spell_check_mock_recompute_fail = 0;
#endif

#define CUPERTINO_SPELL_CHECK_ARROW_HEIGHT 8.0f

ui_error_t cupertino_spell_check_recompute_bounds(
    struct cupertino_spell_check_toolbar *toolbar) {
  size_t i;
  float content_w;
  float total_w;

  if (!toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#ifdef UI_TEST_MOCK_ALLOC
  if (g_cupertino_spell_check_mock_recompute_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
#endif

  content_w = 0.0f;
  for (i = 0; i < toolbar->suggestion_count; i++) {
    size_t len = strlen(toolbar->suggestions[i]);
    content_w += ((float)len * 8.0f) + 16.0f; /* 8pt/char + 16pt cell padding */
  }

  if (toolbar->allow_add_to_dictionary) {
    content_w += 130.0f; /* "Add to Dictionary" button width */
  }

  total_w = content_w + 16.0f; /* Left and right margin */
  if (total_w < 160.0f) {
    total_w = 160.0f;
  }

  toolbar->bubble_w = total_w;
  toolbar->bubble_h =
      CUPERTINO_SPELL_CHECK_TOOLBAR_HEIGHT + CUPERTINO_SPELL_CHECK_ARROW_HEIGHT;

  /* Position centered above target word */
  toolbar->bubble_x = toolbar->target_x + (toolbar->target_w - total_w) * 0.5f;
  if (toolbar->target_y >= toolbar->bubble_h + 12.0f) {
    toolbar->bubble_y = toolbar->target_y - toolbar->bubble_h - 6.0f;
    toolbar->arrow_on_bottom = 1;
  } else {
    /* Below target */
    toolbar->bubble_y = toolbar->target_y + toolbar->target_h + 6.0f;
    toolbar->arrow_on_bottom = 0;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_spell_check_toolbar_create(
    struct ui_engine *engine,
    const struct cupertino_spell_check_toolbar_descriptor *desc,
    struct cupertino_spell_check_toolbar **out_toolbar) {
  struct cupertino_spell_check_toolbar *tb;

  if (!engine || !desc || !out_toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tb = (struct cupertino_spell_check_toolbar *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_spell_check_toolbar));
  if (!tb) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(tb, 0, sizeof(*tb));
  tb->allow_add_to_dictionary = desc->allow_add_to_dictionary ? 1 : 0;
  tb->is_visible = 0;

  *out_toolbar = tb;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_spell_check_toolbar_destroy(
    struct cupertino_spell_check_toolbar *toolbar) {
  if (!toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(toolbar);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_spell_check_toolbar_show(
    struct cupertino_spell_check_toolbar *toolbar, float target_x,
    float target_y, float target_w, float target_h, const char *misspelled_word,
    const char **suggestions, size_t suggestion_count) {
  size_t i;
  size_t count;
  ui_error_t rc;

  if (!toolbar || !misspelled_word || (!suggestions && suggestion_count > 0)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toolbar->target_x = target_x;
  toolbar->target_y = target_y;
  toolbar->target_w = target_w;
  toolbar->target_h = target_h;

#if defined(_MSC_VER)
  strncpy_s(toolbar->misspelled_word, sizeof(toolbar->misspelled_word),
            misspelled_word, _TRUNCATE);
#else
  strncpy(toolbar->misspelled_word, misspelled_word,
          sizeof(toolbar->misspelled_word) - 1);
  toolbar->misspelled_word[sizeof(toolbar->misspelled_word) - 1] = '\0';
#endif

  count = (suggestion_count > CUPERTINO_SPELL_CHECK_MAX_SUGGESTIONS)
              ? CUPERTINO_SPELL_CHECK_MAX_SUGGESTIONS
              : suggestion_count;
  toolbar->suggestion_count = count;

  for (i = 0; i < count; i++) {
    if (suggestions[i]) {
#if defined(_MSC_VER)
      strncpy_s(toolbar->suggestions[i], sizeof(toolbar->suggestions[i]),
                suggestions[i], _TRUNCATE);
#else
      strncpy(toolbar->suggestions[i], suggestions[i],
              sizeof(toolbar->suggestions[i]) - 1);
      toolbar->suggestions[i][sizeof(toolbar->suggestions[i]) - 1] = '\0';
#endif
    } else {
      toolbar->suggestions[i][0] = '\0';
    }
  }

  rc = cupertino_spell_check_recompute_bounds(toolbar);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  toolbar->is_visible = 1;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_spell_check_toolbar_hide(
    struct cupertino_spell_check_toolbar *toolbar) {
  if (!toolbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toolbar->is_visible = 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_spell_check_toolbar_is_visible(
    const struct cupertino_spell_check_toolbar *toolbar, int *out_visible) {
  if (!toolbar || !out_visible) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_visible = toolbar->is_visible;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_spell_check_toolbar_get_suggestion_count(
    const struct cupertino_spell_check_toolbar *toolbar, size_t *out_count) {
  if (!toolbar || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = toolbar->suggestion_count;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_spell_check_toolbar_select_suggestion(
    struct cupertino_spell_check_toolbar *toolbar, size_t index,
    const char **out_replacement) {
  if (!toolbar || !out_replacement || index >= toolbar->suggestion_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_replacement = toolbar->suggestions[index];
  toolbar->is_visible = 0; /* Auto dismiss on selection */

  return UI_ERROR_NONE;
}

ui_error_t cupertino_spell_check_toolbar_add_to_dictionary(
    struct cupertino_spell_check_toolbar *toolbar, int *out_added) {
  if (!toolbar || !out_added) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (toolbar->allow_add_to_dictionary) {
    *out_added = 1;
    toolbar->is_visible = 0; /* Auto dismiss on add */
  } else {
    *out_added = 0;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_spell_check_toolbar_get_misspelled_word(
    const struct cupertino_spell_check_toolbar *toolbar,
    const char **out_word) {
  if (!toolbar || !out_word) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_word = toolbar->misspelled_word;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_spell_check_toolbar_get_bounds(
    const struct cupertino_spell_check_toolbar *toolbar, float *out_x,
    float *out_y, float *out_w, float *out_h) {
  if (!toolbar || !out_x || !out_y || !out_w || !out_h) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_x = toolbar->bubble_x;
  *out_y = toolbar->bubble_y;
  *out_w = toolbar->bubble_w;
  *out_h = toolbar->bubble_h;

  return UI_ERROR_NONE;
}
