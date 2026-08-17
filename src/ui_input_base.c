/**
 * \file ui_input_base.c
 * \brief Implementation of the generic input base component.
 */
/* clang-format off */
#include "ui_input_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include "ui_web_bridge.h"
#include <stddef.h>
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

static const char *ui_input_base_default_css =
    "input { "
    "background: var(--input-bg, #fff); "
    "color: var(--input-color, #000); "
    "border: var(--input-border, 1px solid #ccc); "
    "border-radius: var(--input-border-radius, 4px); "
    "padding: var(--input-padding, 4px 8px); "
    "caret-color: var(--input-cursor-color, #000); "
    "} "
    "input::placeholder { "
    "color: var(--input-placeholder-color, #999); "
    "} "
    "input[aria-disabled=\"true\"] { "
    "background: var(--input-disabled-bg, #eee); "
    "color: var(--input-disabled-color, #999); "
    "cursor: not-allowed; "
    "}";

/** \brief ui_input_base */
/**
 * \struct ui_input_base
 * \brief Internal state for a generic input component (e.g. textbox).
 */
struct ui_input_base {
  struct ui_component *component;
  struct ui_gesture_recognizer *gesture_recognizer;
  char *text;
  char *placeholder;
  int disabled;
  int cursor_position;
  ui_input_on_change_t on_change;
  void *user_data;
};

/**
 * \brief Updates the underlying DOM to reflect the input base state.
 * \param[in,out] input The input base widget.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t update_dom_state(struct ui_input_base *input) {
#if defined(__EMSCRIPTEN__)
  if (input->text) {
    ui_error_t wb_rc = ui_web_bridge_set_property(
        (uint32_t)(uintptr_t)input->component->shadow_root, "value",
        input->text);
    if (wb_rc != UI_ERROR_NONE)
      return wb_rc;
  }
#endif
  if (input->text) {
    ui_error_t rc1 = ui_dom_node_set_attribute(input->component->shadow_root,
                                               "value", input->text);
    (void)rc1;
  } else {
    ui_error_t rc2 =
        ui_dom_node_remove_attribute(input->component->shadow_root, "value");
    (void)rc2;
  }

  if (input->placeholder) {
    ui_error_t rc3 = ui_dom_node_set_attribute(
        input->component->shadow_root, "placeholder", input->placeholder);
    (void)rc3;
  } else {
    ui_error_t rc4 = ui_dom_node_remove_attribute(input->component->shadow_root,
                                                  "placeholder");
    (void)rc4;
  }

  if (input->disabled) {
    ui_error_t rc5 = ui_dom_node_set_attribute(input->component->shadow_root,
                                               "disabled", "");
    (void)rc5;
    {
      ui_error_t rc6 = ui_dom_node_set_attribute(input->component->shadow_root,
                                                 "aria-disabled", "true");
      (void)rc6;
    }
  } else {
    ui_error_t rc7 =
        ui_dom_node_remove_attribute(input->component->shadow_root, "disabled");
    (void)rc7;
    {
      ui_error_t rc8 = ui_dom_node_remove_attribute(
          input->component->shadow_root, "aria-disabled");
      (void)rc8;
    }
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Creates a new input base widget.
 * \param[out] out_input Pointer to store the created widget.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_input_base_create(struct ui_input_base **out_input) {
  struct ui_input_base *input;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_input) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  input = (struct ui_input_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_input_base));
  if (!input) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  input->component = NULL;
  input->gesture_recognizer = NULL;
  input->text = NULL;
  input->placeholder = NULL;
  input->disabled = 0;
  input->cursor_position = 0;
  input->on_change = NULL;
  input->user_data = NULL;

  rc = ui_component_create(&input->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_gesture_recognizer_create(&input->gesture_recognizer);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(root_node, "input");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "type", "text");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "role", "textbox");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "tabindex", "0");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_css_parse_stylesheet(ui_input_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)ui_component_set_default_style(input->component, default_style);

  input->component->shadow_root = root_node;
  root_node = NULL; /* Owned by component now */

  *out_input = input;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    (void)ui_dom_node_destroy(root_node);
  }
  if (input->gesture_recognizer) {
    (void)ui_gesture_recognizer_destroy(input->gesture_recognizer);
  }
  if (input->component) {
    (void)ui_component_destroy(input->component);
  }
  C_MULTIPLATFORM_FREE(input);
  return rc;
}

