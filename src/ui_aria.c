/* clang-format off */
#include "ui_aria.h"
#include "ui_internal_mem.h"
#include "ui_web_bridge.h"
#include <string.h>
/* clang-format on */

enum ui_error ui_aria_role_from_string(const char *role_str,
                                       enum ui_aria_role *out_role) {
  if (!out_role)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_role = UI_ARIA_ROLE_NONE;
  if (!role_str)
    return UI_ERROR_NONE;
  if (strcmp(role_str, "button") == 0)
    *out_role = UI_ARIA_ROLE_BUTTON;
  else if (strcmp(role_str, "checkbox") == 0)
    *out_role = UI_ARIA_ROLE_CHECKBOX;
  else if (strcmp(role_str, "dialog") == 0)
    *out_role = UI_ARIA_ROLE_DIALOG;
  else if (strcmp(role_str, "alert") == 0)
    *out_role = UI_ARIA_ROLE_ALERT;
  else if (strcmp(role_str, "status") == 0)
    *out_role = UI_ARIA_ROLE_STATUS;
  else if (strcmp(role_str, "slider") == 0)
    *out_role = UI_ARIA_ROLE_SLIDER;
  else if (strcmp(role_str, "progressbar") == 0)
    *out_role = UI_ARIA_ROLE_PROGRESSBAR;
  else if (strcmp(role_str, "heading") == 0)
    *out_role = UI_ARIA_ROLE_HEADING;
  else if (strcmp(role_str, "link") == 0)
    *out_role = UI_ARIA_ROLE_LINK;
  else if (strcmp(role_str, "textbox") == 0)
    *out_role = UI_ARIA_ROLE_TEXTBOX;
  else if (strcmp(role_str, "separator") == 0)
    *out_role = UI_ARIA_ROLE_SEPARATOR;
  else if (strcmp(role_str, "list") == 0)
    *out_role = UI_ARIA_ROLE_LIST;
  else if (strcmp(role_str, "listitem") == 0)
    *out_role = UI_ARIA_ROLE_LISTITEM;
  else if (strcmp(role_str, "radiogroup") == 0)
    *out_role = UI_ARIA_ROLE_RADIOGROUP;
  else if (strcmp(role_str, "tablist") == 0)
    *out_role = UI_ARIA_ROLE_TABLIST;
  else if (strcmp(role_str, "navigation") == 0)
    *out_role = UI_ARIA_ROLE_NAVIGATION;
  else if (strcmp(role_str, "group") == 0)
    *out_role = UI_ARIA_ROLE_GROUP;
  return UI_ERROR_NONE;
}

