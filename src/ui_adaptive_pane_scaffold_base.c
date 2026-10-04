/**
 * @file ui_adaptive_pane_scaffold_base.c
 * @brief Implementation of base unstyled adaptive multi-pane scaffold
 * coordinator.
 */

/* clang-format off */
#include "ui_adaptive_pane_scaffold_base.h"
#include "ui_dom_node.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct ui_adaptive_pane_scaffold_base {
  struct ui_component *component;
  struct ui_dom_node *root_node;
  struct ui_dom_node *primary_slot_node;
  struct ui_dom_node *secondary_slot_node;
  struct ui_dom_node *supporting_slot_node;
  struct ui_component *primary_pane;
  struct ui_component *secondary_pane;
  struct ui_component *supporting_pane;
  enum ui_adaptive_scaffold_type type;
  enum ui_adaptive_window_width_class width_class;
  enum ui_adaptive_pane_role active_pane;
  int dialog_levitation;
};

static const char *
sampler_width_class_to_str(enum ui_adaptive_window_width_class wc) {
  switch (wc) {
  case UI_WINDOW_WIDTH_COMPACT:
    return "compact";
  case UI_WINDOW_WIDTH_MEDIUM:
    return "medium";
  case UI_WINDOW_WIDTH_EXPANDED:
  default:
    return "expanded";
  }
}

