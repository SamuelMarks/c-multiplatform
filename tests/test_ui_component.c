/* clang-format off */
#include <stdio.h>
#include <string.h>
#include "../include/ui_component.h"
#include "../include/ui_dom_node.h"
#include "../include/ui_cssom.h"
#include "../src/ui_internal_mem.h"
/* clang-format on */

extern int g_malloc_fail_countdown;
extern int g_malloc_called;

static int run_normal_tests(void) {
  struct ui_component *comp = NULL;
  struct ui_dom_node *host_node = NULL;
  struct ui_dom_node *shadow_root = NULL;
  struct ui_css_stylesheet *style = NULL;
  struct ui_dom_node *btn_node = NULL;
  struct ui_css_rule *btn_rule = NULL;
  const char *scoped_class = NULL;
  enum ui_error rc;

  printf("Running normal ui_component tests...\n");

  /* 1. Test standard lifecycle */
  rc = ui_component_create(&comp);
  if (rc != UI_ERROR_NONE || !comp) {
    printf("Failed to create component\n");
    return 1;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &host_node);
  if (rc != UI_ERROR_NONE || !host_node) {
    printf("Failed to create host node\n");
    return 1;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &shadow_root);
  if (rc != UI_ERROR_NONE || !shadow_root) {
    printf("Failed to create shadow root\n");
    return 1;
  }

  rc = ui_css_stylesheet_create(&style);
  if (rc != UI_ERROR_NONE || !style) {
    printf("Failed to create stylesheet\n");
    return 1;
  }

  /* Assign opaque state via API */
  comp->shadow_root = shadow_root;
  rc = ui_component_set_default_style(comp, style);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to set default style\n");
    return 1;
  }

  /* Test string injection override */
  rc = ui_component_inject_style_override(comp, ".btn { color: red; }");
  if (rc != UI_ERROR_NONE) {
    printf("Failed to inject style override\n");
    return 1;
  }
  if (!comp->override_style) {
    printf("Override style was not generated\n");
    return 1;
  }

  /* Test replacing default and override style */
  rc = ui_css_stylesheet_create(&style);
  ui_component_set_default_style(comp, style);
  ui_component_inject_style_override(comp, ".btn2 { color: blue; }");

  /* Add a node to shadow root to test scoping with trailing spaces */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &btn_node);
  ui_dom_node_set_tag_name(btn_node, "button");
  ui_dom_node_set_attribute(btn_node, "class", "btn btn-primary ");
  ui_dom_node_append_child(shadow_root, btn_node);

  /* Add another node to test scoping without trailing spaces */
  struct ui_dom_node *btn2_node;
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &btn2_node);
  ui_dom_node_set_tag_name(btn2_node, "button");
  ui_dom_node_set_attribute(btn2_node, "class", "btn btn-secondary");
  ui_dom_node_append_child(shadow_root, btn2_node);

  /* Add a matching rule to internal style and a non-class rule */
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &btn_rule);
  ui_css_rule_append_selector(btn_rule, UI_CSS_SELECTOR_TYPE_CLASS, "btn");
  ui_css_rule_append_declaration(btn_rule, "color", "blue", 0);
  ui_css_stylesheet_append_rule(style, btn_rule);

  struct ui_css_rule *tag_rule;
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &tag_rule);
  ui_css_rule_append_selector(tag_rule, UI_CSS_SELECTOR_TYPE_TAG, "button");
  ui_css_stylesheet_append_rule(style, tag_rule);

  /* Apply Scoped CSS encapsulation */
  rc = ui_component_scope_styles(comp);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to scope styles\n");
    return 1;
  }

  /* Create an empty component and scope to test NULL properties branches */
  struct ui_component *empty_comp;
  ui_component_create(&empty_comp);
  ui_component_scope_styles(empty_comp);
  ui_component_destroy(empty_comp);

  /* Try to scope again (should return INVALID_ARGUMENT because already scoped)
   */
  if (ui_component_scope_styles(comp) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Test Property Binding */
  rc = ui_component_set_property(comp, "--primary-color", "#f00");
  if (rc != UI_ERROR_NONE) {
    printf("Failed to set property\n");
    return 1;
  }

  /* Test Property Binding when it already exists */
  rc = ui_component_set_property(comp, "--secondary-color", "#0f0");
  if (rc != UI_ERROR_NONE)
    return 1;

  if (!comp->bound_properties || !comp->bound_properties->rules ||
      !comp->bound_properties->rules->declarations) {
    printf("Bound properties were not generated\n");
    return 1;
  }

  /* Check that both properties are present */
  {
    struct ui_css_declaration *decl =
        comp->bound_properties->rules->declarations;
    int found_primary = 0;
    int found_secondary = 0;

    while (decl) {
      if (strcmp(decl->property_name, "--primary-color") == 0 &&
          strcmp(decl->property_value, "#f00") == 0) {
        found_primary = 1;
      }
      if (strcmp(decl->property_name, "--secondary-color") == 0 &&
          strcmp(decl->property_value, "#0f0") == 0) {
        found_secondary = 1;
      }
      decl = decl->next;
    }
    if (!found_primary || !found_secondary) {
      printf("Bound property values mismatch\n");
      return 1;
    }
  }

  if (!comp->scope_id) {
    printf("Scope ID was not generated\n");
    return 1;
  }

  /* Verify DOM node class rewriting */
  ui_dom_node_get_attribute(btn_node, "class", &scoped_class);
  if (!scoped_class || strstr(scoped_class, comp->scope_id) == NULL) {
    printf("DOM class was not scoped properly: %s\n",
           scoped_class ? scoped_class : "null");
    return 1;
  }

  /* Verify internal rule rewriting */
  if (!style->rules || !style->rules->selectors ||
      strstr(style->rules->selectors->value, comp->scope_id) == NULL) {
    printf("CSS selector was not scoped properly: %s\n",
           style->rules
               ? (style->rules->selectors ? style->rules->selectors->value
                                          : "null")
               : "null");
    return 1;
  }

  rc = ui_component_mount(comp, host_node);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to mount component\n");
    return 1;
  }

  if (comp->host_node != host_node) {
    printf("Component host node mismatch\n");
    return 1;
  }

  /* Destroying component should destroy its opaque DOM/style trees */
  ui_component_destroy(comp);

  /* Clean up host node manually as it sits outside the component */
  ui_dom_node_destroy(host_node);

  /* 2. Test Invalid Arguments */
  if (ui_component_create(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("ui_component_create(NULL) did not fail properly\n");
    return 1;
  }

  /* Just call destroy on NULL to ensure it doesn't crash */
  ui_component_destroy(NULL);

  ui_component_create(&comp);
  if (ui_component_mount(NULL, host_node) != UI_ERROR_INVALID_ARGUMENT) {
    printf("ui_component_mount(NULL) did not fail properly\n");
    return 1;
  }
  if (ui_component_mount(comp, NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("ui_component_mount(NULL host) did not fail properly\n");
    return 1;
  }

  if (ui_component_set_default_style(NULL, style) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_component_set_default_style(comp, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_component_inject_style_override(NULL, ".x{}") !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_component_inject_style_override(comp, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_component_set_property(NULL, "--x", "1") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_component_set_property(comp, NULL, "1") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_component_set_property(comp, "--x", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_component_scope_styles(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_component_destroy(comp);
  return 0;
}

static int run_oom_tests(void) {
  struct ui_component *comp = NULL;
  enum ui_error rc;
  int i;
  struct ui_css_stylesheet *style = NULL;
  struct ui_dom_node *shadow_root = NULL;
  struct ui_dom_node *btn_node = NULL;
  struct ui_css_rule *btn_rule = NULL;

  printf("Running component OOM tests...\n");

  /* Test Error Percolation (Mock Memory Failures) */
  g_malloc_fail_countdown = 0;
  rc = ui_component_create(&comp);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Expected OUT_OF_MEMORY on create\n");
    return 1;
  }

  rc = ui_component_create(&comp);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test set property OOM */
  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_component_set_property(comp, "--prop", "val");
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_OUT_OF_MEMORY) {
      /* Continue loop to test further failures */
      if (comp->bound_properties) {
        ui_css_stylesheet_destroy(comp->bound_properties);
        comp->bound_properties = NULL;
      }
      continue;
    } else if (rc == UI_ERROR_NONE) {
      break;
    } else {
      return 1; /* unexpected error */
    }
  }

  ui_component_destroy(comp);

  /* Test scope_styles OOM */
  rc = ui_component_create(&comp);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &shadow_root);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &btn_node);
  ui_dom_node_set_tag_name(btn_node, "div");
  ui_dom_node_set_attribute(btn_node, "class", "btn");
  ui_dom_node_append_child(shadow_root, btn_node);
  comp->shadow_root = shadow_root;

  ui_css_stylesheet_create(&style);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &btn_rule);
  ui_css_rule_append_selector(btn_rule, UI_CSS_SELECTOR_TYPE_CLASS, "btn");
  ui_css_stylesheet_append_rule(style, btn_rule);
  ui_component_set_default_style(comp, style);

  for (i = 0; i < 6; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_component_scope_styles(comp);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_OUT_OF_MEMORY) {
      /* To retry scope styles, we must reset the scope_id */
      if (comp->scope_id) {
        free(comp->scope_id);
        comp->scope_id = NULL;
      }
      continue;
    } else if (rc == UI_ERROR_NONE) {
      break;
    } else {
      return 1;
    }
  }

  ui_component_destroy(comp);

  return 0;
}

int main(void) {
  int failed = 0;
  failed |= run_normal_tests();
  failed |= run_oom_tests();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All test_ui_component passed.\n");
  return 0;
}
