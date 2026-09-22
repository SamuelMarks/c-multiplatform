/**
 * @file ui_autocomplete_base.c
 * @brief Implementation of the autocomplete combo box base component.
 */

/* clang-format off */
#include "ui_autocomplete_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_ac_mock_fail = 0;
int g_ac_mock_is_open = 0;

/**
 * @brief mock_listbox_get_active_index.
 * @param lb Parameter lb.
 * @param out Parameter out.
 * @return Return value.
 */
static ui_error_t mock_listbox_get_active_index(struct ui_listbox_base *lb,
                                                int *out) {
  if (g_ac_mock_fail == 18)
    return UI_ERROR_UNKNOWN;
  if ((g_ac_mock_fail == 10 || g_ac_mock_fail == 11) && out) {
    *out = 0;
    return UI_ERROR_NONE;
  }
  return (ui_listbox_base_get_active_index)(lb, out);
}
#undef ui_listbox_base_get_active_index
/** @cond */
#define ui_listbox_base_get_active_index mock_listbox_get_active_index
/** @endcond */

/**
 * @brief mock_listbox_get_comp.
 * @param lb Parameter lb.
 * @param c Parameter c.
 * @return Return value.
 */
static ui_error_t mock_listbox_get_comp(struct ui_listbox_base *lb,
                                        struct ui_component **c) {
  if (g_ac_mock_fail == 17)
    return UI_ERROR_UNKNOWN;
  return (ui_listbox_base_get_component)(lb, c);
}
#undef ui_listbox_base_get_component
/** @cond */
#define ui_listbox_base_get_component mock_listbox_get_comp
/** @endcond */

/**
 * @brief mock_popover_process_event.
 * @param p Parameter p.
 * @param e Parameter e.
 * @return Return value.
 */
static ui_error_t mock_popover_process_event(struct ui_popover_base *p,
                                             const struct ui_event *e) {
  if (g_ac_mock_fail == 13)
    return UI_ERROR_UNKNOWN;
  if (e->type == UI_EVENT_MOUSE_DOWN)
    g_ac_mock_is_open = 0;
  return (ui_popover_base_process_event)(p, e);
}
#undef ui_popover_base_process_event
/** @cond */
#define ui_popover_base_process_event mock_popover_process_event
/** @endcond */

/**
 * @brief mock_input_process_event.
 * @param i Parameter i.
 * @param e Parameter e.
 * @param t Parameter t.
 * @return Return value.
 */
static ui_error_t mock_input_process_event(struct ui_input_base *i,
                                           const struct ui_event *e, double t) {
  if (g_ac_mock_fail == 14)
    return UI_ERROR_UNKNOWN;
  return (ui_input_base_process_event)(i, e, t);
}
#undef ui_input_base_process_event
/** @cond */
#define ui_input_base_process_event mock_input_process_event
/** @endcond */

/**
 * @brief mock_listbox_create.
 * @param out Parameter out.
 * @param cva Parameter cva.
 * @return Return value.
 */
static ui_error_t mock_listbox_create(struct ui_listbox_base **out,
                                      struct ui_control_value_accessor *cva) {
  if (g_ac_mock_fail == 15)
    return UI_ERROR_UNKNOWN;
  return (ui_listbox_base_create)(out, cva);
}
#undef ui_listbox_base_create
/** @cond */
#define ui_listbox_base_create mock_listbox_create
/** @endcond */

/**
 * @brief mock_popover_create.
 * @param out Parameter out.
 * @return Return value.
 */
static ui_error_t mock_popover_create(struct ui_popover_base **out) {
  if (g_ac_mock_fail == 16)
    return UI_ERROR_UNKNOWN;
  return (ui_popover_base_create)(out);
}
#undef ui_popover_base_create
/** @cond */
#define ui_popover_base_create mock_popover_create
/** @endcond */

/**
 * @brief mock_dom_node_append_child.
 * @param parent Parameter parent.
 * @param child Parameter child.
 * @return Return value.
 */
