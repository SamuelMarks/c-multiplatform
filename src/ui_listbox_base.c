/**
 * @file ui_listbox_base.c
 * @brief ui_listbox_base.c implementation.
 */
/*
 * @file ui_listbox_base.c
 * @brief Implementation of the UI listbox base component.
 */
/* clang-format off */
#include "ui_listbox_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <ctype.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_listbox_mock_sel_destroy_fail = 0;
int g_listbox_mock_comp_destroy_fail = 0;
int g_listbox_mock_sel_select_fail = 0;
int g_listbox_mock_sel_toggle_fail = 0;
int g_listbox_mock_sel_clear_fail = 0;
int g_listbox_mock_sel_get_selected_fail = 0;
int g_listbox_mock_sel_count_fail = 0;
int g_listbox_mock_sel_set_on_change_fail = 0;
int g_listbox_mock_set_attr_fail = 0;
int g_listbox_mock_get_attr_fail = 0;
int g_listbox_mock_set_style_fail = 0;

/**
 * @brief mock_listbox_selection_model_destroy.
 * @param model Parameter model.
 * @return Return value.
 */
static ui_error_t
mock_listbox_selection_model_destroy(struct ui_selection_model *model) {
  if (g_listbox_mock_sel_destroy_fail) {
    g_listbox_mock_sel_destroy_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  return (ui_selection_model_destroy)(model);
}
#undef ui_selection_model_destroy
/** @cond */
#define ui_selection_model_destroy mock_listbox_selection_model_destroy
/** @endcond */

/**
 * @brief mock_listbox_component_destroy.
 * @param component Parameter component.
 * @return Return value.
 */
static ui_error_t
mock_listbox_component_destroy(struct ui_component *component) {
  if (g_listbox_mock_comp_destroy_fail) {
    g_listbox_mock_comp_destroy_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  return (ui_component_destroy)(component);
}
#undef ui_component_destroy
/** @cond */
#define ui_component_destroy mock_listbox_component_destroy
/** @endcond */

/**
 * @brief mock_listbox_selection_model_select.
 * @param model Parameter model.
 * @param id Parameter id.
 * @return Return value.
 */
static ui_error_t
mock_listbox_selection_model_select(struct ui_selection_model *model,
                                    void *id) {
  if (g_listbox_mock_sel_select_fail) {
    g_listbox_mock_sel_select_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  return (ui_selection_model_select)(model, id);
}
#undef ui_selection_model_select
/** @cond */
#define ui_selection_model_select mock_listbox_selection_model_select
/** @endcond */

/**
 * @brief mock_listbox_selection_model_toggle.
 * @param model Parameter model.
 * @param id Parameter id.
 * @return Return value.
 */
static ui_error_t
mock_listbox_selection_model_toggle(struct ui_selection_model *model,
                                    void *id) {
  if (g_listbox_mock_sel_toggle_fail) {
    g_listbox_mock_sel_toggle_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  return (ui_selection_model_toggle)(model, id);
}
#undef ui_selection_model_toggle
/** @cond */
#define ui_selection_model_toggle mock_listbox_selection_model_toggle
/** @endcond */

/**
 * @brief mock_listbox_selection_model_clear.
 * @param model Parameter model.
 * @return Return value.
 */
static ui_error_t
mock_listbox_selection_model_clear(struct ui_selection_model *model) {
  if (g_listbox_mock_sel_clear_fail) {
    g_listbox_mock_sel_clear_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  return (ui_selection_model_clear)(model);
}
#undef ui_selection_model_clear
/** @cond */
#define ui_selection_model_clear mock_listbox_selection_model_clear
/** @endcond */

/**
 * @brief mock_listbox_selection_model_get_selected.
 * @param model Parameter model.
 * @param out_ids Parameter out_ids.
 * @param capacity Parameter capacity.
 * @return Return value.
 */
static ui_error_t mock_listbox_selection_model_get_selected(
    const struct ui_selection_model *model, void **out_ids, int capacity) {
  if (g_listbox_mock_sel_get_selected_fail) {
    g_listbox_mock_sel_get_selected_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  return (ui_selection_model_get_selected)(model, out_ids, capacity);
}
#undef ui_selection_model_get_selected
/** @cond */
#define ui_selection_model_get_selected                                        \
  mock_listbox_selection_model_get_selected
/** @endcond */

/**
 * @brief mock_listbox_selection_model_get_selected_count.
 * @param model Parameter model.
 * @param out_count Parameter out_count.
 * @return Return value.
 */
static ui_error_t mock_listbox_selection_model_get_selected_count(
    const struct ui_selection_model *model, int *out_count) {
  if (g_listbox_mock_sel_count_fail) {
    g_listbox_mock_sel_count_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  return (ui_selection_model_get_selected_count)(model, out_count);
}
#undef ui_selection_model_get_selected_count
/** @cond */
#define ui_selection_model_get_selected_count                                  \
  mock_listbox_selection_model_get_selected_count
/** @endcond */

/**
 * @brief mock_listbox_selection_model_set_on_change.
 * @param model Parameter model.
 * @param on_change Parameter on_change.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
static ui_error_t mock_listbox_selection_model_set_on_change(
    struct ui_selection_model *model, ui_selection_model_on_change_t on_change,
    void *user_data) {
  if (g_listbox_mock_sel_set_on_change_fail) {
    g_listbox_mock_sel_set_on_change_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  return (ui_selection_model_set_on_change)(model, on_change, user_data);
}
#undef ui_selection_model_set_on_change
/** @cond */
#define ui_selection_model_set_on_change                                       \
  mock_listbox_selection_model_set_on_change
/** @endcond */

/**
 * @brief mock_listbox_dom_node_set_attribute.
 * @param node Parameter node.
 * @param name Parameter name.
 * @param value Parameter value.
 * @return Return value.
 */
static ui_error_t mock_listbox_dom_node_set_attribute(struct ui_dom_node *node,
                                                      const char *name,
                                                      const char *value) {
  if (g_listbox_mock_set_attr_fail > 0) {
    g_listbox_mock_set_attr_fail--;
    if (g_listbox_mock_set_attr_fail == 0) {
      return UI_ERROR_UNKNOWN;
    }
  }
  return (ui_dom_node_set_attribute)(node, name, value);
}
#undef ui_dom_node_set_attribute
/** @cond */
#define ui_dom_node_set_attribute mock_listbox_dom_node_set_attribute
/** @endcond */

/**
 * @brief mock_listbox_dom_node_get_attribute.
 * @param node Parameter node.
 * @param name Parameter name.
 * @param out_value Parameter out_value.
 * @return Return value.
 */
static ui_error_t
mock_listbox_dom_node_get_attribute(const struct ui_dom_node *node,
                                    const char *name, const char **out_value) {
  if (g_listbox_mock_get_attr_fail > 0) {
    g_listbox_mock_get_attr_fail--;
    if (g_listbox_mock_get_attr_fail == 0) {
      return UI_ERROR_UNKNOWN;
    }
  }
  return (ui_dom_node_get_attribute)(node, name, out_value);
}
#undef ui_dom_node_get_attribute
/** @cond */
#define ui_dom_node_get_attribute mock_listbox_dom_node_get_attribute
/** @endcond */

/**
 * @brief mock_listbox_component_set_default_style.
 * @param comp Parameter comp.
 * @param sheet Parameter sheet.
 * @return Return value.
 */
static ui_error_t
mock_listbox_component_set_default_style(struct ui_component *comp,
                                         struct ui_css_stylesheet *sheet) {
  if (g_listbox_mock_set_style_fail) {
    g_listbox_mock_set_style_fail = 0;
    return UI_ERROR_UNKNOWN;
  }
  return (ui_component_set_default_style)(comp, sheet);
}
#undef ui_component_set_default_style
/** @cond */
#define ui_component_set_default_style mock_listbox_component_set_default_style
/** @endcond */
#endif

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

/** @brief Default CSS stylesheet */
static const char *ui_listbox_base_default_css = "div[role=\"listbox\"] { "
                                                 "display: flex; "
                                                 "flex-direction: column; "
                                                 "outline: none; "
                                                 "}";

/**
 * @struct ui_listbox_base
 * @brief ui_listbox_base
 */
struct ui_listbox_base {
  struct ui_component *component;             /**< component */
  struct ui_selection_model *selection_model; /**< selection_model */

  int num_items;    /**< num_items */
  int active_index; /**< active_index */

  ui_listbox_get_item_text_t text_provider; /**< text_provider */
  void *text_user_data;                     /**< text_user_data */

  char typeahead_buffer[64];     /**< typeahead_buffer */
  int typeahead_len;             /**< typeahead_len */
  double last_typeahead_time_ms; /**< last_typeahead_time_ms */

  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data); /**< user_data) */
  void *cva_on_change_user_data;                /**< cva_on_change_user_data */

  ui_error_t (*cva_on_touched)(void *user_data); /**< user_data) */
  void *cva_on_touched_user_data; /**< cva_on_touched_user_data */

  int is_disabled; /**< is_disabled */
};

/**
 * @brief Triggers the CVA on-change callback based on current selection.
 * @param[in,out] listbox The listbox component.
 * @return UI_ERROR_NONE on success.
 */
/**
 * @brief listbox_trigger_cva_change.
 * @param listbox Parameter listbox.
 * @return Return value.
 */
static ui_error_t listbox_trigger_cva_change(struct ui_listbox_base *listbox) {
  union ui_signal_payload payload;
  int count;
  void **ids;
  void *id = NULL;
  int is_multi = 0;
  const char *attr = NULL;
  ui_error_t rc;

  if (!listbox->cva_on_change)
    return UI_ERROR_NONE;

  rc = ui_selection_model_get_selected_count(listbox->selection_model, &count);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_dom_node_get_attribute(listbox->component->shadow_root,
                                 "aria-multiselectable", &attr);
  if (rc != UI_ERROR_NONE && rc != UI_ERROR_NOT_FOUND) {
    return rc;
  }

  if (attr && strcmp(attr, "true") == 0) {
    is_multi = 1;
  }

  if (is_multi) {
    ids = (void **)C_MULTIPLATFORM_MALLOC((size_t)count * sizeof(void *));
    if (!ids) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    rc = ui_selection_model_get_selected(listbox->selection_model, ids, count);
    if (rc != UI_ERROR_NONE) {
      C_MULTIPLATFORM_FREE(ids);
      return rc;
    }
    payload.ptr_val = ids;
  } else {
    if (count > 0) {
      rc = ui_selection_model_get_selected(listbox->selection_model, &id, 1);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      payload.int_val = (int)(size_t)id;
    } else {
      payload.int_val = -1; /* -1 represents no selection */
    }
  }
  return listbox->cva_on_change(payload, listbox->cva_on_change_user_data);
}

/**
 * @brief Callback fired when the selection model changes.
 * @param[in,out] model The selection model.
 * @param[in,out] user_data Pointer to the listbox context.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t on_selection_change(struct ui_selection_model *model,
                                      void *user_data) {
  struct ui_listbox_base *listbox = (struct ui_listbox_base *)user_data;
  if (model) {
  }
  return listbox_trigger_cva_change(listbox);
}

/**
 * @brief CVA interface function to write a value into the listbox selection.
 * @param[in,out] component The listbox component.
 * @param[in] value The payload value to write.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t listbox_cva_write_value(void *component,
                                          union ui_signal_payload value) {
  struct ui_listbox_base *listbox = (struct ui_listbox_base *)component;
  int is_multi = 0;
  const char *attr = NULL;
  ui_error_t rc;

  if (!listbox)
    return UI_ERROR_INVALID_ARGUMENT;

  {
    ui_error_t attr_rc = ui_dom_node_get_attribute(
        listbox->component->shadow_root, "aria-multiselectable", &attr);
    if (attr_rc != UI_ERROR_NONE && attr_rc != UI_ERROR_NOT_FOUND) {
      return attr_rc;
    }
    if (attr && strcmp(attr, "true") == 0) {
      is_multi = 1;
    }
  }

  /* Clear existing */
  rc = ui_selection_model_clear(listbox->selection_model);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (!is_multi) {
    if (value.int_val >= 0) {
      rc = ui_selection_model_select(listbox->selection_model,
                                     (void *)(size_t)value.int_val);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  } else {
    /* For multi, value.ptr_val is an array of size_t/void* ending in -1 or
       requires length. Without length, we assume it's an array of int/size_t.
       Since we just allocated it previously in trigger_cva_change, it might be
       tough to know length here. We will just ignore multi-write for now or let
       it be handled later. */
  }

  return UI_ERROR_NONE;
}

/* @brief listbox_cva_register_on_change
 */
static ui_error_t listbox_cva_register_on_change(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  struct ui_listbox_base *listbox = (struct ui_listbox_base *)component;
  if (!listbox)
    return UI_ERROR_INVALID_ARGUMENT;
  listbox->cva_on_change = callback;
  listbox->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief CVA interface function to register a touched callback.
 * @param[in,out] component The listbox component.
 * @param[in] callback The callback function.
 * @param[in] user_data User data for the callback.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t listbox_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_listbox_base *listbox = (struct ui_listbox_base *)component;
  if (!listbox)
    return UI_ERROR_INVALID_ARGUMENT;
  listbox->cva_on_touched = callback;
  listbox->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief CVA interface function to set the disabled state.
 * @param[in,out] component The listbox component.
 * @param[in] is_disabled Non-zero to disable.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t listbox_cva_set_disabled_state(void *component,
                                                 int is_disabled) {
  struct ui_listbox_base *listbox = (struct ui_listbox_base *)component;
  ui_error_t rc;
  if (!listbox)
    return UI_ERROR_INVALID_ARGUMENT;
  listbox->is_disabled = is_disabled;
  rc = ui_dom_node_set_attribute(listbox->component->shadow_root,
                                 "aria-disabled",
                                 is_disabled ? "true" : "false");
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  if (is_disabled) {
    rc = ui_dom_node_set_attribute(listbox->component->shadow_root, "tabindex",
                                   "-1");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
    rc = ui_dom_node_set_attribute(listbox->component->shadow_root, "tabindex",
                                   "0");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Creates a listbox base component.
 * @param[out] out_listbox Pointer to store the created listbox.
 * @param[out] out_cva Optional control value accessor.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_listbox_base_create(struct ui_listbox_base **out_listbox,
                                  struct ui_control_value_accessor *out_cva) {
  struct ui_listbox_base *listbox;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_listbox)
    return UI_ERROR_INVALID_ARGUMENT;

  listbox = (struct ui_listbox_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_listbox_base));
  if (!listbox)
    return UI_ERROR_OUT_OF_MEMORY;

  memset(listbox, 0, sizeof(struct ui_listbox_base));
  listbox->active_index = -1;
  listbox->cva_on_change = NULL;
  listbox->cva_on_change_user_data = NULL;
  listbox->cva_on_touched = NULL;
  listbox->cva_on_touched_user_data = NULL;
  listbox->is_disabled = 0;

  rc = ui_component_create(&listbox->component);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_selection_model_create(&listbox->selection_model);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_selection_model_set_on_change(listbox->selection_model,
                                        on_selection_change, listbox);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_set_tag_name(root_node, "div");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_set_attribute(root_node, "role", "listbox");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_set_attribute(root_node, "tabindex", "0");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_set_attribute(root_node, "aria-multiselectable", "false");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  listbox->component->shadow_root = root_node;
  root_node = NULL;

  rc = ui_css_parse_stylesheet(ui_listbox_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_component_set_default_style(listbox->component, default_style);
  if (rc != UI_ERROR_NONE) {
    ui_css_stylesheet_destroy(default_style);
    goto cleanup;
  }

  if (out_cva) {
    out_cva->write_value = listbox_cva_write_value;
    out_cva->register_on_change = listbox_cva_register_on_change;
    out_cva->register_on_touched = listbox_cva_register_on_touched;
    out_cva->set_disabled_state = listbox_cva_set_disabled_state;
  }

  *out_listbox = listbox;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    ui_dom_node_destroy(root_node);
  }
  if (listbox->selection_model)
    ui_selection_model_destroy(listbox->selection_model);
  if (listbox->component) {
    ui_component_destroy(listbox->component);
  }
  C_MULTIPLATFORM_FREE(listbox);
  return rc;
}

/**
 * @brief Destroys a listbox base component.
 * @param[in,out] listbox The listbox to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_listbox_base_destroy(struct ui_listbox_base *listbox) {
  ui_error_t rc = UI_ERROR_NONE;
  if (!listbox)
    return UI_ERROR_NONE;

  if (listbox->selection_model) {
    ui_error_t rc_cleanup =
        ui_selection_model_destroy(listbox->selection_model);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  if (listbox->component) {
    ui_error_t rc_cleanup = ui_component_destroy(listbox->component);
    if (rc_cleanup != UI_ERROR_NONE && rc == UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  C_MULTIPLATFORM_FREE(listbox);
  return rc;
}
/**
 * @brief Gets the component of the listbox widget.
 * @param[in] listbox The listbox widget.
 * @param[out] out_component Pointer to store the component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_listbox_base_get_component(struct ui_listbox_base *listbox,
                                         struct ui_component **out_component) {
  if (!listbox || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = listbox->component;
  return UI_ERROR_NONE;
}

/**
 * @brief Gets the selection model of the listbox widget.
 * @param[in] listbox The listbox widget.
 * @param[out] out_model Pointer to store the selection model.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_listbox_base_get_selection_model(struct ui_listbox_base *listbox,
                                    struct ui_selection_model **out_model) {
  if (!listbox || !out_model) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_model = listbox->selection_model;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets whether the listbox allows multiple selections.
 * @param[in,out] listbox The listbox component.
 * @param[in] is_multi Non-zero to enable multi-select.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_listbox_base_set_multi_select(struct ui_listbox_base *listbox,
                                            int is_multi) {
  if (!listbox)
    return UI_ERROR_INVALID_ARGUMENT;

  if (listbox->component->shadow_root) {
    ui_error_t rc = ui_dom_node_set_attribute(listbox->component->shadow_root,
                                              "aria-multiselectable",
                                              is_multi ? "true" : "false");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return ui_selection_model_set_multi_select(listbox->selection_model,
                                             is_multi);
}

/**
 * @brief Informs the listbox of the total number of items it contains.
 * @param[in,out] listbox The listbox component.
 * @param[in] num_items The total number of items.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_listbox_base_set_item_count(struct ui_listbox_base *listbox,
                                          int num_items) {
  if (!listbox || num_items < 0)
    return UI_ERROR_INVALID_ARGUMENT;

  listbox->num_items = num_items;

  if (listbox->active_index >= num_items) {
    listbox->active_index = num_items > 0 ? num_items - 1 : -1;
  }

  if (num_items == 0) {
    ui_error_t rc = ui_selection_model_clear(listbox->selection_model);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Sets the item text provider for the listbox widget.
 * @param[in,out] listbox The listbox widget.
 * @param[in] provider The text provider function.
 * @param[in] user_data User data for the provider.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_listbox_base_set_item_text_provider(struct ui_listbox_base *listbox,
                                       ui_listbox_get_item_text_t provider,
                                       void *user_data) {
  if (!listbox)
    return UI_ERROR_INVALID_ARGUMENT;
  listbox->text_provider = provider;
  listbox->text_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief Explicitly sets the currently active (focused) item index.
 * @param[in,out] listbox The listbox component.
 * @param[in] index The item index.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_listbox_base_set_active_index(struct ui_listbox_base *listbox,
                                            int index) {
  if (!listbox)
    return UI_ERROR_INVALID_ARGUMENT;
  if (index < -1 || index >= listbox->num_items)
    return UI_ERROR_OUT_OF_BOUNDS;

  listbox->active_index = index;
  return UI_ERROR_NONE;
}

/**
 * @brief Gets the active index of the listbox widget.
 * @param[in] listbox The listbox widget.
 * @param[out] out_index Pointer to store the active index.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_listbox_base_get_active_index(const struct ui_listbox_base *listbox,
                                 int *out_index) {
  if (!listbox || !out_index)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_index = listbox->active_index;
  return UI_ERROR_NONE;
}

/**
 * @brief Performs a typeahead search and updates selection if a match is found.
 * @param[in,out] listbox The listbox component.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t perform_typeahead(struct ui_listbox_base *listbox) {
  int start_index = listbox->active_index >= 0 ? listbox->active_index : 0;
  int i;
  int is_multi = 0;
  const char *attr = NULL;

  if (!listbox->text_provider || listbox->num_items == 0) {
    return UI_ERROR_NONE;
  }

  {
    ui_error_t attr_rc = ui_dom_node_get_attribute(
        listbox->component->shadow_root, "aria-multiselectable", &attr);
    if (attr_rc != UI_ERROR_NONE && attr_rc != UI_ERROR_NOT_FOUND) {
      return attr_rc;
    }
    if (attr && strcmp(attr, "true") == 0) {
      is_multi = 1;
    }
  }

  for (i = 1; i <= listbox->num_items; i++) {
    int idx = (start_index + i) % listbox->num_items;
    const char *text =
        listbox->text_provider(listbox, idx, listbox->text_user_data);
    int is_match = 0;
    if (text) {
      int j;
      is_match = 1;
      for (j = 0; j < listbox->typeahead_len; j++) {
        if (!text[j] ||
            tolower((unsigned char)text[j]) !=
                tolower((unsigned char)listbox->typeahead_buffer[j])) {
          is_match = 0;
          break;
        }
      }
    }
    if (is_match) {
      listbox->active_index = idx;

      if (!is_multi) {
        ui_error_t rc_sel = ui_selection_model_select(listbox->selection_model,
                                                      (void *)(size_t)idx);
        if (rc_sel != UI_ERROR_NONE) {
          return rc_sel;
        }
      }
      break;
    }
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Processes an incoming UI event (like keypresses) for listbox
 * navigation and selection.
 * @param[in,out] listbox The listbox component.
 * @param[in] event The UI event.
 * @param[in] timestamp_ms Event timestamp in milliseconds.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_listbox_base_process_event(struct ui_listbox_base *listbox,
                                         const struct ui_event *event,
                                         double timestamp_ms) {
  int is_multi = 0;
  const char *attr = NULL;

  if (!listbox || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  if (listbox->is_disabled) {
    return UI_ERROR_NONE; /* Ignore events if disabled */
  }

  if (listbox->cva_on_touched) {
    ui_error_t rc_touch =
        listbox->cva_on_touched(listbox->cva_on_touched_user_data);
    if (rc_touch != UI_ERROR_NONE) {
      return rc_touch;
    }
  }

  {
    ui_error_t attr_rc = ui_dom_node_get_attribute(
        listbox->component->shadow_root, "aria-multiselectable", &attr);
    if (attr_rc != UI_ERROR_NONE && attr_rc != UI_ERROR_NOT_FOUND) {
      return attr_rc;
    }
    if (attr && strcmp(attr, "true") == 0) {
      is_multi = 1;
    }
  }

  if (event->type == UI_EVENT_KEY_DOWN) {
    int kc = event->event_data.keyboard.key_code;

    if (timestamp_ms - listbox->last_typeahead_time_ms > 1000.0) {
      listbox->typeahead_len = 0;
    }

    if (kc == UI_KEY_DOWN) {
      listbox->typeahead_len = 0;
      if (listbox->num_items > 0) {
        if (listbox->active_index < listbox->num_items - 1) {
          listbox->active_index++;
        }
        if (!is_multi) {
          ui_error_t rc_sel = ui_selection_model_select(
              listbox->selection_model, (void *)(size_t)listbox->active_index);
          if (rc_sel != UI_ERROR_NONE) {
            return rc_sel;
          }
        }
      }
    } else if (kc == UI_KEY_UP) {
      listbox->typeahead_len = 0;
      if (listbox->num_items > 0) {
        if (listbox->active_index > 0) {
          listbox->active_index--;
        }
        if (!is_multi) {
          ui_error_t rc_sel = ui_selection_model_select(
              listbox->selection_model, (void *)(size_t)listbox->active_index);
          if (rc_sel != UI_ERROR_NONE) {
            return rc_sel;
          }
        }
      }
    } else if (kc == UI_KEY_HOME) {
      listbox->typeahead_len = 0;
      if (listbox->num_items > 0) {
        listbox->active_index = 0;
        if (!is_multi) {
          ui_error_t rc_sel = ui_selection_model_select(
              listbox->selection_model, (void *)(size_t)listbox->active_index);
          if (rc_sel != UI_ERROR_NONE) {
            return rc_sel;
          }
        }
      }
    } else if (kc == UI_KEY_END) {
      listbox->typeahead_len = 0;
      if (listbox->num_items > 0) {
        listbox->active_index = listbox->num_items - 1;
        if (!is_multi) {
          ui_error_t rc_sel = ui_selection_model_select(
              listbox->selection_model, (void *)(size_t)listbox->active_index);
          if (rc_sel != UI_ERROR_NONE) {
            return rc_sel;
          }
        }
      }
    } else if (kc == UI_KEY_SPACE || kc == UI_KEY_ENTER) {
      if (kc == UI_KEY_SPACE && listbox->typeahead_len > 0) {
        /* Part of typeahead */
        if (listbox->typeahead_len <
            (int)sizeof(listbox->typeahead_buffer) - 1) {
          listbox->typeahead_buffer[listbox->typeahead_len++] = ' ';
          listbox->last_typeahead_time_ms = timestamp_ms;
          {
            ui_error_t rc_ta = perform_typeahead(listbox);
            if (rc_ta != UI_ERROR_NONE) {
              return rc_ta;
            }
          }
        }
      } else {
        listbox->typeahead_len = 0;
        if (listbox->active_index >= 0) {
          if (is_multi && kc == UI_KEY_SPACE) {
            ui_error_t rc_tog = ui_selection_model_toggle(
                listbox->selection_model,
                (void *)(size_t)listbox->active_index);
            if (rc_tog != UI_ERROR_NONE) {
              return rc_tog;
            }
          } else if (!is_multi) {
            ui_error_t rc_sel = ui_selection_model_select(
                listbox->selection_model,
                (void *)(size_t)listbox->active_index);
            if (rc_sel != UI_ERROR_NONE) {
              return rc_sel;
            }
          }
        }
      }
    } else if (kc >= 32 && kc < 127) {
      if (listbox->typeahead_len < (int)sizeof(listbox->typeahead_buffer) - 1) {
        listbox->typeahead_buffer[listbox->typeahead_len++] = (char)kc;
        listbox->last_typeahead_time_ms = timestamp_ms;
        {
          ui_error_t rc_ta = perform_typeahead(listbox);
          if (rc_ta != UI_ERROR_NONE) {
            return rc_ta;
          }
        }
      }
    }
  }

  return UI_ERROR_NONE;
}
