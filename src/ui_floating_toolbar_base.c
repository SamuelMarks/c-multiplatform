/**
 * @file ui_floating_toolbar_base.c
 * @brief Implementation of the base unstyled floating toolbar component.
 */

/* clang-format off */
#include "ui_floating_toolbar_base.h"
#include "ui_dom_node.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#define UI_FLOATING_TOOLBAR_INITIAL_CAPACITY 4

struct ui_floating_toolbar_base {
  struct ui_component *component;
  struct ui_dom_node *root_node;
  struct ui_dom_node *fab_slot_node;
  struct ui_dom_node *actions_container_node;
  struct ui_component *fab_component;
  struct ui_component **action_components;
  size_t action_count;
  size_t action_capacity;
  enum ui_floating_toolbar_orientation orientation;
  enum ui_floating_toolbar_state state;
};

ui_error_t
ui_floating_toolbar_base_create(struct ui_floating_toolbar_base **out_toolbar) {
  struct ui_floating_toolbar_base *tb = NULL;
  ui_error_t rc;

  if (out_toolbar == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tb = (struct ui_floating_toolbar_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_floating_toolbar_base));
  if (tb == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(tb, 0, sizeof(*tb));

  tb->orientation = UI_FLOATING_TOOLBAR_HORIZONTAL;
  tb->state = UI_FLOATING_TOOLBAR_EXPANDED;

  rc = ui_component_create(&tb->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

  /* Root toolbar container */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &tb->root_node);
  if (rc != UI_ERROR_NONE) {
    ui_component_destroy(tb->component);
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

  rc = ui_dom_node_set_tag_name(tb->root_node, "div");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(tb->root_node);
    ui_component_destroy(tb->component);
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

  rc = ui_dom_node_set_attribute(tb->root_node, "role", "toolbar");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(tb->root_node);
    ui_component_destroy(tb->component);
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

  rc = ui_dom_node_set_attribute(tb->root_node, "class", "ui-floating-toolbar");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(tb->root_node);
    ui_component_destroy(tb->component);
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

  rc = ui_dom_node_set_attribute(tb->root_node, "data-orientation",
                                 "horizontal");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(tb->root_node);
    ui_component_destroy(tb->component);
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

  rc = ui_dom_node_set_attribute(tb->root_node, "data-state", "expanded");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(tb->root_node);
    ui_component_destroy(tb->component);
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

  /* FAB Slot container */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &tb->fab_slot_node);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(tb->root_node);
    ui_component_destroy(tb->component);
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

  rc = ui_dom_node_set_tag_name(tb->fab_slot_node, "div");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(tb->fab_slot_node);
    ui_dom_node_destroy(tb->root_node);
    ui_component_destroy(tb->component);
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

  rc = ui_dom_node_set_attribute(tb->fab_slot_node, "class",
                                 "ui-floating-toolbar-fab-slot");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(tb->fab_slot_node);
    ui_dom_node_destroy(tb->root_node);
    ui_component_destroy(tb->component);
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

  rc = ui_dom_node_append_child(tb->root_node, tb->fab_slot_node);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(tb->fab_slot_node);
    ui_dom_node_destroy(tb->root_node);
    ui_component_destroy(tb->component);
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

  /* Actions container */
  rc =
      ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &tb->actions_container_node);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(tb->root_node);
    ui_component_destroy(tb->component);
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

  rc = ui_dom_node_set_tag_name(tb->actions_container_node, "div");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(tb->actions_container_node);
    ui_dom_node_destroy(tb->root_node);
    ui_component_destroy(tb->component);
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

  rc = ui_dom_node_set_attribute(tb->actions_container_node, "class",
                                 "ui-floating-toolbar-actions");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(tb->actions_container_node);
    ui_dom_node_destroy(tb->root_node);
    ui_component_destroy(tb->component);
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

  rc = ui_dom_node_append_child(tb->root_node, tb->actions_container_node);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(tb->actions_container_node);
    ui_dom_node_destroy(tb->root_node);
    ui_component_destroy(tb->component);
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

  tb->component->shadow_root = tb->root_node;
  *out_toolbar = tb;
  return UI_ERROR_NONE;
}

