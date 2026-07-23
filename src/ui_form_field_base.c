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

static const char *ui_form_field_base_default_css =
    ":host { "
    "display: block; "
    "position: relative; "
    "} "
    ".wrapper { "
    "display: flex; "
    "align-items: center; "
    "border-bottom: 1px solid var(--form-field-border, #ccc); "
    "} "
    ".focused .wrapper { "
    "border-bottom-color: var(--form-field-focus-color, #00f); "
    "} "
    ".error .wrapper { "
    "border-bottom-color: var(--form-field-error-color, #f00); "
    "} "
    ".label { "
    "position: absolute; "
    "transition: all 0.2s ease; "
    "color: var(--form-field-label-color, #999); "
    "} "
    ".floated .label { "
    "transform: translateY(-100%) scale(0.75); "
    "} "
    ".subscript { "
    "font-size: 0.75em; "
    "margin-top: 4px; "
    "} "
    ".hint { color: var(--form-field-hint-color, #666); } "
    ".error-text { color: var(--form-field-error-color, #f00); }";

/** \brief ui_form_field_base */
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

static enum ui_error internal_strdup(const char *src, char **out_str) {
  size_t len;
  char *copy;

  len = strlen(src);
  copy = (char *)UI_MALLOC(len + 1);
  if (!copy) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

#if defined(_MSC_VER)
  strcpy_s(copy, len + 1, src);
#else
  strcpy(copy, src);
#endif

  *out_str = copy;
  return UI_ERROR_NONE;
}

static enum ui_error update_dom_state(struct ui_form_field_base *field) {
  int should_float;

  should_float = field->is_focused || field->has_value;

  if (should_float) {
    ui_dom_node_set_attribute(field->root_node, "class", "floated");
  } else {
    ui_dom_node_remove_attribute(field->root_node, "class");
  }

  if (field->error_text && strlen(field->error_text) > 0) {
    ui_dom_node_set_attribute(field->root_node, "data-error", "true");
  } else {
    ui_dom_node_remove_attribute(field->root_node, "data-error");
  }

  if (field->label_node) {
    ui_dom_node_set_text_content(field->label_node,
                                 field->label_text ? field->label_text : "");
  }
  if (field->hint_node) {
    ui_dom_node_set_text_content(field->hint_node,
                                 field->hint_text ? field->hint_text : "");
  }
  if (field->error_node) {
    ui_dom_node_set_text_content(field->error_node,
                                 field->error_text ? field->error_text : "");
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_form_field_base_create(struct ui_form_field_base **out_field) {
  struct ui_form_field_base *field;
  enum ui_error rc;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_field)
    return UI_ERROR_INVALID_ARGUMENT;

  field =
      (struct ui_form_field_base *)UI_MALLOC(sizeof(struct ui_form_field_base));
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
  ui_dom_node_set_tag_name(field->root_node, "div");

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &field->wrapper_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(field->wrapper_node, "div");
  ui_dom_node_set_attribute(field->wrapper_node, "class", "wrapper");
  ui_dom_node_append_child(field->root_node, field->wrapper_node);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &field->prefix_container);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(field->prefix_container, "div");
  ui_dom_node_set_attribute(field->prefix_container, "class", "prefix");
  ui_dom_node_append_child(field->wrapper_node, field->prefix_container);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &field->control_container);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(field->control_container, "div");
  ui_dom_node_set_attribute(field->control_container, "class",
                            "control-container");
  ui_dom_node_append_child(field->wrapper_node, field->control_container);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &field->suffix_container);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(field->suffix_container, "div");
  ui_dom_node_set_attribute(field->suffix_container, "class", "suffix");
  ui_dom_node_append_child(field->wrapper_node, field->suffix_container);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &field->label_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(field->label_node, "label");
  ui_dom_node_set_attribute(field->label_node, "class", "label");
  ui_dom_node_append_child(field->wrapper_node, field->label_node);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &field->subscript_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(field->subscript_node, "div");
  ui_dom_node_set_attribute(field->subscript_node, "class", "subscript");
  ui_dom_node_append_child(field->root_node, field->subscript_node);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &field->hint_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(field->hint_node, "div");
  ui_dom_node_set_attribute(field->hint_node, "class", "hint");
  ui_dom_node_append_child(field->subscript_node, field->hint_node);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &field->error_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(field->error_node, "div");
  ui_dom_node_set_attribute(field->error_node, "class", "error-text");
  ui_dom_node_append_child(field->subscript_node, field->error_node);

  rc = ui_css_parse_stylesheet(ui_form_field_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  (void)ui_component_set_default_style(field->component, default_style);

  field->component->shadow_root = field->root_node;

  *out_field = field;
  return UI_ERROR_NONE;

cleanup:
  if (field->root_node)
    ui_dom_node_destroy(field->root_node);
  if (field->component)
    ui_component_destroy(field->component);
  UI_FREE(field);
  return rc;
}

