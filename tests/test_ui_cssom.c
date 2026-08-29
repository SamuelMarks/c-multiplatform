/* clang-format off */
#include "../include/ui_cssom.h"
#include "../include/ui_dom_node.h"
#include "../src/ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#define TEST_ASSERT(cond)                                                      \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "Assert failed: %s at %d\n", #cond, __LINE__);           \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

char *my_strdup(const char *s) {
  size_t len = strlen(s);
  char *d = C_MULTIPLATFORM_MALLOC(len + 1);
  if (d)
    UI_STRCPY(d, 256, s);
  return d;
}

void test_malformed_selector(void) {
  struct ui_dom_node *node = NULL;
  struct ui_dom_node *child = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_css_rule *rule = NULL;
  struct ui_css_selector *sel = NULL;
  struct ui_css_computed_style *style = NULL;

  (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  (void)ui_dom_node_set_tag_name(node, "div");

  (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child);
  (void)ui_dom_node_set_tag_name(child, "span");
  (void)ui_dom_node_append_child(node, child);

  (void)ui_css_stylesheet_create(&sheet);

  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);

  /* Construct malformed selector manually */
  sel = C_MULTIPLATFORM_MALLOC(sizeof(*sel));
  memset(sel, 0, sizeof(*sel));
  sel->type = UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS;
  sel->value = my_strdup("is");
  sel->nested_selector = NULL; /* MALFORMED */

  rule->selectors = sel;
  (void)ui_css_rule_append_declaration(rule, "color", "red", 0);
  (void)ui_css_stylesheet_append_rule(sheet, rule);

  (void)ui_css_resolve_style(sheet, child, &style);
  if (style) {
    (void)ui_css_computed_style_destroy(style);
    style = NULL;
  }

  C_MULTIPLATFORM_FREE(sel->value);
  sel->value = my_strdup("has");
  (void)ui_css_resolve_style(sheet, child, &style);
  if (style) {
    (void)ui_css_computed_style_destroy(style);
    style = NULL;
  }

  C_MULTIPLATFORM_FREE(sel->value);
  sel->value = my_strdup("host");
  (void)ui_css_resolve_style(sheet, child, &style);
  if (style) {
    (void)ui_css_computed_style_destroy(style);
    style = NULL;
  }

  C_MULTIPLATFORM_FREE(sel->value);
  sel->value = my_strdup("host-context");
  (void)ui_css_resolve_style(sheet, child, &style);
  if (style) {
    (void)ui_css_computed_style_destroy(style);
    style = NULL;
  }

  sel->type = UI_CSS_SELECTOR_TYPE_ATTRIBUTE;
  C_MULTIPLATFORM_FREE(sel->value);
  sel->value = NULL; /* MALFORMED ATTRIBUTE */
  (void)ui_css_resolve_style(sheet, child, &style);
  if (style) {
    (void)ui_css_computed_style_destroy(style);
    style = NULL;
  }

  sel->type = (enum ui_css_selector_type)999;
  (void)ui_css_resolve_style(sheet, child, &style);
  if (style) {
    (void)ui_css_computed_style_destroy(style);
    style = NULL;
  }

  sel->type = UI_CSS_SELECTOR_TYPE_ATTRIBUTE;
  sel->value = my_strdup("class");
  sel->attr_value = my_strdup("v");
  sel->attr_op = (enum ui_css_attr_operator)999;
  (void)ui_css_resolve_style(sheet, child, &style);
  if (style) {
    (void)ui_css_computed_style_destroy(style);
    style = NULL;
  }

  (void)ui_css_stylesheet_destroy(sheet);
  (void)ui_dom_node_destroy(node);
}

struct ui_css_selector *create_mock_selector(enum ui_css_selector_type type,
                                             const char *val) {
  struct ui_css_selector *sel = C_MULTIPLATFORM_MALLOC(sizeof(*sel));
  if (sel) {
    memset(sel, 0, sizeof(*sel));
    sel->type = type;
    if (val)
      sel->value = C_MULTIPLATFORM_STRDUP(val);
  }
  return sel;
}

