/**
 * @file ui_component_registry.c
 * @brief Implementation of the UI component registry for dynamic schema
 * widgets.
 */

/* clang-format off */
#include "ui_component_registry.h"
#include "ui_internal_mem.h"
#include "ui_coercion_utils.h"
#include "ui_component.h"
#include "ui_button_base.h"
#include "ui_input_base.h"
#include "ui_card_base.h"
#include "ui_checkbox_base.h"
#include "ui_label_base.h"
#include "ui_slider_base.h"
#include <string.h>
#include <stdlib.h>
/* clang-format on */

/**
 * @struct ui_registry_entry
 * @brief Linked list entry for a registered component type.
 */
struct ui_registry_entry {
  char *type_name;                   /**< Name of the component type */
  struct ui_component_vtable vtable; /**< Vtable operations */
  struct ui_registry_entry *next;    /**< Next entry */
};

/**
 * @struct ui_component_registry
 * @brief Internal registry structure holding registered component types.
 */
struct ui_component_registry {
  struct ui_registry_entry *head; /**< Head of registered entries linked list */
};

static struct ui_component_registry *g_default_registry = NULL;

/**
 * @brief Creates a new component registry.
 *
 * @param out_registry Pointer to receive the allocated registry.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_component_registry_create(struct ui_component_registry **out_registry) {
  struct ui_component_registry *reg;

  if (!out_registry) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  reg = (struct ui_component_registry *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_component_registry));
  if (!reg) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  reg->head = NULL;
  *out_registry = reg;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a component registry and frees its registrations.
 *
 * @param registry The registry to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_component_registry_destroy(struct ui_component_registry *registry) {
  struct ui_registry_entry *curr;

  if (!registry) {
    return UI_ERROR_NONE;
  }

  curr = registry->head;
  while (curr) {
    struct ui_registry_entry *next = curr->next;
    C_MULTIPLATFORM_FREE(curr->type_name);
    C_MULTIPLATFORM_FREE(curr);
    curr = next;
  }

  C_MULTIPLATFORM_FREE(registry);
  return UI_ERROR_NONE;
}

/**
 * @brief Registers a component type with its associated vtable.
 *
 * @param registry The registry to register into.
 * @param type_name The schema type name.
 * @param vtable The vtable operations.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_component_registry_register(struct ui_component_registry *registry,
                               const char *type_name,
                               const struct ui_component_vtable *vtable) {
  struct ui_registry_entry *curr;
  struct ui_registry_entry *entry;
  size_t name_len;

  if (!registry || !type_name || !vtable) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Check for duplicate and update if present */
  curr = registry->head;
  while (curr) {
    if (strcmp(curr->type_name, type_name) == 0) {
      curr->vtable = *vtable;
      return UI_ERROR_NONE;
    }
    curr = curr->next;
  }

  entry = (struct ui_registry_entry *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_registry_entry));
  if (!entry) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  name_len = strlen(type_name);
  entry->type_name = (char *)C_MULTIPLATFORM_MALLOC(name_len + 1);
  if (!entry->type_name) {
    C_MULTIPLATFORM_FREE(entry);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  if (UI_STRCPY(entry->type_name, name_len + 1, type_name) != 0) {
    C_MULTIPLATFORM_FREE(entry->type_name);
    C_MULTIPLATFORM_FREE(entry);
    return UI_ERROR_UNKNOWN;
  }

  entry->vtable = *vtable;
  entry->next = registry->head;
  registry->head = entry;

  return UI_ERROR_NONE;
}

/**
 * @brief Looks up a registered component type vtable.
 *
 * @param registry The registry to query.
 * @param type_name The schema type name to find.
 * @param out_vtable Pointer to receive the found vtable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if not registered.
 */
ui_error_t
ui_component_registry_lookup(const struct ui_component_registry *registry,
                             const char *type_name,
                             const struct ui_component_vtable **out_vtable) {
  const struct ui_registry_entry *curr;

  if (!registry || !type_name || !out_vtable) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = registry->head;
  while (curr) {
    if (strcmp(curr->type_name, type_name) == 0) {
      *out_vtable = &curr->vtable;
      return UI_ERROR_NONE;
    }
    curr = curr->next;
  }

  return UI_ERROR_NOT_FOUND;
}

/* ========================================================================= */
/* Default Component Implementations                                         */
/* ========================================================================= */

/* --- ui_button_base wrapper --- */
static ui_error_t def_button_factory(void **out_instance,
                                     struct ui_dom_node **out_dom_node) {
  struct ui_button_base *btn = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  rc = ui_button_base_create(&btn);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_button_base_get_component(btn, &comp);
  (void)rc;

  *out_instance = btn;
  *out_dom_node = comp->shadow_root;
  return UI_ERROR_NONE;
}

