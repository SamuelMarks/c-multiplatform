
/* clang-format off */
#include "ui_autocomplete_base.h"
#include "ui_internal_mem.h"
/* clang-format on */

struct ui_autocomplete_base {
  struct ui_component *root_component;
  struct ui_input_base *input;
  struct ui_listbox_base *listbox;
  struct ui_popover_base *popover;

  struct ui_overlay_director *overlay_director;
  struct ui_focus_manager *focus_manager;

  ui_autocomplete_on_selection_t on_selection;
  void *selection_user_data;

  ui_autocomplete_on_text_change_t on_text_change;
  void *text_change_user_data;

  enum ui_error (*cva_on_change)(union ui_signal_payload new_value,
                                 void *user_data);
  void *cva_on_change_user_data;

  enum ui_error (*cva_on_touched)(void *user_data);
  void *cva_on_touched_user_data;
};

static enum ui_error on_input_text_change(struct ui_input_base *input,
                                          const char *text, void *user_data) {
  struct ui_autocomplete_base *ac = (struct ui_autocomplete_base *)user_data;
  union ui_signal_payload payload;
  enum ui_error rc = UI_ERROR_NONE;
  (void)input;

  if (ac->cva_on_change) {
    payload.ptr_val = (void *)text;
    rc = ac->cva_on_change(payload, ac->cva_on_change_user_data);
  }

  if (ac->on_text_change) {
    ac->on_text_change(ac, text, ac->text_change_user_data);
  }
  return rc;
}

static enum ui_error
autocomplete_cva_write_value(void *component, union ui_signal_payload value) {
  struct ui_autocomplete_base *ac = (struct ui_autocomplete_base *)component;
  if (!ac)
    return UI_ERROR_INVALID_ARGUMENT;
  if (value.ptr_val) {
    return ui_input_base_set_text(ac->input, (const char *)value.ptr_val);
  } else {
    return ui_input_base_set_text(ac->input, "");
  }
}