void test_cssom_oom(void) {
  struct ui_dom_node *node = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_css_computed_style *style = NULL;
  struct ui_css_variable_store *store = NULL;
  char *resolved = NULL;
  struct ui_css_selector *sel = NULL;
  int i;
  for (i = 1; i < 20; i++) {
    g_malloc_fail_countdown = i;

    if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node) == UI_ERROR_NONE) {
      if (ui_css_stylesheet_create(&sheet) == UI_ERROR_NONE) {
        struct ui_css_rule *rule;
        struct ui_css_selector *sel = NULL;
        if (ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule) ==
            UI_ERROR_NONE) {
          (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG,
                                            "div");
          (void)ui_css_rule_append_declaration(rule, "color", "red", 0);
          (void)ui_css_stylesheet_append_rule(sheet, rule);
        }
        (void)ui_css_resolve_style(sheet, node, &style);
        ui_css_computed_style_destroy(style);
        style = NULL;
        ui_css_stylesheet_destroy(sheet);
        sheet = NULL;
      }
      ui_dom_node_destroy(node);
      node = NULL;
    }

    if (ui_css_variable_store_create(&store) == UI_ERROR_NONE) {
      (void)ui_css_variable_store_set(store, "--my-var", "red");
      (void)ui_css_resolve_variables(store, "var(--my-var, blue)", &resolved);
      if (resolved) {
        C_MULTIPLATFORM_FREE(resolved);
        resolved = NULL;
      }
      ui_css_variable_store_destroy(store);
      store = NULL;
    }

    {
      struct ui_css_variable_store *store = NULL;
      char *resolved = NULL;
      if (ui_css_variable_store_create(&store) == UI_ERROR_NONE) {
        if (ui_css_variable_store_set(store, "--my-var", "red") ==
            UI_ERROR_NONE) {
          (void)ui_css_variable_store_set(store, "--my-var", "blue");
        }
        if (ui_css_resolve_variables(store, "var(--my-var, blue)", &resolved) ==
            UI_ERROR_NONE) {
          C_MULTIPLATFORM_FREE(resolved);
        }
        (void)ui_css_variable_store_destroy(store);
      }
    }
    if (g_malloc_fail_countdown > 0) {
      g_malloc_fail_countdown = -1;
      break;
    }
  }
  g_malloc_fail_countdown = -1;
}

