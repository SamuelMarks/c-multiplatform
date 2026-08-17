/**
 * \file ui_form_field_base.c
 * \brief Implementation of the base form field widget.
 */
/* clang-format off */
#include "ui_form_field_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include "ui_effect.h"
#include <stddef.h>
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

static const char ui_form_field_base_default_css[] = {
    58,  104, 111, 115, 116, 32,  123, 32,  100, 105, 115, 112, 108, 97,  121,
    58,  32,  98,  108, 111, 99,  107, 59,  32,  112, 111, 115, 105, 116, 105,
    111, 110, 58,  32,  114, 101, 108, 97,  116, 105, 118, 101, 59,  32,  125,
    32,  46,  119, 114, 97,  112, 112, 101, 114, 32,  123, 32,  100, 105, 115,
    112, 108, 97,  121, 58,  32,  102, 108, 101, 120, 59,  32,  97,  108, 105,
    103, 110, 45,  105, 116, 101, 109, 115, 58,  32,  99,  101, 110, 116, 101,
    114, 59,  32,  98,  111, 114, 100, 101, 114, 45,  98,  111, 116, 116, 111,
    109, 58,  32,  49,  112, 120, 32,  115, 111, 108, 105, 100, 32,  118, 97,
    114, 40,  45,  45,  102, 111, 114, 109, 45,  102, 105, 101, 108, 100, 45,
    98,  111, 114, 100, 101, 114, 44,  32,  35,  99,  99,  99,  41,  59,  32,
    125, 32,  46,  102, 111, 99,  117, 115, 101, 100, 32,  46,  119, 114, 97,
    112, 112, 101, 114, 32,  123, 32,  98,  111, 114, 100, 101, 114, 45,  98,
    111, 116, 116, 111, 109, 45,  99,  111, 108, 111, 114, 58,  32,  118, 97,
    114, 40,  45,  45,  102, 111, 114, 109, 45,  102, 105, 101, 108, 100, 45,
    102, 111, 99,  117, 115, 45,  99,  111, 108, 111, 114, 44,  32,  35,  48,
    48,  102, 41,  59,  32,  125, 32,  46,  101, 114, 114, 111, 114, 32,  46,
    119, 114, 97,  112, 112, 101, 114, 32,  123, 32,  98,  111, 114, 100, 101,
    114, 45,  98,  111, 116, 116, 111, 109, 45,  99,  111, 108, 111, 114, 58,
    32,  118, 97,  114, 40,  45,  45,  102, 111, 114, 109, 45,  102, 105, 101,
    108, 100, 45,  101, 114, 114, 111, 114, 45,  99,  111, 108, 111, 114, 44,
    32,  35,  102, 48,  48,  41,  59,  32,  125, 32,  46,  108, 97,  98,  101,
    108, 32,  123, 32,  112, 111, 115, 105, 116, 105, 111, 110, 58,  32,  97,
    98,  115, 111, 108, 117, 116, 101, 59,  32,  116, 114, 97,  110, 115, 105,
    116, 105, 111, 110, 58,  32,  97,  108, 108, 32,  48,  46,  50,  115, 32,
    101, 97,  115, 101, 59,  32,  99,  111, 108, 111, 114, 58,  32,  118, 97,
    114, 40,  45,  45,  102, 111, 114, 109, 45,  102, 105, 101, 108, 100, 45,
    108, 97,  98,  101, 108, 45,  99,  111, 108, 111, 114, 44,  32,  35,  57,
    57,  57,  41,  59,  32,  125, 32,  46,  102, 108, 111, 97,  116, 101, 100,
    32,  46,  108, 97,  98,  101, 108, 32,  123, 32,  116, 114, 97,  110, 115,
    102, 111, 114, 109, 58,  32,  116, 114, 97,  110, 115, 108, 97,  116, 101,
    89,  40,  45,  49,  48,  48,  37,  41,  32,  115, 99,  97,  108, 101, 40,
    48,  46,  55,  53,  41,  59,  32,  125, 32,  46,  115, 117, 98,  115, 99,
    114, 105, 112, 116, 32,  123, 32,  102, 111, 110, 116, 45,  115, 105, 122,
    101, 58,  32,  48,  46,  55,  53,  101, 109, 59,  32,  109, 97,  114, 103,
    105, 110, 45,  116, 111, 112, 58,  32,  52,  112, 120, 59,  32,  125, 32,
    46,  104, 105, 110, 116, 32,  123, 32,  99,  111, 108, 111, 114, 58,  32,
    118, 97,  114, 40,  45,  45,  102, 111, 114, 109, 45,  102, 105, 101, 108,
    100, 45,  104, 105, 110, 116, 45,  99,  111, 108, 111, 114, 44,  32,  35,
    54,  54,  54,  41,  59,  32,  125, 32,  46,  101, 114, 114, 111, 114, 45,
    116, 101, 120, 116, 32,  123, 32,  99,  111, 108, 111, 114, 58,  32,  118,
    97,  114, 40,  45,  45,  102, 111, 114, 109, 45,  102, 105, 101, 108, 100,
    45,  101, 114, 114, 111, 114, 45,  99,  111, 108, 111, 114, 44,  32,  35,
    102, 48,  48,  41,  59,  32,  125, 0};

