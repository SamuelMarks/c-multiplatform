/**
 * @file ui_aria.c
 * @brief Implementation of ARIA state parsing.
 */

/* clang-format off */
#include "ui_aria.h"
#include "ui_internal_mem.h"
#include "ui_web_bridge.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_aria_mock_fail = 0;
int g_aria_mock_fail_idx = -1;
extern int g_malloc_fail_countdown;

static ui_error_t mock_dom_node_get_attribute(const struct ui_dom_node *node,
                                              const char *name,
                                              const char **out_value) {
  if (g_aria_mock_fail == 1) {
    if (g_aria_mock_fail_idx == 0)
      return UI_ERROR_UNKNOWN;
    if (g_aria_mock_fail_idx > 0)
      g_aria_mock_fail_idx--;
  }
  return (ui_dom_node_get_attribute)(node, name, out_value);
}
#undef ui_dom_node_get_attribute
/** @cond */
#define ui_dom_node_get_attribute mock_dom_node_get_attribute
/** @endcond */
#endif

ui_error_t ui_aria_role_from_string(const char *role_str,
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

#ifdef UI_TEST_MOCK_ALLOC
static ui_error_t mock_aria_role_from_string(const char *str,
                                             enum ui_aria_role *out_role) {
  if (g_aria_mock_fail == 2)
    return UI_ERROR_UNKNOWN;
  return (ui_aria_role_from_string)(str, out_role);
}

static ui_error_t mock_aria_state_cleanup(struct ui_aria_state *state) {
  if (g_aria_mock_fail == 4)
    return UI_ERROR_UNKNOWN;
  return (ui_aria_state_cleanup)(state);
}
#endif
#ifdef UI_TEST_MOCK_ALLOC
/** @cond */
#define ui_aria_role_from_string mock_aria_role_from_string
/** @endcond */
/** @cond */
#define ui_aria_state_cleanup mock_aria_state_cleanup
/** @endcond */
#endif
ui_error_t ui_aria_state_parse(const struct ui_dom_node *node,
                               struct ui_aria_state *out_state) {
  const char *val = NULL;
  ui_error_t rc = UI_ERROR_NONE;
  ui_error_t role_rc = UI_ERROR_NONE;
  ui_error_t type_rc = UI_ERROR_NONE;
  ui_error_t dis_rc = UI_ERROR_NONE;
  ui_error_t chk_rc = UI_ERROR_NONE;
  ui_error_t cl_rc = UI_ERROR_NONE;
#if defined(__EMSCRIPTEN__)
  ui_error_t em_rc = UI_ERROR_NONE;
  const char *role_str = NULL;
#endif

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
  rc = ui_dom_node_get_attribute(node, "role", &val);
  if (rc != UI_ERROR_NONE && rc != UI_ERROR_NOT_FOUND) {
    return rc;
  }
  if (rc == UI_ERROR_NONE) {
    role_rc = ui_aria_role_from_string(val, &out_state->role);
    if (role_rc != UI_ERROR_NONE) {
      return role_rc;
    }
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
      type_rc = ui_dom_node_get_attribute(node, "type", &type_val);
      if (type_rc != UI_ERROR_NONE && type_rc != UI_ERROR_NOT_FOUND) {
        return type_rc;
      }
      if (type_rc == UI_ERROR_NONE) {
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
  rc = ui_dom_node_get_attribute(node, "aria-hidden", &val);
  if (rc != UI_ERROR_NONE && rc != UI_ERROR_NOT_FOUND) {
    return rc;
  }
  if (rc == UI_ERROR_NONE) {
    if (strcmp(val, "true") == 0)
      out_state->is_hidden = 1;
  }

  /* Parse disabled (HTML native) or aria-disabled */
  rc = ui_dom_node_get_attribute(node, "disabled", &val);
  if (rc != UI_ERROR_NONE && rc != UI_ERROR_NOT_FOUND) {
    return rc;
  }
  if (rc == UI_ERROR_NONE) {
    out_state->is_disabled = 1;
  } else {
    dis_rc = ui_dom_node_get_attribute(node, "aria-disabled", &val);
    if (dis_rc != UI_ERROR_NONE && dis_rc != UI_ERROR_NOT_FOUND) {
      return dis_rc;
    }
    if (dis_rc == UI_ERROR_NONE) {
      if (strcmp(val, "true") == 0)
        out_state->is_disabled = 1;
    }
  }

  /* Parse aria-expanded */
  rc = ui_dom_node_get_attribute(node, "aria-expanded", &val);
  if (rc != UI_ERROR_NONE && rc != UI_ERROR_NOT_FOUND) {
    return rc;
  }
  if (rc == UI_ERROR_NONE) {
    if (strcmp(val, "true") == 0)
      out_state->is_expanded = 1;
    else if (strcmp(val, "false") == 0)
      out_state->is_expanded = 0;
  }

  /* Parse aria-checked */
  rc = ui_dom_node_get_attribute(node, "aria-checked", &val);
  if (rc != UI_ERROR_NONE && rc != UI_ERROR_NOT_FOUND) {
    return rc;
  }
  if (rc == UI_ERROR_NONE) {
    if (strcmp(val, "true") == 0)
      out_state->is_checked = 1;
    else if (strcmp(val, "false") == 0)
      out_state->is_checked = 0;
    else if (strcmp(val, "mixed") == 0)
      out_state->is_checked = 2;
  } else {
    chk_rc = ui_dom_node_get_attribute(node, "checked", &val);
    if (chk_rc != UI_ERROR_NONE && chk_rc != UI_ERROR_NOT_FOUND) {
      return chk_rc;
    }
    if (chk_rc == UI_ERROR_NONE) {
      out_state->is_checked = 1;
    }
  }

  /* Parse aria-label */
  rc = ui_dom_node_get_attribute(node, "aria-label", &val);
  if (rc != UI_ERROR_NONE && rc != UI_ERROR_NOT_FOUND) {
    return rc;
  }
  if (rc == UI_ERROR_NONE) {
    size_t len = strlen(val);
    out_state->label = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
    if (!out_state->label)
      return UI_ERROR_OUT_OF_MEMORY;
#if defined(_MSC_VER)
    strcpy_s(out_state->label, len + 1, val);
#else
    UI_STRCPY(out_state->label, sizeof(out_state->label), val);
#endif
  }

  /* Parse aria-description */
  rc = ui_dom_node_get_attribute(node, "aria-description", &val);
  if (rc != UI_ERROR_NONE && rc != UI_ERROR_NOT_FOUND) {
    return rc;
  }
  if (rc == UI_ERROR_NONE) {
    size_t len = strlen(val);
    out_state->description = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
    if (!out_state->description) {
      cl_rc = ui_aria_state_cleanup(out_state);
      if (cl_rc != UI_ERROR_NONE) {
        return cl_rc;
      }
      return UI_ERROR_OUT_OF_MEMORY;
    }
#if defined(_MSC_VER)
    strcpy_s(out_state->description, len + 1, val);
#else
    UI_STRCPY(out_state->description, sizeof(out_state->description), val);
#endif
  }

#if defined(__EMSCRIPTEN__)
  em_rc = ui_dom_node_get_attribute(node, "role", &role_str);
  if (em_rc != UI_ERROR_NONE && em_rc != UI_ERROR_NOT_FOUND) {
    return em_rc;
  }
  if (em_rc != UI_ERROR_NONE) {
    role_str = NULL;
  }
  em_rc = ui_web_bridge_set_aria((uint32_t)(uintptr_t)node, role_str,
                                 out_state->label, out_state->is_hidden,
                                 out_state->is_disabled, out_state->is_expanded,
                                 out_state->is_checked);
  if (em_rc != UI_ERROR_NONE) {
    return em_rc;
  }
#endif
  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
#undef ui_aria_role_from_string
#undef ui_aria_state_cleanup
#endif
ui_error_t ui_aria_state_cleanup(struct ui_aria_state *state) {
  if (!state)
    return UI_ERROR_INVALID_ARGUMENT;
  if (state->label) {
    C_MULTIPLATFORM_FREE(state->label);
    state->label = NULL;
  }
  if (state->description) {
    C_MULTIPLATFORM_FREE(state->description);
    state->description = NULL;
  }
  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
ui_error_t run_aria_coverage(void);
ui_error_t run_aria_coverage(void) {
  struct ui_aria_state state;
  struct ui_dom_node *node = NULL;
  int i;
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

  g_aria_mock_fail = 1;
  g_aria_mock_fail_idx = -1;
  ui_aria_state_parse(node, &state);
  g_aria_mock_fail = 0;

  for (i = 0; i < 20; i++) {
    g_aria_mock_fail = 1;
    g_aria_mock_fail_idx = i;
    ui_aria_state_parse(node, &state);
  }
  g_aria_mock_fail = 0;

  ui_dom_node_set_attribute(node, "role", "button");
  g_aria_mock_fail = 2;
  ui_aria_state_parse(node, &state);
  g_aria_mock_fail = 0;

  ui_dom_node_remove_attribute(node, "role");
  ui_dom_node_set_tag_name(node, "button");
  ui_aria_state_parse(node, &state);
  ui_dom_node_set_tag_name(node, "a");
  ui_dom_node_set_attribute(node, "href", "#");
  ui_aria_state_parse(node, &state);
  ui_dom_node_set_tag_name(node, "input");
  ui_dom_node_set_attribute(node, "type", "checkbox");
  ui_aria_state_parse(node, &state);
  ui_dom_node_set_attribute(node, "type", "range");
  ui_aria_state_parse(node, &state);
  ui_dom_node_set_attribute(node, "type", "radio");
  ui_aria_state_parse(node, &state);
  ui_dom_node_set_attribute(node, "type", "text");
  ui_aria_state_parse(node, &state);

  g_aria_mock_fail = 1;
  g_aria_mock_fail_idx = -1;
  ui_aria_state_parse(node, &state);
  g_aria_mock_fail = 0;

  for (i = 0; i < 20; i++) {
    g_aria_mock_fail = 1;
    g_aria_mock_fail_idx = i;
    ui_aria_state_parse(node, &state);
  }

  ui_dom_node_set_attribute(node, "aria-checked", "true");
  ui_aria_state_parse(node, &state);
  ui_dom_node_set_attribute(node, "aria-checked", "false");
  ui_aria_state_parse(node, &state);
  ui_dom_node_set_attribute(node, "aria-checked", "mixed");
  ui_aria_state_parse(node, &state);

  ui_dom_node_set_attribute(node, "aria-pressed", "true");
  ui_aria_state_parse(node, &state);
  ui_dom_node_set_attribute(node, "aria-pressed", "false");
  ui_aria_state_parse(node, &state);
  ui_dom_node_set_attribute(node, "aria-pressed", "mixed");
  ui_aria_state_parse(node, &state);

  g_aria_mock_fail = 4;
  ui_dom_node_set_attribute(node, "aria-description", "foo");
  g_malloc_fail_countdown = 0;
  ui_aria_state_parse(node, &state);
  g_malloc_fail_countdown = -1;
  g_aria_mock_fail = 0;

  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(node);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* expected error */
    }
  }
  return UI_ERROR_NONE;
}
#endif