void test_coverage_ui_cssom(void) {
  struct ui_dom_node *parent = NULL, *child = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_css_computed_style *style = NULL;
  struct ui_css_rule *rule = NULL;
  struct ui_css_selector *sel = NULL;

  (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &parent);
  (void)ui_dom_node_set_tag_name(parent, "div");
  (void)ui_dom_node_set_attribute(parent, "id", "parent-id");
  (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child);
  (void)ui_dom_node_set_tag_name(child, "span");
  (void)ui_dom_node_set_attribute(child, "id", "child-id");
  (void)ui_dom_node_set_attribute(child, "class", "myclass");
  (void)ui_dom_node_append_child(parent, child);

  (void)ui_css_stylesheet_create(&sheet);

  /* Reverse order for return UI_ERROR_NONE coverage */
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_ID, "child-id");
  (void)ui_css_rule_append_declaration(rule, "prop_a", "E", 0);
  (void)ui_css_stylesheet_append_rule(sheet, rule);

  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS,
                                    "myclass");
  (void)ui_css_rule_append_declaration(rule, "prop_a", "C", 0); /* spec_a > */
  (void)ui_css_rule_append_declaration(rule, "prop_b", "C", 0);
  (void)ui_css_stylesheet_append_rule(sheet, rule);

  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "span");
  (void)ui_css_rule_append_declaration(rule, "prop_b", "A", 0); /* spec_b > */
  (void)ui_css_rule_append_declaration(rule, "prop_c", "A", 0);
  (void)ui_css_stylesheet_append_rule(sheet, rule);

  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_UNIVERSAL, NULL);
  (void)ui_css_rule_append_declaration(rule, "prop_c", "B", 0); /* spec_c > */
  (void)ui_css_stylesheet_append_rule(sheet, rule);

  (void)ui_css_resolve_style(sheet, child, &style);
  (void)ui_css_computed_style_destroy(style);
  (void)ui_css_stylesheet_destroy(sheet);

  (void)ui_css_stylesheet_create(&sheet);
  /* Forward order for goto replace coverage */
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_UNIVERSAL, NULL);
  (void)ui_css_rule_append_declaration(rule, "prop_cc", "B", 0);
  (void)ui_css_stylesheet_append_rule(sheet, rule);

  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "span");
  (void)ui_css_rule_append_declaration(rule, "prop_bb", "A", 0);
  (void)ui_css_rule_append_declaration(rule, "prop_cc", "A",
                                       0); /* replace spec_c */
  (void)ui_css_stylesheet_append_rule(sheet, rule);

  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS,
                                    "myclass");
  (void)ui_css_rule_append_declaration(rule, "prop_aa", "C", 0);
  (void)ui_css_rule_append_declaration(rule, "prop_bb", "C",
                                       0); /* replace spec_b */
  (void)ui_css_stylesheet_append_rule(sheet, rule);

  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_ID, "child-id");
  (void)ui_css_rule_append_declaration(rule, "prop_aa", "E",
                                       0); /* replace spec_a */
  (void)ui_css_stylesheet_append_rule(sheet, rule);

  /* Layer order overrides */
  {
    int order1 = 0, order2 = 0;
    struct ui_css_rule *layer_rule1, *layer_rule2;
    (void)ui_css_stylesheet_register_layer(sheet, "L1", &order1);
    (void)ui_css_stylesheet_register_layer(sheet, "L2", &order2);

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_LAYER, &layer_rule1);
    layer_rule1->layer_name = C_MULTIPLATFORM_STRDUP("L1");

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_LAYER, &layer_rule2);
    layer_rule2->layer_name = C_MULTIPLATFORM_STRDUP("L2");

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "span");
    (void)ui_css_rule_append_declaration(rule, "prop_layer1", "B", 0);
    (void)ui_css_rule_append_declaration(rule, "prop_layer2", "B", 1);
    layer_rule2->nested_rules = rule;

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "span");
    (void)ui_css_rule_append_declaration(rule, "prop_layer1", "A", 0);
    (void)ui_css_rule_append_declaration(rule, "prop_layer2", "A", 1);
    layer_rule1->nested_rules = rule;

    (void)ui_css_stylesheet_append_rule(sheet, layer_rule2);
    (void)ui_css_stylesheet_append_rule(sheet, layer_rule1);
  }

  (void)ui_css_resolve_style(sheet, child, &style);
  (void)ui_css_computed_style_destroy(style);
  (void)ui_css_stylesheet_destroy(sheet);

  (void)ui_css_stylesheet_create(&sheet);
  /* :not(span) on child */
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                    "not");
  sel = rule->selectors;
  sel->nested_selector = create_mock_selector(UI_CSS_SELECTOR_TYPE_TAG, "span");
  (void)ui_css_rule_append_declaration(rule, "color", "red", 0);
  (void)ui_css_stylesheet_append_rule(sheet, rule);

  /* pseudo-element ::before */
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_PSEUDO_ELEMENT,
                                    "before");
  (void)ui_css_rule_append_declaration(rule, "color", "blue", 0);
  (void)ui_css_stylesheet_append_rule(sheet, rule);

  /* id doesn't match */
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_ID, "wrong-id");
  (void)ui_css_rule_append_declaration(rule, "color", "green", 0);
  (void)ui_css_stylesheet_append_rule(sheet, rule);

  /* :has(div) with nested selector to cover 1100s branch */
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                    "has");
  sel = rule->selectors;
  sel->nested_selector = create_mock_selector(UI_CSS_SELECTOR_TYPE_TAG, "div");
  (void)ui_css_rule_append_declaration(rule, "margin", "10px", 0);
  (void)ui_css_stylesheet_append_rule(sheet, rule);

  (void)ui_css_resolve_style(sheet, child, &style);
  (void)ui_css_computed_style_destroy(style);

  struct ui_dom_node *cov_parent = NULL, *cov_child = NULL;
  (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &cov_parent);
  (void)ui_dom_node_set_tag_name(cov_parent, "div");
  (void)ui_dom_node_set_attribute(cov_parent, "id", "parent-id");
  (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &cov_child);
  (void)ui_dom_node_set_tag_name(cov_child, "span");
  (void)ui_dom_node_set_attribute(cov_child, "id", "child-id");
  (void)ui_dom_node_set_attribute(cov_child, "class", "myclass");
  (void)ui_dom_node_append_child(cov_parent, cov_child);

  (void)ui_css_stylesheet_create(&sheet);
  (void)ui_css_stylesheet_create(&sheet);
  /* :not(span) on child */
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                    "not");
  sel = rule->selectors;
  sel->nested_selector = create_mock_selector(UI_CSS_SELECTOR_TYPE_TAG, "span");
  (void)ui_css_rule_append_declaration(rule, "color", "red", 0);
  (void)ui_css_stylesheet_append_rule(sheet, rule);

  /* pseudo-element ::before */
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_PSEUDO_ELEMENT,
                                    "before");
  (void)ui_css_rule_append_declaration(rule, "color", "blue", 0);
  (void)ui_css_stylesheet_append_rule(sheet, rule);

  /* id doesn't match */
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_ID, "wrong-id");
  (void)ui_css_rule_append_declaration(rule, "color", "green", 0);
  (void)ui_css_stylesheet_append_rule(sheet, rule);

  /* :has(div) with nested selector to cover 1100s branch */
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                    "has");
  sel = rule->selectors;
  sel->nested_selector = create_mock_selector(UI_CSS_SELECTOR_TYPE_TAG, "div");
  (void)ui_css_rule_append_declaration(rule, "margin", "10px", 0);
  (void)ui_css_stylesheet_append_rule(sheet, rule);

  (void)ui_css_resolve_style(sheet, cov_child, &style);
  (void)ui_css_computed_style_destroy(style);
  (void)ui_css_stylesheet_destroy(sheet);

  /* Layer order overrides: L1 then L2 */
  (void)ui_css_stylesheet_create(&sheet);
  {
    int order1 = 0, order2 = 0;
    struct ui_css_rule *layer_rule1, *layer_rule2;
    (void)ui_css_stylesheet_register_layer(sheet, "L3", &order1);
    (void)ui_css_stylesheet_register_layer(sheet, "L4", &order2);

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_LAYER, &layer_rule1);
    layer_rule1->layer_name = C_MULTIPLATFORM_STRDUP("L3");

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_LAYER, &layer_rule2);
    layer_rule2->layer_name = C_MULTIPLATFORM_STRDUP("L4");

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "span");
    (void)ui_css_rule_append_declaration(rule, "prop_layer_normal2", "A", 0);
    (void)ui_css_rule_append_declaration(rule, "prop_layer_important2", "A", 1);
    layer_rule1->nested_rules = rule;

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "span");
    (void)ui_css_rule_append_declaration(rule, "prop_layer_normal2", "B", 0);
    (void)ui_css_rule_append_declaration(rule, "prop_layer_important2", "B", 1);
    layer_rule2->nested_rules = rule;

    (void)ui_css_stylesheet_append_rule(sheet, layer_rule1);
    (void)ui_css_stylesheet_append_rule(sheet, layer_rule2);
  }
  (void)ui_css_resolve_style(sheet, cov_child, &style);
  (void)ui_css_computed_style_destroy(style);
  (void)ui_css_stylesheet_destroy(sheet);
  (void)ui_css_stylesheet_create(&sheet);
  {
    int order1 = 0, order2 = 0;
    struct ui_css_rule *layer_rule1, *layer_rule2;
    (void)ui_css_stylesheet_register_layer(sheet, "L3", &order1);
    (void)ui_css_stylesheet_register_layer(sheet, "L4", &order2);

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_LAYER, &layer_rule1);
    layer_rule1->layer_name = C_MULTIPLATFORM_STRDUP("L3");

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_LAYER, &layer_rule2);
    layer_rule2->layer_name = C_MULTIPLATFORM_STRDUP("L4");

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "span");
    (void)ui_css_rule_append_declaration(rule, "prop_layer_normal2", "A", 0);
    (void)ui_css_rule_append_declaration(rule, "prop_layer_important2", "A", 1);
    layer_rule1->nested_rules = rule;

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "span");
    (void)ui_css_rule_append_declaration(rule, "prop_layer_normal2", "B", 0);
    (void)ui_css_rule_append_declaration(rule, "prop_layer_important2", "B", 1);
    layer_rule2->nested_rules = rule;

    (void)ui_css_stylesheet_append_rule(sheet, layer_rule1);
    (void)ui_css_stylesheet_append_rule(sheet, layer_rule2);
  }
  (void)ui_css_resolve_style(sheet, cov_child, &style);
  (void)ui_css_computed_style_destroy(style);
  (void)ui_css_stylesheet_destroy(sheet);
  (void)ui_dom_node_destroy(cov_parent);
  (void)ui_dom_node_destroy(parent);
}

struct ui_css_selector *create_mock_selector(enum ui_css_selector_type type,
                                             const char *val);
int test_cssom_part1_selectors(void);
int test_cssom_part2_rules(void);
int test_cssom_part3_declarations(void);
int test_cssom_part4_stylesheets(void);
int test_cssom_part5_serialization(void);
int test_cssom_part6_memory(void);
int test_cssom_part7_vars(void);

int main(void) {
  if (test_cssom_part1_selectors() != 0)
    return 1;
  if (test_cssom_part2_rules() != 0)
    return 1;
  if (test_cssom_part3_declarations() != 0)
    return 1;
  if (test_cssom_part4_stylesheets() != 0)
    return 1;
  if (test_cssom_part5_serialization() != 0)
    return 1;
  if (test_cssom_part6_memory() != 0)
    return 1;
  if (test_cssom_part7_vars() != 0)
    return 1;
  return 0;
}
