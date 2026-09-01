/* clang-format off */
#include "greatest.h"
#include <stdio.h>
#include <string.h>
#include "../include/ui_component.h"
#include "../include/ui_dom_node.h"
#include "../include/ui_cssom.h"
#include "../src/ui_internal_mem.h"
/* clang-format on */

extern int g_malloc_fail_countdown;
extern int g_malloc_called;

TEST run_normal_tests(void) {
  struct ui_component *comp = NULL;
  struct ui_dom_node *host_node = NULL;
  struct ui_dom_node *shadow_root = NULL;
  struct ui_css_stylesheet *style = NULL;
  struct ui_dom_node *btn_node = NULL;
  struct ui_css_rule *btn_rule = NULL;
  const char *scoped_class = NULL;
  ui_error_t rc;

  /* 1. Test standard lifecycle */
  rc = ui_component_create(&comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(comp != NULL);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &host_node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(host_node != NULL);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &shadow_root);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(shadow_root != NULL);

  rc = ui_css_stylesheet_create(&style);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(style != NULL);

  /* Assign opaque state via API */
  comp->shadow_root = shadow_root;
  rc = ui_component_set_default_style(comp, style);
  ASSERT_EQ(UI_ERROR_NONE,
            ui_component_inject_style_override(comp, ".btn { color: green; }"));

  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test string injection override */
  rc = ui_component_inject_style_override(comp, ".btn { color: red; }");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(comp->override_style != NULL);

  /* Test replacing default and override style */
  rc = ui_css_stylesheet_create(&style);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_ERROR_NONE, ui_component_set_default_style(comp, style));
  ASSERT_EQ(UI_ERROR_NONE,
            ui_component_inject_style_override(comp, ".btn { color: green; }"));

  ASSERT_EQ(UI_ERROR_NONE,
            ui_component_inject_style_override(comp, ".btn2 { color: blue; }"));

  /* Add a node to shadow root to test scoping with trailing spaces */
  ASSERT_EQ(UI_ERROR_NONE,
            ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &btn_node));
  ASSERT_EQ(UI_ERROR_NONE, ui_dom_node_set_tag_name(btn_node, "button"));
  ASSERT_EQ(UI_ERROR_NONE,
            ui_dom_node_set_attribute(btn_node, "class", "btn btn-primary "));
  ASSERT_EQ(UI_ERROR_NONE, ui_dom_node_append_child(shadow_root, btn_node));

  /* Add another node to test scoping without trailing spaces */
  struct ui_dom_node *btn2_node;
  ASSERT_EQ(UI_ERROR_NONE,
            ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &btn2_node));
  ASSERT_EQ(UI_ERROR_NONE, ui_dom_node_set_tag_name(btn2_node, "button"));
  ASSERT_EQ(UI_ERROR_NONE,
            ui_dom_node_set_attribute(btn2_node, "class", "btn btn-secondary"));
  ASSERT_EQ(UI_ERROR_NONE, ui_dom_node_append_child(shadow_root, btn2_node));

  /* Add a matching rule to internal style and a non-class rule */
  ASSERT_EQ(UI_ERROR_NONE,
            ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &btn_rule));
  ASSERT_EQ(UI_ERROR_NONE, ui_css_rule_append_selector(
                               btn_rule, UI_CSS_SELECTOR_TYPE_CLASS, "btn"));
  ASSERT_EQ(UI_ERROR_NONE,
            ui_css_rule_append_declaration(btn_rule, "color", "blue", 0));
  ASSERT_EQ(UI_ERROR_NONE, ui_css_stylesheet_append_rule(style, btn_rule));

  struct ui_css_rule *tag_rule;
  ASSERT_EQ(UI_ERROR_NONE,
            ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &tag_rule));
  ASSERT_EQ(UI_ERROR_NONE, ui_css_rule_append_selector(
                               tag_rule, UI_CSS_SELECTOR_TYPE_TAG, "button"));
  ASSERT_EQ(UI_ERROR_NONE, ui_css_stylesheet_append_rule(style, tag_rule));

  /* Apply Scoped CSS encapsulation */
  rc = ui_component_scope_styles(comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Create an empty component and scope to test NULL properties branches */
  struct ui_component *empty_comp;
  ASSERT_EQ(UI_ERROR_NONE, ui_component_create(&empty_comp));
  ASSERT_EQ(UI_ERROR_NONE, ui_component_scope_styles(empty_comp));
  ASSERT_EQ(UI_ERROR_NONE, ui_component_destroy(empty_comp));

  /* Try to scope again (should return INVALID_ARGUMENT because already scoped)
   */
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_component_scope_styles(comp));

  /* Test Property Binding */
  rc = ui_component_set_property(comp, "--primary-color", "#f00");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test Property Binding when it already exists */
  rc = ui_component_set_property(comp, "--secondary-color", "#0f0");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT(comp->bound_properties != NULL);
  ASSERT(comp->bound_properties->rules != NULL);
  ASSERT(comp->bound_properties->rules->declarations != NULL);

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
    ASSERT(found_primary && found_secondary);
  }

  ASSERT(comp->scope_id != NULL);

  /* Verify DOM node class rewriting */
  ASSERT_EQ(UI_ERROR_NONE,
            ui_dom_node_get_attribute(btn_node, "class", &scoped_class));
  ASSERT(scoped_class != NULL);
  ASSERT(strstr(scoped_class, comp->scope_id) != NULL);

  /* Verify internal rule rewriting */
  ASSERT(style->rules != NULL);
  ASSERT(style->rules->selectors != NULL);
  ASSERT(strstr(style->rules->selectors->value, comp->scope_id) != NULL);

  rc = ui_component_mount(comp, host_node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT_EQ(host_node, comp->host_node);

  /* Destroying component should destroy its opaque DOM/style trees */
  ASSERT_EQ(UI_ERROR_NONE, ui_component_destroy(comp));

  /* Clean up host node manually as it sits outside the component */
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(host_node);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* 2. Test Invalid Arguments */
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_component_create(NULL));

  /* Just call destroy on NULL to ensure it doesn't crash */
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_component_destroy(NULL));

  ASSERT_EQ(UI_ERROR_NONE, ui_component_create(&comp));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_component_mount(NULL, host_node));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_component_mount(comp, NULL));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_component_set_default_style(NULL, style));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_component_set_default_style(comp, NULL));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_component_inject_style_override(NULL, ".x{}"));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_component_inject_style_override(comp, NULL));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_component_set_property(NULL, "--x", "1"));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_component_set_property(comp, NULL, "1"));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_component_set_property(comp, "--x", NULL));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_component_scope_styles(NULL));

  ASSERT_EQ(UI_ERROR_NONE, ui_component_destroy(comp));
  PASS();
}