/**
 * \brief Internal wrapper for CVA on-change events.
 * \param[in,out] input The input base widget.
 * \param[in] value The new string value.
 * \param[in] user_data User data for the callback.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t input_cva_on_change_wrapper(struct ui_input_base *input,
                                              const char *value,
                                              void *user_data);

/**
 * \brief Destroys an input base widget.
 * \param[in,out] input The widget to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_input_base_destroy(struct ui_input_base *input) {
  if (!input)
    return UI_ERROR_NONE;
  if (input->text)
    C_MULTIPLATFORM_FREE(input->text);
  C_MULTIPLATFORM_FREE(input->placeholder);
  if (input->on_change == input_cva_on_change_wrapper)
    C_MULTIPLATFORM_FREE(input->user_data);
  (void)ui_gesture_recognizer_destroy(input->gesture_recognizer);
  (void)ui_component_destroy(input->component);
  C_MULTIPLATFORM_FREE(input);
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the current text of the input widget.
 * \param[in,out] input The input widget.
 * \param[in] text The new text string.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_input_base_set_text(struct ui_input_base *input,
                                  const char *text) {
  char *new_text = NULL;
  ui_error_t rc;

  if (!input)
    return UI_ERROR_INVALID_ARGUMENT;

  if (text) {
    rc = ((new_text = C_MULTIPLATFORM_STRDUP(text)) ? UI_ERROR_NONE
                                                    : UI_ERROR_OUT_OF_MEMORY);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  if (input->text)
    C_MULTIPLATFORM_FREE(input->text);
  input->text = new_text;
  input->cursor_position = new_text ? (int)strlen(new_text) : 0;
  {
    ui_error_t uds_rc = update_dom_state(input);
    (void)uds_rc;
  }

  if (input->on_change) {
    ui_error_t change_rc = input->on_change(
        input, input->text ? input->text : "", input->user_data);
    if (change_rc != UI_ERROR_NONE) {
      return change_rc;
    }
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Gets the current text of the input widget.
 * \param[in] input The input widget.
 * \param[out] out_text Pointer to store the text string.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_input_base_get_text(const struct ui_input_base *input,
                                  const char **out_text) {
  if (!input || !out_text)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_text = input->text ? input->text : "";
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the HTML type attribute (e.g. "text", "password").
 * \param[in,out] input The input widget.
 * \param[in] type The type string.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_input_base_set_type(struct ui_input_base *input,
                                  const char *type) {
  if (!input || !type) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_dom_node_set_attribute(input->component->shadow_root, "type", type);
}

/**
 * \brief Sets the placeholder text for the input widget.
 * \param[in,out] input The input widget.
 * \param[in] placeholder The placeholder text.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_input_base_set_placeholder(struct ui_input_base *input,
                                         const char *placeholder) {
  char *new_ph = NULL;
  ui_error_t rc;

  if (!input)
    return UI_ERROR_INVALID_ARGUMENT;

  if (placeholder) {
    rc = ((new_ph = C_MULTIPLATFORM_STRDUP(placeholder))
              ? UI_ERROR_NONE
              : UI_ERROR_OUT_OF_MEMORY);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  if (input->placeholder)
    C_MULTIPLATFORM_FREE(input->placeholder);
  input->placeholder = new_ph;
  {
    ui_error_t uds_rc = update_dom_state(input);
    (void)uds_rc;
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Sets the disabled state of the input widget.
 * \param[in,out] input The input widget.
 * \param[in] disabled 1 to disable, 0 to enable.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_input_base_set_disabled(struct ui_input_base *input,
                                      int disabled) {
  if (!input)
    return UI_ERROR_INVALID_ARGUMENT;
  input->disabled = disabled;
  {
    ui_error_t uds_rc = update_dom_state(input);
    (void)uds_rc;
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the change callback for the input widget.
 * \param[in,out] input The input widget.
 * \param[in] on_change The callback function.
 * \param[in] user_data Optional user data for the callback.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_input_base_set_on_change(struct ui_input_base *input,
                                       ui_input_on_change_t on_change,
                                       void *user_data) {
  if (!input)
    return UI_ERROR_INVALID_ARGUMENT;
  input->on_change = on_change;
  input->user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * \brief Processes an incoming UI event (like keypresses) to update the input
 * state.
 * \param[in,out] input The input widget.
 * \param[in] event The UI event.
 * \param[in] timestamp_ms Event timestamp in ms.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_input_base_process_event(struct ui_input_base *input,
                                       const struct ui_event *event,
                                       double timestamp_ms) {
  (void)timestamp_ms;
  if (!input || !event)
    return UI_ERROR_INVALID_ARGUMENT;
  if (input->disabled)
    return UI_ERROR_NONE;

  if (event->type == UI_EVENT_KEY_DOWN) {
    if (event->event_data.keyboard.key_code == UI_KEY_BACKSPACE) {
      if (input->text && input->cursor_position > 0) {
        size_t len = strlen(input->text);
        char *new_text = (char *)C_MULTIPLATFORM_MALLOC(len);
        if (new_text) {
          size_t i;
          size_t dst = 0;
          for (i = 0; i < len; ++i) {
            if (i != (size_t)(input->cursor_position - 1)) {
              new_text[dst++] = input->text[i];
            }
          }
          new_text[dst] = '\0';
          C_MULTIPLATFORM_FREE(input->text);
          input->text = new_text;
          input->cursor_position--;
          {
            ui_error_t uds_rc = update_dom_state(input);
            (void)uds_rc;
          }
          if (input->on_change) {
            ui_error_t change_rc =
                input->on_change(input, input->text, input->user_data);
            if (change_rc != UI_ERROR_NONE)
              return change_rc;
          }
        } else {
          return UI_ERROR_OUT_OF_MEMORY;
        }
      }
    } else if (event->event_data.keyboard.key_code == UI_KEY_LEFT) {
      if (input->cursor_position > 0)
        input->cursor_position--;
    } else if (event->event_data.keyboard.key_code == UI_KEY_RIGHT) {
      if (input->text && input->cursor_position < (int)strlen(input->text))
        input->cursor_position++;
    } else {
      char c = (char)event->event_data.keyboard.key_code;
      if (c >= 32 && c <= 126) {
        size_t len = input->text ? strlen(input->text) : 0;
        char *new_text = (char *)C_MULTIPLATFORM_MALLOC(len + 2);
        if (new_text) {
          if (input->text) {
            size_t i;
            for (i = 0; i < (size_t)input->cursor_position; ++i) {
              new_text[i] = input->text[i];
            }
            new_text[input->cursor_position] = c;
            for (i = (size_t)input->cursor_position; i < len; ++i) {
              new_text[i + 1] = input->text[i];
            }
            new_text[len + 1] = '\0';
            C_MULTIPLATFORM_FREE(input->text);
          } else {
            new_text[0] = c;
            new_text[1] = '\0';
          }
          input->text = new_text;
          input->cursor_position++;
          {
            ui_error_t uds_rc = update_dom_state(input);
            (void)uds_rc;
          }
          if (input->on_change) {
            ui_error_t change_rc =
                input->on_change(input, input->text, input->user_data);
            if (change_rc != UI_ERROR_NONE)
              return change_rc;
          }
        } else {
          return UI_ERROR_OUT_OF_MEMORY;
        }
      }
    }
  }

  return UI_ERROR_NONE;
}

/**
 * \brief CVA interface function to write a value to the input.
 * \param[in,out] component The input component.
 * \param[in] value The payload value to write.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t input_cva_write_value(void *component,
                                        union ui_signal_payload value) {
  struct ui_input_base *input = (struct ui_input_base *)component;
  if (!input)
    return UI_ERROR_INVALID_ARGUMENT;
  return ui_input_base_set_text(input, (const char *)value.ptr_val);
}

/** \brief input_cva_wrapper */
struct input_cva_wrapper {
  ui_error_t (*callback)(union ui_signal_payload, void *);
  void *user_data;
};

