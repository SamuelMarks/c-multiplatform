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

static int g_scope_counter = 0;

static enum ui_error internal_strdup(const char *src, char **out_str) {
  size_t len;
  char *copy;

  len = strlen(src);
  copy = (char *)UI_MALLOC(len + 1);
  if (!copy) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

#if defined(_MSC_VER)
  strcpy_s(copy, len + 1, src);
#else
  strcpy(copy, src);
#endif

  *out_str = copy;
  return UI_ERROR_NONE;
}

static enum ui_error rewrite_classes_for_node(struct ui_dom_node *node,
                                              const char *scope_id) {
  const char *attr_val;
  struct ui_dom_node *child;
  enum ui_error rc;

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
      char *new_classes = (char *)UI_MALLOC(new_len);
      char *dst;
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

      ui_dom_node_set_attribute(node, "class", new_classes);
      UI_FREE(new_classes);
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

static enum ui_error
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
        char *new_val = (char *)UI_MALLOC(new_len);
        if (!new_val)
          return UI_ERROR_OUT_OF_MEMORY;

#if defined(_MSC_VER)
        sprintf_s(new_val, new_len, "%s-%s", sel->value, scope_id);
#else
        sprintf(new_val, "%s-%s", sel->value, scope_id);
#endif
        UI_FREE(sel->value);
        sel->value = new_val;
      }
      sel = sel->next;
    }
    rule = rule->next;
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_component_create(struct ui_component **out_component) {
  struct ui_component *comp;

  if (!out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  comp = (struct ui_component *)UI_MALLOC(sizeof(struct ui_component));
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

void ui_component_destroy(struct ui_component *component) {
  if (!component) {
    return;
  }

  if (component->shadow_root) {
    ui_dom_node_destroy(component->shadow_root);
  }
  if (component->internal_style) {
    ui_css_stylesheet_destroy(component->internal_style);
  }
  if (component->override_style) {
    ui_css_stylesheet_destroy(component->override_style);
  }
  if (component->bound_properties) {
    ui_css_stylesheet_destroy(component->bound_properties);
  }
  if (component->scope_id) {
    UI_FREE(component->scope_id);
  }

  UI_FREE(component);
}

/** \brief ui_error */
enum ui_error
ui_component_set_default_style(struct ui_component *component,
                               struct ui_css_stylesheet *stylesheet) {
  if (!component || !stylesheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (component->internal_style) {
    ui_css_stylesheet_destroy(component->internal_style);
  }
  component->internal_style = stylesheet;

  return UI_ERROR_NONE;
}

enum ui_error ui_component_inject_style_override(struct ui_component *component,
                                                 const char *css_string) {
  struct ui_css_stylesheet *new_override = NULL;

  if (!component || !css_string) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  (void)ui_css_parse_stylesheet(css_string, &new_override);

  if (component->override_style) {
    ui_css_stylesheet_destroy(component->override_style);
  }
  component->override_style = new_override;

  return UI_ERROR_NONE;
}

enum ui_error ui_component_set_property(struct ui_component *component,
                                        const char *property_name,
                                        const char *property_value) {
  struct ui_css_rule *rule = NULL;
  enum ui_error rc;

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
    ui_css_stylesheet_append_rule(component->bound_properties, rule);
  }

  /* Note: A robust implementation would check if the declaration already exists
     and update it. For this engine, append_declaration adds to the linked list
     which will overwrite previous declarations in the cascade. */
  return ui_css_rule_append_declaration(rule, property_name, property_value, 0);
}

enum ui_error ui_component_mount(struct ui_component *component,
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

enum ui_error ui_component_scope_styles(struct ui_component *component) {
  char scope_buf[64];
  enum ui_error rc;

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

  rc = internal_strdup(scope_buf, &component->scope_id);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (component->shadow_root) {
    rewrite_classes_for_node(component->shadow_root, component->scope_id);
  }

  if (component->internal_style) {
    rewrite_stylesheet_selectors(component->internal_style,
                                 component->scope_id);
  }

  if (component->override_style) {
    rewrite_stylesheet_selectors(component->override_style,
                                 component->scope_id);
  }

  return UI_ERROR_NONE;
}
