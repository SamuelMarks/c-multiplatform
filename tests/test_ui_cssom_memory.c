/* clang-format off */
#include "../include/ui_cssom.h"

struct ui_css_selector *create_mock_selector(enum ui_css_selector_type type, const char *val);
void test_malformed_selector(void);
void test_coverage_ui_cssom(void);
void test_cssom_oom(void);

#include "../include/ui_cssom.h"
#include "../include/ui_dom_node.h"
#include "../src/ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#ifndef TEST_ASSERT
#define TEST_ASSERT(cond)                                                      \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "Assert failed: %s at %d\n", #cond, __LINE__);           \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)
#endif

static char *my_strdup(const char *s) {
  size_t len = strlen(s);
  char *d = C_MULTIPLATFORM_MALLOC(len + 1);
  if (d)
    UI_STRCPY(d, len + 1, s);
  return d;
}

int test_cssom_part6_memory(void) {

  {
    /* More coverage */
    struct ui_dom_node *node;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_stylesheet *sheet;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_rule *r1, *r2, *r3, *r4, *r5;
    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r1);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_rule_append_selector(
          r1, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS, "not");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, r1);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r2);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    ui_css_rule_append_selector(r2, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "checked");
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, r2);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r3);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    ui_css_rule_append_selector(r3, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "target");
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, r3);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r4);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    ui_css_rule_append_selector(r4, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "target-within");
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, r4);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_SCOPE, &r5);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    r5->scope_start = NULL;
    r5->scope_end = NULL;
    struct ui_css_rule *r5_nested;
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r5_nested);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_rule_append_selector(
          r5_nested, UI_CSS_SELECTOR_TYPE_TAG, "div");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    r5->nested_rules = r5_nested;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, r5);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    /* has(.container) */
    struct ui_css_rule *r6;
    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r6);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    struct ui_css_selector *has_container =
        create_mock_selector(UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS, "has");
    struct ui_css_selector *nested_container =
        create_mock_selector(UI_CSS_SELECTOR_TYPE_CLASS, ".container");
    has_container->nested_selector = nested_container;
    r6->selectors = has_container;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, r6);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_dom_node_set_tag_name(node, "div");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_set_attribute(node, "checked", "");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_dom_node *container_child;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container_child);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(container_child, "class", "container");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_append_child(node, container_child);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_computed_style *style;
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_dom_node_remove_attribute(node, "checked");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "aria-checked", "true");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "aria-checked", "false");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
  }

  {
    /* Missing edge cases */
    struct ui_dom_node *node;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_set_tag_name(node, "div");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_stylesheet *sheet;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_rule *r1;
    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r1);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    ui_css_rule_append_selector(r1, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "read-write");
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, r1);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_computed_style *style;

    /* :read-write with disabled attribute */
    {
      ui_error_t rc_cleanup = ui_dom_node_set_attribute(node, "disabled", "");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    /* :read-write with aria-disabled */
    {
      ui_error_t rc_cleanup = ui_dom_node_remove_attribute(node, "disabled");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "aria-disabled", "true");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
  }

  {
    /* Text node matching */
    struct ui_dom_node *node;
    {
      ui_error_t rc_cleanup = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_stylesheet *sheet;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_computed_style *style;
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_dom_node *parent;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &parent);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_dom_node *wrapper;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &wrapper);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_append_child(wrapper, node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_append_child(parent, wrapper);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_rule *has_rule;
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &has_rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_selector *has_sel =
        create_mock_selector(UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS, "has");
    struct ui_css_selector *nested_div =
        create_mock_selector(UI_CSS_SELECTOR_TYPE_TAG, "div");
    has_sel->nested_selector = nested_div;
    has_rule->selectors = has_sel;

    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, has_rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, parent, &style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_dom_node *cov_parent = NULL, *cov_child = NULL;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &cov_parent);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_set_tag_name(cov_parent, "div");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(cov_parent, "id", "parent-id");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &cov_child);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_set_tag_name(cov_child, "span");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(cov_child, "id", "child-id");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(cov_child, "class", "myclass");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_append_child(cov_parent, cov_child);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    /* :not(span) on child */
    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_rule_append_selector(
          rule, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS, "not");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    sel = rule->selectors;
    sel->nested_selector =
        create_mock_selector(UI_CSS_SELECTOR_TYPE_TAG, "span");
    {
      ui_error_t rc_cleanup =
          ui_css_rule_append_declaration(rule, "color", "red", 0);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    /* pseudo-element ::before */
    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_rule_append_selector(
          rule, UI_CSS_SELECTOR_TYPE_PSEUDO_ELEMENT, "before");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_css_rule_append_declaration(rule, "color", "blue", 0);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    /* id doesn't match */
    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_rule_append_selector(
          rule, UI_CSS_SELECTOR_TYPE_ID, "wrong-id");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_css_rule_append_declaration(rule, "color", "green", 0);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    /* :has(div) with nested selector to cover 1100s branch */
    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_rule_append_selector(
          rule, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS, "has");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    sel = rule->selectors;
    sel->nested_selector =
        create_mock_selector(UI_CSS_SELECTOR_TYPE_TAG, "div");
    {
      ui_error_t rc_cleanup =
          ui_css_rule_append_declaration(rule, "margin", "10px", 0);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, cov_child, &style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    /* Layer order overrides: L1 then L2 */
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      int order1 = 0, order2 = 0;
      struct ui_css_rule *layer_rule1, *layer_rule2;
      {
        ui_error_t rc_cleanup =
            ui_css_stylesheet_register_layer(sheet, "L3", &order1);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_stylesheet_register_layer(sheet, "L4", &order2);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_LAYER, &layer_rule1);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      layer_rule1->layer_name = C_MULTIPLATFORM_STRDUP("L3");

      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_LAYER, &layer_rule2);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      layer_rule2->layer_name = C_MULTIPLATFORM_STRDUP("L4");

      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "span");
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_rule_append_declaration(rule, "prop_layer_normal2", "A", 0);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup = ui_css_rule_append_declaration(
            rule, "prop_layer_important2", "A", 1);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      layer_rule1->nested_rules = rule;

      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "span");
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_rule_append_declaration(rule, "prop_layer_normal2", "B", 0);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup = ui_css_rule_append_declaration(
            rule, "prop_layer_important2", "B", 1);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      layer_rule2->nested_rules = rule;

      {
        ui_error_t rc_cleanup =
            ui_css_stylesheet_append_rule(sheet, layer_rule1);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_stylesheet_append_rule(sheet, layer_rule2);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, cov_child, &style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      int order1 = 0, order2 = 0;
      struct ui_css_rule *layer_rule1, *layer_rule2;
      {
        ui_error_t rc_cleanup =
            ui_css_stylesheet_register_layer(sheet, "L3", &order1);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_stylesheet_register_layer(sheet, "L4", &order2);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_LAYER, &layer_rule1);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      layer_rule1->layer_name = C_MULTIPLATFORM_STRDUP("L3");

      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_LAYER, &layer_rule2);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      layer_rule2->layer_name = C_MULTIPLATFORM_STRDUP("L4");

      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "span");
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_rule_append_declaration(rule, "prop_layer_normal2", "A", 0);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup = ui_css_rule_append_declaration(
            rule, "prop_layer_important2", "A", 1);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      layer_rule1->nested_rules = rule;

      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "span");
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_rule_append_declaration(rule, "prop_layer_normal2", "B", 0);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup = ui_css_rule_append_declaration(
            rule, "prop_layer_important2", "B", 1);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      layer_rule2->nested_rules = rule;

      {
        ui_error_t rc_cleanup =
            ui_css_stylesheet_append_rule(sheet, layer_rule1);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_stylesheet_append_rule(sheet, layer_rule2);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, cov_child, &style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(cov_parent);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(parent);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    /* node is destroyed when parent is destroyed */
  }
  {
    /* Nested parens in supports condition */
    struct ui_dom_node *node;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_stylesheet *sheet;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    rule->supports_condition = my_strdup("(not ((display: flex)))");
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_computed_style *style;
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
  }
  {
    /* Reverts Author origin test */
    struct ui_dom_node *node;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_stylesheet *sheet;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_css_rule_append_declaration(rule, "color", "revert", 0);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_computed_style *style;
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
  }
  int cssom_oom_cnt = 0;
  while (1) {
    g_malloc_fail_countdown = cssom_oom_cnt;
    cssom_oom_cnt++;

    struct ui_css_stylesheet *cov_sheet = NULL;
    ui_error_t rc_sheet = ui_css_stylesheet_create(&cov_sheet);

    if (rc_sheet == UI_ERROR_NONE && cov_sheet != NULL) {
      struct ui_css_rule *cov_rule = NULL;
      struct ui_css_rule *rule_media = NULL;
      struct ui_css_rule *rule_scope = NULL;
      struct ui_css_rule *rule_scope2 = NULL;
      struct ui_dom_node *container_node = NULL;
      struct ui_css_computed_style *container_style = NULL;
      struct ui_css_rule *rule_supports = NULL;
      struct ui_css_rule *rule_container = NULL;
      struct ui_css_rule *rule_property = NULL;
      struct ui_css_rule *rule_layer = NULL;
      int order2 = 2;

      if (ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &cov_rule) ==
          UI_ERROR_NONE) {
        ui_css_rule_append_selector(cov_rule, UI_CSS_SELECTOR_TYPE_CLASS,
                                    "myclass");
        ui_css_rule_append_selector_attr(cov_rule, "href",
                                         UI_CSS_ATTR_OP_EQUALS, "http");
        ui_css_rule_append_selector_attr(cov_rule, "disabled",
                                         UI_CSS_ATTR_OP_NONE, NULL);
        ui_css_rule_append_selector_attr(cov_rule, "lang", UI_CSS_ATTR_OP_DASH,
                                         "en");
        ui_css_rule_append_selector_attr(cov_rule, "href",
                                         UI_CSS_ATTR_OP_PREFIX, "https");
        ui_css_rule_append_selector_attr(cov_rule, "href",
                                         UI_CSS_ATTR_OP_SUFFIX, ".pdf");
        ui_css_rule_append_selector_attr(cov_rule, "href",
                                         UI_CSS_ATTR_OP_SUBSTRING, "example");
        ui_css_rule_append_selector_attr(cov_rule, "class",
                                         UI_CSS_ATTR_OP_INCLUDES, "btn");
        {
          ui_error_t rc_cleanup =
              ui_css_rule_append_declaration(cov_rule, "color", "red", 0);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                      rc_cleanup == UI_ERROR_OUT_OF_MEMORY);
        }
        {
          ui_error_t rc_cleanup =
              ui_css_rule_append_declaration(cov_rule, "margin", "10px", 0);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                      rc_cleanup == UI_ERROR_OUT_OF_MEMORY);
        }
        {
          ui_error_t rc_cleanup =
              ui_css_stylesheet_append_rule(cov_sheet, cov_rule);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                      rc_cleanup == UI_ERROR_OUT_OF_MEMORY);
        }
      }

      if (ui_css_rule_create(UI_CSS_RULE_TYPE_MEDIA, &rule_media) ==
          UI_ERROR_NONE) {
        rule_media->media_condition = my_strdup("(max-width: 600px)");
        {
          ui_error_t rc_cleanup =
              ui_css_rule_append_declaration(rule_media, "padding", "5px", 0);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                      rc_cleanup == UI_ERROR_OUT_OF_MEMORY);
        }
        {
          ui_error_t rc_cleanup =
              ui_css_rule_append_declaration(rule_media, "margin", "2px", 0);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                      rc_cleanup == UI_ERROR_OUT_OF_MEMORY);
        }
        {
          ui_error_t rc_cleanup =
              ui_css_stylesheet_append_rule(cov_sheet, rule_media);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                      rc_cleanup == UI_ERROR_OUT_OF_MEMORY);
        }
      }

      if (ui_css_rule_create(UI_CSS_RULE_TYPE_SCOPE, &rule_scope) ==
          UI_ERROR_NONE) {
        rule_scope->scope_start =
            create_mock_selector(UI_CSS_SELECTOR_TYPE_CLASS, ".card");
        rule_scope->scope_end =
            create_mock_selector(UI_CSS_SELECTOR_TYPE_CLASS, ".hole");
        {
          ui_error_t rc_cleanup =
              ui_css_stylesheet_append_rule(cov_sheet, rule_scope);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                      rc_cleanup == UI_ERROR_OUT_OF_MEMORY);
        }
      }

      if (ui_css_rule_create(UI_CSS_RULE_TYPE_SCOPE, &rule_scope2) ==
          UI_ERROR_NONE) {
        rule_scope2->scope_start =
            create_mock_selector(UI_CSS_SELECTOR_TYPE_CLASS, ".container");
        rule_scope2->scope_end = NULL;
        {
          ui_error_t rc_cleanup =
              ui_css_stylesheet_append_rule(cov_sheet, rule_scope2);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                      rc_cleanup == UI_ERROR_OUT_OF_MEMORY);
        }
      }

      if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container_node) ==
          UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup =
              ui_dom_node_set_attribute(container_node, "class", "container");
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                      rc_cleanup == UI_ERROR_OUT_OF_MEMORY);
        }
        {
          ui_error_t rc_cleanup =
              ui_css_resolve_style(cov_sheet, container_node, &container_style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                      rc_cleanup == UI_ERROR_OUT_OF_MEMORY);
        }
        {
          ui_error_t rc_cleanup =
              ui_css_computed_style_destroy(container_style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
        {
          ui_error_t rc_cleanup = ui_dom_node_destroy(container_node);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
      }

      if (ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_supports) ==
          UI_ERROR_NONE) {
        rule_supports->supports_condition =
            my_strdup("not (display: grid) and (color: red) or selector(div)");
        {
          ui_error_t rc_cleanup =
              ui_css_stylesheet_append_rule(cov_sheet, rule_supports);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                      rc_cleanup == UI_ERROR_OUT_OF_MEMORY);
        }
      }

      if (ui_css_rule_create(UI_CSS_RULE_TYPE_CONTAINER, &rule_container) ==
          UI_ERROR_NONE) {
        rule_container->container_condition = my_strdup("(min-width: 700px)");
        {
          ui_error_t rc_cleanup =
              ui_css_stylesheet_append_rule(cov_sheet, rule_container);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                      rc_cleanup == UI_ERROR_OUT_OF_MEMORY);
        }
      }

      if (ui_css_rule_create(UI_CSS_RULE_TYPE_PROPERTY, &rule_property) ==
          UI_ERROR_NONE) {
        rule_property->property_name = my_strdup("--my-prop");
        rule_property->property_syntax = my_strdup("<color>");
        rule_property->property_initial_value = my_strdup("red");
        {
          ui_error_t rc_cleanup =
              ui_css_stylesheet_append_rule(cov_sheet, rule_property);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                      rc_cleanup == UI_ERROR_OUT_OF_MEMORY);
        }
      }

      if (ui_css_rule_create(UI_CSS_RULE_TYPE_LAYER, &rule_layer) ==
          UI_ERROR_NONE) {
        rule_layer->layer_name = my_strdup("theme");
        {
          ui_error_t rc_cleanup =
              ui_css_stylesheet_append_rule(cov_sheet, rule_layer);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                      rc_cleanup == UI_ERROR_OUT_OF_MEMORY);
        }
      }

      {
        ui_error_t rc_cleanup =
            ui_css_stylesheet_register_layer(cov_sheet, "L2", &order2);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                    rc_cleanup == UI_ERROR_OUT_OF_MEMORY);
      }

      struct ui_css_rule *oom_rule = NULL;
      if (ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &oom_rule) ==
          UI_ERROR_NONE) {
        ui_css_rule_append_selector(oom_rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
        ui_css_rule_append_declaration(oom_rule, "color", "red", 0);
        ui_css_stylesheet_append_rule(cov_sheet, oom_rule);
      }

      struct ui_dom_node *oom_node = NULL;
      if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &oom_node) ==
          UI_ERROR_NONE) {
        ui_dom_node_set_tag_name(oom_node, "div");
        struct ui_css_computed_style *oom_style = NULL;
        {
          ui_error_t rc_cleanup =
              ui_css_resolve_style(cov_sheet, oom_node, &oom_style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                      rc_cleanup == UI_ERROR_OUT_OF_MEMORY);
        }
        if (oom_style) {
          ui_css_computed_style_destroy(oom_style);
        }
        ui_dom_node_destroy(oom_node);
      }

      {
        ui_error_t rc_cleanup = ui_css_stylesheet_destroy(cov_sheet);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
    }

    {
      struct ui_css_variable_store *store = NULL;
      char *resolved = NULL;
      if (ui_css_variable_store_create(&store) == UI_ERROR_NONE) {
        if (ui_css_variable_store_set(store, "--my-var", "red") ==
            UI_ERROR_NONE) {
          {
            ui_error_t rc_cleanup =
                ui_css_variable_store_set(store, "--my-var", "blue");
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                        rc_cleanup == UI_ERROR_OUT_OF_MEMORY);
          }
        }
        if (ui_css_resolve_variables(store, "var(--my-var, blue)", &resolved) ==
            UI_ERROR_NONE) {
          C_MULTIPLATFORM_FREE(resolved);
        }
        {
          ui_error_t rc_cleanup = ui_css_variable_store_destroy(store);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
      }
    }
    if (g_malloc_fail_countdown > 0) {
      g_malloc_fail_countdown = -1;
      break;
    }
  }

  test_malformed_selector();
  test_coverage_ui_cssom();

  /* Branch coverage for nested parens in media */
  {
    struct ui_dom_node *node_tmp = NULL;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node_tmp);
    struct ui_css_stylesheet *sheet_tmp = NULL;
    ui_css_stylesheet_create(&sheet_tmp);
    struct ui_css_rule *media_tmp;
    ui_css_rule_create(UI_CSS_RULE_TYPE_MEDIA, &media_tmp);
    media_tmp->media_condition = C_MULTIPLATFORM_STRDUP("(a(b))");
    ui_css_stylesheet_append_rule(sheet_tmp, media_tmp);

    struct ui_css_computed_style *style_tmp;
    ui_css_resolve_style(sheet_tmp, node_tmp, &style_tmp);
    ui_css_computed_style_destroy(style_tmp);
    ui_css_stylesheet_destroy(sheet_tmp);
    ui_dom_node_destroy(node_tmp);
  }

  /* Branch coverage for variable resolution without var() or missing ) */
  {
    char *res = NULL;
    struct ui_css_variable_store *store = NULL;
    ui_css_variable_store_create(&store);
    ui_css_resolve_variables(store, "red", &res);
    if (res)
      C_MULTIPLATFORM_FREE(res);

    ui_css_resolve_variables(store, "var(--my-var", &res);
    if (res)
      C_MULTIPLATFORM_FREE(res);
  }

  /* Branch coverage for specificity */
  {
    struct ui_dom_node *node_tmp = NULL;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node_tmp);
    struct ui_css_stylesheet *sheet_tmp = NULL;
    ui_css_stylesheet_create(&sheet_tmp);
    struct ui_css_rule *style_tmp;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &style_tmp);

    struct ui_css_selector *s1 =
        create_mock_selector(UI_CSS_SELECTOR_TYPE_CLASS, ".a");
    struct ui_css_selector *s2 =
        create_mock_selector(UI_CSS_SELECTOR_TYPE_CLASS, ".b");
    struct ui_css_selector *s3 =
        create_mock_selector(UI_CSS_SELECTOR_TYPE_TAG, "div");
    s2->next = s3;
    s1->nested_selector = s2;
    style_tmp->selectors = s1;

    ui_css_stylesheet_append_rule(sheet_tmp, style_tmp);

    struct ui_css_computed_style *c_style;
    ui_css_resolve_style(sheet_tmp, node_tmp, &c_style);

    ui_css_computed_style_destroy(c_style);
    ui_css_stylesheet_destroy(sheet_tmp);
    ui_dom_node_destroy(node_tmp);
  }

  /* Recursive resolution OOM tests */
  {
    struct ui_dom_node *node = NULL;
    struct ui_css_stylesheet *sheet = NULL;
    struct ui_css_rule *layer_rule = NULL;
    struct ui_css_rule *media_rule = NULL;
    struct ui_css_rule *supports_rule = NULL;
    struct ui_css_rule *container_rule = NULL;
    struct ui_css_rule *scope_rule = NULL;
    struct ui_css_rule *style_rule = NULL;
    struct ui_css_rule *nested_style = NULL;
    struct ui_css_computed_style *style = NULL;
    ui_error_t rc;

    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_dom_node_set_tag_name(node, "div");
    TEST_ASSERT(rc == UI_ERROR_NONE);

    /* 1. LAYER rule with nested style rule */
    rc = ui_css_stylesheet_create(&sheet);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_create(UI_CSS_RULE_TYPE_LAYER, &layer_rule);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_style);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_append_selector(nested_style, UI_CSS_SELECTOR_TYPE_TAG,
                                     "div");
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_append_declaration(nested_style, "color", "red", 0);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    layer_rule->nested_rules = nested_style;
    rc = ui_css_stylesheet_append_rule(sheet, layer_rule);
    TEST_ASSERT(rc == UI_ERROR_NONE);

    g_malloc_fail_countdown = 1;
    rc = ui_css_resolve_style(sheet, node, &style);
    TEST_ASSERT(rc != UI_ERROR_NONE);
    g_malloc_fail_countdown = -1;

    ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    /* 2. MEDIA rule with nested style rule */
    rc = ui_css_stylesheet_create(&sheet);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_create(UI_CSS_RULE_TYPE_MEDIA, &media_rule);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_style);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_append_selector(nested_style, UI_CSS_SELECTOR_TYPE_TAG,
                                     "div");
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_append_declaration(nested_style, "color", "red", 0);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    media_rule->nested_rules = nested_style;
    rc = ui_css_stylesheet_append_rule(sheet, media_rule);
    TEST_ASSERT(rc == UI_ERROR_NONE);

    g_malloc_fail_countdown = 1;
    rc = ui_css_resolve_style(sheet, node, &style);
    TEST_ASSERT(rc != UI_ERROR_NONE);
    g_malloc_fail_countdown = -1;

    ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    /* 3. SUPPORTS rule with nested style rule */
    rc = ui_css_stylesheet_create(&sheet);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &supports_rule);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    supports_rule->supports_condition = my_strdup("(display: (flex))");
    rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_style);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_append_selector(nested_style, UI_CSS_SELECTOR_TYPE_TAG,
                                     "div");
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_append_declaration(nested_style, "color", "red", 0);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    supports_rule->nested_rules = nested_style;
    rc = ui_css_stylesheet_append_rule(sheet, supports_rule);
    TEST_ASSERT(rc == UI_ERROR_NONE);

    g_malloc_fail_countdown = 1;
    rc = ui_css_resolve_style(sheet, node, &style);
    TEST_ASSERT(rc != UI_ERROR_NONE);
    g_malloc_fail_countdown = -1;

    ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    /* 4. CONTAINER rule with nested style rule */
    rc = ui_css_stylesheet_create(&sheet);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_create(UI_CSS_RULE_TYPE_CONTAINER, &container_rule);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_style);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_append_selector(nested_style, UI_CSS_SELECTOR_TYPE_TAG,
                                     "div");
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_append_declaration(nested_style, "color", "red", 0);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    container_rule->nested_rules = nested_style;
    rc = ui_css_stylesheet_append_rule(sheet, container_rule);
    TEST_ASSERT(rc == UI_ERROR_NONE);

    g_malloc_fail_countdown = 1;
    rc = ui_css_resolve_style(sheet, node, &style);
    TEST_ASSERT(rc != UI_ERROR_NONE);
    g_malloc_fail_countdown = -1;

    ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    /* 5. SCOPE rule with nested style rule */
    rc = ui_css_stylesheet_create(&sheet);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_create(UI_CSS_RULE_TYPE_SCOPE, &scope_rule);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_style);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_append_selector(nested_style, UI_CSS_SELECTOR_TYPE_TAG,
                                     "div");
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_append_declaration(nested_style, "color", "red", 0);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    scope_rule->nested_rules = nested_style;
    rc = ui_css_stylesheet_append_rule(sheet, scope_rule);
    TEST_ASSERT(rc == UI_ERROR_NONE);

    g_malloc_fail_countdown = 1;
    rc = ui_css_resolve_style(sheet, node, &style);
    TEST_ASSERT(rc != UI_ERROR_NONE);
    g_malloc_fail_countdown = -1;

    ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    /* 6. STYLE rule with nested style rule */
    rc = ui_css_stylesheet_create(&sheet);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &style_rule);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_append_selector(style_rule, UI_CSS_SELECTOR_TYPE_TAG,
                                     "div");
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_style);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_append_selector(nested_style, UI_CSS_SELECTOR_TYPE_TAG,
                                     "div");
    TEST_ASSERT(rc == UI_ERROR_NONE);
    rc = ui_css_rule_append_declaration(nested_style, "color", "red", 0);
    TEST_ASSERT(rc == UI_ERROR_NONE);
    style_rule->nested_rules = nested_style;
    rc = ui_css_stylesheet_append_rule(sheet, style_rule);
    TEST_ASSERT(rc == UI_ERROR_NONE);

    g_malloc_fail_countdown = 1;
    rc = ui_css_resolve_style(sheet, node, &style);
    TEST_ASSERT(rc != UI_ERROR_NONE);
    g_malloc_fail_countdown = -1;

    ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_dom_node_destroy(node);
  }
  return 0;
}
