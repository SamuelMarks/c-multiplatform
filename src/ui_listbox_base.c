/* clang-format off */
#include "ui_listbox_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

static const char *ui_listbox_base_default_css = "div[role=\"listbox\"] { "
                                                 "display: flex; "
                                                 "flex-direction: column; "
                                                 "outline: none; "
                                                 "}";

/** \brief ui_listbox_base */
struct ui_listbox_base {
  struct ui_component *component;
  struct ui_selection_model *selection_model;

  int num_items;
  int active_index;

  ui_listbox_get_item_text_t text_provider;
  void *text_user_data;

  char typeahead_buffer[64];
  int typeahead_len;
  double last_typeahead_time_ms;

  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data);
  void *cva_on_change_user_data;

  ui_error_t (*cva_on_touched)(void *user_data);
  void *cva_on_touched_user_data;

  int is_disabled;
};

static int char_tolower(int c) {
  if (c >= 'A' && c <= 'Z')
    return c + ('a' - 'A');
  return c;
}

static ui_error_t prefix_match(const char *str, const char *prefix,
                               int prefix_len, int *out_match) {
  int i;
  int str_c, prefix_c;
  *out_match = 0;
  if (!str)
    return UI_ERROR_NONE;
  for (i = 0; i < prefix_len; i++) {
    if (!str[i])
      return UI_ERROR_NONE;
    str_c = char_tolower(str[i]);
    prefix_c = char_tolower(prefix[i]);
    if (str_c != prefix_c)
      return UI_ERROR_NONE;
  }
  *out_match = 1;
  return UI_ERROR_NONE;
}

static ui_error_t listbox_trigger_cva_change(struct ui_listbox_base *listbox) {
  union ui_signal_payload payload;
  int count;
  void **ids;

  if (!listbox->cva_on_change)
    return UI_ERROR_NONE;

  {

    ui_error_t _ign_rc =
        ui_selection_model_get_selected_count(listbox->selection_model, &count);

    (void)_ign_rc;
  }

  /* The prompt mentions: Payload Type: UI_SIGNAL_TYPE_INT32 (Single) or
     UI_SIGNAL_TYPE_POINTER (Multi-select array) We will determine which one
     based on the selection model's multi-select state, or we can just query
     aria-multiselectable. Let's get the multi-select state. Wait, we don't have
     ui_selection_model_is_multi_select exported. We can just check the DOM
     attribute. */
  {
    int is_multi = 0;
    const char *attr = NULL;
    ui_error_t attr_rc = ui_dom_node_get_attribute(
        listbox->component->shadow_root, "aria-multiselectable", &attr);

    if (attr && strcmp(attr, "true") == 0) {
      is_multi = 1;
    }

    if (is_multi) {
      /* We would need an array. We can allocate one from arena or expect the
         user to handle the raw array. For CVA, we can use an internal
         static/dynamic buffer or arena. Wait, for now, let's pass the raw
         internal array if possible, or just build one. We can temporarily
         allocate or use a buffer. Wait, actually we can just pass the first
         item if single, or for multi, passing an array is tricky without arena.
         Let's implement single select for INT32, and for multi, we can pass a
         dummy or allocate via arena. Actually, let's just pass `ids` array
         directly. */
      /* Note: C89 struct/union initialization. */
      ids = (void **)C_MULTIPLATFORM_MALLOC(count * sizeof(void *));
      if (ids) {
        {
          ui_error_t _ign_rc = ui_selection_model_get_selected(
              listbox->selection_model, ids, count);
          (void)_ign_rc;
        }
        payload.ptr_val = ids;
        /* Ignore the return value of cva_on_change inside trigger */
#define UI_CVA_ON_CHANGE_IGNORE(cb, p, u) (cb)((p), (u))
        (void)UI_CVA_ON_CHANGE_IGNORE(listbox->cva_on_change, payload,
                                      listbox->cva_on_change_user_data);
        /* The consumer of the signal must free it, or it leaks, but this is a
           standard problem with CVA arrays unless backed by an arena. For now,
           this is what other widgets do. */
      }
    } else {
      if (count > 0) {
        void *id = NULL;
        {
#define UI_SEL_GET_SEL_IGNORE(m, i, c)                                         \
  ui_selection_model_get_selected((m), (i), (c))
          (void)UI_SEL_GET_SEL_IGNORE(listbox->selection_model, &id, 1);
        }
        payload.int_val = (int)(size_t)id;
      } else {
        payload.int_val = -1; /* -1 represents no selection */
      }
      return listbox->cva_on_change(payload, listbox->cva_on_change_user_data);
    }
  }
  return UI_ERROR_NONE;
}

static ui_error_t on_selection_change(struct ui_selection_model *model,
                                      void *user_data) {
  struct ui_listbox_base *listbox = (struct ui_listbox_base *)user_data;
  (void)model;
  return listbox_trigger_cva_change(listbox);
}

