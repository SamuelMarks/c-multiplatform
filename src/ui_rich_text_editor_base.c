/* clang-format off */
#include "ui_rich_text_editor_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
#include <string.h>
/* clang-format on */

static const char *ui_rich_text_editor_base_default_css =
    ".rte-container { "
    "display: flex; "
    "flex-direction: column; "
    "outline: none; "
    "white-space: pre-wrap; "
    "}";

/** \brief ui_rich_text_editor_base */
struct ui_rich_text_editor_base {
  struct ui_component *component;
  char *html_buffer;
  size_t html_capacity;

  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data);
  void *cva_on_change_user_data;

  ui_error_t (*cva_on_touched)(void *user_data);
  void *cva_on_touched_user_data;

  int is_disabled;
};

static ui_error_t trigger_cva_change(struct ui_rich_text_editor_base *rte) {
  if (rte && rte->cva_on_change) {
    union ui_signal_payload payload;
    payload.ptr_val = rte->html_buffer;
    return rte->cva_on_change(payload, rte->cva_on_change_user_data);
  }
  return UI_ERROR_NONE;
}

static ui_error_t trigger_cva_touched(struct ui_rich_text_editor_base *rte) {
  if (rte && rte->cva_on_touched) {
    return rte->cva_on_touched(rte->cva_on_touched_user_data);
  }
  return UI_ERROR_NONE;
}

static ui_error_t rte_cva_write_value(void *component,
                                      union ui_signal_payload value) {
  struct ui_rich_text_editor_base *rte =
      (struct ui_rich_text_editor_base *)component;
  const char *str;
  size_t len;

  if (!rte) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  str = (const char *)value.ptr_val;
  if (!str) {
    str = "";
  }

  len = strlen(str);
  if (len + 1 > rte->html_capacity) {
    size_t new_cap = rte->html_capacity == 0 ? 128 : rte->html_capacity;
    char *new_buf;
    while (new_cap < len + 1) {
      new_cap *= 2;
    }
    new_buf = (char *)C_MULTIPLATFORM_REALLOC(rte->html_buffer, new_cap);
    if (!new_buf) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    rte->html_buffer = new_buf;
    rte->html_capacity = new_cap;
  }

  strcpy(rte->html_buffer, str);
  return UI_ERROR_NONE;
}

/** \brief rte_cva_register_on_change */
static ui_error_t rte_cva_register_on_change(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  struct ui_rich_text_editor_base *rte =
      (struct ui_rich_text_editor_base *)component;
  if (!rte)
    return UI_ERROR_INVALID_ARGUMENT;
  rte->cva_on_change = callback;
  rte->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t rte_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_rich_text_editor_base *rte =
      (struct ui_rich_text_editor_base *)component;
  if (!rte)
    return UI_ERROR_INVALID_ARGUMENT;
  rte->cva_on_touched = callback;
  rte->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t rte_cva_set_disabled_state(void *component, int is_disabled) {
  struct ui_rich_text_editor_base *rte =
      (struct ui_rich_text_editor_base *)component;
  if (!rte)
    return UI_ERROR_INVALID_ARGUMENT;
  rte->is_disabled = is_disabled;
  {
    ui_error_t s_rc1 = ui_dom_node_set_attribute(
        rte->component->shadow_root, "contenteditable",
        is_disabled ? "false" : "true");
    if (s_rc1 != UI_ERROR_NONE) {
      return s_rc1;
    }
  }
  {
    ui_error_t s_rc2 =
        ui_dom_node_set_attribute(rte->component->shadow_root, "aria-disabled",
                                  is_disabled ? "true" : "false");
    if (s_rc2 != UI_ERROR_NONE) {
      return s_rc2;
    }
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_rich_text_editor_base_create(struct ui_rich_text_editor_base **out_rte,
                                struct ui_control_value_accessor *out_cva) {
  struct ui_rich_text_editor_base *rte;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_rte) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rte = (struct ui_rich_text_editor_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_rich_text_editor_base));
  if (!rte) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(rte, 0, sizeof(struct ui_rich_text_editor_base));
  rte->component = NULL;
  rte->html_buffer = NULL;
  rte->html_capacity = 0;
  rte->cva_on_change = NULL;
  rte->cva_on_change_user_data = NULL;
  rte->cva_on_touched = NULL;
  rte->cva_on_touched_user_data = NULL;
  rte->is_disabled = 0;

  rc = ui_component_create(&rte->component);
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

  rc = ui_dom_node_set_attribute(root_node, "class", "rte-container");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "contenteditable", "true");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_css_parse_stylesheet(ui_rich_text_editor_base_default_css,
                               &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  (void)ui_component_set_default_style(rte->component, default_style);

  rte->component->shadow_root = root_node;
  root_node = NULL;

  if (out_cva) {
    out_cva->write_value = rte_cva_write_value;
    out_cva->register_on_change = rte_cva_register_on_change;
    out_cva->register_on_touched = rte_cva_register_on_touched;
    out_cva->set_disabled_state = rte_cva_set_disabled_state;
  }

  *out_rte = rte;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    (void)ui_dom_node_destroy(root_node);
  }
  if (rte && rte->component) {
    (void)ui_component_destroy(rte->component);
  }
  C_MULTIPLATFORM_FREE(rte);
  return rc;
}