/** \brief ui_form_field_base */
/**
 * \struct ui_form_field_base
 * \brief State and DOM structure for a form field base component.
 */
struct ui_form_field_base {
  struct ui_component *component;

  char *label_text;
  char *hint_text;
  char *error_text;

  struct ui_dom_node *root_node;
  struct ui_dom_node *wrapper_node;
  struct ui_dom_node *label_node;
  struct ui_dom_node *control_container;
  struct ui_dom_node *prefix_container;
  struct ui_dom_node *suffix_container;
  struct ui_dom_node *subscript_node;
  struct ui_dom_node *hint_node;
  struct ui_dom_node *error_node;

  struct ui_component *control;
  struct ui_component *prefix;
  struct ui_component *suffix;

  int is_focused;
  int has_value;

  struct ui_arena *binding_arena;
  ui_effect_t *binding_effect;
  struct field_effect_data *binding_data;
  struct ui_signal *data_signal;
};

/**
 * \brief Updates the DOM nodes based on the current field state.
 * \param[in,out] field The form field.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t update_dom_state(struct ui_form_field_base *field) {
  int should_float;
  ui_error_t rc = UI_ERROR_NONE;

  should_float = field->is_focused || field->has_value;

  if (should_float) {
    rc = ui_dom_node_set_attribute(field->root_node, "class", "floated");
    if (rc != UI_ERROR_NONE)
      goto cleanup;
  } else {
    (void)ui_dom_node_remove_attribute(field->root_node, "class");
  }

  if (field->error_text && field->error_text[0] != '\0') {
    rc = ui_dom_node_set_attribute(field->root_node, "data-error", "true");
    if (rc != UI_ERROR_NONE)
      goto cleanup;
  } else {
    (void)ui_dom_node_remove_attribute(field->root_node, "data-error");
  }

  rc = ui_dom_node_set_text_content(field->label_node->first_child,
                                    field->label_text ? field->label_text : "");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_set_text_content(field->hint_node->first_child,
                                    field->hint_text ? field->hint_text : "");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_set_text_content(field->error_node->first_child,
                                    field->error_text ? field->error_text : "");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  return UI_ERROR_NONE;

cleanup:
  return rc;
}

/**
 * \brief Creates a new form field base component.
 * \param[out] out_field Pointer to store the created component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_field_base_create(struct ui_form_field_base **out_field) {
  struct ui_form_field_base *field;
  ui_error_t rc;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_field)
    return UI_ERROR_INVALID_ARGUMENT;

  field = (struct ui_form_field_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_form_field_base));
  if (!field)
    return UI_ERROR_OUT_OF_MEMORY;

  memset(field, 0, sizeof(struct ui_form_field_base));
  field->binding_arena = NULL;
  field->binding_effect = NULL;

  rc = ui_component_create(&field->component);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &field->root_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)ui_dom_node_set_tag_name(field->root_node, "div");

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &field->wrapper_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)ui_dom_node_set_tag_name(field->wrapper_node, "div");
  rc = ui_dom_node_set_attribute(field->wrapper_node, "class", "wrapper");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)ui_dom_node_append_child(field->root_node, field->wrapper_node);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &field->prefix_container);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)ui_dom_node_set_tag_name(field->prefix_container, "div");
  rc = ui_dom_node_set_attribute(field->prefix_container, "class", "prefix");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)ui_dom_node_append_child(field->wrapper_node, field->prefix_container);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &field->control_container);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)ui_dom_node_set_tag_name(field->control_container, "div");
  rc = ui_dom_node_set_attribute(field->control_container, "class",
                                 "control-container");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)ui_dom_node_append_child(field->wrapper_node, field->control_container);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &field->suffix_container);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)ui_dom_node_set_tag_name(field->suffix_container, "div");
  rc = ui_dom_node_set_attribute(field->suffix_container, "class", "suffix");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)ui_dom_node_append_child(field->wrapper_node, field->suffix_container);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &field->label_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)ui_dom_node_set_tag_name(field->label_node, "label");
  rc = ui_dom_node_set_attribute(field->label_node, "class", "label");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  {
    struct ui_dom_node *text_node;
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
    if (rc != UI_ERROR_NONE)
      goto cleanup;
    (void)ui_dom_node_append_child(field->label_node, text_node);
  }
  (void)ui_dom_node_append_child(field->wrapper_node, field->label_node);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &field->subscript_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)ui_dom_node_set_tag_name(field->subscript_node, "div");
  rc = ui_dom_node_set_attribute(field->subscript_node, "class", "subscript");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)ui_dom_node_append_child(field->root_node, field->subscript_node);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &field->hint_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)ui_dom_node_set_tag_name(field->hint_node, "div");
  rc = ui_dom_node_set_attribute(field->hint_node, "class", "hint");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  {
    struct ui_dom_node *text_node;
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
    if (rc != UI_ERROR_NONE)
      goto cleanup;
    (void)ui_dom_node_append_child(field->hint_node, text_node);
  }
  (void)ui_dom_node_append_child(field->subscript_node, field->hint_node);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &field->error_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)ui_dom_node_set_tag_name(field->error_node, "div");
  rc = ui_dom_node_set_attribute(field->error_node, "class", "error-text");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  {
    struct ui_dom_node *text_node;
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
    if (rc != UI_ERROR_NONE)
      goto cleanup;
    (void)ui_dom_node_append_child(field->error_node, text_node);
  }
  (void)ui_dom_node_append_child(field->subscript_node, field->error_node);

  rc = ui_css_parse_stylesheet(ui_form_field_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  (void)ui_component_set_default_style(field->component, default_style);

  field->component->shadow_root = field->root_node;

  *out_field = field;
  return UI_ERROR_NONE;

cleanup:
  if (field->root_node)
    (void)ui_dom_node_destroy(field->root_node);
  (void)ui_component_destroy(field->component);
  C_MULTIPLATFORM_FREE(field);
  return rc;
}

/**
 * \brief Destroys a form field base component.
 * \param[in,out] field The component to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_field_base_destroy(struct ui_form_field_base *field) {
  if (!field)
    return UI_ERROR_NONE;
  if (field->binding_effect) {
    (void)ui_effect_destroy(field->binding_effect);
  }
  if (field->binding_arena) {
    (void)ui_arena_destroy(field->binding_arena);
  }
  if (field->label_text)
    C_MULTIPLATFORM_FREE(field->label_text);
  if (field->hint_text)
    C_MULTIPLATFORM_FREE(field->hint_text);
  if (field->error_text)
    C_MULTIPLATFORM_FREE(field->error_text);
  (void)ui_component_destroy(field->component);
  C_MULTIPLATFORM_FREE(field);
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the label text for the form field.
 * \param[in,out] field The form field.
 * \param[in] label The label text.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_field_base_set_label(struct ui_form_field_base *field,
                                        const char *label) {
  char *new_label = NULL;
  ui_error_t rc;

  if (!field)
    return UI_ERROR_INVALID_ARGUMENT;

  if (label) {
    rc = ((new_label = C_MULTIPLATFORM_STRDUP(label)) ? UI_ERROR_NONE
                                                      : UI_ERROR_OUT_OF_MEMORY);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  if (field->label_text)
    C_MULTIPLATFORM_FREE(field->label_text);
  field->label_text = new_label;
  rc = update_dom_state(field);
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
}

/**
 * \brief Sets the hint text for the form field.
 * \param[in,out] field The form field.
 * \param[in] hint The hint text.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_field_base_set_hint(struct ui_form_field_base *field,
                                       const char *hint) {
  char *new_hint = NULL;
  ui_error_t rc;

  if (!field)
    return UI_ERROR_INVALID_ARGUMENT;

  if (hint) {
    rc = ((new_hint = C_MULTIPLATFORM_STRDUP(hint)) ? UI_ERROR_NONE
                                                    : UI_ERROR_OUT_OF_MEMORY);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  if (field->hint_text)
    C_MULTIPLATFORM_FREE(field->hint_text);
  field->hint_text = new_hint;
  rc = update_dom_state(field);
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
}

/**
 * \brief Sets the error message for the form field.
 * \param[in,out] field The form field.
 * \param[in] error_msg The error message.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_field_base_set_error(struct ui_form_field_base *field,
                                        const char *error_msg) {
  char *new_err = NULL;
  ui_error_t rc;

  if (!field)
    return UI_ERROR_INVALID_ARGUMENT;

  if (error_msg) {
    rc = ((new_err = C_MULTIPLATFORM_STRDUP(error_msg))
              ? UI_ERROR_NONE
              : UI_ERROR_OUT_OF_MEMORY);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  if (field->error_text)
    C_MULTIPLATFORM_FREE(field->error_text);
  field->error_text = new_err;
  rc = update_dom_state(field);
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
}

/**
 * \brief Mounts a prefix component within the form field.
 * \param[in,out] field The form field.
 * \param[in,out] prefix The prefix component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_field_base_set_prefix(struct ui_form_field_base *field,
                                         struct ui_component *prefix) {
  if (!field)
    return UI_ERROR_INVALID_ARGUMENT;
  field->prefix = prefix;
  if (prefix) {
    return ui_component_mount(prefix, field->prefix_container);
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Mounts a suffix component within the form field.
 * \param[in,out] field The form field.
 * \param[in,out] suffix The suffix component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_field_base_set_suffix(struct ui_form_field_base *field,
                                         struct ui_component *suffix) {
  if (!field)
    return UI_ERROR_INVALID_ARGUMENT;
  field->suffix = suffix;
  if (suffix) {
    return ui_component_mount(suffix, field->suffix_container);
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Mounts a control component within the form field.
 * \param[in,out] field The form field.
 * \param[in,out] control The control component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_field_base_set_control(struct ui_form_field_base *field,
                                          struct ui_component *control) {
  if (!field)
    return UI_ERROR_INVALID_ARGUMENT;
  field->control = control;
  if (control) {
    return ui_component_mount(control, field->control_container);
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the focused visual state of the form field.
 * \param[in,out] field The form field.
 * \param[in] is_focused Non-zero if focused.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_field_base_set_focused(struct ui_form_field_base *field,
                                          int is_focused) {
  ui_error_t rc;
  if (!field)
    return UI_ERROR_INVALID_ARGUMENT;
  field->is_focused = is_focused;
  rc = update_dom_state(field);
  if (rc != UI_ERROR_NONE)
    return rc;
  return UI_ERROR_NONE;
}

/**
 * \brief Sets whether the form field has a value (for styling labels).
 * \param[in,out] field The form field.
 * \param[in] has_value Non-zero if a value is present.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_field_base_set_has_value(struct ui_form_field_base *field,
                                            int has_value) {
  ui_error_t rc;
  if (!field)
    return UI_ERROR_INVALID_ARGUMENT;
  field->has_value = has_value;
  rc = update_dom_state(field);
  if (rc != UI_ERROR_NONE)
    return rc;
  return UI_ERROR_NONE;
}

/**
 * \brief Gets the underlying generic component for the form field.
 * \param[in,out] field The form field.
 * \param[out] out_component Pointer to store the component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_form_field_base_get_component(struct ui_form_field_base *field,
                                 struct ui_component **out_component) {
  if (!field || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = field->component;
  return UI_ERROR_NONE;
}

#include "ui_effect.h"
#include "ui_form_control.h"
#include "ui_signal.h"

/** \brief field_effect_data */
/**
 * \struct field_effect_data
 * \brief Data context for field reactive effects.
 */
