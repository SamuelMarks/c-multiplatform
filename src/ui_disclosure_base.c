/* clang-format off */
#include "ui_disclosure_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

static const char *ui_disclosure_base_default_css =
    "details { "
    "background: var(--disclosure-bg, transparent); "
    "border: var(--disclosure-border, 1px solid #ccc); "
    "} "
    "summary { "
    "padding: var(--disclosure-padding, 8px); "
    "color: var(--disclosure-color, inherit); "
    "cursor: pointer; "
    "list-style: none; "
    "} "
    "summary::-webkit-details-marker { "
    "display: none; "
    "} "
    "details[open] > div { "
    "padding: var(--disclosure-content-padding, 8px); "
    "background: var(--disclosure-content-bg, transparent); "
    "} "
    "details[aria-disabled=\"true\"] summary { "
    "cursor: not-allowed; "
    "opacity: 0.5; "
    "}";

/** \brief ui_disclosure_base */
struct ui_disclosure_base {
  struct ui_component *component;
  struct ui_gesture_recognizer *gesture_recognizer;
  int is_expanded;
  int disabled;
  ui_disclosure_on_toggle_t on_toggle;
  void *user_data;
  struct ui_signal *data_signal;
};

static ui_error_t update_dom_state(struct ui_disclosure_base *disclosure) {
  if (disclosure->is_expanded) {
    ui_error_t rc = ui_dom_node_set_attribute(
        disclosure->component->shadow_root, "aria-expanded", "true");
    (void)rc;
  } else {
    ui_error_t rc = ui_dom_node_set_attribute(
        disclosure->component->shadow_root, "aria-expanded", "false");
    (void)rc;
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_disclosure_base_create(struct ui_disclosure_base **out_disclosure) {
  struct ui_disclosure_base *disclosure;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_disclosure) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  disclosure = (struct ui_disclosure_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_disclosure_base));
  if (!disclosure) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  disclosure->component = NULL;
  disclosure->gesture_recognizer = NULL;
  disclosure->is_expanded = 0;
  disclosure->disabled = 0;
  disclosure->on_toggle = NULL;
  disclosure->user_data = NULL;

  rc = ui_component_create(&disclosure->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_gesture_recognizer_create(&disclosure->gesture_recognizer);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* Conceptually maps to <details> */
  rc = ui_dom_node_set_tag_name(root_node, "details");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* Usually, the trigger has role="button" and aria-expanded, but we'll place
   * it on the root wrapper for now */
  rc = ui_dom_node_set_attribute(root_node, "role", "group");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "aria-expanded", "false");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_css_parse_stylesheet(ui_disclosure_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  (void)ui_component_set_default_style(disclosure->component, default_style);

  disclosure->component->shadow_root = root_node;
  root_node = NULL; /* Owned by component now */

  *out_disclosure = disclosure;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    (void)ui_dom_node_destroy(root_node);
  }
  (void)ui_gesture_recognizer_destroy(disclosure->gesture_recognizer);
  (void)ui_component_destroy(disclosure->component);
  C_MULTIPLATFORM_FREE(disclosure);
  return rc;
}

ui_error_t ui_disclosure_base_destroy(struct ui_disclosure_base *disclosure) {
  if (!disclosure) {
    return UI_ERROR_NONE;
  }

  (void)ui_gesture_recognizer_destroy(disclosure->gesture_recognizer);
  (void)ui_component_destroy(disclosure->component);

  C_MULTIPLATFORM_FREE(disclosure);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_disclosure_base_set_disabled(struct ui_disclosure_base *disclosure,
                                int disabled) {

  if (!disclosure) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  disclosure->disabled = disabled;

  if (disabled) {
    ui_error_t rc = ui_dom_node_set_attribute(
        disclosure->component->shadow_root, "aria-disabled", "true");
    (void)rc;
  } else {
    ui_error_t rc = ui_dom_node_set_attribute(
        disclosure->component->shadow_root, "aria-disabled", "false");
    (void)rc;
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_disclosure_base_set_expanded(struct ui_disclosure_base *disclosure,
                                int is_expanded) {
  if (!disclosure) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (disclosure->is_expanded != is_expanded) {
    disclosure->is_expanded = is_expanded;
    {
      ui_error_t rc = update_dom_state(disclosure);
      (void)rc;
    }

    if (disclosure->on_toggle) {
      ui_error_t toggle_rc = disclosure->on_toggle(
          disclosure, disclosure->is_expanded, disclosure->user_data);
      if (toggle_rc != UI_ERROR_NONE)
        return toggle_rc;
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_disclosure_base_is_expanded(const struct ui_disclosure_base *disclosure,
                               int *out_is_expanded) {
  if (!disclosure) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!out_is_expanded) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_expanded = disclosure->is_expanded;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_disclosure_base_set_on_toggle(struct ui_disclosure_base *disclosure,
                                 ui_disclosure_on_toggle_t on_toggle,
                                 void *user_data) {
  if (!disclosure) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  disclosure->on_toggle = on_toggle;
  disclosure->user_data = user_data;

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_disclosure_base_process_event(struct ui_disclosure_base *disclosure,
                                 const struct ui_event *event,
                                 double timestamp_ms) {
  struct ui_gesture_event gesture_evt;

  if (!disclosure || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (disclosure->disabled) {
    return UI_ERROR_NONE;
  }

  /* Process gesture to detect taps on the trigger area */
  {
    ui_error_t rc = ui_gesture_recognizer_process_event(
        disclosure->gesture_recognizer, event, timestamp_ms, &gesture_evt);
    (void)rc;
  }

  if (gesture_evt.type == UI_GESTURE_TAP) {
    return ui_disclosure_base_set_expanded(disclosure,
                                           !disclosure->is_expanded);
  }

  /* Handle Enter/Space key as toggle */
  if (event->type == UI_EVENT_KEY_DOWN) {
    if (event->event_data.keyboard.key_code == 32 || /* Space */
        event->event_data.keyboard.key_code == 13) { /* Enter */
      return ui_disclosure_base_set_expanded(disclosure,
                                             !disclosure->is_expanded);
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_disclosure_base_get_component(struct ui_disclosure_base *disclosure,
                                 struct ui_component **out_component) {
  if (!out_component)
    return UI_ERROR_INVALID_ARGUMENT;
  if (!disclosure) {
    *out_component = NULL;
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = disclosure->component;
  return UI_ERROR_NONE;
}

ui_error_t ui_disclosure_base_bind_data(struct ui_disclosure_base *widget,
                                        struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