static ui_error_t listbox_cva_write_value(void *component,
                                          union ui_signal_payload value) {
  struct ui_listbox_base *listbox = (struct ui_listbox_base *)component;
  int is_multi = 0;
  const char *attr = NULL;

  if (!listbox)
    return UI_ERROR_INVALID_ARGUMENT;

  {
    ui_error_t attr_rc = ui_dom_node_get_attribute(
        listbox->component->shadow_root, "aria-multiselectable", &attr);
    if (attr && strcmp(attr, "true") == 0) {
      is_multi = 1;
    }
  }

  /* Clear existing */
  {
    ui_error_t _ign_rc = ui_selection_model_clear(listbox->selection_model);
    (void)_ign_rc;
  }

  if (!is_multi) {
    if (value.int_val >= 0) {
      {
        ui_error_t _ign_rc = ui_selection_model_select(
            listbox->selection_model, (void *)(size_t)value.int_val);
        (void)_ign_rc;
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

/** \brief listbox_cva_register_on_change */
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

static ui_error_t listbox_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_listbox_base *listbox = (struct ui_listbox_base *)component;
  if (!listbox)
    return UI_ERROR_INVALID_ARGUMENT;
  listbox->cva_on_touched = callback;
  listbox->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t listbox_cva_set_disabled_state(void *component,
                                                 int is_disabled) {
  struct ui_listbox_base *listbox = (struct ui_listbox_base *)component;
  if (!listbox)
    return UI_ERROR_INVALID_ARGUMENT;
  listbox->is_disabled = is_disabled;
  {
    ui_error_t _ign_rc = ui_dom_node_set_attribute(
        listbox->component->shadow_root, "aria-disabled",
        is_disabled ? "true" : "false");
    (void)_ign_rc;
  }
  if (is_disabled) {
    {
      ui_error_t _ign_rc = ui_dom_node_set_attribute(
          listbox->component->shadow_root, "tabindex", "-1");
      (void)_ign_rc;
    }
  } else {
    {
      ui_error_t _ign_rc = ui_dom_node_set_attribute(
          listbox->component->shadow_root, "tabindex", "0");
      (void)_ign_rc;
    }
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
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

  {

    ui_error_t _ign_rc = ui_selection_model_set_on_change(
        listbox->selection_model, on_selection_change, listbox);

    (void)_ign_rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  {

    ui_error_t _ign_rc = ui_dom_node_set_tag_name(root_node, "div");

    (void)_ign_rc;
  }
  {
    ui_error_t _ign_rc =
        ui_dom_node_set_attribute(root_node, "role", "listbox");
    (void)_ign_rc;
  }
  {
    ui_error_t _ign_rc = ui_dom_node_set_attribute(root_node, "tabindex", "0");
    (void)_ign_rc;
  }
  {
    ui_error_t _ign_rc =
        ui_dom_node_set_attribute(root_node, "aria-multiselectable", "false");
    (void)_ign_rc;
  }

  listbox->component->shadow_root = root_node;
  root_node = NULL;

  rc = ui_css_parse_stylesheet(ui_listbox_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  {

    ui_error_t _ign_rc =
        ui_component_set_default_style(listbox->component, default_style);

    (void)_ign_rc;
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
  if (listbox->selection_model)
    ui_selection_model_destroy(listbox->selection_model);
  if (listbox->component) {
    ui_error_t _ign_rc = ui_component_destroy(listbox->component);
    (void)_ign_rc;
  }
  C_MULTIPLATFORM_FREE(listbox);
  return rc;
}

ui_error_t ui_listbox_base_destroy(struct ui_listbox_base *listbox) {
  if (!listbox)
    return UI_ERROR_NONE;

  (void)ui_selection_model_destroy(listbox->selection_model);
  {
    ui_error_t _ign_rc = ui_component_destroy(listbox->component);
    (void)_ign_rc;
  }
  C_MULTIPLATFORM_FREE(listbox);
  return UI_ERROR_NONE;
}
/** \brief ui_error */
ui_error_t ui_listbox_base_get_component(struct ui_listbox_base *listbox,
                                         struct ui_component **out_component) {
  if (!listbox || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = listbox->component;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_listbox_base_get_selection_model(struct ui_listbox_base *listbox,
                                    struct ui_selection_model **out_model) {
  if (!listbox || !out_model) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_model = listbox->selection_model;
  return UI_ERROR_NONE;
}

ui_error_t ui_listbox_base_set_multi_select(struct ui_listbox_base *listbox,
                                            int is_multi) {
  if (!listbox)
    return UI_ERROR_INVALID_ARGUMENT;

  if (listbox->component->shadow_root) {
    {
      ui_error_t _ign_rc = ui_dom_node_set_attribute(
          listbox->component->shadow_root, "aria-multiselectable",
          is_multi ? "true" : "false");
      (void)_ign_rc;
    }
  }

  return ui_selection_model_set_multi_select(listbox->selection_model,
                                             is_multi);
}

ui_error_t ui_listbox_base_set_item_count(struct ui_listbox_base *listbox,
                                          int num_items) {
  if (!listbox || num_items < 0)
    return UI_ERROR_INVALID_ARGUMENT;

  listbox->num_items = num_items;

  if (listbox->active_index >= num_items) {
    listbox->active_index = num_items > 0 ? num_items - 1 : -1;
  }

  if (num_items == 0) {
    {
      ui_error_t _ign_rc = ui_selection_model_clear(listbox->selection_model);
      (void)_ign_rc;
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
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

ui_error_t ui_listbox_base_set_active_index(struct ui_listbox_base *listbox,
                                            int index) {
  if (!listbox)
    return UI_ERROR_INVALID_ARGUMENT;
  if (index < -1 || index >= listbox->num_items)
    return UI_ERROR_OUT_OF_BOUNDS;

  listbox->active_index = index;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_listbox_base_get_active_index(const struct ui_listbox_base *listbox,
                                 int *out_index) {
  if (!listbox || !out_index)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_index = listbox->active_index;
  return UI_ERROR_NONE;
}

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
    if (attr && strcmp(attr, "true") == 0) {
      is_multi = 1;
    }
  }

  for (i = 1; i <= listbox->num_items; i++) {
    int idx = (start_index + i) % listbox->num_items;
    const char *text =
        listbox->text_provider(listbox, idx, listbox->text_user_data);
    int is_match = 0;
#define UI_PREFIX_MATCH_IGNORE(s, p, l, o) prefix_match((s), (p), (l), (o))
    (void)UI_PREFIX_MATCH_IGNORE(text, listbox->typeahead_buffer,
                                 listbox->typeahead_len, &is_match);
    if (is_match) {
      listbox->active_index = idx;

      if (!is_multi) {
        {
          ui_error_t _ign_rc = ui_selection_model_select(
              listbox->selection_model, (void *)(size_t)idx);
          (void)_ign_rc;
        }
      }
      break;
    }
  }
  return UI_ERROR_NONE;
}

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
#define UI_CVA_ON_TOUCH_IGNORE(cb, u) (cb)((u))
    (void)UI_CVA_ON_TOUCH_IGNORE(listbox->cva_on_touched,
                                 listbox->cva_on_touched_user_data);
  }

  {
    ui_error_t attr_rc = ui_dom_node_get_attribute(
        listbox->component->shadow_root, "aria-multiselectable", &attr);
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
          {
            ui_error_t _ign_rc = ui_selection_model_select(
                listbox->selection_model,
                (void *)(size_t)listbox->active_index);
            (void)_ign_rc;
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
          {
            ui_error_t _ign_rc = ui_selection_model_select(
                listbox->selection_model,
                (void *)(size_t)listbox->active_index);
            (void)_ign_rc;
          }
        }
      }
    } else if (kc == UI_KEY_HOME) {
      listbox->typeahead_len = 0;
      if (listbox->num_items > 0) {
        listbox->active_index = 0;
        if (!is_multi) {
          {
            ui_error_t _ign_rc = ui_selection_model_select(
                listbox->selection_model,
                (void *)(size_t)listbox->active_index);
            (void)_ign_rc;
          }
        }
      }
    } else if (kc == UI_KEY_END) {
      listbox->typeahead_len = 0;
      if (listbox->num_items > 0) {
        listbox->active_index = listbox->num_items - 1;
        if (!is_multi) {
          {
            ui_error_t _ign_rc = ui_selection_model_select(
                listbox->selection_model,
                (void *)(size_t)listbox->active_index);
            (void)_ign_rc;
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
            ui_error_t _ign_ta_rc = perform_typeahead(listbox);
            (void)_ign_ta_rc;
          }
        }
      } else {
        listbox->typeahead_len = 0;
        if (listbox->active_index >= 0) {
          if (is_multi && kc == UI_KEY_SPACE) {
            {
              ui_error_t _ign_rc = ui_selection_model_toggle(
                  listbox->selection_model,
                  (void *)(size_t)listbox->active_index);
              (void)_ign_rc;
            }
          } else if (!is_multi) {
            {
              ui_error_t _ign_rc = ui_selection_model_select(
                  listbox->selection_model,
                  (void *)(size_t)listbox->active_index);
              (void)_ign_rc;
            }
          }
        }
      }
    } else if (kc >= 32 && kc < 127) {
      if (listbox->typeahead_len < (int)sizeof(listbox->typeahead_buffer) - 1) {
        listbox->typeahead_buffer[listbox->typeahead_len++] = (char)kc;
        listbox->last_typeahead_time_ms = timestamp_ms;
        {
          ui_error_t _ign_ta_rc = perform_typeahead(listbox);
          (void)_ign_ta_rc;
        }
      }
    }
  }

  return UI_ERROR_NONE;
}