ui_error_t
ui_rich_text_editor_base_destroy(struct ui_rich_text_editor_base *rte) {
  if (!rte) {
    return UI_ERROR_NONE;
  }
  if (rte->component) {
    (void)ui_component_destroy(rte->component);
  }
  if (rte->html_buffer) {
    C_MULTIPLATFORM_FREE(rte->html_buffer);
  }
  C_MULTIPLATFORM_FREE(rte);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_rich_text_editor_base_get_component(struct ui_rich_text_editor_base *rte,
                                       struct ui_component **out_component) {
  if (!rte || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = rte->component;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_rich_text_editor_base_insert_text(struct ui_rich_text_editor_base *rte,
                                     const char *text) {
  size_t cur_len;
  size_t insert_len;

  if (!rte || !text)
    return UI_ERROR_INVALID_ARGUMENT;

  if (rte->is_disabled)
    return UI_ERROR_NONE;

  {
    ui_error_t t_rc = trigger_cva_touched(rte);
    if (t_rc != UI_ERROR_NONE)
      return t_rc;
  }

  cur_len = rte->html_buffer ? strlen(rte->html_buffer) : 0;
  insert_len = strlen(text);

  if (cur_len + insert_len + 1 > rte->html_capacity) {
    size_t new_cap = rte->html_capacity == 0 ? 128 : rte->html_capacity;
    char *new_buf;
    while (new_cap < cur_len + insert_len + 1) {
      new_cap *= 2;
    }
    new_buf = (char *)C_MULTIPLATFORM_REALLOC(rte->html_buffer, new_cap);
    if (!new_buf) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    rte->html_buffer = new_buf;
    rte->html_capacity = new_cap;
  }

  /* Note: Simple append for now, ignoring caret position in this stub */
  if (cur_len == 0 && rte->html_buffer) {
    rte->html_buffer[0] = '\0';
  }
  strcat(rte->html_buffer, text);

  {
    ui_error_t c_rc = trigger_cva_change(rte);
    if (c_rc != UI_ERROR_NONE)
      return c_rc;
  }

  return UI_ERROR_NONE;
}

/** \brief ui_rich_text_editor_base_set_caret_from_point */
ui_error_t ui_rich_text_editor_base_set_caret_from_point(
    struct ui_rich_text_editor_base *rte, float x, float y) {
  (void)x;
  (void)y;
  if (!rte)
    return UI_ERROR_INVALID_ARGUMENT;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_rich_text_editor_base_undo(struct ui_rich_text_editor_base *rte) {
  if (!rte)
    return UI_ERROR_INVALID_ARGUMENT;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_rich_text_editor_base_redo(struct ui_rich_text_editor_base *rte) {
  if (!rte)
    return UI_ERROR_INVALID_ARGUMENT;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_rich_text_editor_base_ime_start(struct ui_rich_text_editor_base *rte) {
  if (!rte)
    return UI_ERROR_INVALID_ARGUMENT;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_rich_text_editor_base_ime_update(struct ui_rich_text_editor_base *rte,
                                    const char *composition) {
  (void)composition;
  if (!rte)
    return UI_ERROR_INVALID_ARGUMENT;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_rich_text_editor_base_ime_end(struct ui_rich_text_editor_base *rte) {
  if (!rte)
    return UI_ERROR_INVALID_ARGUMENT;
  return UI_ERROR_NONE;
}