static ui_error_t mock_dom_node_append_child(struct ui_dom_node *parent,
                                             struct ui_dom_node *child) {
  if (g_ac_mock_fail == 1)
    return UI_ERROR_UNKNOWN;
  return (ui_dom_node_append_child)(parent, child);
}
#undef ui_dom_node_append_child
/** @cond */
#define ui_dom_node_append_child mock_dom_node_append_child
/** @endcond */

/**
 * @brief mock_input_base_set_on_change.
 * @param input Parameter input.
 * @param cb Parameter cb.
 * @param u Parameter u.
 * @return Return value.
 */
static ui_error_t mock_input_base_set_on_change(struct ui_input_base *input,
                                                ui_input_on_change_t cb,
                                                void *u) {
  if (g_ac_mock_fail == 2)
    return UI_ERROR_UNKNOWN;
  return (ui_input_base_set_on_change)(input, cb, u);
}
#undef ui_input_base_set_on_change
/** @cond */
#define ui_input_base_set_on_change mock_input_base_set_on_change
/** @endcond */

/**
 * @brief mock_dom_node_remove_child.
 * @param parent Parameter parent.
 * @param child Parameter child.
 * @return Return value.
 */
static ui_error_t mock_dom_node_remove_child(struct ui_dom_node *parent,
                                             struct ui_dom_node *child) {
  return (ui_dom_node_remove_child)(parent, child);
}

#undef ui_dom_node_remove_child
/** @cond */
#define ui_dom_node_remove_child mock_dom_node_remove_child
/** @endcond */

/**
 * @brief mock_component_destroy.
 * @param comp Parameter comp.
 * @return Return value.
 */
static ui_error_t mock_component_destroy(struct ui_component *comp) {
  return (ui_component_destroy)(comp);
}
#undef ui_component_destroy
/** @cond */
#define ui_component_destroy mock_component_destroy
/** @endcond */

/**
 * @brief mock_input_base_get_component.
 * @param i Parameter i.
 * @param c Parameter c.
 * @return Return value.
 */
static ui_error_t mock_input_base_get_component(struct ui_input_base *i,
                                                struct ui_component **c) {
  if (g_ac_mock_fail == 5)
    return UI_ERROR_UNKNOWN;
  return (ui_input_base_get_component)(i, c);
}
#undef ui_input_base_get_component
/** @cond */
#define ui_input_base_get_component mock_input_base_get_component
/** @endcond */

/**
 * @brief mock_popover_open.
 * @param popover Parameter popover.
 * @param content Parameter content.
 * @param director Parameter director.
 * @param focus_mgr Parameter focus_mgr.
 * @param trigger_layout Parameter trigger_layout.
 * @param anchor_config Parameter anchor_config.
 * @param viewport_width Parameter viewport_width.
 * @param viewport_height Parameter viewport_height.
 * @return Return value.
 */
static ui_error_t
mock_popover_open(struct ui_popover_base *popover, struct ui_dom_node *content,
                  struct ui_overlay_director *director,
                  struct ui_focus_manager *focus_mgr,
                  const struct ui_layout_node *trigger_layout,
                  const struct ui_anchor_config *anchor_config,
                  float viewport_width, float viewport_height) {
  struct ui_popover_base *u_p = popover;
  struct ui_dom_node *u_c = content;
  struct ui_overlay_director *u_d = director;
  struct ui_focus_manager *u_f = focus_mgr;
  const struct ui_layout_node *u_t = trigger_layout;
  const struct ui_anchor_config *u_a = anchor_config;
  float u_w = viewport_width;
  float u_h = viewport_height;
  popover = u_p;
  content = u_c;
  director = u_d;
  focus_mgr = u_f;
  trigger_layout = u_t;
  anchor_config = u_a;
  viewport_width = u_w;
  viewport_height = u_h;
  if (g_ac_mock_fail == 7)
    return UI_ERROR_UNKNOWN;
  g_ac_mock_is_open = 1;
  return UI_ERROR_NONE;
}
#undef ui_popover_base_open
/** @cond */
#define ui_popover_base_open mock_popover_open
/** @endcond */

/**
 * @brief mock_popover_close.
 * @param p Parameter p.
 * @return Return value.
 */
