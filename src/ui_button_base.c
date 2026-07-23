/* clang-format off */
#include "ui_button_base.h"
#include "ui_ripple_base.h"
#include "ui_internal_mem.h"
#include "ui_aria.h"
#include "ui_css_parser.h"
#include <stddef.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

static const char *ui_button_base_default_css =
    "button { "
    "background: var(--btn-bg, transparent); "
    "color: var(--btn-color, inherit); "
    "padding: var(--btn-padding, 0); "
    "border: var(--btn-border, none); "
    "border-radius: var(--btn-border-radius, 0); "
    "cursor: var(--btn-cursor, pointer); "
    "} "
    "button[aria-disabled=\"true\"] { "
    "background: var(--btn-disabled-bg, transparent); "
    "color: var(--btn-disabled-color, inherit); "
    "cursor: not-allowed; "
    "}";

/** \brief ui_button_base */
struct ui_button_base {
  struct ui_component *component;
  struct ui_gesture_recognizer *gesture_recognizer;
  int disabled;
  ui_button_on_click_t on_click;
  void *user_data;
  struct ui_signal *disabled_signal;
  struct ui_signal *text_signal;
  /* Ripple State */
  struct ui_ripple_config ripple_config;
  struct ui_ripple_state ripple_state;
};

enum ui_error ui_button_base_create(struct ui_button_base **out_button) {
  struct ui_button_base *btn;
  enum ui_error rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  btn = (struct ui_button_base *)UI_MALLOC(sizeof(struct ui_button_base));
  if (!btn) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  btn->component = NULL;
  btn->gesture_recognizer = NULL;
  btn->disabled = 0;
  btn->on_click = NULL;
  btn->user_data = NULL;
  ui_ripple_config_init(&btn->ripple_config);
  memset(&btn->ripple_state, 0, sizeof(struct ui_ripple_state));
  btn->disabled_signal = NULL;
  btn->text_signal = NULL;

  rc = ui_component_create(&btn->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_gesture_recognizer_create(&btn->gesture_recognizer);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(root_node, "button");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "role", "button");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "tabindex", "0");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_css_parse_stylesheet(ui_button_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  ui_component_set_default_style(btn->component, default_style);

  btn->component->shadow_root = root_node;
  root_node = NULL; /* Owned by component now */

  *out_button = btn;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    ui_dom_node_destroy(root_node);
  }
  if (btn->gesture_recognizer) {
    ui_gesture_recognizer_destroy(btn->gesture_recognizer);
  }
  if (btn->component) {
    ui_component_destroy(btn->component);
  }
  UI_FREE(btn);
  return rc;
}

void ui_button_base_destroy(struct ui_button_base *button) {
  if (!button) {
    return;
  }

  ui_gesture_recognizer_destroy(button->gesture_recognizer);
  ui_component_destroy(button->component);

  UI_FREE(button);
}

enum ui_error ui_button_base_set_disabled(struct ui_button_base *button,
                                          int disabled) {
  enum ui_error rc;

  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  button->disabled = disabled;

  if (disabled) {
    rc = ui_dom_node_set_attribute(button->component->shadow_root,
                                   "aria-disabled", "true");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    rc = ui_dom_node_set_attribute(button->component->shadow_root, "disabled",
                                   "");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    rc = ui_dom_node_set_attribute(button->component->shadow_root, "tabindex",
                                   "-1");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
    rc = ui_dom_node_set_attribute(button->component->shadow_root,
                                   "aria-disabled", "false");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    ui_dom_node_remove_attribute(button->component->shadow_root, "disabled");

    rc = ui_dom_node_set_attribute(button->component->shadow_root, "tabindex",
                                   "0");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_button_base_set_on_click(struct ui_button_base *button,
                                          ui_button_on_click_t on_click,
                                          void *user_data) {
  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  button->on_click = on_click;
  button->user_data = user_data;

  return UI_ERROR_NONE;
}

enum ui_error ui_button_base_process_event(struct ui_button_base *button,
                                           const struct ui_event *event,
                                           double timestamp_ms) {
  struct ui_gesture_event gesture_evt;

  if (!button || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (button->disabled) {
    return UI_ERROR_NONE;
  }

  ui_gesture_recognizer_process_event(button->gesture_recognizer, event,
                                      timestamp_ms, &gesture_evt);

  /* Handle Mouse Down directly for Ripple Trigger */
  if (event->type == UI_EVENT_MOUSE_DOWN) {
    if (event->event_data.mouse.button == 0) {
      ui_ripple_start(&button->ripple_config, event->event_data.mouse.x,
                      event->event_data.mouse.y, &button->ripple_state);
    }
  }

  if (gesture_evt.type == UI_GESTURE_TAP) {
    if (button->on_click) {
      button->on_click(button, button->user_data);
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_button_base_get_component(struct ui_button_base *button,
                             struct ui_component **out_component) {
  if (!button || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = button->component;
  return UI_ERROR_NONE;
}

enum ui_error ui_button_base_bind_disabled(struct ui_button_base *widget,
                                           struct ui_signal *disabled_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->disabled_signal = disabled_signal;
  return UI_ERROR_NONE;
}

enum ui_error ui_button_base_bind_text(struct ui_button_base *widget,
                                       struct ui_signal *text_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->text_signal = text_signal;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_button_base_get_ripple_state(struct ui_button_base *button,
                                struct ui_ripple_state *out_state) {
  if (!button || !out_state)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_state = button->ripple_state;
  return UI_ERROR_NONE;
}