static ui_error_t sampler_update_pane_visibility(
    struct ui_adaptive_pane_scaffold_base *scaffold) {
  ui_error_t rc;

  if (scaffold->width_class == UI_WINDOW_WIDTH_COMPACT) {
    rc = ui_dom_node_set_attribute(
        scaffold->primary_slot_node, "data-visible",
        (scaffold->active_pane == UI_PANE_ROLE_PRIMARY) ? "1" : "0");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = ui_dom_node_set_attribute(
        scaffold->secondary_slot_node, "data-visible",
        (scaffold->active_pane == UI_PANE_ROLE_SECONDARY) ? "1" : "0");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = ui_dom_node_set_attribute(
        scaffold->supporting_slot_node, "data-visible",
        (scaffold->active_pane == UI_PANE_ROLE_SUPPORTING ||
         scaffold->dialog_levitation)
            ? "1"
            : "0");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else if (scaffold->width_class == UI_WINDOW_WIDTH_MEDIUM) {
    rc = ui_dom_node_set_attribute(scaffold->primary_slot_node, "data-visible",
                                   "1");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = ui_dom_node_set_attribute(scaffold->secondary_slot_node,
                                   "data-visible", "1");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = ui_dom_node_set_attribute(scaffold->supporting_slot_node,
                                   "data-visible",
                                   scaffold->dialog_levitation ? "1" : "0");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
    /* Expanded: all panes visible side by side */
    rc = ui_dom_node_set_attribute(scaffold->primary_slot_node, "data-visible",
                                   "1");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = ui_dom_node_set_attribute(scaffold->secondary_slot_node,
                                   "data-visible", "1");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = ui_dom_node_set_attribute(scaffold->supporting_slot_node,
                                   "data-visible", "1");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_adaptive_pane_scaffold_base_create(
    enum ui_adaptive_scaffold_type type,
    struct ui_adaptive_pane_scaffold_base **out_scaffold) {
  struct ui_adaptive_pane_scaffold_base *scaffold = NULL;
  ui_error_t rc;

  if (out_scaffold == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scaffold = (struct ui_adaptive_pane_scaffold_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_adaptive_pane_scaffold_base));
  if (scaffold == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(scaffold, 0, sizeof(*scaffold));

  scaffold->type = type;
  scaffold->width_class = UI_WINDOW_WIDTH_EXPANDED;
  scaffold->active_pane = UI_PANE_ROLE_PRIMARY;
  scaffold->dialog_levitation = 0;

  rc = ui_component_create(&scaffold->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }

  /* Root container */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &scaffold->root_node);
  if (rc != UI_ERROR_NONE) {
    ui_component_destroy(scaffold->component);
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }

  rc = ui_dom_node_set_tag_name(scaffold->root_node, "div");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->root_node);
    ui_component_destroy(scaffold->component);
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }

  rc = ui_dom_node_set_attribute(scaffold->root_node, "class",
                                 "ui-adaptive-pane-scaffold");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->root_node);
    ui_component_destroy(scaffold->component);
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }

  rc = ui_dom_node_set_attribute(scaffold->root_node, "data-layout",
                                 (type == UI_ADAPTIVE_SCAFFOLD_LIST_DETAIL)
                                     ? "list-detail"
                                     : "supporting-pane");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->root_node);
    ui_component_destroy(scaffold->component);
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }

  rc = ui_dom_node_set_attribute(scaffold->root_node, "data-width-class",
                                 "expanded");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->root_node);
    ui_component_destroy(scaffold->component);
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }

  /* Primary slot */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT,
                          &scaffold->primary_slot_node);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->root_node);
    ui_component_destroy(scaffold->component);
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }
  rc = ui_dom_node_set_tag_name(scaffold->primary_slot_node, "div");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->primary_slot_node);
    ui_dom_node_destroy(scaffold->root_node);
    ui_component_destroy(scaffold->component);
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }
  rc = ui_dom_node_set_attribute(scaffold->primary_slot_node, "class",
                                 "ui-adaptive-pane-slot ui-pane-primary");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->primary_slot_node);
    ui_dom_node_destroy(scaffold->root_node);
    ui_component_destroy(scaffold->component);
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }
  rc = ui_dom_node_append_child(scaffold->root_node,
                                scaffold->primary_slot_node);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->primary_slot_node);
    ui_dom_node_destroy(scaffold->root_node);
    ui_component_destroy(scaffold->component);
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }

  /* Secondary slot */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT,
                          &scaffold->secondary_slot_node);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->root_node);
    ui_component_destroy(scaffold->component);
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }
  rc = ui_dom_node_set_tag_name(scaffold->secondary_slot_node, "div");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->secondary_slot_node);
    ui_dom_node_destroy(scaffold->root_node);
    ui_component_destroy(scaffold->component);
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }
  rc = ui_dom_node_set_attribute(scaffold->secondary_slot_node, "class",
                                 "ui-adaptive-pane-slot ui-pane-secondary");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->secondary_slot_node);
    ui_dom_node_destroy(scaffold->root_node);
    ui_component_destroy(scaffold->component);
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }
  rc = ui_dom_node_append_child(scaffold->root_node,
                                scaffold->secondary_slot_node);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->secondary_slot_node);
    ui_dom_node_destroy(scaffold->root_node);
    ui_component_destroy(scaffold->component);
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }

  /* Supporting slot */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT,
                          &scaffold->supporting_slot_node);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->root_node);
    ui_component_destroy(scaffold->component);
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }
  rc = ui_dom_node_set_tag_name(scaffold->supporting_slot_node, "div");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->supporting_slot_node);
    ui_dom_node_destroy(scaffold->root_node);
    ui_component_destroy(scaffold->component);
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }
  rc = ui_dom_node_set_attribute(scaffold->supporting_slot_node, "class",
                                 "ui-adaptive-pane-slot ui-pane-supporting");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->supporting_slot_node);
    ui_dom_node_destroy(scaffold->root_node);
    ui_component_destroy(scaffold->component);
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }
  rc = ui_dom_node_append_child(scaffold->root_node,
                                scaffold->supporting_slot_node);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->supporting_slot_node);
    ui_dom_node_destroy(scaffold->root_node);
    ui_component_destroy(scaffold->component);
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }

  rc = sampler_update_pane_visibility(scaffold);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(scaffold->root_node);
    ui_component_destroy(scaffold->component);
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }

  scaffold->component->shadow_root = scaffold->root_node;
  *out_scaffold = scaffold;
  return UI_ERROR_NONE;
}

ui_error_t ui_adaptive_pane_scaffold_base_destroy(
    struct ui_adaptive_pane_scaffold_base *scaffold) {
  if (scaffold == NULL) {
    return UI_ERROR_NONE;
  }

  if (scaffold->component != NULL) {
    ui_component_destroy(scaffold->component);
    scaffold->component = NULL;
  }

  C_MULTIPLATFORM_FREE(scaffold);
  return UI_ERROR_NONE;
}

