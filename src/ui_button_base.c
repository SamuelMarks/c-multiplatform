/* clang-format off */
#include "ui_button_base.h"
#include "ui_ripple_base.h"
#include "ui_internal_mem.h"
#include "ui_aria.h"
#include "ui_css_parser.h"
#include <stddef.h>
#include <string.h>
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

#ifdef UI_TEST_MOCK_ALLOC
int g_button_mock_fail = 0;

/**
 * @brief mock_ui_dom_node_set_attribute.
 * @param node Parameter node.
 * @param k Parameter k.
 * @param v Parameter v.
 * @return Return value.
 */
static ui_error_t mock_ui_dom_node_set_attribute(struct ui_dom_node *node,
                                                 const char *k, const char *v) {
  if (g_button_mock_fail == 168 || g_button_mock_fail == 240 ||
      g_button_mock_fail == 282 || g_button_mock_fail == 290 ||
      g_button_mock_fail == 298) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_dom_node_set_attribute(node, k, v);
}
#define ui_dom_node_set_attribute mock_ui_dom_node_set_attribute

/**
 * @brief mock_ui_ripple_config_init.
 * @param config Parameter config.
 * @return Return value.
 */
static ui_error_t mock_ui_ripple_config_init(struct ui_ripple_config *config) {
  if (g_button_mock_fail == 81) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_ripple_config_init(config);
}
#define ui_ripple_config_init mock_ui_ripple_config_init

/**
 * @brief mock_ui_component_set_default_style.
 * @param comp Parameter comp.
 * @param style Parameter style.
 * @return Return value.
 */
static ui_error_t
mock_ui_component_set_default_style(struct ui_component *comp,
                                    struct ui_css_stylesheet *style) {
  if (g_button_mock_fail == 123) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_component_set_default_style(comp, style);
}
#define ui_component_set_default_style mock_ui_component_set_default_style

/**
 * @brief mock_ui_dom_node_remove_attribute.
 * @param node Parameter node.
 * @param name Parameter name.
 * @return Return value.
 */
static ui_error_t mock_ui_dom_node_remove_attribute(struct ui_dom_node *node,
                                                    const char *name) {
  if (g_button_mock_fail == 195) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_dom_node_remove_attribute(node, name);
}
#define ui_dom_node_remove_attribute mock_ui_dom_node_remove_attribute

/**
 * @brief mock_ui_gesture_recognizer_process_event.
 * @param recognizer Parameter recognizer.
 * @param event Parameter event.
 * @param timestamp_ms Parameter timestamp_ms.
 * @param out_event Parameter out_event.
 * @return Return value.
 */
static ui_error_t mock_ui_gesture_recognizer_process_event(
    struct ui_gesture_recognizer *recognizer, const struct ui_event *event,
    double timestamp_ms, struct ui_gesture_event *out_event) {
  if (g_button_mock_fail == 237) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_button_mock_fail == 253) {
    out_event->type = UI_GESTURE_TAP;
    return UI_ERROR_NONE;
  }
  return ui_gesture_recognizer_process_event(recognizer, event, timestamp_ms,
                                             out_event);
}
#define ui_gesture_recognizer_process_event                                    \
  mock_ui_gesture_recognizer_process_event

/**
 * @brief mock_ui_ripple_start.
 * @param config Parameter config.
 * @param x Parameter x.
 * @param y Parameter y.
 * @param state Parameter state.
 * @return Return value.
 */
static ui_error_t mock_ui_ripple_start(struct ui_ripple_config *config, float x,
                                       float y, struct ui_ripple_state *state) {
  if (g_button_mock_fail == 245) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_ripple_start(config, x, y, state);
}
#define ui_ripple_start mock_ui_ripple_start
#endif

/**
 * @struct ui_button_base
 * @brief Internal representation of a base button component.
 */
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

/**
 * @brief ui_button_base_create.
 * @param out_button Parameter out_button.
 * @return Return value.
 */