/**
 * \brief Internal wrapper for CVA on-change events.
 * \param[in,out] input The input base widget.
 * \param[in] value The new string value.
 * \param[in] user_data User data for the callback.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t input_cva_on_change_wrapper(struct ui_input_base *input,
                                              const char *text,
                                              void *user_data) {
  struct input_cva_wrapper *wrap = (struct input_cva_wrapper *)user_data;
  (void)input;
  if (wrap->callback) {
    union ui_signal_payload p;
    p.ptr_val = (void *)text;
    return wrap->callback(p, wrap->user_data);
  }
  return UI_ERROR_NONE;
}

/** \brief input_cva_register_on_change */
static ui_error_t input_cva_register_on_change(
    void *component, ui_error_t (*callback)(union ui_signal_payload, void *),
    void *user_data) {
  struct ui_input_base *input = (struct ui_input_base *)component;
  struct input_cva_wrapper *wrap;
  if (!input)
    return UI_ERROR_INVALID_ARGUMENT;
  wrap = (struct input_cva_wrapper *)C_MULTIPLATFORM_MALLOC(sizeof(*wrap));
  if (!wrap)
    return UI_ERROR_OUT_OF_MEMORY;
  wrap->callback = callback;
  wrap->user_data = user_data;
  return ui_input_base_set_on_change(input, input_cva_on_change_wrapper, wrap);
}

