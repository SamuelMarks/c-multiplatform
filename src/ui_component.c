/**
 * @file ui_component.c
 * @brief ui_component.c implementation.
 */
/* clang-format off */
#include "ui_component.h"
#include "ui_internal_mem.h"
#include <stddef.h>

#include "ui_css_parser.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

/** @brief Global scope counter for generating unique IDs */
static int g_scope_counter = 0;

/**
 * @brief rewrite_classes_for_node.
 * @param node Parameter node.
 * @param scope_id Parameter scope_id.
 * @return Return value.
 */
static ui_error_t rewrite_classes_for_node(struct ui_dom_node *node,
                                           const char *scope_id) {
  const char *attr_val;
  struct ui_dom_node *child;
  ui_error_t rc;
  if (ui_dom_node_get_attribute(node, "class", &attr_val) == UI_ERROR_NONE) {
    size_t word_count = 0;
    const char *p = attr_val;
    while (*p) {
      while (*p == ' ')
        p++;
      if (*p) {
        word_count++;
        while (*p && *p != ' ')
          p++;
      }
    }
    {
      size_t new_len =
          strlen(attr_val) + word_count * (strlen(scope_id) + 2) + 1;
      char *new_classes = (char *)C_MULTIPLATFORM_MALLOC(new_len);
      char *dst;
      ui_error_t set_rc;
      if (!new_classes)
        return UI_ERROR_OUT_OF_MEMORY;

      dst = new_classes;
      p = attr_val;
      while (*p) {
        while (*p == ' ')
          p++;
        if (*p) {
          if (dst != new_classes)
            *dst++ = ' ';
          while (*p && *p != ' ') {
            *dst++ = *p++;
          }
          *dst++ = '-';
#if defined(_MSC_VER)
          strcpy_s(dst, new_len - (dst - new_classes), scope_id);
#else
          strcpy(dst, scope_id);
#endif
          dst += strlen(scope_id);
        }
      }
      *dst = '\0';

      set_rc = ui_dom_node_set_attribute(node, "class", new_classes);
      if (set_rc != UI_ERROR_NONE) {
        C_MULTIPLATFORM_FREE(new_classes);
        return set_rc;
      }
      C_MULTIPLATFORM_FREE(new_classes);
    }
  }

  child = node->first_child;
  while (child) {
    rc = rewrite_classes_for_node(child, scope_id);
    if (rc != UI_ERROR_NONE)
      return rc;
    child = child->next_sibling;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief rewrite_stylesheet_selectors.
 * @param stylesheet Parameter stylesheet.
 * @param scope_id Parameter scope_id.
 * @return Return value.
 */
static ui_error_t
rewrite_stylesheet_selectors(struct ui_css_stylesheet *stylesheet,
                             const char *scope_id) {
  struct ui_css_rule *rule;
  struct ui_css_selector *sel;

  rule = stylesheet->rules;
  while (rule) {
    sel = rule->selectors;
    while (sel) {
      if (sel->type == UI_CSS_SELECTOR_TYPE_CLASS) {
        size_t new_len = strlen(sel->value) + strlen(scope_id) + 2;
        char *new_val = (char *)C_MULTIPLATFORM_MALLOC(new_len);
        if (!new_val)
          return UI_ERROR_OUT_OF_MEMORY;

#if defined(_MSC_VER)
        sprintf_s(new_val, new_len, "%s-%s", sel->value, scope_id);
#else
        sprintf(new_val, "%s-%s", sel->value, scope_id);
#endif
        C_MULTIPLATFORM_FREE(sel->value);
        sel->value = new_val;
      }
      sel = sel->next;
    }
    rule = rule->next;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_component_create.
 * @param out_component Parameter out_component.
 * @return Return value.
 */
ui_error_t ui_component_create(struct ui_component **out_component) {
  struct ui_component *comp;

  if (!out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  comp = (struct ui_component *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_component));
  if (!comp) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  comp->shadow_root = NULL;
  comp->internal_style = NULL;
  comp->override_style = NULL;
  comp->bound_properties = NULL;
  comp->host_node = NULL;
  comp->scope_id = NULL;

  *out_component = comp;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_component_destroy.
 * @param component Parameter component.
 * @return Return value.
 */
ui_error_t ui_component_destroy(struct ui_component *component) {
  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (component->shadow_root) {
    ui_error_t rc = ui_dom_node_destroy(component->shadow_root);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  if (component->internal_style) {
    ui_error_t rc = ui_css_stylesheet_destroy(component->internal_style);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  if (component->override_style) {
    ui_error_t rc = ui_css_stylesheet_destroy(component->override_style);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  if (component->bound_properties) {
    ui_error_t rc = ui_css_stylesheet_destroy(component->bound_properties);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  if (component->scope_id) {
    C_MULTIPLATFORM_FREE(component->scope_id);
  }

  C_MULTIPLATFORM_FREE(component);
  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
ui_error_t
ui_component_set_default_style(struct ui_component *component,
                               struct ui_css_stylesheet *stylesheet) {
  if (!component || !stylesheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (component->internal_style) {
    (void)ui_css_stylesheet_destroy(component->internal_style);
  }
  component->internal_style = stylesheet;

  return UI_ERROR_NONE;
}

/**
 * @brief ui_component_inject_style_override.
 * @param component Parameter component.
 * @param css_string Parameter css_string.
 * @return Return value.
 */
ui_error_t ui_component_inject_style_override(struct ui_component *component,
                                              const char *css_string) {
  struct ui_css_stylesheet *new_override = NULL;

  if (!component || !css_string) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  {
    ui_error_t rc = ui_css_parse_stylesheet(css_string, &new_override);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  if (component->override_style) {
    (void)ui_css_stylesheet_destroy(component->override_style);
  }
  component->override_style = new_override;

  return UI_ERROR_NONE;
}

/**
 * @brief ui_component_set_property.
 * @param component Parameter component.
 * @param property_name Parameter property_name.
 * @param property_value Parameter property_value.
 * @return Return value.
 */
ui_error_t ui_component_set_property(struct ui_component *component,
                                     const char *property_name,
                                     const char *property_value) {
  struct ui_css_rule *rule = NULL;
  ui_error_t rc;

  if (!component || !property_name || !property_value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!component->bound_properties) {
    rc = ui_css_stylesheet_create(&component->bound_properties);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  /* Check if we already have a universal rule for bound properties */
  if (component->bound_properties->rules) {
    rule = component->bound_properties->rules;
  } else {
    rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc =
        ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_UNIVERSAL, NULL);
    if (rc != UI_ERROR_NONE) {
      ui_css_rule_destroy(rule);
      return rc;
    }
    (void)ui_css_stylesheet_append_rule(component->bound_properties, rule);
  }

  /* Note: A robust implementation would check if the declaration already exists
     and update it. For this engine, append_declaration adds to the linked list
     which will overwrite previous declarations in the cascade. */
  return ui_css_rule_append_declaration(rule, property_name, property_value, 0);
}

/**
 * @brief ui_component_mount.
 * @param component Parameter component.
 * @param host_node Parameter host_node.
 * @return Return value.
 */
ui_error_t ui_component_mount(struct ui_component *component,
                              struct ui_dom_node *host_node) {
  if (!component || !host_node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* For now, just attach the reference. A real implementation might
   * append the shadow_root to a hidden slot or track it via layout engine.
   */
  component->host_node = host_node;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_component_scope_styles.
 * @param component Parameter component.
 * @return Return value.
 */
ui_error_t ui_component_scope_styles(struct ui_component *component) {
  char scope_buf[64];
  ui_error_t rc;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (component->scope_id) {
    return UI_ERROR_INVALID_ARGUMENT; /* Already scoped */
  }

  g_scope_counter++;
#if defined(_MSC_VER)
  sprintf_s(scope_buf, sizeof(scope_buf), "uiscope-%d", g_scope_counter);
#else
  sprintf(scope_buf, "uiscope-%d", g_scope_counter);
#endif

  rc = ((component->scope_id = C_MULTIPLATFORM_STRDUP(scope_buf))
            ? UI_ERROR_NONE
            : UI_ERROR_OUT_OF_MEMORY);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (component->shadow_root) {
    rc = rewrite_classes_for_node(component->shadow_root, component->scope_id);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  if (component->internal_style) {
    rc = rewrite_stylesheet_selectors(component->internal_style,
                                      component->scope_id);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  if (component->override_style) {
    rc = rewrite_stylesheet_selectors(component->override_style,
                                      component->scope_id);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  return UI_ERROR_NONE;
}