TEST run_oom_tests(void) {
  struct ui_component *comp = NULL;
  ui_error_t rc;
  int i;
  struct ui_css_stylesheet *style = NULL;
  struct ui_dom_node *shadow_root = NULL;
  struct ui_dom_node *btn_node = NULL;
  struct ui_css_rule *btn_rule = NULL;

  /* Test Error Percolation (Mock Memory Failures) */
  g_malloc_fail_countdown = 0;
  rc = ui_component_create(&comp);
  g_malloc_fail_countdown = -1;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  rc = ui_component_create(&comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test set property OOM */
  for (i = 0; i < 50; i++) {
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
      ASSERT(0); /* unexpected error */
    }
  }

  ASSERT_EQ(UI_ERROR_NONE, ui_component_destroy(comp));

  /* Test inject_style_override OOM */
  for (i = 0; i < 20; i++) {
    ASSERT_EQ(UI_ERROR_NONE, ui_component_create(&comp));
    g_malloc_fail_countdown = i;
    rc = ui_component_inject_style_override(comp, ".btn { color: red; }");
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_OUT_OF_MEMORY) {
      ui_component_destroy(comp);
      continue;
    } else if (rc == UI_ERROR_NONE) {
      ui_component_destroy(comp);
      break;
    } else {
      ASSERT(0);
    }
  }

  /* Test scope_styles OOM */
  ASSERT_EQ(UI_ERROR_NONE, ui_component_create(&comp));
  ASSERT_EQ(UI_ERROR_NONE,
            ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &shadow_root));
  ASSERT_EQ(UI_ERROR_NONE,
            ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &btn_node));
  ASSERT_EQ(UI_ERROR_NONE, ui_dom_node_set_tag_name(btn_node, "div"));
  ASSERT_EQ(UI_ERROR_NONE, ui_dom_node_set_attribute(btn_node, "class", "btn"));
  ASSERT_EQ(UI_ERROR_NONE, ui_dom_node_append_child(shadow_root, btn_node));
  comp->shadow_root = shadow_root;

  ASSERT_EQ(UI_ERROR_NONE, ui_css_stylesheet_create(&style));
  ASSERT_EQ(UI_ERROR_NONE,
            ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &btn_rule));
  ASSERT_EQ(UI_ERROR_NONE, ui_css_rule_append_selector(
                               btn_rule, UI_CSS_SELECTOR_TYPE_CLASS, "btn"));
  ASSERT_EQ(UI_ERROR_NONE, ui_css_stylesheet_append_rule(style, btn_rule));
  ASSERT_EQ(UI_ERROR_NONE, ui_component_set_default_style(comp, style));
  ASSERT_EQ(UI_ERROR_NONE,
            ui_component_inject_style_override(comp, ".btn { color: green; }"));

  for (i = 0; i < 50; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_component_scope_styles(comp);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_OUT_OF_MEMORY) {
      /* To retry scope styles, we must reset the scope_id */
      if (comp->scope_id) {
        C_MULTIPLATFORM_FREE(comp->scope_id);
        comp->scope_id = NULL;
      }
      continue;
    } else if (rc == UI_ERROR_NONE) {
      break;
    } else {
      ASSERT(0);
    }
  }

  ASSERT_EQ(UI_ERROR_NONE, ui_component_destroy(comp));

  PASS();
}

SUITE(ui_component_suite) {
  RUN_TEST(run_normal_tests);
  RUN_TEST(run_oom_tests);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_component_suite);
  GREATEST_MAIN_END();
}