static ui_error_t mock_popover_close(struct ui_popover_base *p) {
  if (g_ac_mock_fail == 9)
    return UI_ERROR_UNKNOWN;
  g_ac_mock_is_open = 0;
  return (ui_popover_base_close)(p);
}
#undef ui_popover_base_close
/** @cond */
#define ui_popover_base_close mock_popover_close
/** @endcond */

int g_popover_is_open_calls = 0;
/**
 * @brief mock_popover_is_open.
 * @param p Parameter p.
 * @param o Parameter o.
 * @return Return value.
 */
static ui_error_t mock_popover_is_open(struct ui_popover_base *p, int *o) {
  struct ui_popover_base *u_p = p;
  p = u_p;
  g_popover_is_open_calls++;
  if (g_ac_mock_fail == 12)
    return UI_ERROR_UNKNOWN;
  if (g_ac_mock_fail == 19 && g_popover_is_open_calls == 2)
    return UI_ERROR_UNKNOWN;
  *o = g_ac_mock_is_open;
  return UI_ERROR_NONE;
}
#undef ui_popover_base_is_open
/** @cond */
#define ui_popover_base_is_open mock_popover_is_open
/** @endcond */

/**
 * @brief mock_dom_node_set_attribute.
 * @param n Parameter n.
 * @param k Parameter k.
 * @param v Parameter v.
 * @return Return value.
 */
static ui_error_t mock_dom_node_set_attribute(struct ui_dom_node *n,
                                              const char *k, const char *v) {
  if (g_ac_mock_fail == 8)
    return UI_ERROR_UNKNOWN;
  return (ui_dom_node_set_attribute)(n, k, v);
}
#undef ui_dom_node_set_attribute
/** @cond */
#define ui_dom_node_set_attribute mock_dom_node_set_attribute
/** @endcond */

/**
 * @brief mock_listbox_get_selection_model.
 * @param lb Parameter lb.
 * @param m Parameter m.
 * @return Return value.
 */
static ui_error_t
mock_listbox_get_selection_model(struct ui_listbox_base *lb,
                                 struct ui_selection_model **m) {
  if (g_ac_mock_fail == 10)
    return UI_ERROR_UNKNOWN;
  return (ui_listbox_base_get_selection_model)(lb, m);
}
#undef ui_listbox_base_get_selection_model
/** @cond */
#define ui_listbox_base_get_selection_model mock_listbox_get_selection_model
/** @endcond */

/**
 * @brief mock_selection_model_select.
 * @param m Parameter m.
 * @param item Parameter item.
 * @return Return value.
 */
static ui_error_t mock_selection_model_select(struct ui_selection_model *m,
                                              void *item) {
  if (g_ac_mock_fail == 11)
    return UI_ERROR_UNKNOWN;
  return (ui_selection_model_select)(m, item);
}
#undef ui_selection_model_select
/** @cond */
#define ui_selection_model_select mock_selection_model_select
/** @endcond */

#endif

/**
 * @struct ui_autocomplete_base
 * @struct ui_autocomplete_base
 * @brief Internal representation of an autocomplete component.
 */
struct ui_autocomplete_base {
  struct ui_component *root_component; /**< The root component. */
  struct ui_input_base *input;         /**< The input text field. */
  struct ui_listbox_base *listbox;     /**< The suggestion listbox. */
  struct ui_popover_base *popover;     /**< Popover for the listbox. */
  struct ui_overlay_director
      *overlay_director;                  /**< Overlay director dependency. */
  struct ui_focus_manager *focus_manager; /**< Focus manager dependency. */

  ui_autocomplete_on_selection_t on_selection; /**< on_selection */
  void *selection_user_data;                   /**< selection_user_data */

  ui_autocomplete_on_text_change_t on_text_change; /**< on_text_change */
  void *text_change_user_data;                     /**< text_change_user_data */

  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data); /**< user_data) */
  void *cva_on_change_user_data;                /**< cva_on_change_user_data */

  ui_error_t (*cva_on_touched)(void *user_data); /**< user_data) */
  void *cva_on_touched_user_data; /**< cva_on_touched_user_data */
};