static ui_error_t def_button_set_prop(void *instance, const char *key,
                                      const char *value) {
  struct ui_button_base *btn = (struct ui_button_base *)instance;
  if (!btn || !key || !value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (strcmp(key, "text") == 0) {
    return ui_button_base_set_text(btn, value);
  }
  if (strcmp(key, "disabled") == 0) {
    int bval = 0;
    ui_error_t rc = ui_coerce_string_to_bool(value, &bval);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    return ui_button_base_set_disabled(btn, bval);
  }

  return UI_ERROR_NONE;
}

struct btn_event_adapter {
  ui_error_t (*callback)(void *user_data);
  void *user_data;
};

static ui_error_t on_button_click_adapter(struct ui_button_base *btn,
                                          void *user_data) {
  struct btn_event_adapter *ad = (struct btn_event_adapter *)user_data;
  (void)btn;
  return ad->callback(ad->user_data);
}

static ui_error_t def_button_event(void *instance, const char *event_name,
                                   ui_error_t (*callback)(void *user_data),
                                   void *user_data) {
  struct ui_button_base *btn = (struct ui_button_base *)instance;
  struct btn_event_adapter *ad;

  if (!btn || !event_name || !callback) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (strcmp(event_name, "on_click") == 0) {
    ad = (struct btn_event_adapter *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct btn_event_adapter));
    if (!ad) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    ad->callback = callback;
    ad->user_data = user_data;
    return ui_button_base_set_on_click(btn, on_button_click_adapter, ad);
  }

  return UI_ERROR_NONE;
}

static ui_error_t def_button_destroy(void *instance) {
  return ui_button_base_destroy((struct ui_button_base *)instance);
}

/* --- ui_input_base wrapper --- */
static ui_error_t def_input_factory(void **out_instance,
                                    struct ui_dom_node **out_dom_node) {
  struct ui_input_base *inp = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  rc = ui_input_base_create(&inp);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_input_base_get_component(inp, &comp);
  (void)rc;

  *out_instance = inp;
  *out_dom_node = comp->shadow_root;
  return UI_ERROR_NONE;
}

static ui_error_t def_input_get_cva(void *instance,
                                    struct ui_control_value_accessor *out_cva) {
  return ui_input_base_get_cva((struct ui_input_base *)instance, out_cva);
}