ui_error_t ui_adaptive_pane_scaffold_base_get_component(
    struct ui_adaptive_pane_scaffold_base *scaffold,
    struct ui_component **out_component) {
  if (scaffold == NULL || out_component == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_component = scaffold->component;
  return UI_ERROR_NONE;
}

ui_error_t ui_adaptive_pane_scaffold_base_set_window_width_class(
    struct ui_adaptive_pane_scaffold_base *scaffold,
    enum ui_adaptive_window_width_class width_class) {
  ui_error_t rc;

  if (scaffold == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scaffold->width_class = width_class;
  rc = ui_dom_node_set_attribute(scaffold->root_node, "data-width-class",
                                 sampler_width_class_to_str(width_class));
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return sampler_update_pane_visibility(scaffold);
}

ui_error_t ui_adaptive_pane_scaffold_base_get_window_width_class(
    const struct ui_adaptive_pane_scaffold_base *scaffold,
    enum ui_adaptive_window_width_class *out_width_class) {
  if (scaffold == NULL || out_width_class == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_width_class = scaffold->width_class;
  return UI_ERROR_NONE;
}

ui_error_t ui_adaptive_pane_scaffold_base_set_pane(
    struct ui_adaptive_pane_scaffold_base *scaffold,
    enum ui_adaptive_pane_role role, struct ui_component *pane_component) {
  struct ui_dom_node *target_slot = NULL;
  ui_error_t rc;

  if (scaffold == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  switch (role) {
  case UI_PANE_ROLE_PRIMARY:
    scaffold->primary_pane = pane_component;
    target_slot = scaffold->primary_slot_node;
    break;
  case UI_PANE_ROLE_SECONDARY:
    scaffold->secondary_pane = pane_component;
    target_slot = scaffold->secondary_slot_node;
    break;
  case UI_PANE_ROLE_SUPPORTING:
  default:
    scaffold->supporting_pane = pane_component;
    target_slot = scaffold->supporting_slot_node;
    break;
  }

  if (pane_component != NULL) {
    rc = ui_component_mount(pane_component, target_slot);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_adaptive_pane_scaffold_base_get_pane(
    const struct ui_adaptive_pane_scaffold_base *scaffold,
    enum ui_adaptive_pane_role role, struct ui_component **out_pane_component) {
  if (scaffold == NULL || out_pane_component == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  switch (role) {
  case UI_PANE_ROLE_PRIMARY:
    *out_pane_component = scaffold->primary_pane;
    break;
  case UI_PANE_ROLE_SECONDARY:
    *out_pane_component = scaffold->secondary_pane;
    break;
  case UI_PANE_ROLE_SUPPORTING:
  default:
    *out_pane_component = scaffold->supporting_pane;
    break;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_adaptive_pane_scaffold_base_set_active_pane(
    struct ui_adaptive_pane_scaffold_base *scaffold,
    enum ui_adaptive_pane_role role) {
  if (scaffold == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scaffold->active_pane = role;
  return sampler_update_pane_visibility(scaffold);
}

ui_error_t ui_adaptive_pane_scaffold_base_get_active_pane(
    const struct ui_adaptive_pane_scaffold_base *scaffold,
    enum ui_adaptive_pane_role *out_role) {
  if (scaffold == NULL || out_role == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_role = scaffold->active_pane;
  return UI_ERROR_NONE;
}

ui_error_t ui_adaptive_pane_scaffold_base_set_dialog_levitation(
    struct ui_adaptive_pane_scaffold_base *scaffold, int levitate_in_dialog) {
  ui_error_t rc;

  if (scaffold == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scaffold->dialog_levitation = levitate_in_dialog ? 1 : 0;
  rc = ui_dom_node_set_attribute(scaffold->root_node, "data-dialog-levitated",
                                 scaffold->dialog_levitation ? "1" : "0");
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return sampler_update_pane_visibility(scaffold);
}

ui_error_t ui_adaptive_pane_scaffold_base_is_dialog_levitated(
    const struct ui_adaptive_pane_scaffold_base *scaffold, int *out_levitated) {
  if (scaffold == NULL || out_levitated == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_levitated = scaffold->dialog_levitation;
  return UI_ERROR_NONE;
}
