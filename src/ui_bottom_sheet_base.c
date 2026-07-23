/* clang-format off */
#include "ui_bottom_sheet_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include "ui_backdrop.h"
#include "ui_gesture.h"
#include <stddef.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

static const char *ui_bottom_sheet_base_default_css =
    ":host { "
    "display: flex; "
    "flex-direction: column; "
    "justify-content: flex-end; "
    "position: absolute; "
    "top: 0; left: 0; right: 0; bottom: 0; "
    "pointer-events: none; "
    "z-index: 100; "
    "} "
    ".sheet { "
    "pointer-events: auto; "
    "background: var(--bottom-sheet-bg, #fff); "
    "border-top-left-radius: var(--bottom-sheet-radius, 16px); "
    "border-top-right-radius: var(--bottom-sheet-radius, 16px); "
    "box-shadow: var(--bottom-sheet-shadow, 0 -4px 10px rgba(0,0,0,0.2)); "
    "transform: translateY(100%); "
    "transition: transform 0.3s cubic-bezier(0.4, 0.0, 0.2, 1); "
    "} "
    ".sheet[data-open=\"true\"] { "
    "transform: translateY(0); "
    "} "
    ".drag-handle { "
    "width: 32px; "
    "height: 4px; "
    "background: var(--bottom-sheet-handle, #ccc); "
    "border-radius: 2px; "
    "margin: 8px auto; "
    "}";

/** \brief ui_bottom_sheet_base */
struct ui_bottom_sheet_base {
  struct ui_component *component;

  struct ui_dom_node *root_node;
  struct ui_dom_node *sheet_node;
  struct ui_dom_node *drag_handle_node;
  struct ui_dom_node *content_node;

  struct ui_component *content_component;

  int is_open;

  struct ui_overlay_director *director;
  struct ui_backdrop *backdrop_logic;
  struct ui_overlay *overlay;

  struct ui_gesture_recognizer *gesture_recognizer;

  struct ui_spring_config spring_config;
  struct ui_spring_state spring_state;

  ui_bottom_sheet_on_close_t on_close;
  void *user_data;

  /* Reactive integration */
  struct ui_signal *open_signal;
  struct ui_computed *animating_signal;
};

static enum ui_error update_dom_state(struct ui_bottom_sheet_base *sheet) {