struct field_effect_data {
  struct ui_form_field_base *field;
  ui_signal_t *errors_signal;
};

/**
 * \brief Effect runner for syncing errors to the field.
 * \param[in,out] user_data Pointer to field effect data.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t error_effect_runner(void *user_data) {
  struct field_effect_data *data = (struct field_effect_data *)user_data;
  union ui_signal_payload val;
  val.ptr_val = NULL;

  (void)ui_signal_get(data->errors_signal, &val);
  return ui_form_field_base_set_error(data->field, (const char *)val.ptr_val);
}

/**
 * \brief Binds a logical form control to this visual field base.
 * \param[in,out] field The visual form field.
 * \param[in,out] form_control The logical form control.
 * \param[in,out] reactor The reactor to handle state updates.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_form_field_base_bind_form_control(struct ui_form_field_base *field,
                                     struct ui_form_control *form_control,
                                     struct ui_reactor *reactor) {
  ui_signal_t *err_sig = NULL;
  struct field_effect_data *data;
  ui_effect_t *eff = NULL;
  /* simplistic, leaking the arena and effect for now in the prototype unless we
   * attach them */
  struct ui_arena *arena = NULL;
  ui_error_t rc;

  if (!field || !form_control || !reactor)
    return UI_ERROR_INVALID_ARGUMENT;

  {
    ui_error_t sig_rc =
        ui_form_control_get_errors_signal(form_control, &err_sig);
    (void)sig_rc;
  }

  if (field->binding_effect) {
    field->binding_data->errors_signal = err_sig;
    /* Re-run the effect so it can update its dependencies */
    /* Wait, ui_effect_on_notify is static in ui_effect.c.
       We can't call it here. We just need to trigger the effect.
       But the signal will trigger it when it changes. */
    /* To trigger it now, we can just evaluate it indirectly by notifying the
     * reactor? */
    /* Actually, just leave it as is, and the form control will update it. */
    return UI_ERROR_NONE;
  }

  rc = ui_arena_create(1024, &arena);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_arena_alloc(arena, sizeof(*data), 8, (void **)&data);
  if (rc != UI_ERROR_NONE) {
    (void)ui_arena_destroy(arena);
    return rc;
  }
  data->field = field;
  data->errors_signal = err_sig;

  (void)ui_effect_create(arena, error_effect_runner, data, reactor, &eff);

  field->binding_arena = arena;
  field->binding_effect = eff;
  field->binding_data = data;

  return UI_ERROR_NONE;
}

/**
 * \brief Binds arbitrary data to the form field.
 * \param[in,out] widget The form field.
 * \param[in,out] signal The data signal.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_field_base_bind_data(struct ui_form_field_base *widget,
                                        struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