ui_error_t ui_button_base_create(struct ui_button_base **out_button) {
  struct ui_button_base *btn;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  btn = (struct ui_button_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_button_base));
  if (!btn) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  btn->component = NULL;
  btn->gesture_recognizer = NULL;
  btn->disabled = 0;
  btn->on_click = NULL;
  btn->user_data = NULL;
  rc = ui_ripple_config_init(&btn->ripple_config);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
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

  rc = ui_component_set_default_style(btn->component, default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  btn->component->shadow_root = root_node;
  root_node = NULL; /* Owned by component now */

  *out_button = btn;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    (void)ui_dom_node_destroy(root_node);
  }
  if (btn->gesture_recognizer) {
    (void)ui_gesture_recognizer_destroy(btn->gesture_recognizer);
  }
  if (btn->component) {
    (void)ui_component_destroy(btn->component);
  }
  C_MULTIPLATFORM_FREE(btn);
  return rc;
}

/**
 * @brief ui_button_base_destroy.
 * @param button Parameter button.
 * @return Return value.
 */
ui_error_t ui_button_base_destroy(struct ui_button_base *button) {
  if (!button) {
    return UI_ERROR_NONE;
  }

  (void)ui_gesture_recognizer_destroy(button->gesture_recognizer);
  (void)ui_component_destroy(button->component);

  C_MULTIPLATFORM_FREE(button);
  return UI_ERROR_NONE;
}

/**
 * @brief ui_button_base_set_disabled.
 * @param button Parameter button.
 * @param disabled Parameter disabled.
 * @return Return value.
 */
ui_error_t ui_button_base_set_disabled(struct ui_button_base *button,
                                       int disabled) {
  ui_error_t rc;

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

    rc = ui_dom_node_remove_attribute(button->component->shadow_root,
                                      "disabled");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    rc = ui_dom_node_set_attribute(button->component->shadow_root, "tabindex",
                                   "0");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_button_base_set_on_click.
 * @param button Parameter button.
 * @param on_click Parameter on_click.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
ui_error_t ui_button_base_set_on_click(struct ui_button_base *button,
                                       ui_button_on_click_t on_click,
                                       void *user_data) {
  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  button->on_click = on_click;
  button->user_data = user_data;

  return UI_ERROR_NONE;
}

/**
 * @brief ui_button_base_process_event.
 * @param button Parameter button.
 * @param event Parameter event.
 * @param timestamp_ms Parameter timestamp_ms.
 * @return Return value.
 */
ui_error_t ui_button_base_process_event(struct ui_button_base *button,
                                        const struct ui_event *event,
                                        double timestamp_ms) {
  struct ui_gesture_event gesture_evt;
  ui_error_t rc;

  if (!button || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (button->disabled) {
    return UI_ERROR_NONE;
  }

  rc = ui_gesture_recognizer_process_event(button->gesture_recognizer, event,
                                           timestamp_ms, &gesture_evt);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  /* Handle Mouse Down directly for Ripple Trigger */
  if (event->type == UI_EVENT_MOUSE_DOWN) {
    if (event->event_data.mouse.button == 0) {
      rc = ui_ripple_start(&button->ripple_config, event->event_data.mouse.x,
                           event->event_data.mouse.y, &button->ripple_state);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  }

  if (gesture_evt.type == UI_GESTURE_TAP) {
    if (button->on_click) {
      rc = button->on_click(button, button->user_data);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_button_base_get_component.
 * @param button Parameter button.
 * @param out_component Parameter out_component.
 * @return Return value.
 */
ui_error_t ui_button_base_get_component(struct ui_button_base *button,
                                        struct ui_component **out_component) {
  if (!button || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = button->component;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_button_base_bind_disabled.
 * @param widget Parameter widget.
 * @param disabled_signal Parameter disabled_signal.
 * @return Return value.
 */
ui_error_t ui_button_base_bind_disabled(struct ui_button_base *widget,
                                        struct ui_signal *disabled_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->disabled_signal = disabled_signal;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_button_base_bind_text.
 * @param widget Parameter widget.
 * @param text_signal Parameter text_signal.
 * @return Return value.
 */
ui_error_t ui_button_base_bind_text(struct ui_button_base *widget,
                                    struct ui_signal *text_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->text_signal = text_signal;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_button_base_get_ripple_state.
 * @param button Parameter button.
 * @param out_state Parameter out_state.
 * @return Return value.
 */
ui_error_t ui_button_base_get_ripple_state(struct ui_button_base *button,
                                           struct ui_ripple_state *out_state) {
  if (!button || !out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_state = button->ripple_state;
  return UI_ERROR_NONE;
}