/**
 * @brief on_input_text_change.
 * @param input Parameter input.
 * @param text Parameter text.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
static ui_error_t on_input_text_change(struct ui_input_base *input,
                                       const char *text, void *user_data) {
  struct ui_autocomplete_base *ac = (struct ui_autocomplete_base *)user_data;
  struct ui_input_base *unused_input = input;
  union ui_signal_payload payload;
  ui_error_t rc = UI_ERROR_NONE;
  input = unused_input;

  if (ac->cva_on_change) {
    payload.ptr_val = (void *)text;
    rc = ac->cva_on_change(payload, ac->cva_on_change_user_data);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  if (ac->on_text_change) {
    rc = ac->on_text_change(ac, text, ac->text_change_user_data);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  return rc;
}

/**
 * @brief autocomplete_cva_write_value.
 * @param component Parameter component.
 * @param value Parameter value.
 * @return Return value.
 */
static ui_error_t autocomplete_cva_write_value(void *component,
                                               union ui_signal_payload value) {
  struct ui_autocomplete_base *ac = (struct ui_autocomplete_base *)component;
  if (!ac)
    return UI_ERROR_INVALID_ARGUMENT;
  if (value.ptr_val) {
    return ui_input_base_set_text(ac->input, (const char *)value.ptr_val);
  } else {
    return ui_input_base_set_text(ac->input, "");
  }
}

/**
 * @brief autocomplete_cva_register_on_change.
 * @param component Parameter component.
 * @param callback Parameter callback.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
static ui_error_t autocomplete_cva_register_on_change(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  struct ui_autocomplete_base *ac = (struct ui_autocomplete_base *)component;
  if (!ac)
    return UI_ERROR_INVALID_ARGUMENT;
  ac->cva_on_change = callback;
  ac->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief autocomplete_cva_register_on_touched.
 * @param component Parameter component.
 * @param callback Parameter callback.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
static ui_error_t autocomplete_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_autocomplete_base *ac = (struct ui_autocomplete_base *)component;
  if (!ac)
    return UI_ERROR_INVALID_ARGUMENT;
  ac->cva_on_touched = callback;
  ac->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief autocomplete_cva_set_disabled_state.
 * @param component Parameter component.
 * @param is_disabled Parameter is_disabled.
 * @return Return value.
 */
static ui_error_t autocomplete_cva_set_disabled_state(void *component,
                                                      int is_disabled) {
  struct ui_autocomplete_base *ac = (struct ui_autocomplete_base *)component;
  if (!ac)
    return UI_ERROR_INVALID_ARGUMENT;
  return ui_input_base_set_disabled(ac->input, is_disabled);
}

static ui_error_t
destroy_partially_created_autocomplete(struct ui_autocomplete_base *ac,
                                       int input_attached,
                                       struct ui_dom_node *root_node) {
  if (ac->popover) {
    ui_popover_base_destroy(ac->popover);
  }
  if (ac->listbox) {
    ui_listbox_base_destroy(ac->listbox);
  }
  if (ac->input) {
    if (input_attached) {
      struct ui_component *tmp_comp = NULL;
      ui_input_base_get_component(ac->input, &tmp_comp);
      ui_dom_node_remove_child(ac->root_component->shadow_root,
                               tmp_comp->shadow_root);
    }
    ui_input_base_destroy(ac->input);
  }
  if (ac->root_component) {
    ui_component_destroy(ac->root_component);
  }
  if (root_node) {
    ui_dom_node_destroy(root_node);
  }
  C_MULTIPLATFORM_FREE(ac);
  return UI_ERROR_NONE;
}

/**
 * @brief ui_autocomplete_base_create.
 * @param out_autocomplete Parameter out_autocomplete.
 * @param out_cva Parameter out_cva.
 * @return Return value.
 */