static ui_error_t def_input_set_prop(void *instance, const char *key,
                                     const char *value) {
  struct ui_input_base *inp = (struct ui_input_base *)instance;
  if (!inp || !key || !value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (strcmp(key, "placeholder") == 0) {
    return ui_input_base_set_placeholder(inp, value);
  }
  if (strcmp(key, "type") == 0) {
    return ui_input_base_set_type(inp, value);
  }
  if (strcmp(key, "disabled") == 0) {
    int bval = 0;
    ui_error_t rc = ui_coerce_string_to_bool(value, &bval);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    return ui_input_base_set_disabled(inp, bval);
  }
  if (strcmp(key, "text") == 0 || strcmp(key, "value") == 0) {
    return ui_input_base_set_text(inp, value);
  }

  return UI_ERROR_NONE;
}

static ui_error_t def_input_destroy(void *instance) {
  return ui_input_base_destroy((struct ui_input_base *)instance);
}

/* --- ui_card_base wrapper --- */
static ui_error_t def_card_factory(void **out_instance,
                                   struct ui_dom_node **out_dom_node) {
  struct ui_card_base *card = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  rc = ui_card_base_create(&card);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_card_base_get_component(card, &comp);
  (void)rc;

  *out_instance = card;
  *out_dom_node = comp->shadow_root;
  return UI_ERROR_NONE;
}

static ui_error_t def_card_set_prop(void *instance, const char *key,
                                    const char *value) {
  struct ui_card_base *card = (struct ui_card_base *)instance;
  if (!card || !key || !value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (strcmp(key, "title") == 0) {
    return ui_card_base_set_title(card, value);
  }
  if (strcmp(key, "subtitle") == 0) {
    return ui_card_base_set_subtitle(card, value);
  }

  return UI_ERROR_NONE;
}

static ui_error_t def_card_append(void *instance,
                                  struct ui_dom_node *child_node) {
  struct ui_card_base *card = (struct ui_card_base *)instance;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  if (!card || !child_node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_card_base_get_component(card, &comp);
  (void)rc;

  return ui_dom_node_append_child(comp->shadow_root, child_node);
}

static ui_error_t def_card_destroy(void *instance) {
  return ui_card_base_destroy((struct ui_card_base *)instance);
}

/* --- ui_checkbox_base wrapper --- */
static ui_error_t def_checkbox_factory(void **out_instance,
                                       struct ui_dom_node **out_dom_node) {
  struct ui_checkbox_base *chk = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  rc = ui_checkbox_base_create(&chk);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_checkbox_base_get_component(chk, &comp);
  (void)rc;

  *out_instance = chk;
  *out_dom_node = comp->shadow_root;
  return UI_ERROR_NONE;
}

static ui_error_t
def_checkbox_get_cva(void *instance,
                     struct ui_control_value_accessor *out_cva) {
  return ui_checkbox_base_get_cva((struct ui_checkbox_base *)instance, out_cva);
}

static ui_error_t def_checkbox_set_prop(void *instance, const char *key,
                                        const char *value) {
  struct ui_checkbox_base *chk = (struct ui_checkbox_base *)instance;
  if (!chk || !key || !value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (strcmp(key, "label") == 0) {
    return ui_checkbox_base_set_label(chk, value);
  }
  if (strcmp(key, "checked") == 0) {
    int bval = 0;
    ui_error_t rc = ui_coerce_string_to_bool(value, &bval);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    return ui_checkbox_base_set_checked(chk, bval);
  }

  return UI_ERROR_NONE;
}

static ui_error_t def_checkbox_destroy(void *instance) {
  return ui_checkbox_base_destroy((struct ui_checkbox_base *)instance);
}

/* --- ui_label_base wrapper --- */
static ui_error_t def_label_factory(void **out_instance,
                                    struct ui_dom_node **out_dom_node) {
  struct ui_label_base *lbl = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  rc = ui_label_base_create(&lbl);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_label_base_get_component(lbl, &comp);
  (void)rc;

  *out_instance = lbl;
  *out_dom_node = comp->shadow_root;
  return UI_ERROR_NONE;
}

static ui_error_t def_label_set_prop(void *instance, const char *key,
                                     const char *value) {
  struct ui_label_base *lbl = (struct ui_label_base *)instance;
  if (!lbl || !key || !value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (strcmp(key, "text") == 0) {
    return ui_label_base_set_text(lbl, value);
  }

  return UI_ERROR_NONE;
}

static ui_error_t def_label_destroy(void *instance) {
  return ui_label_base_destroy((struct ui_label_base *)instance);
}

/* --- ui_slider_base wrapper --- */
static ui_error_t def_slider_factory(void **out_instance,
                                     struct ui_dom_node **out_dom_node) {
  struct ui_slider_base *sld = NULL;
  struct ui_control_value_accessor cva;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  rc = ui_slider_base_create(&sld, &cva);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_slider_base_get_component(sld, &comp);
  (void)rc;

  *out_instance = sld;
  *out_dom_node = comp->shadow_root;
  return UI_ERROR_NONE;
}

static ui_error_t
def_slider_get_cva(void *instance, struct ui_control_value_accessor *out_cva) {
  struct ui_slider_base *sld = (struct ui_slider_base *)instance;
  struct ui_control_value_accessor cva;
  struct ui_slider_base *dummy = NULL;
  ui_error_t rc;

  /* Retrieve CVA via slider create/getter */
  if (!sld || !out_cva) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  rc = ui_slider_base_create(&dummy, &cva);
  if (rc == UI_ERROR_NONE) {
    ui_slider_base_destroy(dummy);
    *out_cva = cva;
    out_cva->component = sld;
    return UI_ERROR_NONE;
  }
  return rc;
}

static ui_error_t def_slider_destroy(void *instance) {
  return ui_slider_base_destroy((struct ui_slider_base *)instance);
}

/* --- Layout Primitives: row, column, grid, container --- */
static ui_error_t
def_layout_factory_internal(const char *style_str, void **out_instance,
                            struct ui_dom_node **out_dom_node) {
  struct ui_dom_node *node = NULL;
  ui_error_t rc;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_dom_node_set_tag_name(node, "div");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(node);
    return rc;
  }

  rc = ui_dom_node_set_attribute(node, "style", style_str);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(node);
    return rc;
  }

  *out_instance = node;
  *out_dom_node = node;
  return UI_ERROR_NONE;
}

static ui_error_t def_row_factory(void **out_instance,
                                  struct ui_dom_node **out_dom_node) {
  return def_layout_factory_internal("display: flex; flex-direction: row;",
                                     out_instance, out_dom_node);
}

static ui_error_t def_col_factory(void **out_instance,
                                  struct ui_dom_node **out_dom_node) {
  return def_layout_factory_internal("display: flex; flex-direction: column;",
                                     out_instance, out_dom_node);
}

static ui_error_t def_grid_factory(void **out_instance,
                                   struct ui_dom_node **out_dom_node) {
  return def_layout_factory_internal("display: grid;", out_instance,
                                     out_dom_node);
}

static ui_error_t def_container_factory(void **out_instance,
                                        struct ui_dom_node **out_dom_node) {
  return def_layout_factory_internal("display: block;", out_instance,
                                     out_dom_node);
}

static ui_error_t def_dom_set_prop(void *instance, const char *key,
                                   const char *value) {
  struct ui_dom_node *node = (struct ui_dom_node *)instance;
  if (!node || !key || !value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_dom_node_set_attribute(node, key, value);
}

static ui_error_t def_dom_append(void *instance,
                                 struct ui_dom_node *child_node) {
  struct ui_dom_node *parent = (struct ui_dom_node *)instance;
  if (!parent || !child_node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_dom_node_append_child(parent, child_node);
}

static ui_error_t def_dom_destroy(void *instance) {
  return ui_dom_node_destroy((struct ui_dom_node *)instance);
}

/**
 * @brief Registers all default standard engine widgets and layout primitives.
 *
 * @param registry The registry to populate.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_component_registry_register_defaults(
    struct ui_component_registry *registry) {
  ui_error_t rc;
  struct ui_component_vtable vt;

  if (!registry) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* ui_button_base */
  memset(&vt, 0, sizeof(vt));
  vt.factory = def_button_factory;
  vt.set_prop = def_button_set_prop;
  vt.attach_event = def_button_event;
  vt.destroy = def_button_destroy;
  rc = ui_component_registry_register(registry, "ui_button_base", &vt);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  /* ui_input_base */
  memset(&vt, 0, sizeof(vt));
  vt.factory = def_input_factory;
  vt.get_cva = def_input_get_cva;
  vt.set_prop = def_input_set_prop;
  vt.destroy = def_input_destroy;
  rc = ui_component_registry_register(registry, "ui_input_base", &vt);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  /* ui_card_base */
  memset(&vt, 0, sizeof(vt));
  vt.factory = def_card_factory;
  vt.set_prop = def_card_set_prop;
  vt.append_child = def_card_append;
  vt.destroy = def_card_destroy;
  rc = ui_component_registry_register(registry, "ui_card_base", &vt);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  /* ui_checkbox_base */
  memset(&vt, 0, sizeof(vt));
  vt.factory = def_checkbox_factory;
  vt.get_cva = def_checkbox_get_cva;
  vt.set_prop = def_checkbox_set_prop;
  vt.destroy = def_checkbox_destroy;
  rc = ui_component_registry_register(registry, "ui_checkbox_base", &vt);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  /* ui_label_base */
  memset(&vt, 0, sizeof(vt));
  vt.factory = def_label_factory;
  vt.set_prop = def_label_set_prop;
  vt.destroy = def_label_destroy;
  rc = ui_component_registry_register(registry, "ui_label_base", &vt);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  /* ui_slider_base */
  memset(&vt, 0, sizeof(vt));
  vt.factory = def_slider_factory;
  vt.get_cva = def_slider_get_cva;
  vt.destroy = def_slider_destroy;
  rc = ui_component_registry_register(registry, "ui_slider_base", &vt);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  /* Layout: row */
  memset(&vt, 0, sizeof(vt));
  vt.factory = def_row_factory;
  vt.set_prop = def_dom_set_prop;
  vt.append_child = def_dom_append;
  vt.destroy = def_dom_destroy;
  rc = ui_component_registry_register(registry, "row", &vt);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  /* Layout: column */
  memset(&vt, 0, sizeof(vt));
  vt.factory = def_col_factory;
  vt.set_prop = def_dom_set_prop;
  vt.append_child = def_dom_append;
  vt.destroy = def_dom_destroy;
  rc = ui_component_registry_register(registry, "column", &vt);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  /* Layout: grid */
  memset(&vt, 0, sizeof(vt));
  vt.factory = def_grid_factory;
  vt.set_prop = def_dom_set_prop;
  vt.append_child = def_dom_append;
  vt.destroy = def_dom_destroy;
  rc = ui_component_registry_register(registry, "grid", &vt);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  /* Layout: container */
  memset(&vt, 0, sizeof(vt));
  vt.factory = def_container_factory;
  vt.set_prop = def_dom_set_prop;
  vt.append_child = def_dom_append;
  vt.destroy = def_dom_destroy;
  rc = ui_component_registry_register(registry, "container", &vt);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves the global default component registry instance.
 *
 * @param out_registry Pointer to receive the shared default registry.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_component_registry_get_default(struct ui_component_registry **out_registry) {
  ui_error_t rc;

  if (!out_registry) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!g_default_registry) {
    rc = ui_component_registry_create(&g_default_registry);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = ui_component_registry_register_defaults(g_default_registry);
    if (rc != UI_ERROR_NONE) {
      ui_component_registry_destroy(g_default_registry);
      g_default_registry = NULL;
      return rc;
    }
  }

  *out_registry = g_default_registry;
  return UI_ERROR_NONE;
}

/**
 * @brief Shuts down and frees the global default component registry instance.
 *
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_component_registry_shutdown_default(void) {
  if (g_default_registry) {
    ui_component_registry_destroy(g_default_registry);
    g_default_registry = NULL;
  }
  return UI_ERROR_NONE;
}
