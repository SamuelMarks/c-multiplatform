/**
 * @file cupertino_sheet.c
 * @brief Cupertino Bottom Sheet & Action Sheet component implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_sheet.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_sheet_mock_base_destroy_fail = 0;
int g_cupertino_sheet_mock_base_set_open_fail = 0;
static ui_error_t
mock_bottom_sheet_base_destroy(struct ui_bottom_sheet_base *sheet) {
  if (g_cupertino_sheet_mock_base_destroy_fail) {
    (ui_bottom_sheet_base_destroy)(sheet);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_bottom_sheet_base_destroy)(sheet);
}
static ui_error_t
mock_bottom_sheet_base_set_open(struct ui_bottom_sheet_base *sheet,
                                int is_open) {
  if (g_cupertino_sheet_mock_base_set_open_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_bottom_sheet_base_set_open)(sheet, is_open);
}
#undef ui_bottom_sheet_base_destroy
/** @cond */
#define ui_bottom_sheet_base_destroy mock_bottom_sheet_base_destroy
/** @endcond */
#undef ui_bottom_sheet_base_set_open
/** @cond */
#define ui_bottom_sheet_base_set_open mock_bottom_sheet_base_set_open
/** @endcond */
#endif

ui_error_t cupertino_sheet_create(struct ui_engine *engine,
                                  const struct cupertino_sheet_descriptor *desc,
                                  struct cupertino_sheet **out_sheet) {
  struct cupertino_sheet *sheet;
  ui_error_t rc;

  if (!engine || !desc || !out_sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sheet = (struct cupertino_sheet *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_sheet));
  if (!sheet) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(sheet, 0, sizeof(*sheet));
  sheet->mode = desc->mode;
  sheet->detent = desc->detent;
  sheet->custom_detent_height = desc->custom_detent_height;
  sheet->is_open = 0;
  sheet->show_drag_grabber = desc->show_drag_grabber;
  sheet->action_count = 0;
  sheet->has_cancel_action = 0;

  if (desc->title) {
#if defined(_MSC_VER)
    strncpy_s(sheet->title, sizeof(sheet->title), desc->title, _TRUNCATE);
#else
    strncpy(sheet->title, desc->title, sizeof(sheet->title) - 1);
    sheet->title[sizeof(sheet->title) - 1] = '\0';
#endif
  }

  if (desc->message) {
#if defined(_MSC_VER)
    strncpy_s(sheet->message, sizeof(sheet->message), desc->message, _TRUNCATE);
#else
    strncpy(sheet->message, desc->message, sizeof(sheet->message) - 1);
    sheet->message[sizeof(sheet->message) - 1] = '\0';
#endif
  }

  sheet->largest_undimmed_detent = CUPERTINO_SHEET_DETENT_MEDIUM;
  sheet->passthrough_enabled = 0;

  rc = ui_bottom_sheet_base_create(&sheet->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(sheet);
    return rc;
  }

  *out_sheet = sheet;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_sheet_destroy(struct cupertino_sheet *sheet) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (sheet->base) {
    rc = ui_bottom_sheet_base_destroy(sheet->base);
    sheet->base = NULL;
  }

  C_MULTIPLATFORM_FREE(sheet);
  return rc;
}