  if (sheet->is_open) {
    ui_dom_node_set_attribute(sheet->sheet_node, "data-open", "true");
  } else {
    ui_dom_node_remove_attribute(sheet->sheet_node, "data-open");
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_bottom_sheet_base_create(struct ui_bottom_sheet_base **out_sheet) {
  struct ui_bottom_sheet_base *sheet;
  enum ui_error rc;
  struct ui_css_stylesheet *default_style = NULL;
  (void)rc;

  if (!out_sheet)
    return UI_ERROR_INVALID_ARGUMENT;

  sheet = (struct ui_bottom_sheet_base *)UI_MALLOC(
      sizeof(struct ui_bottom_sheet_base));
  if (!sheet)
    return UI_ERROR_OUT_OF_MEMORY;

  memset(sheet, 0, sizeof(struct ui_bottom_sheet_base));

  rc = ui_component_create(&sheet->component);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_backdrop_create(&sheet->backdrop_logic);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_gesture_recognizer_create(&sheet->gesture_recognizer);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sheet->root_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(sheet->root_node, "div");

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sheet->sheet_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(sheet->sheet_node, "div");
  ui_dom_node_set_attribute(sheet->sheet_node, "class", "sheet");
  ui_dom_node_append_child(sheet->root_node, sheet->sheet_node);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sheet->drag_handle_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(sheet->drag_handle_node, "div");
  ui_dom_node_set_attribute(sheet->drag_handle_node, "class", "drag-handle");
  ui_dom_node_append_child(sheet->sheet_node, sheet->drag_handle_node);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sheet->content_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(sheet->content_node, "div");
  ui_dom_node_append_child(sheet->sheet_node, sheet->content_node);

  rc =
      ui_css_parse_stylesheet(ui_bottom_sheet_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  (void)ui_component_set_default_style(sheet->component, default_style);

  sheet->component->shadow_root = sheet->root_node;

  sheet->spring_config.damping = 1.0f; /* critically damped */
  sheet->spring_config.stiffness = 300.0f;
  sheet->spring_config.mass = 1.0f;
  sheet->spring_state.value = 0.0f;
  sheet->spring_state.velocity = 0.0f;

  sheet->open_signal = NULL;
  sheet->animating_signal = NULL;

  (void)update_dom_state(sheet);

  *out_sheet = sheet;
  return UI_ERROR_NONE;

cleanup:
  if (sheet->root_node)
    ui_dom_node_destroy(sheet->root_node);
  if (sheet->gesture_recognizer)
    ui_gesture_recognizer_destroy(sheet->gesture_recognizer);
  if (sheet->backdrop_logic)
    ui_backdrop_destroy(sheet->backdrop_logic);
  if (sheet->component)
    ui_component_destroy(sheet->component);
  UI_FREE(sheet);
  return rc;
}

void ui_bottom_sheet_base_destroy(struct ui_bottom_sheet_base *sheet) {
  if (!sheet)
    return;

  if (sheet->is_open && sheet->director && sheet->overlay) {
    ui_overlay_director_unmount(sheet->director, sheet->overlay);
  }

  ui_gesture_recognizer_destroy(sheet->gesture_recognizer);
  ui_backdrop_destroy(sheet->backdrop_logic);
  ui_component_destroy(sheet->component);

  UI_FREE(sheet);
}

/** \brief ui_error */
enum ui_error
ui_bottom_sheet_base_set_content(struct ui_bottom_sheet_base *sheet,
                                 struct ui_component *content) {
  if (!sheet)
    return UI_ERROR_INVALID_ARGUMENT;
  sheet->content_component = content;
  if (content) {
    return ui_component_mount(content, sheet->content_node);
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_bottom_sheet_base_set_open(struct ui_bottom_sheet_base *sheet,
                                            int is_open) {
  enum ui_error rc = UI_ERROR_NONE;

  if (!sheet)
    return UI_ERROR_INVALID_ARGUMENT;
  if (sheet->is_open == is_open)
    return UI_ERROR_NONE;

  if (is_open) {
    if (sheet->director) {
      /* High z-index to overlay app content */
      rc = ui_overlay_director_mount_component(
          sheet->director, sheet->component, 1000, &sheet->overlay);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
  } else {
    if (sheet->director && sheet->overlay) {
      ui_overlay_director_unmount(sheet->director, sheet->overlay);
      sheet->overlay = NULL;
    }
  }

  sheet->is_open = is_open;
  (void)update_dom_state(sheet);

  if (sheet->open_signal) {
    union ui_signal_payload payload;
    payload.bool_val = is_open;
    ui_signal_set(sheet->open_signal, payload);
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_bottom_sheet_base_is_open(const struct ui_bottom_sheet_base *sheet,
                             int *out_is_open) {
  if (!sheet || !out_is_open)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_is_open = sheet->is_open;
  return UI_ERROR_NONE;
}

/** \brief ui_bottom_sheet_base_set_overlay_director */
enum ui_error ui_bottom_sheet_base_set_overlay_director(
    struct ui_bottom_sheet_base *sheet, struct ui_overlay_director *director) {
  if (!sheet)
    return UI_ERROR_INVALID_ARGUMENT;
  sheet->director = director;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_bottom_sheet_base_set_on_close(struct ui_bottom_sheet_base *sheet,
                                  ui_bottom_sheet_on_close_t on_close,
                                  void *user_data) {
  if (!sheet)
    return UI_ERROR_INVALID_ARGUMENT;
  sheet->on_close = on_close;
  sheet->user_data = user_data;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_bottom_sheet_base_process_event(struct ui_bottom_sheet_base *sheet,
                                   const struct ui_event *event,
                                   double timestamp_ms) {
  int should_dismiss = 0;
  enum ui_error rc;
  struct ui_gesture_event gesture_ev;
  (void)rc;

  if (!sheet || !event)
    return UI_ERROR_INVALID_ARGUMENT;
  if (!sheet->is_open)
    return UI_ERROR_NONE;

  /* Process Backdrop Dismissal */
  rc = ui_backdrop_process_event(sheet->backdrop_logic, event, 0.0f, 0.0f,
                                 100.0f, 100.0f, &should_dismiss);
  if (rc != UI_ERROR_NONE)
    return rc;

  if (should_dismiss) {
    if (sheet->on_close) {
      enum ui_error close_rc = sheet->on_close(sheet, sheet->user_data);
      if (close_rc != UI_ERROR_NONE)
        return close_rc;
    } else {
      ui_bottom_sheet_base_set_open(sheet, 0);
    }
    return UI_ERROR_NONE; /* Event consumed via dismissal */
  }

  /* Process Swipe Down Dismissal */
  rc = ui_gesture_recognizer_process_event(sheet->gesture_recognizer, event,
                                           timestamp_ms, &gesture_ev);
  if (rc != UI_ERROR_NONE)
    return rc;

  if (gesture_ev.type == UI_GESTURE_SWIPE) {
    /* Check if it's a downward swipe */
    if (gesture_ev.delta_y > 0.0f && gesture_ev.velocity_y > 300.0f) {
      if (sheet->on_close) {
        enum ui_error close_rc = sheet->on_close(sheet, sheet->user_data);
        if (close_rc != UI_ERROR_NONE)
          return close_rc;
      } else {
        ui_bottom_sheet_base_set_open(sheet, 0);
      }
    }
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_bottom_sheet_base_update(struct ui_bottom_sheet_base *sheet,
                                          double timestamp_ms) {
  struct ui_gesture_event gesture_ev;
  struct ui_spring_state next_state;
  float target;
  /* simplistic delta time since we don't store previous tick here, ideally we
   * would. Using 16ms for a 60fps assumption for testing. */
  float dt = 0.016f;

  if (!sheet)
    return UI_ERROR_INVALID_ARGUMENT;

  if (sheet->is_open) {
    target = 100.0f; /* 100% open */
  } else {
    target = 0.0f;
  }

  ui_spring_update(&sheet->spring_config, &sheet->spring_state, target, dt,
                   &next_state);
  sheet->spring_state = next_state;

  if (!sheet->is_open)
    return UI_ERROR_NONE;

  return ui_gesture_recognizer_update(sheet->gesture_recognizer, timestamp_ms,
                                      &gesture_ev);
}
/** \brief ui_error */
enum ui_error
ui_bottom_sheet_base_get_component(struct ui_bottom_sheet_base *sheet,
                                   struct ui_component **out_component) {
  if (!sheet || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = sheet->component;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_bottom_sheet_base_set_spring_config(struct ui_bottom_sheet_base *sheet,
                                       const struct ui_spring_config *config) {
  if (!sheet || !config) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  sheet->spring_config = *config;
  return UI_ERROR_NONE;
}

enum ui_error ui_bottom_sheet_base_bind_open(struct ui_bottom_sheet_base *sheet,
                                             struct ui_signal *open_signal) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  sheet->open_signal = open_signal;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_bottom_sheet_base_get_animating_signal(struct ui_bottom_sheet_base *sheet,
                                          struct ui_computed **out_animating) {
  if (!sheet || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_animating = sheet->animating_signal;
  return UI_ERROR_NONE;
}