ui_error_t
ui_autocomplete_base_create(struct ui_autocomplete_base **out_autocomplete,
                            struct ui_control_value_accessor *out_cva) {
  struct ui_autocomplete_base *ac;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  int input_attached = 0;

  if (!out_autocomplete)
    return UI_ERROR_INVALID_ARGUMENT;

  ac = (struct ui_autocomplete_base *)C_MULTIPLATFORM_MALLOC(
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

  rc = ui_dom_node_set_tag_name(root_node, "div");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_attribute(root_node, "role", "combobox");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_attribute(root_node, "aria-expanded", "false");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_attribute(root_node, "aria-haspopup", "listbox");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  ac->root_component->shadow_root = root_node;
  root_node = NULL;

  rc = ui_input_base_create(&ac->input);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  {
    struct ui_component *tmp_comp;
    rc = ui_input_base_get_component(ac->input, &tmp_comp);
    if (rc != UI_ERROR_NONE)
      goto cleanup;
    rc = ui_dom_node_append_child(ac->root_component->shadow_root,
                                  tmp_comp->shadow_root);
    if (rc != UI_ERROR_NONE)
      goto cleanup;
    input_attached = 1;
  }

  rc = ui_listbox_base_create(&ac->listbox, NULL);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_popover_base_create(&ac->popover);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_input_base_set_on_change(ac->input, on_input_text_change, ac);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
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
  destroy_partially_created_autocomplete(ac, input_attached, root_node);
  return rc;
}

/**
 * @brief ui_autocomplete_base_destroy.
 * @param autocomplete Parameter autocomplete.
 * @return Return value.
 */
ui_error_t
ui_autocomplete_base_destroy(struct ui_autocomplete_base *autocomplete) {
  if (!autocomplete)
    return UI_ERROR_INVALID_ARGUMENT;

  return destroy_partially_created_autocomplete(autocomplete, 1, NULL);
}

/**
 * @brief ui_autocomplete_base_get_component.
 * @param autocomplete Parameter autocomplete.
 * @param out_component Parameter out_component.
 * @return Return value.
 */
ui_error_t
ui_autocomplete_base_get_component(struct ui_autocomplete_base *autocomplete,
                                   struct ui_component **out_component) {
  if (!autocomplete || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = autocomplete->root_component;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_autocomplete_base_get_input.
 * @param autocomplete Parameter autocomplete.
 * @param out_input Parameter out_input.
 * @return Return value.
 */
ui_error_t
ui_autocomplete_base_get_input(struct ui_autocomplete_base *autocomplete,
                               struct ui_input_base **out_input) {
  if (!autocomplete || !out_input) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_input = autocomplete->input;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_autocomplete_base_get_listbox.
 * @param autocomplete Parameter autocomplete.
 * @param out_listbox Parameter out_listbox.
 * @return Return value.
 */
ui_error_t
ui_autocomplete_base_get_listbox(struct ui_autocomplete_base *autocomplete,
                                 struct ui_listbox_base **out_listbox) {
  if (!autocomplete || !out_listbox) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_listbox = autocomplete->listbox;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_autocomplete_base_set_overlay_dependencies.
 * @param autocomplete Parameter autocomplete.
 * @param director Parameter director.
 * @param focus_mgr Parameter focus_mgr.
 * @return Return value.
 */
ui_error_t ui_autocomplete_base_set_overlay_dependencies(
    struct ui_autocomplete_base *autocomplete,
    struct ui_overlay_director *director, struct ui_focus_manager *focus_mgr) {

  if (!autocomplete)
    return UI_ERROR_INVALID_ARGUMENT;
  autocomplete->overlay_director = director;
  autocomplete->focus_manager = focus_mgr;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_autocomplete_base_set_on_text_change.
 * @param autocomplete Parameter autocomplete.
 * @param on_text_change Parameter on_text_change.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
ui_error_t ui_autocomplete_base_set_on_text_change(
    struct ui_autocomplete_base *autocomplete,
    ui_autocomplete_on_text_change_t on_text_change, void *user_data) {

  if (!autocomplete)
    return UI_ERROR_INVALID_ARGUMENT;
  autocomplete->on_text_change = on_text_change;
  autocomplete->text_change_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_autocomplete_base_set_on_selection.
 * @param autocomplete Parameter autocomplete.
 * @param on_selection Parameter on_selection.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
ui_error_t ui_autocomplete_base_set_on_selection(
    struct ui_autocomplete_base *autocomplete,
    ui_autocomplete_on_selection_t on_selection, void *user_data) {

  if (!autocomplete)
    return UI_ERROR_INVALID_ARGUMENT;
  autocomplete->on_selection = on_selection;
  autocomplete->selection_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_autocomplete_base_open.
 * @param autocomplete Parameter autocomplete.
 * @param trigger_layout Parameter trigger_layout.
 * @param viewport_width Parameter viewport_width.
 * @param viewport_height Parameter viewport_height.
 * @return Return value.
 */
ui_error_t
ui_autocomplete_base_open(struct ui_autocomplete_base *autocomplete,
                          const struct ui_layout_node *trigger_layout,
                          float viewport_width, float viewport_height) {
  struct ui_anchor_config anchor;
  ui_error_t rc;
  int is_open = 0;

  if (!autocomplete || !trigger_layout)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = ui_popover_base_is_open(autocomplete->popover, &is_open);
  if (rc != UI_ERROR_NONE)
    return rc;
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
    rc = ui_listbox_base_get_component(autocomplete->listbox, &tmp_comp);
    if (rc != UI_ERROR_NONE)
      return rc;

    rc = ui_popover_base_open(autocomplete->popover, tmp_comp->shadow_root,
                              autocomplete->overlay_director,
                              autocomplete->focus_manager, trigger_layout,
                              &anchor, viewport_width, viewport_height);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  rc = ui_dom_node_set_attribute(autocomplete->root_component->shadow_root,
                                 "aria-expanded", "true");
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
}

/**
 * @brief ui_autocomplete_base_close.
 * @param autocomplete Parameter autocomplete.
 * @return Return value.
 */
ui_error_t
ui_autocomplete_base_close(struct ui_autocomplete_base *autocomplete) {
  int is_open = 0;
  ui_error_t rc;

  if (!autocomplete)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = ui_popover_base_is_open(autocomplete->popover, &is_open);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (is_open) {
    rc = ui_popover_base_close(autocomplete->popover);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_dom_node_set_attribute(autocomplete->root_component->shadow_root,
                                   "aria-expanded", "false");
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_autocomplete_base_process_event.
 * @param autocomplete Parameter autocomplete.
 * @param event Parameter event.
 * @param timestamp_ms Parameter timestamp_ms.
 * @return Return value.
 */
ui_error_t
ui_autocomplete_base_process_event(struct ui_autocomplete_base *autocomplete,
                                   const struct ui_event *event,
                                   double timestamp_ms) {
  int is_open = 0;
  ui_error_t rc;

  if (!autocomplete || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = ui_popover_base_is_open(autocomplete->popover, &is_open);
  if (rc != UI_ERROR_NONE)
    return rc;

  if (is_open) {
    /* Route click-outside events to popover base */
    rc = ui_popover_base_process_event(autocomplete->popover, event);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_popover_base_is_open(autocomplete->popover, &is_open);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (!is_open) {
      /* Popover decided to close (e.g. click outside) */
      rc = ui_dom_node_set_attribute(autocomplete->root_component->shadow_root,
                                     "aria-expanded", "false");
      if (rc != UI_ERROR_NONE)
        return rc;
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
        rc = ui_listbox_base_get_active_index(autocomplete->listbox, &active);
        if (rc != UI_ERROR_NONE)
          return rc;
        if (active >= 0) {
          struct ui_selection_model *model = NULL;
          rc = ui_listbox_base_get_selection_model(autocomplete->listbox,
                                                   &model);
          if (rc != UI_ERROR_NONE)
            return rc;
          rc = ui_selection_model_select(model, (void *)(size_t)active);
          if (rc != UI_ERROR_NONE)
            return rc;
        }
        return UI_ERROR_NONE;
      }
    }
  }
  /* Fallback: input processes text characters */
  return ui_input_base_process_event(autocomplete->input, event, timestamp_ms);
}

#ifdef UI_TEST_MOCK_ALLOC
ui_error_t run_ac_coverage(void);
/**
 * @brief run_ac_coverage.
 * @return Return value.
 */
ui_error_t run_ac_coverage(void) { return UI_ERROR_NONE; }
#endif
