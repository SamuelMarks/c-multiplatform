/*
 * \file ui_pin_input_base.c
 * \brief Implementation of the UI PIN/OTP input component.
 */
/* clang-format off */
#include "ui_pin_input_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
#include <string.h>
/* clang-format on */

static const char *ui_pin_input_base_default_css = ".pin-input-container { "
                                                   "display: flex; "
                                                   "gap: 8px; "
                                                   "}";

/* \brief ui_pin_input_base
 */
/**
 * @struct ui_pin_input_base
 * \struct ui_pin_input_base
 * \brief State and DOM mapping for a multi-character PIN input widget.
 */
struct ui_pin_input_base {
  struct ui_component *component; /**< component */
  int length;                     /**< length */
  char *buffer;                   /**< Nul-terminated concatenated string */

  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data); /**< user_data) */
  void *cva_on_change_user_data;                /**< cva_on_change_user_data */

  ui_error_t (*cva_on_touched)(void *user_data); /**< user_data) */
  void *cva_on_touched_user_data; /**< cva_on_touched_user_data */

  int is_disabled; /**< is_disabled */
};

/*
 * \brief Internal helper to trigger the CVA on_change callback.
 * \param[in,out] pin_input The PIN input widget.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t trigger_change(struct ui_pin_input_base *pin_input) {
  if (pin_input->cva_on_change) {
    union ui_signal_payload payload;
    payload.ptr_val = pin_input->buffer;
    return pin_input->cva_on_change(payload,
                                    pin_input->cva_on_change_user_data);
  }
  return UI_ERROR_NONE;
}

/*
 * \brief Internal helper to trigger the CVA on_touched callback.
 * \param[in,out] pin_input The PIN input widget.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t trigger_touched(struct ui_pin_input_base *pin_input) {
  if (pin_input->cva_on_touched) {
    return pin_input->cva_on_touched(pin_input->cva_on_touched_user_data);
  }
  return UI_ERROR_NONE;
}

/*
 * \brief CVA interface function to update the PIN input's value from code.
 * \param[in,out] component The PIN input widget.
 * \param[in] value The payload string.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t pin_input_cva_write_value(void *component,
                                            union ui_signal_payload value) {
  struct ui_pin_input_base *pin_input = (struct ui_pin_input_base *)component;
  const char *str;

  if (!pin_input) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  str = (const char *)value.ptr_val;
  if (!str) {
    memset(pin_input->buffer, 0, (size_t)pin_input->length + 1);
  } else {
    /* Copy up to length */
    size_t len = strlen(str);
    size_t copy_len =
        len < (size_t)pin_input->length ? len : (size_t)pin_input->length;
    memset(pin_input->buffer, 0, (size_t)pin_input->length + 1);
    memcpy(pin_input->buffer, str, copy_len);
  }

  /* Update visual DOM if needed */

  return UI_ERROR_NONE;
}

