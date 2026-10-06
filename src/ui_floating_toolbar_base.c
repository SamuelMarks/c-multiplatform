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

#ifdef UI_TEST_MOCK_ALLOC
/** @brief Mock failure flag for ui_floating_toolbar_base testing. */
int g_floating_toolbar_mock_fail = 0;
/** @brief Mock destroy failure flag for ui_floating_toolbar_base testing. */
int g_floating_toolbar_destroy_mock_fail = 0;

/**
 * @brief Mock implementation of ui_dom_node_append_child for failure testing.
 * @param parent The parent DOM node.
 * @param child The child DOM node to append.
 * @return UI_ERROR_UNKNOWN on injected mock failure, or result of
 * ui_dom_node_append_child.
 */
static ui_error_t
mock_floating_toolbar_append_child(struct ui_dom_node *parent,
                                   struct ui_dom_node *child) {
  if (g_floating_toolbar_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_floating_toolbar_mock_fail == 2) {
    g_floating_toolbar_mock_fail = 1;
    return ui_dom_node_append_child(parent, child);
  }
  return ui_dom_node_append_child(parent, child);
}
#undef ui_dom_node_append_child
/** @cond */
#define ui_dom_node_append_child mock_floating_toolbar_append_child
/** @endcond */

/**
 * @brief Mock implementation of ui_component_mount for failure testing.
 * @param comp The component to mount.
 * @param host The host DOM node.
 * @return UI_ERROR_UNKNOWN on injected mock failure, or result of
 * ui_component_mount.
 */
static ui_error_t mock_floating_toolbar_mount(struct ui_component *comp,
                                              struct ui_dom_node *host) {
  if (g_floating_toolbar_mock_fail == 3 || g_floating_toolbar_mock_fail == 4) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_component_mount(comp, host);
}
#undef ui_component_mount
/** @cond */
#define ui_component_mount mock_floating_toolbar_mount
/** @endcond */

/**
 * @brief Mock implementation of ui_dom_node_destroy for failure testing.
 * @param n DOM node to destroy.
 * @return UI_ERROR_UNKNOWN on injected mock failure, or result of
 * ui_dom_node_destroy.
 */
static ui_error_t mock_floating_toolbar_dom_destroy(struct ui_dom_node *n) {
  if (g_floating_toolbar_destroy_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_dom_node_destroy(n);
}
#undef ui_dom_node_destroy
/** @cond */
#define ui_dom_node_destroy mock_floating_toolbar_dom_destroy
/** @endcond */

/**
 * @brief Mock implementation of ui_component_destroy for failure testing.
 * @param c Component to destroy.
 * @return UI_ERROR_UNKNOWN on injected mock failure, or result of
 * ui_component_destroy.
 */
static ui_error_t mock_floating_toolbar_comp_destroy(struct ui_component *c) {
  if (g_floating_toolbar_destroy_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_component_destroy(c);
}
#undef ui_component_destroy
/** @cond */
#define ui_component_destroy mock_floating_toolbar_comp_destroy
/** @endcond */
#endif

#define UI_FLOATING_TOOLBAR_INITIAL_CAPACITY 4

/**
 * @struct ui_floating_toolbar_base
 * @brief Internal implementation structure for the base floating toolbar
 * component.
 */
struct ui_floating_toolbar_base {
  struct ui_component *component;    /**< Underlying UI component. */
  struct ui_dom_node *root_node;     /**< Root DOM node container. */
  struct ui_dom_node *fab_slot_node; /**< Slot container for primary FAB. */
  struct ui_dom_node
      *actions_container_node; /**< Slot container for action components. */
  struct ui_component
      *fab_component; /**< Reference to primary FAB component. */
  struct ui_component **action_components; /**< Array of action components. */
  size_t action_count;    /**< Current count of action components. */
  size_t action_capacity; /**< Allocated capacity of action components array. */
  enum ui_floating_toolbar_orientation
      orientation;                      /**< Orientation alignment. */
  enum ui_floating_toolbar_state state; /**< Current expansion state. */
};

ui_error_t
ui_floating_toolbar_base_create(struct ui_floating_toolbar_base **out_toolbar) {
  struct ui_floating_toolbar_base *tb = NULL;
  ui_error_t rc;
  ui_error_t rc_cleanup;

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
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(tb->root_node, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(tb->root_node, "role", "toolbar");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(tb->root_node, "class", "ui-floating-toolbar");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(tb->root_node, "data-orientation",
                                 "horizontal");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(tb->root_node, "data-state", "expanded");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* FAB Slot container */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &tb->fab_slot_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(tb->fab_slot_node, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(tb->fab_slot_node, "class",
                                 "ui-floating-toolbar-fab-slot");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_append_child(tb->root_node, tb->fab_slot_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* Actions container */
  rc =
      ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &tb->actions_container_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(tb->actions_container_node, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(tb->actions_container_node, "class",
                                 "ui-floating-toolbar-actions");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_append_child(tb->root_node, tb->actions_container_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  tb->component->shadow_root = tb->root_node;
  *out_toolbar = tb;
  return UI_ERROR_NONE;

cleanup:
  if (tb->actions_container_node != NULL) {
    rc_cleanup = ui_dom_node_destroy(tb->actions_container_node);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  if (tb->fab_slot_node != NULL && tb->fab_slot_node->parent == NULL) {
    rc_cleanup = ui_dom_node_destroy(tb->fab_slot_node);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  if (tb->root_node != NULL) {
    rc_cleanup = ui_dom_node_destroy(tb->root_node);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  rc_cleanup = ui_component_destroy(tb->component);
  if (rc_cleanup != UI_ERROR_NONE) {
    rc = rc_cleanup;
  }
  C_MULTIPLATFORM_FREE(tb);
  return rc;
}

ui_error_t
ui_floating_toolbar_base_destroy(struct ui_floating_toolbar_base *toolbar) {
  ui_error_t rc = UI_ERROR_NONE;
  ui_error_t rc_cleanup;

  if (toolbar == NULL) {
    return UI_ERROR_NONE;
  }

  if (toolbar->action_components != NULL) {
    C_MULTIPLATFORM_FREE(toolbar->action_components);
    toolbar->action_components = NULL;
  }

  if (toolbar->component != NULL) {
    rc_cleanup = ui_component_destroy(toolbar->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
    toolbar->component = NULL;
  }

  C_MULTIPLATFORM_FREE(toolbar);
  return rc;
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
