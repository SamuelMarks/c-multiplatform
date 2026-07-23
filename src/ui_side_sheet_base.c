/* clang-format off */
#include "ui_side_sheet_base.h"
#include "ui_component.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

struct ui_side_sheet_base {
  struct ui_component *component;
  struct ui_component *content_component;
  struct ui_overlay_director *director;
  struct ui_signal *open_signal;

  enum ui_side_sheet_edge edge;
  enum ui_side_sheet_mode mode;
  int is_open;

  ui_side_sheet_on_close_t on_close;
  void *on_close_user_data;
};

enum ui_error ui_side_sheet_base_create(struct ui_side_sheet_base **out_sheet) {
  struct ui_side_sheet_base *sheet;
  enum ui_error rc;

  if (!out_sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sheet =
      (struct ui_side_sheet_base *)UI_MALLOC(sizeof(struct ui_side_sheet_base));
  if (!sheet) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_component_create(&sheet->component);
  if (rc != UI_ERROR_NONE) {
    UI_FREE(sheet);
    return rc;
  }

  sheet->content_component = NULL;
  sheet->director = NULL;
  sheet->open_signal = NULL;
  sheet->edge = UI_SIDE_SHEET_EDGE_LEFT;
  sheet->mode = UI_SIDE_SHEET_MODE_SLIDE_OVER;
  sheet->is_open = 0;
  sheet->on_close = NULL;
  sheet->on_close_user_data = NULL;

  *out_sheet = sheet;
  return UI_ERROR_NONE;
}

void ui_side_sheet_base_destroy(struct ui_side_sheet_base *sheet) {
  if (!sheet) {
    return;
  }
  ui_component_destroy(sheet->component);
  UI_FREE(sheet);
}

enum ui_error ui_side_sheet_base_set_content(struct ui_side_sheet_base *sheet,
                                             struct ui_component *content) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  sheet->content_component = content;
  /* Actual component hierarchy building goes here. */
  return UI_ERROR_NONE;
}

enum ui_error ui_side_sheet_base_set_edge(struct ui_side_sheet_base *sheet,
                                          enum ui_side_sheet_edge edge) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  sheet->edge = edge;
  return UI_ERROR_NONE;
}

enum ui_error ui_side_sheet_base_set_mode(struct ui_side_sheet_base *sheet,
                                          enum ui_side_sheet_mode mode) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  sheet->mode = mode;
  return UI_ERROR_NONE;
}

enum ui_error ui_side_sheet_base_set_open(struct ui_side_sheet_base *sheet,
                                          int is_open) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (sheet->is_open != is_open) {
    sheet->is_open = is_open;

    if (!is_open && sheet->on_close) {
      sheet->on_close(sheet, sheet->on_close_user_data);
    }
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_side_sheet_base_is_open(const struct ui_side_sheet_base *sheet,
                                         int *out_is_open) {
  if (!sheet || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_open = sheet->is_open;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_side_sheet_base_set_overlay_director(struct ui_side_sheet_base *sheet,
                                        struct ui_overlay_director *director) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  sheet->director = director;
  return UI_ERROR_NONE;
}

enum ui_error ui_side_sheet_base_set_on_close(struct ui_side_sheet_base *sheet,
                                              ui_side_sheet_on_close_t on_close,
                                              void *user_data) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  sheet->on_close = on_close;
  sheet->on_close_user_data = user_data;
  return UI_ERROR_NONE;
}

enum ui_error ui_side_sheet_base_process_event(struct ui_side_sheet_base *sheet,
                                               const struct ui_event *event,
                                               double timestamp_ms) {
  (void)timestamp_ms;
  if (!sheet || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (sheet->is_open) {
    if (event->type == UI_EVENT_KEY_DOWN &&
        event->event_data.keyboard.key_code == UI_KEY_ESCAPE) {
      ui_side_sheet_base_set_open(sheet, 0);
    }
    /* Backdrop click detection would go here if event coords are outside sheet
     * bounds. */
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_side_sheet_base_get_component(struct ui_side_sheet_base *sheet,
                                 struct ui_component **out_component) {
  if (!sheet || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = sheet->component;
  return UI_ERROR_NONE;
}

enum ui_error ui_side_sheet_base_bind_open(struct ui_side_sheet_base *sheet,
                                           struct ui_signal *open_signal) {
  if (!sheet || !open_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  sheet->open_signal = open_signal;
  return UI_ERROR_NONE;
}