/*
 * \brief CVA interface function to register the change callback.
 * \param[in,out] component The PIN input widget.
 * \param[in] callback The callback function.
 * \param[in] user_data User data for the callback.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t pin_input_cva_register_on_change(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  struct ui_pin_input_base *pin_input = (struct ui_pin_input_base *)component;
  if (!pin_input)
    return UI_ERROR_INVALID_ARGUMENT;
  pin_input->cva_on_change = callback;
  pin_input->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

/*
 * \brief CVA interface function to register the touched callback.
 * \param[in,out] component The PIN input widget.
 * \param[in] callback The callback function.
 * \param[in] user_data User data for the callback.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t pin_input_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_pin_input_base *pin_input = (struct ui_pin_input_base *)component;
  if (!pin_input)
    return UI_ERROR_INVALID_ARGUMENT;
  pin_input->cva_on_touched = callback;
  pin_input->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

/*
 * \brief CVA interface function to set the disabled state.
 * \param[in,out] component The PIN input widget.
 * \param[in] is_disabled Non-zero if disabled.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t pin_input_cva_set_disabled_state(void *component,
                                                   int is_disabled) {
  struct ui_pin_input_base *pin_input = (struct ui_pin_input_base *)component;
  if (!pin_input)
    return UI_ERROR_INVALID_ARGUMENT;
  pin_input->is_disabled = is_disabled;
  {
    ui_error_t attr_rc = ui_dom_node_set_attribute(
        pin_input->component->shadow_root, "aria-disabled",
        is_disabled ? "true" : "false");
    if (attr_rc != UI_ERROR_NONE)
      return attr_rc;
  }
  return UI_ERROR_NONE;
}

/*
 * \brief Creates a new PIN input base widget.
 * \param[out] out_pin_input Pointer to store the created widget.
 * \param[in] length The number of characters in the PIN.
 * \param[out] out_cva Optional pointer to receive the CVA interface.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_pin_input_base_create(struct ui_pin_input_base **out_pin_input,
                                    int length,
                                    struct ui_control_value_accessor *out_cva) {
  struct ui_pin_input_base *pin_input;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_pin_input || length <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  pin_input = (struct ui_pin_input_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_pin_input_base));
  if (!pin_input) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(pin_input, 0, sizeof(struct ui_pin_input_base));
  pin_input->length = length;

  pin_input->buffer = (char *)C_MULTIPLATFORM_MALLOC((size_t)length + 1);
  if (!pin_input->buffer) {
    C_MULTIPLATFORM_FREE(pin_input->buffer);
    C_MULTIPLATFORM_FREE(pin_input);
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(pin_input->buffer, 0, (size_t)length + 1);

  rc = ui_component_create(&pin_input->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(root_node, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "class", "pin-input-container");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_css_parse_stylesheet(ui_pin_input_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  (void)ui_component_set_default_style(pin_input->component, default_style);

  pin_input->component->shadow_root = root_node;
  root_node = NULL;

  if (out_cva) {
    out_cva->write_value = pin_input_cva_write_value;
    out_cva->register_on_change = pin_input_cva_register_on_change;
    out_cva->register_on_touched = pin_input_cva_register_on_touched;
    out_cva->set_disabled_state = pin_input_cva_set_disabled_state;
  }

  *out_pin_input = pin_input;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    (void)ui_dom_node_destroy(root_node);
  }
  if (pin_input->component) {
    (void)ui_component_destroy(pin_input->component);
  }
  C_MULTIPLATFORM_FREE(pin_input->buffer);
  C_MULTIPLATFORM_FREE(pin_input);
  return rc;
}

/*
 * \brief Destroys a PIN input base widget.
 * \param[in,out] pin_input The widget to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_pin_input_base_destroy(struct ui_pin_input_base *pin_input) {
  if (!pin_input) {
    return UI_ERROR_NONE;
  }
  (void)ui_component_destroy(pin_input->component);
  C_MULTIPLATFORM_FREE(pin_input->buffer);
  C_MULTIPLATFORM_FREE(pin_input);
  return UI_ERROR_NONE;
}

/*
 * \brief Gets the underlying DOM component for the PIN input.
 * \param[in,out] pin_input The PIN input widget.
 * \param[out] out_component Pointer to store the DOM component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_pin_input_base_get_component(struct ui_pin_input_base *pin_input,
                                struct ui_component **out_component) {
  if (!out_component)
    return UI_ERROR_INVALID_ARGUMENT;
  if (!pin_input) {
    *out_component = NULL;
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = pin_input->component;
  return UI_ERROR_NONE;
}

/*
 * \brief Handles input of a single character at a specific index.
 * \param[in,out] pin_input The PIN input widget.
 * \param[in] index The zero-based index of the input box.
 * \param[in] c A string containing the character typed.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_pin_input_base_on_input(struct ui_pin_input_base *pin_input,
                                      int index, const char *c) {
  if (!pin_input || !c) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (pin_input->is_disabled) {
    return UI_ERROR_NONE;
  }

  if (index >= 0 && index < pin_input->length) {
    pin_input->buffer[index] = c[0];
    {
      ui_error_t rc = trigger_change(pin_input);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    {
      ui_error_t rc = trigger_touched(pin_input);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
  }

  /* Auto-advance logic would go here: move focus to index + 1 */
  return UI_ERROR_NONE;
}

/*
 * \brief Handles backspace/deletion at a specific index.
 * \param[in,out] pin_input The PIN input widget.
 * \param[in] index The zero-based index of the input box.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_pin_input_base_on_backspace(struct ui_pin_input_base *pin_input,
                                          int index) {
  if (!pin_input) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (pin_input->is_disabled) {
    return UI_ERROR_NONE;
  }

  if (index >= 0 && index < pin_input->length) {
    pin_input->buffer[index] = '\0';
    {
      ui_error_t rc = trigger_change(pin_input);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    {
      ui_error_t rc = trigger_touched(pin_input);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
  }

  /* Cascading backspace logic: if empty, move focus to index - 1 and clear */
  return UI_ERROR_NONE;
}

/*
 * \brief Handles pasting a string into the PIN input.
 * \param[in,out] pin_input The PIN input widget.
 * \param[in] pasted_text The text string pasted by the user.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_pin_input_base_on_paste(struct ui_pin_input_base *pin_input,
                                      const char *pasted_text) {
  size_t pasted_len;
  size_t copy_len;

  if (!pin_input || !pasted_text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (pin_input->is_disabled) {
    return UI_ERROR_NONE;
  }

  pasted_len = strlen(pasted_text);
  copy_len = pasted_len < (size_t)pin_input->length ? pasted_len
                                                    : (size_t)pin_input->length;

  memset(pin_input->buffer, 0, (size_t)pin_input->length + 1);
  memcpy(pin_input->buffer, pasted_text, copy_len);

  {
    ui_error_t rc = trigger_change(pin_input);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  {
    ui_error_t rc = trigger_touched(pin_input);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  /* Paste splitting logic: distribute characters across consecutive inputs */
  return UI_ERROR_NONE;
}