/** \brief input_cva_register_on_touched */
static ui_error_t input_cva_register_on_touched(void *component,
                                                ui_error_t (*callback)(void *),
                                                void *user_data) {
  (void)component;
  (void)callback;
  (void)user_data;
  return UI_ERROR_NONE;
}

/**
 * \brief CVA interface function to set the disabled state.
 * \param[in,out] component The input component.
 * \param[in] is_disabled The disabled state boolean.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t input_cva_set_disabled_state(void *component,
                                               ui_bool_t is_disabled) {
  struct ui_input_base *input = (struct ui_input_base *)component;
  if (!input)
    return UI_ERROR_INVALID_ARGUMENT;
  return ui_input_base_set_disabled(input, is_disabled ? 1 : 0);
}

/**
 * \brief Gets the Control Value Accessor (CVA) interface for the input.
 * \param[in,out] input The input widget.
 * \param[out] out_cva Pointer to store the populated CVA struct.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_input_base_get_cva(struct ui_input_base *input,
                                 struct ui_control_value_accessor *out_cva) {
  if (!input || !out_cva)
    return UI_ERROR_INVALID_ARGUMENT;
  out_cva->write_value = input_cva_write_value;
  out_cva->register_on_change = input_cva_register_on_change;
  out_cva->register_on_touched = input_cva_register_on_touched;
  out_cva->set_disabled_state = input_cva_set_disabled_state;
  return UI_ERROR_NONE;
}

/**
 * \brief Retrieves the underlying generic DOM component.
 * \param[in] input The input widget.
 * \param[out] out_component Pointer to store the DOM component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_input_base_get_component(struct ui_input_base *input,
                                       struct ui_component **out_component) {
  if (!input || !out_component)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_component = input->component;
  return UI_ERROR_NONE;
}