ui_error_t
ui_floating_toolbar_base_destroy(struct ui_floating_toolbar_base *toolbar) {
  if (toolbar == NULL) {
    return UI_ERROR_NONE;
  }

  if (toolbar->action_components != NULL) {
    C_MULTIPLATFORM_FREE(toolbar->action_components);
    toolbar->action_components = NULL;
  }

  if (toolbar->component != NULL) {
    ui_component_destroy(toolbar->component);
    toolbar->component = NULL;
  }

  C_MULTIPLATFORM_FREE(toolbar);
  return UI_ERROR_NONE;
}

ui_error_t
ui_floating_toolbar_base_get_component(struct ui_floating_toolbar_base *toolbar,
                                       struct ui_component **out_component) {
  if (toolbar == NULL || out_component == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_component = toolbar->component;
  return UI_ERROR_NONE;
}

ui_error_t ui_floating_toolbar_base_set_orientation(
    struct ui_floating_toolbar_base *toolbar,
    enum ui_floating_toolbar_orientation orientation) {
  ui_error_t rc;

  if (toolbar == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toolbar->orientation = orientation;
  rc = ui_dom_node_set_attribute(toolbar->root_node, "data-orientation",
                                 (orientation == UI_FLOATING_TOOLBAR_VERTICAL)
                                     ? "vertical"
                                     : "horizontal");
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_floating_toolbar_base_get_orientation(
    const struct ui_floating_toolbar_base *toolbar,
    enum ui_floating_toolbar_orientation *out_orientation) {
  if (toolbar == NULL || out_orientation == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_orientation = toolbar->orientation;
  return UI_ERROR_NONE;
}

ui_error_t
ui_floating_toolbar_base_set_state(struct ui_floating_toolbar_base *toolbar,
                                   enum ui_floating_toolbar_state state) {
  ui_error_t rc;

  if (toolbar == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toolbar->state = state;
  rc = ui_dom_node_set_attribute(
      toolbar->root_node, "data-state",
      (state == UI_FLOATING_TOOLBAR_COLLAPSED) ? "collapsed" : "expanded");
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_floating_toolbar_base_get_state(
    const struct ui_floating_toolbar_base *toolbar,
    enum ui_floating_toolbar_state *out_state) {
  if (toolbar == NULL || out_state == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_state = toolbar->state;
  return UI_ERROR_NONE;
}

ui_error_t
ui_floating_toolbar_base_set_fab_slot(struct ui_floating_toolbar_base *toolbar,
                                      struct ui_component *fab_component) {
  ui_error_t rc;

  if (toolbar == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toolbar->fab_component = fab_component;
  if (fab_component != NULL) {
    rc = ui_component_mount(fab_component, toolbar->fab_slot_node);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t
ui_floating_toolbar_base_append_action(struct ui_floating_toolbar_base *toolbar,
                                       struct ui_component *action_component) {
  ui_error_t rc;

  if (toolbar == NULL || action_component == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (toolbar->action_count >= toolbar->action_capacity) {
    size_t new_cap = (toolbar->action_capacity == 0)
                         ? UI_FLOATING_TOOLBAR_INITIAL_CAPACITY
                         : toolbar->action_capacity * 2;
    struct ui_component **new_arr =
        (struct ui_component **)C_MULTIPLATFORM_REALLOC(
            toolbar->action_components,
            new_cap * sizeof(struct ui_component *));
    if (new_arr == NULL) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    toolbar->action_components = new_arr;
    toolbar->action_capacity = new_cap;
  }

  rc = ui_component_mount(action_component, toolbar->actions_container_node);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  toolbar->action_components[toolbar->action_count] = action_component;
  toolbar->action_count++;

  return UI_ERROR_NONE;
}

ui_error_t ui_floating_toolbar_base_get_action_count(
    const struct ui_floating_toolbar_base *toolbar, size_t *out_count) {
  if (toolbar == NULL || out_count == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = toolbar->action_count;
  return UI_ERROR_NONE;
}