ui_error_t cupertino_sheet_set_open(struct cupertino_sheet *sheet,
                                    int is_open) {
  ui_error_t rc;

  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sheet->is_open = is_open ? 1 : 0;

  if (sheet->base) {
    rc = ui_bottom_sheet_base_set_open(sheet->base, sheet->is_open);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_sheet_is_open(const struct cupertino_sheet *sheet,
                                   int *out_is_open) {
  if (!sheet || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_open = sheet->is_open;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_sheet_set_detent(struct cupertino_sheet *sheet,
                                      enum cupertino_sheet_detent detent) {
  if (!sheet || (int)detent < 0 ||
      (int)detent > CUPERTINO_SHEET_DETENT_CUSTOM) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sheet->detent = detent;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_sheet_get_detent(const struct cupertino_sheet *sheet,
                                      enum cupertino_sheet_detent *out_detent) {
  if (!sheet || !out_detent) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_detent = sheet->detent;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_sheet_set_custom_detent_height(struct cupertino_sheet *sheet,
                                         float height) {
  if (!sheet || height <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sheet->custom_detent_height = height;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_sheet_get_computed_height(const struct cupertino_sheet *sheet,
                                    float viewport_height, float *out_height) {
  if (!sheet || !out_height || viewport_height <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (sheet->mode == CUPERTINO_SHEET_MODE_ACTION_SHEET) {
    float h;
    h = 16.0f; /* Base margins */
    if (sheet->title[0] != '\0' || sheet->message[0] != '\0') {
      h += 60.0f; /* Header text box */
    }
    h += (float)sheet->action_count * 56.0f;
    if (sheet->has_cancel_action) {
      h += 64.0f; /* Cancel pill + 8pt gap */
    }
    *out_height = h;
    return UI_ERROR_NONE;
  }

  if (sheet->detent == CUPERTINO_SHEET_DETENT_MEDIUM) {
    *out_height = viewport_height * 0.50f;
  } else if (sheet->detent == CUPERTINO_SHEET_DETENT_LARGE) {
    *out_height = viewport_height * 0.92f;
  } else {
    *out_height = (sheet->custom_detent_height > 0.0f)
                      ? sheet->custom_detent_height
                      : viewport_height * 0.50f;
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_sheet_get_parent_card_scale(const struct cupertino_sheet *sheet,
                                      float progress, float *out_scale,
                                      float *out_corner_radius) {
  if (!sheet || !out_scale || !out_corner_radius || progress < 0.0f ||
      progress > 1.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Apple HIG Card Sheet: Scales underlying root view down to 0.92,
   * corners interpolate linearly from 0pt to 12pt */
  *out_scale = 1.0f - (0.08f * progress);
  *out_corner_radius = 12.0f * progress;

  return UI_ERROR_NONE;
}

ui_error_t cupertino_sheet_add_action(struct cupertino_sheet *sheet,
                                      const char *title, int is_destructive,
                                      size_t *out_index) {
  size_t idx;

  if (!sheet || !title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (sheet->action_count >= CUPERTINO_SHEET_MAX_ACTIONS) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  idx = sheet->action_count;
  sheet->actions[idx].is_destructive = is_destructive ? 1 : 0;

#if defined(_MSC_VER)
  strncpy_s(sheet->actions[idx].title, sizeof(sheet->actions[idx].title), title,
            _TRUNCATE);
#else
  strncpy(sheet->actions[idx].title, title,
          sizeof(sheet->actions[idx].title) - 1);
  sheet->actions[idx].title[sizeof(sheet->actions[idx].title) - 1] = '\0';
#endif

  sheet->action_count++;
  if (out_index) {
    *out_index = idx;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_sheet_set_cancel_action(struct cupertino_sheet *sheet,
                                             const char *title) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!title) {
    sheet->has_cancel_action = 0;
    sheet->cancel_title[0] = '\0';
    return UI_ERROR_NONE;
  }

  sheet->has_cancel_action = 1;
#if defined(_MSC_VER)
  strncpy_s(sheet->cancel_title, sizeof(sheet->cancel_title), title, _TRUNCATE);
#else
  strncpy(sheet->cancel_title, title, sizeof(sheet->cancel_title) - 1);
  sheet->cancel_title[sizeof(sheet->cancel_title) - 1] = '\0';
#endif

  return UI_ERROR_NONE;
}

ui_error_t cupertino_sheet_get_action_count(const struct cupertino_sheet *sheet,
                                            size_t *out_count) {
  if (!sheet || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = sheet->action_count;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_sheet_get_base(struct cupertino_sheet *sheet,
                                    struct ui_bottom_sheet_base **out_base) {
  if (!sheet || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = sheet->base;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_sheet_set_largest_undimmed_detent(struct cupertino_sheet *sheet,
                                            enum cupertino_sheet_detent detent,
                                            int enable_passthrough) {
  if (!sheet || (int)detent < 0 ||
      (int)detent > CUPERTINO_SHEET_DETENT_CUSTOM) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sheet->largest_undimmed_detent = detent;
  sheet->passthrough_enabled = enable_passthrough ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_sheet_get_largest_undimmed_detent(
    const struct cupertino_sheet *sheet,
    enum cupertino_sheet_detent *out_detent, int *out_passthrough_enabled) {
  if (!sheet || !out_detent || !out_passthrough_enabled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_detent = sheet->largest_undimmed_detent;
  *out_passthrough_enabled = sheet->passthrough_enabled;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_sheet_is_passthrough_active(const struct cupertino_sheet *sheet,
                                      int *out_active) {
  if (!sheet || !out_active) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (sheet->is_open && sheet->passthrough_enabled &&
      (int)sheet->detent <= (int)sheet->largest_undimmed_detent) {
    *out_active = 1;
  } else {
    *out_active = 0;
  }

  return UI_ERROR_NONE;
}