/** \brief autocomplete_cva_register_on_change */
static enum ui_error autocomplete_cva_register_on_change(
    void *component,
    enum ui_error (*callback)(union ui_signal_payload new_value,
                              void *user_data),
    void *user_data) {
  struct ui_autocomplete_base *ac = (struct ui_autocomplete_base *)component;
  if (!ac)
    return UI_ERROR_INVALID_ARGUMENT;
  ac->cva_on_change = callback;
  ac->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

static enum ui_error
autocomplete_cva_register_on_touched(void *component,
                                     enum ui_error (*callback)(void *user_data),
                                     void *user_data) {
  struct ui_autocomplete_base *ac = (struct ui_autocomplete_base *)component;
  if (!ac)
    return UI_ERROR_INVALID_ARGUMENT;
  ac->cva_on_touched = callback;
  ac->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

static enum ui_error autocomplete_cva_set_disabled_state(void *component,
                                                         int is_disabled) {
  struct ui_autocomplete_base *ac = (struct ui_autocomplete_base *)component;
  if (!ac)
    return UI_ERROR_INVALID_ARGUMENT;
  return ui_input_base_set_disabled(ac->input, is_disabled);
}

#if 0
static void on_listbox_selection_change(struct ui_selection_model* model, void* user_data) {
    struct ui_autocomplete_base* ac = (struct ui_autocomplete_base*)user_data;
    int index = -1;
    ui_listbox_base_get_active_index(ac->listbox, &index);
    int is_selected = 0;

    if (index >= 0) {
        ui_selection_model_is_selected(model, (void*)(size_t)index, &is_selected);
        if (is_selected) {
            if (ac->on_selection) {
                ac->on_selection(ac, index, ac->selection_user_data);
            }
            ui_autocomplete_base_close(ac);
        }
    }
}
#endif

/** \brief ui_error */
enum ui_error
ui_autocomplete_base_create(struct ui_autocomplete_base **out_autocomplete,
                            struct ui_control_value_accessor *out_cva) {
  struct ui_autocomplete_base *ac;
  enum ui_error rc;
  struct ui_dom_node *root_node = NULL;

  if (!out_autocomplete)
    return UI_ERROR_INVALID_ARGUMENT;

  ac = (struct ui_autocomplete_base *)UI_MALLOC(
      sizeof(struct ui_autocomplete_base));
  if (!ac)
    return UI_ERROR_OUT_OF_MEMORY;

  ac->root_component = NULL;
  ac->input = NULL;
  ac->listbox = NULL;
  ac->popover = NULL;
  ac->overlay_director = NULL;
  ac->focus_manager = NULL;
  ac->on_selection = NULL;
  ac->selection_user_data = NULL;
  ac->on_text_change = NULL;
  ac->text_change_user_data = NULL;
  ac->cva_on_change = NULL;
  ac->cva_on_change_user_data = NULL;
  ac->cva_on_touched = NULL;
  ac->cva_on_touched_user_data = NULL;

  rc = ui_component_create(&ac->root_component);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  ui_dom_node_set_tag_name(root_node, "div");
  ui_dom_node_set_attribute(root_node, "role", "combobox");
  ui_dom_node_set_attribute(root_node, "aria-expanded", "false");
  ui_dom_node_set_attribute(root_node, "aria-haspopup", "listbox");

  ac->root_component->shadow_root = root_node;
  root_node = NULL;

  rc = ui_input_base_create(&ac->input);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  {
    struct ui_component *tmp_comp;
    ui_input_base_get_component(ac->input, &tmp_comp);
    ui_dom_node_append_child(ac->root_component->shadow_root,
                             tmp_comp->shadow_root);
  }

  rc = ui_listbox_base_create(&ac->listbox, NULL);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_popover_base_create(&ac->popover);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  (void)ui_input_base_set_on_change(ac->input, on_input_text_change, ac);
  /* TODO: hook up selection model on_change */

  if (out_cva) {
    out_cva->write_value = autocomplete_cva_write_value;
    out_cva->register_on_change = autocomplete_cva_register_on_change;
    out_cva->register_on_touched = autocomplete_cva_register_on_touched;
    out_cva->set_disabled_state = autocomplete_cva_set_disabled_state;
  }

  *out_autocomplete = ac;
  return UI_ERROR_NONE;

cleanup:
  if (ac->listbox)
    ui_listbox_base_destroy(ac->listbox);
  if (ac->input) {
    struct ui_component *tmp_comp;
    ui_input_base_get_component(ac->input, &tmp_comp);
    ui_dom_node_remove_child(ac->root_component->shadow_root,
                             tmp_comp->shadow_root);
    ui_input_base_destroy(ac->input);
  }
  if (ac->root_component)
    ui_component_destroy(ac->root_component);
  UI_FREE(ac);
  return rc;
}

void ui_autocomplete_base_destroy(struct ui_autocomplete_base *autocomplete) {
  struct ui_component *tmp_comp;
  if (!autocomplete)
    return;

  ui_popover_base_destroy(autocomplete->popover);
  ui_listbox_base_destroy(autocomplete->listbox);

  ui_input_base_get_component(autocomplete->input, &tmp_comp);
  ui_dom_node_remove_child(autocomplete->root_component->shadow_root,
                           tmp_comp->shadow_root);
  ui_input_base_destroy(autocomplete->input);
  ui_component_destroy(autocomplete->root_component);

  UI_FREE(autocomplete);
}

/** \brief ui_error */
enum ui_error
ui_autocomplete_base_get_component(struct ui_autocomplete_base *autocomplete,
                                   struct ui_component **out_component) {
  if (!autocomplete || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = autocomplete->root_component;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_autocomplete_base_get_input(struct ui_autocomplete_base *autocomplete,
                               struct ui_input_base **out_input) {
  if (!autocomplete || !out_input) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_input = autocomplete->input;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_autocomplete_base_get_listbox(struct ui_autocomplete_base *autocomplete,
                                 struct ui_listbox_base **out_listbox) {
  if (!autocomplete || !out_listbox) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_listbox = autocomplete->listbox;
  return UI_ERROR_NONE;
}

/** \brief ui_autocomplete_base_set_overlay_dependencies */
enum ui_error ui_autocomplete_base_set_overlay_dependencies(
    struct ui_autocomplete_base *autocomplete,
    struct ui_overlay_director *director, struct ui_focus_manager *focus_mgr) {

  if (!autocomplete)
    return UI_ERROR_INVALID_ARGUMENT;
  autocomplete->overlay_director = director;
  autocomplete->focus_manager = focus_mgr;
  return UI_ERROR_NONE;
}

/** \brief ui_autocomplete_base_set_on_text_change */
enum ui_error ui_autocomplete_base_set_on_text_change(
    struct ui_autocomplete_base *autocomplete,
    ui_autocomplete_on_text_change_t on_text_change, void *user_data) {

  if (!autocomplete)
    return UI_ERROR_INVALID_ARGUMENT;
  autocomplete->on_text_change = on_text_change;
  autocomplete->text_change_user_data = user_data;
  return UI_ERROR_NONE;
}

/** \brief ui_autocomplete_base_set_on_selection */
enum ui_error ui_autocomplete_base_set_on_selection(
    struct ui_autocomplete_base *autocomplete,
    ui_autocomplete_on_selection_t on_selection, void *user_data) {

  if (!autocomplete)
    return UI_ERROR_INVALID_ARGUMENT;
  autocomplete->on_selection = on_selection;
  autocomplete->selection_user_data = user_data;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_autocomplete_base_open(struct ui_autocomplete_base *autocomplete,
                          const struct ui_layout_node *trigger_layout,
                          float viewport_width, float viewport_height) {

  struct ui_anchor_config anchor;
  enum ui_error rc;
  int is_open = 0;

  if (!autocomplete || !trigger_layout)
    return UI_ERROR_INVALID_ARGUMENT;

  ui_popover_base_is_open(autocomplete->popover, &is_open);
  if (is_open) {
    return UI_ERROR_NONE; /* Already open */
  }

  anchor.target_x = UI_ANCHOR_EDGE_START;
  anchor.target_y = UI_ANCHOR_EDGE_END;
  anchor.overlay_x = UI_ANCHOR_EDGE_START;
  anchor.overlay_y = UI_ANCHOR_EDGE_START;
  anchor.offset_x = 0;
  anchor.offset_y = 4;

  {
    struct ui_component *tmp_comp;
    ui_listbox_base_get_component(autocomplete->listbox, &tmp_comp);

    rc = ui_popover_base_open(autocomplete->popover, tmp_comp->shadow_root,
                              autocomplete->overlay_director,
                              autocomplete->focus_manager, trigger_layout,
                              &anchor, viewport_width, viewport_height);
  }

  ui_dom_node_set_attribute(autocomplete->root_component->shadow_root,
                            "aria-expanded", "true");

  return rc;
}

/** \brief ui_error */
enum ui_error
ui_autocomplete_base_close(struct ui_autocomplete_base *autocomplete) {
  int is_open = 0;

  if (!autocomplete)
    return UI_ERROR_INVALID_ARGUMENT;

  ui_popover_base_is_open(autocomplete->popover, &is_open);
  if (is_open) {
    ui_popover_base_close(autocomplete->popover);
    ui_dom_node_set_attribute(autocomplete->root_component->shadow_root,
                              "aria-expanded", "false");
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_autocomplete_base_process_event(struct ui_autocomplete_base *autocomplete,
                                   const struct ui_event *event,
                                   double timestamp_ms) {
  int is_open = 0;

  if (!autocomplete || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  ui_popover_base_is_open(autocomplete->popover, &is_open);
  if (is_open) {
    /* Route click-outside events to popover base */
    ui_popover_base_process_event(autocomplete->popover, event);

    ui_popover_base_is_open(autocomplete->popover, &is_open);
    if (!is_open) {
      /* Popover decided to close (e.g. click outside) */
      ui_dom_node_set_attribute(autocomplete->root_component->shadow_root,
                                "aria-expanded", "false");
      return UI_ERROR_NONE;
    }

    if (event->type == UI_EVENT_KEY_DOWN) {
      int kc = event->event_data.keyboard.key_code;
      if (kc == UI_KEY_UP || kc == UI_KEY_DOWN || kc == UI_KEY_HOME ||
          kc == UI_KEY_END) {
        return ui_listbox_base_process_event(autocomplete->listbox, event,
                                             timestamp_ms);
      }

      if (kc == UI_KEY_ENTER) {
        int active = -1;
        ui_listbox_base_get_active_index(autocomplete->listbox, &active);
        if (active >= 0) {
          struct ui_selection_model *model = NULL;
          ui_listbox_base_get_selection_model(autocomplete->listbox, &model);
          ui_selection_model_select(model, (void *)(size_t)active);
        }
        return UI_ERROR_NONE;
      }
    }
  }

  /* Fallback: input processes text characters */
  return ui_input_base_process_event(autocomplete->input, event, timestamp_ms);
}