enum ui_error ui_aria_state_parse(const struct ui_dom_node *node,
                                  struct ui_aria_state *out_state) {
  const char *val = NULL;

  if (!out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_state->role = UI_ARIA_ROLE_NONE;
  out_state->is_hidden = 0;
  out_state->is_disabled = 0;
  out_state->is_expanded = -1;
  out_state->is_checked = -1;
  out_state->label = NULL;
  out_state->description = NULL;

  if (!node) {
    return UI_ERROR_NONE; /* Not an element, no ARIA state */
  }
  if (node->type != UI_DOM_NODE_TYPE_ELEMENT) {
    return UI_ERROR_NONE; /* Not an element, no ARIA state */
  }

  /* Parse Role */
  if (ui_dom_node_get_attribute(node, "role", &val) == UI_ERROR_NONE) {
    ui_aria_role_from_string(val, &out_state->role);
  } else if (node->tag_name) {
    /* Implicit roles from tag name */
    if (strcmp(node->tag_name, "button") == 0) {
      out_state->role = UI_ARIA_ROLE_BUTTON;
    } else if (strcmp(node->tag_name, "a") == 0) {
      out_state->role = UI_ARIA_ROLE_LINK;
    } else if (strcmp(node->tag_name, "h1") == 0 ||
               strcmp(node->tag_name, "h2") == 0 ||
               strcmp(node->tag_name, "h3") == 0 ||
               strcmp(node->tag_name, "h4") == 0 ||
               strcmp(node->tag_name, "h5") == 0 ||
               strcmp(node->tag_name, "h6") == 0) {
      out_state->role = UI_ARIA_ROLE_HEADING;
    } else if (strcmp(node->tag_name, "input") == 0) {
      const char *type_val = NULL;
      if (ui_dom_node_get_attribute(node, "type", &type_val) == UI_ERROR_NONE) {
        if (strcmp(type_val, "checkbox") == 0) {
          out_state->role = UI_ARIA_ROLE_CHECKBOX;
        } else if (strcmp(type_val, "range") == 0) {
          out_state->role = UI_ARIA_ROLE_SLIDER;
        } else {
          out_state->role = UI_ARIA_ROLE_TEXTBOX;
        }
      } else {
        out_state->role = UI_ARIA_ROLE_TEXTBOX; /* Default input type */
      }
    } else if (strcmp(node->tag_name, "ul") == 0 ||
               strcmp(node->tag_name, "ol") == 0) {
      out_state->role = UI_ARIA_ROLE_LIST;
    } else if (strcmp(node->tag_name, "li") == 0) {
      out_state->role = UI_ARIA_ROLE_LISTITEM;
    } else if (strcmp(node->tag_name, "nav") == 0) {
      out_state->role = UI_ARIA_ROLE_NAVIGATION;
    }
  }

  /* Parse aria-hidden */
  if (ui_dom_node_get_attribute(node, "aria-hidden", &val) == UI_ERROR_NONE) {
    if (strcmp(val, "true") == 0)
      out_state->is_hidden = 1;
  }

  /* Parse disabled (HTML native) or aria-disabled */
  if (ui_dom_node_get_attribute(node, "disabled", &val) == UI_ERROR_NONE) {
    out_state->is_disabled = 1;
  } else if (ui_dom_node_get_attribute(node, "aria-disabled", &val) ==
             UI_ERROR_NONE) {
    if (strcmp(val, "true") == 0)
      out_state->is_disabled = 1;
  }

  /* Parse aria-expanded */
  if (ui_dom_node_get_attribute(node, "aria-expanded", &val) == UI_ERROR_NONE) {
    if (strcmp(val, "true") == 0)
      out_state->is_expanded = 1;
    else if (strcmp(val, "false") == 0)
      out_state->is_expanded = 0;
  }

  /* Parse aria-checked */
  if (ui_dom_node_get_attribute(node, "aria-checked", &val) == UI_ERROR_NONE) {
    if (strcmp(val, "true") == 0)
      out_state->is_checked = 1;
    else if (strcmp(val, "false") == 0)
      out_state->is_checked = 0;
    else if (strcmp(val, "mixed") == 0)
      out_state->is_checked = 2;
  } else if (ui_dom_node_get_attribute(node, "checked", &val) ==
             UI_ERROR_NONE) {
    out_state->is_checked = 1;
  }

  /* Parse aria-label */
  if (ui_dom_node_get_attribute(node, "aria-label", &val) == UI_ERROR_NONE) {
    size_t len = strlen(val);
    out_state->label = (char *)UI_MALLOC(len + 1);
    if (!out_state->label)
      return UI_ERROR_OUT_OF_MEMORY;
#if defined(_MSC_VER)
    strcpy_s(out_state->label, len + 1, val);
#else
    strcpy(out_state->label, val);
#endif
  }

  /* Parse aria-description */
  if (ui_dom_node_get_attribute(node, "aria-description", &val) ==
      UI_ERROR_NONE) {
    size_t len = strlen(val);
    out_state->description = (char *)UI_MALLOC(len + 1);
    if (!out_state->description) {
      (void)ui_aria_state_cleanup(out_state);
      return UI_ERROR_OUT_OF_MEMORY;
    }
#if defined(_MSC_VER)
    strcpy_s(out_state->description, len + 1, val);
#else
    strcpy(out_state->description, val);
#endif
  }

#if defined(__EMSCRIPTEN__)
  {
    const char *role_str = NULL;
    if (ui_dom_node_get_attribute(node, "role", &role_str) != UI_ERROR_NONE)
      role_str = NULL;
    ui_web_bridge_set_aria((uint32_t)(uintptr_t)node, role_str,
                           out_state->label, out_state->is_hidden,
                           out_state->is_disabled, out_state->is_expanded,
                           out_state->is_checked);
  }
#endif
  return UI_ERROR_NONE;
}

enum ui_error ui_aria_state_cleanup(struct ui_aria_state *state) {
  if (!state)
    return UI_ERROR_INVALID_ARGUMENT;
  if (state->label) {
    UI_FREE(state->label);
    state->label = NULL;
  }
  if (state->description) {
    UI_FREE(state->description);
    state->description = NULL;
  }
  return UI_ERROR_NONE;
}