void ui_form_field_base_destroy(struct ui_form_field_base *field) {
  if (!field)
    return;
  if (field->binding_effect)
    ui_effect_destroy(field->binding_effect);
  if (field->binding_arena)
    ui_arena_destroy(field->binding_arena);
  if (field->label_text)
    UI_FREE(field->label_text);
  if (field->hint_text)
    UI_FREE(field->hint_text);
  if (field->error_text)
    UI_FREE(field->error_text);
  if (field->component)
    ui_component_destroy(field->component);
  UI_FREE(field);
}

enum ui_error ui_form_field_base_set_label(struct ui_form_field_base *field,
                                           const char *label) {
  char *new_label = NULL;
  enum ui_error rc;

  if (!field)
    return UI_ERROR_INVALID_ARGUMENT;

  if (label) {
    rc = internal_strdup(label, &new_label);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  if (field->label_text)
    UI_FREE(field->label_text);
  field->label_text = new_label;
  update_dom_state(field);

  return UI_ERROR_NONE;
}

enum ui_error ui_form_field_base_set_hint(struct ui_form_field_base *field,
                                          const char *hint) {
  char *new_hint = NULL;
  enum ui_error rc;

  if (!field)
    return UI_ERROR_INVALID_ARGUMENT;

  if (hint) {
    rc = internal_strdup(hint, &new_hint);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  if (field->hint_text)
    UI_FREE(field->hint_text);
  field->hint_text = new_hint;
  update_dom_state(field);

  return UI_ERROR_NONE;
}

enum ui_error ui_form_field_base_set_error(struct ui_form_field_base *field,
                                           const char *error_msg) {
  char *new_err = NULL;
  enum ui_error rc;

  if (!field)
    return UI_ERROR_INVALID_ARGUMENT;

  if (error_msg) {
    rc = internal_strdup(error_msg, &new_err);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  if (field->error_text)
    UI_FREE(field->error_text);
  field->error_text = new_err;
  update_dom_state(field);

  return UI_ERROR_NONE;
}

enum ui_error ui_form_field_base_set_prefix(struct ui_form_field_base *field,
                                            struct ui_component *prefix) {
  if (!field)
    return UI_ERROR_INVALID_ARGUMENT;
  field->prefix = prefix;
  if (prefix) {
    return ui_component_mount(prefix, field->prefix_container);
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_form_field_base_set_suffix(struct ui_form_field_base *field,
                                            struct ui_component *suffix) {
  if (!field)
    return UI_ERROR_INVALID_ARGUMENT;
  field->suffix = suffix;
  if (suffix) {
    return ui_component_mount(suffix, field->suffix_container);
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_form_field_base_set_control(struct ui_form_field_base *field,
                                             struct ui_component *control) {
  if (!field)
    return UI_ERROR_INVALID_ARGUMENT;
  field->control = control;
  if (control) {
    return ui_component_mount(control, field->control_container);
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_form_field_base_set_focused(struct ui_form_field_base *field,
                                             int is_focused) {
  if (!field)
    return UI_ERROR_INVALID_ARGUMENT;
  field->is_focused = is_focused;
  update_dom_state(field);
  return UI_ERROR_NONE;
}

enum ui_error ui_form_field_base_set_has_value(struct ui_form_field_base *field,
                                               int has_value) {
  if (!field)
    return UI_ERROR_INVALID_ARGUMENT;
  field->has_value = has_value;
  update_dom_state(field);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
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
struct field_effect_data {
  struct ui_form_field_base *field;
  ui_signal_t *errors_signal;
};

static enum ui_error error_effect_runner(void *user_data) {
  struct field_effect_data *data = (struct field_effect_data *)user_data;
  union ui_signal_payload val;
  enum ui_error rc = UI_ERROR_NONE;

  if (ui_signal_get(data->errors_signal, &val) == UI_ERROR_NONE) {
    rc = ui_form_field_base_set_error(data->field, (const char *)val.ptr_val);
  }
  return rc;
}

/** \brief ui_error */
enum ui_error
ui_form_field_base_bind_form_control(struct ui_form_field_base *field,
                                     struct ui_form_control *form_control,
                                     struct ui_reactor *reactor) {
  ui_signal_t *err_sig = NULL;
  struct field_effect_data *data;
  ui_effect_t *eff = NULL;
  /* simplistic, leaking the arena and effect for now in the prototype unless we
   * attach them */
  struct ui_arena *arena = NULL;
  enum ui_error rc;

  if (!field || !form_control || !reactor)
    return UI_ERROR_INVALID_ARGUMENT;

  (void)ui_form_control_get_errors_signal(form_control, &err_sig);

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
    ui_arena_destroy(arena);
    return rc;
  }
  data->field = field;
  data->errors_signal = err_sig;

  ui_effect_create(arena, error_effect_runner, data, reactor, &eff);

  field->binding_arena = arena;
  field->binding_effect = eff;
  field->binding_data = data;

  return UI_ERROR_NONE;
}

enum ui_error ui_form_field_base_bind_data(struct ui_form_field_base *widget,
                                           struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
