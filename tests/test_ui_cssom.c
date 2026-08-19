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

static char *my_strdup(const char *s) {
  size_t len = strlen(s);
  char *d = C_MULTIPLATFORM_MALLOC(len + 1);
  if (d)
    strcpy(d, s);
  return d;
}

static void test_malformed_selector(void) {
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

static struct ui_css_selector *
create_mock_selector(enum ui_css_selector_type type, const char *val) {
  struct ui_css_selector *sel = C_MULTIPLATFORM_MALLOC(sizeof(*sel));
  if (sel) {
    memset(sel, 0, sizeof(*sel));
    sel->type = type;
    if (val)
      sel->value = C_MULTIPLATFORM_STRDUP(val);
  }
  return sel;
}

static void test_cssom_oom(void) {
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

static void test_coverage_ui_cssom(void) {
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

int main(void) {
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_css_rule *rule = NULL, *rule2 = NULL, *rule3 = NULL;
  struct ui_dom_node *node = NULL, *child = NULL, *grandchild = NULL;
  struct ui_css_computed_style *style = NULL;
  struct ui_css_variable_store *store = NULL;
  ui_error_t rc;
  int order;
  const char *val;

  /* Basic Create / Destroy / Errors */
  TEST_ASSERT(ui_css_stylesheet_create(NULL) == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_css_stylesheet_create(&sheet);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  (void)ui_css_stylesheet_destroy(NULL);

  /* Namespaces */
  TEST_ASSERT(ui_css_stylesheet_register_namespace(NULL, "a", "b") ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_stylesheet_register_namespace(sheet, "a", NULL) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_stylesheet_register_namespace(NULL, NULL, NULL) ==
              UI_ERROR_INVALID_ARGUMENT);
  rc = ui_css_stylesheet_register_namespace(sheet, "svg",
                                            "http://www.w3.org/2000/svg");
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_stylesheet_register_namespace(sheet, NULL, "http://default");
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_stylesheet_register_namespace(sheet, "math", "http://math");
  TEST_ASSERT(rc == UI_ERROR_NONE);

  /* Layers */
  TEST_ASSERT(ui_css_stylesheet_register_layer(NULL, "base", &order) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_stylesheet_register_layer(sheet, "base", NULL) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_stylesheet_register_layer(NULL, NULL, NULL) ==
              UI_ERROR_INVALID_ARGUMENT);
  rc = ui_css_stylesheet_register_layer(sheet, NULL, &order); /* Unlayered */
  TEST_ASSERT(rc == UI_ERROR_NONE && order == 0x7FFFFFFF);
  rc = ui_css_stylesheet_register_layer(sheet, "base", &order);
  TEST_ASSERT(rc == UI_ERROR_NONE && order == 0);
  rc = ui_css_stylesheet_register_layer(sheet, "theme", &order);
  TEST_ASSERT(rc == UI_ERROR_NONE && order == 1);
  rc = ui_css_stylesheet_register_layer(sheet, "base", &order); /* existing */
  TEST_ASSERT(rc == UI_ERROR_NONE && order == 0);

  /* Rules */
  TEST_ASSERT(ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, NULL) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_rule_destroy(NULL) == UI_ERROR_NONE);

  /* STYLE rule */
  rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  TEST_ASSERT(ui_css_stylesheet_append_rule(NULL, rule) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_stylesheet_append_rule(sheet, NULL) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_stylesheet_append_rule(NULL, NULL) ==
              UI_ERROR_INVALID_ARGUMENT);
  rc = ui_css_stylesheet_append_rule(sheet, rule);
  TEST_ASSERT(rc == UI_ERROR_NONE);

  /* Append second rule */
  rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule2);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_stylesheet_append_rule(sheet, rule2);
  TEST_ASSERT(rc == UI_ERROR_NONE);

  /* LAYER rule */
  rc = ui_css_rule_create(UI_CSS_RULE_TYPE_LAYER, &rule3);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rule3->layer_name = my_strdup("theme");
  rc = ui_css_stylesheet_append_rule(sheet, rule3);
  TEST_ASSERT(rc == UI_ERROR_NONE);

  /* Anonymous layer */
  struct ui_css_rule *anon_layer;
  rc = ui_css_rule_create(UI_CSS_RULE_TYPE_LAYER, &anon_layer);
  rc = ui_css_stylesheet_append_rule(sheet, anon_layer);

  struct ui_css_rule *nested_anon_layer;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_anon_layer);
  ui_css_rule_append_selector(nested_anon_layer, UI_CSS_SELECTOR_TYPE_TAG,
                              "div");
  (void)ui_css_rule_append_declaration(nested_anon_layer, "color", "green", 0);
  (void)ui_css_rule_append_declaration(nested_anon_layer, "margin", "0", 0);
  anon_layer->nested_rules = nested_anon_layer;

  /* Nested rule inside LAYER */
  struct ui_css_rule *nested_layer;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_layer);
  (void)ui_css_rule_append_selector(nested_layer, UI_CSS_SELECTOR_TYPE_TAG,
                                    "div");
  (void)ui_css_rule_append_declaration(nested_layer, "color", "red", 0);
  rule3->nested_rules = nested_layer;

  /* MEDIA rule */
  struct ui_css_rule *media_rule;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_MEDIA, &media_rule);
  media_rule->media_condition = my_strdup("screen and (min-width: 900px)");
  (void)ui_css_stylesheet_append_rule(sheet, media_rule);
  struct ui_css_rule *nested_media;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_media);
  (void)ui_css_rule_append_selector(nested_media, UI_CSS_SELECTOR_TYPE_TAG,
                                    "div");
  (void)ui_css_rule_append_declaration(nested_media, "margin", "10px", 0);
  media_rule->nested_rules = nested_media;

  /* SUPPORTS rule */
  struct ui_css_rule *supp_rule;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &supp_rule);
  supp_rule->supports_condition = my_strdup("display: flex");
  (void)ui_css_stylesheet_append_rule(sheet, supp_rule);
  struct ui_css_rule *nested_supp;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_supp);
  (void)ui_css_rule_append_selector(nested_supp, UI_CSS_SELECTOR_TYPE_TAG,
                                    "div");
  (void)ui_css_rule_append_declaration(nested_supp, "display", "flex", 0);
  supp_rule->nested_rules = nested_supp;

  /* CONTAINER rule */
  struct ui_css_rule *cont_rule;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_CONTAINER, &cont_rule);
  cont_rule->container_condition = my_strdup("(min-width: 500px)");
  (void)ui_css_stylesheet_append_rule(sheet, cont_rule);
  struct ui_css_rule *nested_cont;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_cont);
  (void)ui_css_rule_append_selector(nested_cont, UI_CSS_SELECTOR_TYPE_TAG,
                                    "div");
  (void)ui_css_rule_append_declaration(nested_cont, "width", "100%", 0);
  cont_rule->nested_rules = nested_cont;

  /* SCOPE rule */
  struct ui_css_rule *scope_rule;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SCOPE, &scope_rule);
  scope_rule->scope_start =
      create_mock_selector(UI_CSS_SELECTOR_TYPE_CLASS, ".card");
  scope_rule->scope_end =
      create_mock_selector(UI_CSS_SELECTOR_TYPE_CLASS, ".hole");
  (void)ui_css_stylesheet_append_rule(sheet, scope_rule);
  struct ui_css_rule *nested_scope;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_scope);
  (void)ui_css_rule_append_selector(nested_scope, UI_CSS_SELECTOR_TYPE_TAG,
                                    "div");
  (void)ui_css_rule_append_declaration(nested_scope, "background", "blue", 0);
  scope_rule->nested_rules = nested_scope;

  /* PROPERTY rule */
  struct ui_css_rule *prop_rule;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_PROPERTY, &prop_rule);
  prop_rule->property_name = my_strdup("--my-var");
  prop_rule->property_syntax = my_strdup("<color>");
  prop_rule->property_initial_value = my_strdup("red");
  (void)ui_css_stylesheet_append_rule(sheet, prop_rule);

  struct ui_css_rule *prop_rule2;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_PROPERTY, &prop_rule2);
  prop_rule2->property_name = my_strdup("--my-var2");
  prop_rule2->property_syntax = my_strdup("<color>");
  prop_rule2->property_initial_value = my_strdup("red");
  (void)ui_css_rule_append_declaration(prop_rule2, "color", "blue", 0);
  (void)ui_css_rule_append_declaration(prop_rule2, "margin", "0", 0);
  (void)ui_css_rule_destroy(prop_rule2);

  struct ui_css_rule *unknown_rule;
  (void)ui_css_rule_create((enum ui_css_rule_type)999, &unknown_rule);
  (void)ui_css_rule_destroy(unknown_rule);

  /* Selectors */
  TEST_ASSERT(ui_css_rule_append_selector(NULL, UI_CSS_SELECTOR_TYPE_TAG,
                                          "div") == UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG,
                                          NULL) == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_UNIVERSAL, NULL);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "btn");
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_ID, "main");
  TEST_ASSERT(rc == UI_ERROR_NONE);

  /* Attribute Selectors */
  TEST_ASSERT(ui_css_rule_append_selector_attr(NULL, "type",
                                               UI_CSS_ATTR_OP_EQUALS, "text") ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_rule_append_selector_attr(rule, NULL,
                                               UI_CSS_ATTR_OP_EQUALS, "text") ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_rule_append_selector_attr(NULL, NULL,
                                               UI_CSS_ATTR_OP_EQUALS, "text") ==
              UI_ERROR_INVALID_ARGUMENT);

  rc = ui_css_rule_append_selector_attr(rule, "data-val", UI_CSS_ATTR_OP_NONE,
                                        NULL);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_rule_append_selector_attr(rule, "data-val2", UI_CSS_ATTR_OP_NONE,
                                        "val");
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_rule_append_selector_attr(rule, "type", UI_CSS_ATTR_OP_EQUALS,
                                        "text");
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_rule_append_selector_attr(rule, "class", UI_CSS_ATTR_OP_INCLUDES,
                                        "active");
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc =
      ui_css_rule_append_selector_attr(rule, "lang", UI_CSS_ATTR_OP_DASH, "en");
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_rule_append_selector_attr(rule, "href", UI_CSS_ATTR_OP_PREFIX,
                                        "https");
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_rule_append_selector_attr(rule, "href", UI_CSS_ATTR_OP_SUFFIX,
                                        ".pdf");
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_rule_append_selector_attr(rule, "title", UI_CSS_ATTR_OP_SUBSTRING,
                                        "hello");
  TEST_ASSERT(rc == UI_ERROR_NONE);

  /* Declarations */
  TEST_ASSERT(ui_css_rule_append_declaration(NULL, "color", "red", 0) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_rule_append_declaration(rule, NULL, "red", 0) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_rule_append_declaration(rule, "color", NULL, 0) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_rule_append_declaration(NULL, NULL, NULL, 0) ==
              UI_ERROR_INVALID_ARGUMENT);

  rc = ui_css_rule_append_declaration(rule, "color", "red", 0);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_rule_append_declaration(rule, "display", "block",
                                      1); /* important */
  TEST_ASSERT(rc == UI_ERROR_NONE);

  /* DOM Node resolution */
  (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  (void)ui_dom_node_set_tag_name(node, "div");
  (void)ui_dom_node_set_attribute(node, "id", "main");
  (void)ui_dom_node_set_attribute(node, "class", "btn active card");
  (void)ui_dom_node_set_attribute(node, "data-val", "yes");
  (void)ui_dom_node_set_attribute(node, "type", "text");
  (void)ui_dom_node_set_attribute(node, "lang", "en-US");
  (void)ui_dom_node_set_attribute(node, "href", "https://example.pdf");
  (void)ui_dom_node_set_attribute(node, "title", "say hello there");

  /* Add pseudo class rules */
  const char *pseudos[] = {
      "first-child", "last-child",    "empty",         "hover",
      "active",      "focus",         "focus-visible", "focus-within",
      "target",      "target-within", "checked",       "disabled",
      "enabled",     "required",      "optional",      "read-only",
      "read-write",  "indeterminate", "default",       "invalid",
      "valid",       "unknown-pseudo"};
  size_t i;
  for (i = 0; i < sizeof(pseudos) / sizeof(pseudos[0]); i++) {
    struct ui_css_rule *pr;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &pr);
    ui_css_rule_append_selector(pr, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                pseudos[i]);
    (void)ui_css_rule_append_declaration(pr, pseudos[i], "true", 0);
    (void)ui_css_stylesheet_append_rule(sheet, pr);
  }

  /* Structural/State pseudo matches */
  node->state_flags |=
      UI_DOM_NODE_STATE_HOVER | UI_DOM_NODE_STATE_ACTIVE |
      UI_DOM_NODE_STATE_FOCUS | UI_DOM_NODE_STATE_FOCUS_VISIBLE |
      UI_DOM_NODE_STATE_FOCUS_WITHIN | UI_DOM_NODE_STATE_TARGET |
      UI_DOM_NODE_STATE_TARGET_WITHIN;
  (void)ui_dom_node_set_attribute(node, "checked", "");
  (void)ui_dom_node_set_attribute(node, "disabled", "");
  (void)ui_dom_node_set_attribute(node, "required", "");
  (void)ui_dom_node_set_attribute(node, "readonly", "");
  (void)ui_dom_node_set_attribute(node, "indeterminate", "");
  (void)ui_dom_node_set_attribute(node, "default", "");
  (void)ui_dom_node_set_attribute(node, "aria-invalid", "true");

  TEST_ASSERT(ui_css_resolve_style(NULL, node, &style) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_resolve_style(sheet, NULL, &style) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_resolve_style(sheet, node, NULL) ==
              UI_ERROR_INVALID_ARGUMENT);

  rc = ui_css_resolve_style(sheet, node, &style);
  TEST_ASSERT(rc == UI_ERROR_NONE);

  /* Validate some properties */
  TEST_ASSERT(ui_css_computed_style_get_property(NULL, "color", &val) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_computed_style_get_property(style, NULL, &val) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_computed_style_get_property(style, "color", NULL) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_computed_style_get_property(NULL, NULL, NULL) ==
              UI_ERROR_INVALID_ARGUMENT);

  rc = ui_css_computed_style_get_property(style, "color", &val);
  TEST_ASSERT(rc == UI_ERROR_NONE && strcmp(val, "red") == 0);

  rc = ui_css_computed_style_get_property(style, "first-child", &val);
  TEST_ASSERT(rc == UI_ERROR_NONE && strcmp(val, "true") == 0);

  rc = ui_css_computed_style_get_property(style, "checked", &val);
  TEST_ASSERT(rc == UI_ERROR_NONE && strcmp(val, "true") == 0);

  (void)ui_css_computed_style_destroy(style);

  /* Children and ancestry logic */
  (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child);
  (void)ui_dom_node_set_tag_name(child, "span");
  (void)ui_dom_node_append_child(node, child);

  (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &grandchild);
  (void)ui_dom_node_set_tag_name(grandchild, "p");
  (void)ui_dom_node_append_child(child, grandchild);

  /* Add :has and :is, :where, :not */
  struct ui_css_rule *func_rule;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &func_rule);

  struct ui_css_selector *is_sel = calloc(1, sizeof(*is_sel));
  is_sel->type = UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS;
  is_sel->value = my_strdup("is");
  struct ui_css_selector *nested1 = calloc(1, sizeof(*nested1));
  nested1->type = UI_CSS_SELECTOR_TYPE_TAG;
  nested1->value = my_strdup("div");
  is_sel->nested_selector = nested1;
  func_rule->selectors = is_sel;
  (void)ui_css_rule_append_declaration(func_rule, "is-matched", "yes", 0);
  (void)ui_css_stylesheet_append_rule(sheet, func_rule);

  struct ui_css_rule *has_rule;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &has_rule);
  struct ui_css_selector *has_sel = calloc(1, sizeof(*has_sel));
  has_sel->type = UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS;
  has_sel->value = my_strdup("has");
  struct ui_css_selector *nested2 = calloc(1, sizeof(*nested2));
  nested2->type = UI_CSS_SELECTOR_TYPE_TAG;
  nested2->value = my_strdup("p");
  has_sel->nested_selector = nested2;
  has_rule->selectors = has_sel;
  (void)ui_css_rule_append_declaration(has_rule, "has-matched", "yes", 0);
  (void)ui_css_stylesheet_append_rule(sheet, has_rule);

  rc = ui_css_resolve_style(sheet, node, &style);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_computed_style_get_property(style, "is-matched", &val);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_computed_style_get_property(style, "has-matched", &val);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  (void)ui_css_computed_style_destroy(style);

  /* CSS Variables */
  TEST_ASSERT(ui_css_variable_store_create(NULL) == UI_ERROR_INVALID_ARGUMENT);
  (void)ui_css_variable_store_destroy(NULL);
  rc = ui_css_variable_store_create(&store);
  TEST_ASSERT(rc == UI_ERROR_NONE);

  TEST_ASSERT(ui_css_variable_store_set(NULL, "a", "b") ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_variable_store_set(store, NULL, "b") ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_variable_store_set(store, "a", NULL) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_variable_store_set(NULL, NULL, NULL) ==
              UI_ERROR_INVALID_ARGUMENT);

  rc = ui_css_variable_store_set(store, "--bg", "blue");
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_variable_store_set(store, "--bg", "green"); /* overwrite */
  TEST_ASSERT(rc == UI_ERROR_NONE);

  char *resolved;
  TEST_ASSERT(ui_css_resolve_variables(NULL, "var(--bg)", &resolved) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_resolve_variables(store, NULL, &resolved) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_resolve_variables(store, "var(--bg)", NULL) ==
              UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_resolve_variables(NULL, NULL, &resolved) ==
              UI_ERROR_INVALID_ARGUMENT);

  rc = ui_css_resolve_variables(store, "var(--bg)", &resolved);
  TEST_ASSERT(rc == UI_ERROR_NONE && strcmp(resolved, "green") == 0);
  free(resolved);

  /* Fallback test */
  rc = ui_css_resolve_variables(store, "var(--unknown, default)", &resolved);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  free(resolved);

  (void)ui_css_variable_store_destroy(store);

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }

  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) and"); /* missing end term */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }

  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) or"); /* missing end term */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("(not (display: flex)"); /* missing inner end */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) or )"); /* trailing paren */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;

    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    (void)ui_css_stylesheet_create(&tmp_sheet);

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

    (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);

    (void)ui_css_stylesheet_destroy(tmp_sheet);
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    (void)ui_css_stylesheet_create(&tmp_sheet);
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) and"); /* missing end term */
    (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

    (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
    (void)ui_css_stylesheet_destroy(tmp_sheet);
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    (void)ui_css_stylesheet_create(&tmp_sheet);
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) or"); /* missing end term */
    (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

    (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
    (void)ui_css_stylesheet_destroy(tmp_sheet);
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }

  (void)ui_dom_node_destroy(node);
  (void)ui_css_stylesheet_destroy(sheet);

  /* OOM Mocks - Very high coverage */
  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_stylesheet_create(&sheet) == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  rc = ui_css_stylesheet_create(&sheet);
  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_stylesheet_register_namespace(sheet, "a", "b") ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  TEST_ASSERT(ui_css_stylesheet_register_namespace(sheet, "a", "b") ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 2;
  TEST_ASSERT(ui_css_stylesheet_register_namespace(sheet, "a", "b") ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_stylesheet_register_namespace(sheet, NULL, "b") ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  TEST_ASSERT(ui_css_stylesheet_register_namespace(sheet, NULL, "b") ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_stylesheet_register_layer(sheet, "a", &order) ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  TEST_ASSERT(ui_css_stylesheet_register_layer(sheet, "a", &order) ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule) ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);

  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS,
                                          "a") == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  TEST_ASSERT(ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS,
                                          "a") == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_UNIVERSAL,
                                          NULL) == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_rule_append_selector_attr(rule, "a", UI_CSS_ATTR_OP_EQUALS,
                                               "b") == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  TEST_ASSERT(ui_css_rule_append_selector_attr(rule, "a", UI_CSS_ATTR_OP_EQUALS,
                                               "b") == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 2;
  ui_error_t rc_debug =
      ui_css_rule_append_selector_attr(rule, "a", UI_CSS_ATTR_OP_EQUALS, "b");
  printf("DEBUG: rc_debug=%d\n", rc_debug);
  TEST_ASSERT(rc_debug == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_rule_append_selector_attr(rule, "a", UI_CSS_ATTR_OP_NONE,
                                               NULL) == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  TEST_ASSERT(ui_css_rule_append_selector_attr(rule, "a", UI_CSS_ATTR_OP_NONE,
                                               NULL) == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_rule_append_declaration(rule, "a", "b", 0) ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  TEST_ASSERT(ui_css_rule_append_declaration(rule, "a", "b", 0) ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 2;
  TEST_ASSERT(ui_css_rule_append_declaration(rule, "a", "b", 0) ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  (void)ui_css_rule_append_declaration(rule, "prop1", "val1", 0);
  (void)ui_css_rule_append_declaration(rule, "prop2", "val2", 0);

  (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  (void)ui_css_stylesheet_append_rule(sheet, rule);
  (void)ui_css_rule_append_declaration(rule, "a", "b", 0);

  {
    int oom_cnt = 0;
    while (1) {
      g_malloc_fail_countdown = oom_cnt;
      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_OUT_OF_MEMORY) {
        oom_cnt++;
      } else {
        break;
      }
    }
  }
  (void)ui_css_computed_style_destroy(style);
  g_malloc_fail_countdown = -1;

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }

  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) and"); /* missing end term */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }

  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) or"); /* missing end term */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("(not (display: flex)"); /* missing inner end */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) or )"); /* trailing paren */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;

    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    (void)ui_css_stylesheet_create(&tmp_sheet);

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

    (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);

    (void)ui_css_stylesheet_destroy(tmp_sheet);
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    (void)ui_css_stylesheet_create(&tmp_sheet);
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) and"); /* missing end term */
    (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

    (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
    (void)ui_css_stylesheet_destroy(tmp_sheet);
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    (void)ui_css_stylesheet_create(&tmp_sheet);
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) or"); /* missing end term */
    (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

    (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
    (void)ui_css_stylesheet_destroy(tmp_sheet);
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }

  (void)ui_css_stylesheet_destroy(sheet);
  (void)ui_dom_node_destroy(node);

  /* Eval condition paths */
  /* These are static functions but reached via supports/media rules */
  rc = ui_css_stylesheet_create(&sheet);
  struct ui_css_rule *r;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &r);
  struct ui_css_rule *nested_r;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_r);
  (void)ui_css_rule_append_selector(nested_r, UI_CSS_SELECTOR_TYPE_TAG, "div");
  (void)ui_css_rule_append_declaration(nested_r, "display", "flex", 0);
  r->nested_rules = nested_r;
  (void)ui_css_stylesheet_append_rule(sheet, r);

  struct ui_css_rule *media_r;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_MEDIA, &media_r);
  media_r->media_condition = my_strdup("screen");
  struct ui_css_rule *nested_mr;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_mr);
  (void)ui_css_rule_append_selector(nested_mr, UI_CSS_SELECTOR_TYPE_TAG, "div");
  (void)ui_css_rule_append_declaration(nested_mr, "color", "red", 0);
  media_r->nested_rules = nested_mr;
  (void)ui_css_stylesheet_append_rule(sheet, media_r);

  struct ui_css_rule *cont_r;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_CONTAINER, &cont_r);
  cont_r->container_condition = my_strdup("(min-width: 100px)");
  struct ui_css_rule *nested_cr;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_cr);
  (void)ui_css_rule_append_selector(nested_cr, UI_CSS_SELECTOR_TYPE_TAG, "div");
  (void)ui_css_rule_append_declaration(nested_cr, "color", "blue", 0);
  cont_r->nested_rules = nested_cr;
  (void)ui_css_stylesheet_append_rule(sheet, cont_r);

  struct ui_css_rule *scope_r;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SCOPE, &scope_r);
  scope_r->scope_start =
      create_mock_selector(UI_CSS_SELECTOR_TYPE_CLASS, ".card");
  struct ui_css_rule *nested_scope_r;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_scope_r);
  (void)ui_css_rule_append_selector(nested_scope_r, UI_CSS_SELECTOR_TYPE_TAG,
                                    "div");
  (void)ui_css_rule_append_declaration(nested_scope_r, "background", "yellow",
                                       0);
  scope_r->nested_rules = nested_scope_r;
  (void)ui_css_stylesheet_append_rule(sheet, scope_r);

  struct ui_css_rule *style_r;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &style_r);
  (void)ui_css_rule_append_selector(style_r, UI_CSS_SELECTOR_TYPE_TAG, "div");
  struct ui_css_rule *nested_sr;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_sr);
  (void)ui_css_rule_append_selector(nested_sr, UI_CSS_SELECTOR_TYPE_CLASS,
                                    "nested");
  (void)ui_css_rule_append_declaration(nested_sr, "margin", "0", 0);
  style_r->nested_rules = nested_sr;
  (void)ui_css_stylesheet_append_rule(sheet, style_r);

  /* Condition variations */
  const char *conds[] = {"not (display: flex)",
                         "(display: grid) or (display: flex)",
                         "(display: flex) and (display: none)",
                         "selector(:has(.a))",
                         "selector(:invalid)",
                         "(((a)))",
                         "not not",
                         "(not (display: flex))",
                         "(not display: flex)",
                         "(display: flex) and (not (display: grid))",
                         "(display: flex) or (not (display: grid))",
                         "and and or or not not",
                         "and (display: flex)",
                         "or (display: flex)",
                         "not display: flex",
                         "(((",
                         "selector",
                         "selector(",
                         "selector(:has",
                         "selector(:has()",
                         "not",
                         "not (",
                         "not )",
                         "(a) and",
                         "(a) or"};
  for (i = 0; i < sizeof(conds) / sizeof(conds[0]); i++) {
    if (r->supports_condition) {
      C_MULTIPLATFORM_FREE(r->supports_condition);
    }
    r->supports_condition = my_strdup(conds[i]);
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);
    (void)ui_dom_node_destroy(node);
  }

  (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  (void)ui_dom_node_set_tag_name(node, "div");
  (void)ui_dom_node_set_attribute(node, "class", "nested");
  {
    int oom_cnt = 0;
    while (1) {
      g_malloc_fail_countdown = oom_cnt;
      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_OUT_OF_MEMORY) {
        oom_cnt++;
      } else {
        break;
      }
    }
  }
  (void)ui_css_computed_style_destroy(style);
  g_malloc_fail_countdown = -1;
  (void)ui_dom_node_destroy(node);

  /* Check supports evaluation failure */
  if (r->supports_condition) {
    C_MULTIPLATFORM_FREE(r->supports_condition);
  }
  r->supports_condition = my_strdup("(a) and"); /* parse error */
  (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  rc = ui_css_resolve_style(sheet, node, &style);
  if (rc != UI_ERROR_PARSE_FAILED) {
    printf("Expected PARSE_FAILED, got %d\n", rc);
    TEST_ASSERT(rc == UI_ERROR_PARSE_FAILED);
  }
  (void)ui_dom_node_destroy(node);

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }

  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) and"); /* missing end term */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }

  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) or"); /* missing end term */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("(not (display: flex)"); /* missing inner end */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) or )"); /* trailing paren */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;

    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    (void)ui_css_stylesheet_create(&tmp_sheet);

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

    (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);

    (void)ui_css_stylesheet_destroy(tmp_sheet);
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    (void)ui_css_stylesheet_create(&tmp_sheet);
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) and"); /* missing end term */
    (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

    (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
    (void)ui_css_stylesheet_destroy(tmp_sheet);
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    (void)ui_css_stylesheet_create(&tmp_sheet);
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) or"); /* missing end term */
    (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

    (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
    (void)ui_css_stylesheet_destroy(tmp_sheet);
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      (void)ui_css_computed_style_destroy(style);
    }
  }

  (void)ui_css_stylesheet_destroy(sheet);

  {
    int match = 0;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);

    ui_css_rule_append_selector_attr(rule, "href", UI_CSS_ATTR_OP_EQUALS,
                                     "link");

    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    /* Cover class_list_contains missing branches */
    ui_css_rule_append_selector_attr(rule, "class", UI_CSS_ATTR_OP_INCLUDES,
                                     "btn");

    struct ui_css_computed_style *test_style = NULL;
    struct ui_css_stylesheet *sheet;
    (void)ui_css_stylesheet_create(&sheet);
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    (void)ui_dom_node_set_attribute(node, "class", "btn\tother");
    (void)ui_css_resolve_style(sheet, node, &test_style);
    (void)ui_css_computed_style_destroy(test_style);

    (void)ui_dom_node_set_attribute(node, "class", "btn\rother");
    (void)ui_css_resolve_style(sheet, node, &test_style);
    (void)ui_css_computed_style_destroy(test_style);

    (void)ui_dom_node_set_attribute(node, "class", "btn\nother");
    (void)ui_css_resolve_style(sheet, node, &test_style);
    (void)ui_css_computed_style_destroy(test_style);

    (void)ui_dom_node_set_attribute(node, "class", "btn");
    (void)ui_css_resolve_style(sheet, node, &test_style);
    (void)ui_css_computed_style_destroy(test_style);

    /* Cover strings */
    (void)ui_dom_node_set_attribute(node, "data-test", "val");
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_PREFIX,
                                     "value");
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_SUFFIX,
                                     "value");

    struct ui_css_computed_style *style;
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;

      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);

      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);

      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    (void)ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }

  {
    /* Mismatches and default operators */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    (void)ui_css_rule_append_selector_attr(rule, "data-test", 99, "value");
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_PREFIX,
                                     "somethingverylong");
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_SUFFIX,
                                     "somethingverylong");
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_INCLUDES,
                                     "v");
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_DASH,
                                     "somethingverylong");

    struct ui_dom_node *node;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    (void)ui_dom_node_set_attribute(node, "data-test", "val");

    struct ui_css_stylesheet *sheet;
    (void)ui_css_stylesheet_create(&sheet);
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    struct ui_css_computed_style *style;
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;

      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);

      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);

      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    (void)ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }

  {
    /* Combinator checks */
    struct ui_dom_node *node;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    (void)ui_dom_node_set_attribute(node, "class", "card");

    struct ui_dom_node *child1;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
    (void)ui_dom_node_set_attribute(child1, "class", "container");
    (void)ui_dom_node_append_child(node, child1);

    struct ui_dom_node *child2;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child2);
    (void)ui_dom_node_set_attribute(child2, "class", "hole");
    (void)ui_dom_node_append_child(child1, child2);

    /* Child matching */
    int m = 0;

    /* Cover :has */
    struct ui_css_rule *rule_has;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_has);
    ui_css_rule_append_selector(rule_has, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "has(.hole)");

    /* Scope checking logic */
    struct ui_css_rule *rule_scope;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_scope);
    ui_css_rule_append_selector(rule_scope, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "scope(.card, .container)");

    struct ui_css_stylesheet *sheet;
    (void)ui_css_stylesheet_create(&sheet);
    (void)ui_css_stylesheet_append_rule(sheet, rule_has);
    (void)ui_css_stylesheet_append_rule(sheet, rule_scope);

    struct ui_css_computed_style *style;
    (void)ui_css_resolve_style(sheet, child1, &style);
    (void)ui_css_computed_style_destroy(style);

    (void)ui_css_resolve_style(sheet, child2, &style);
    (void)ui_css_computed_style_destroy(style);

    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;

      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);

      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);

      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    (void)ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }

  {
    /* Combinator checks */
    struct ui_dom_node *node;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    (void)ui_dom_node_set_attribute(node, "class", "card container hole");

    struct ui_css_rule *rule_has;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_has);

    /* Child matching coverage */
    struct ui_dom_node *child1;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
    (void)ui_dom_node_set_attribute(child1, "class", "container");
    (void)ui_dom_node_append_child(node, child1);

    struct ui_css_selector *sel1;
    /* We can't directly manipulate selector easily so we just parse or set it
     */

    (void)ui_css_rule_destroy(rule_has);
    (void)ui_dom_node_destroy(node);
  }

  {
    /* Combinator checks for child matching */
    struct ui_dom_node *node;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    struct ui_css_rule *rule_has;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_has);

    /* check_mock_string_selector edge cases */
    struct ui_css_stylesheet *sheet;
    (void)ui_css_stylesheet_create(&sheet);
    ui_css_rule_append_selector(rule_has, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "has");
    (void)ui_css_stylesheet_append_rule(sheet, rule_has);

    struct ui_css_computed_style *style;
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;

      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);

      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);

      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    (void)ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }

  {
    /* Combinator checks for child matching and scope boundary */
    struct ui_dom_node *node;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    struct ui_css_rule *rule_has;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_has);

    /* check_mock_string_selector edge cases */
    struct ui_css_stylesheet *sheet;
    (void)ui_css_stylesheet_create(&sheet);
    ui_css_rule_append_selector(rule_has, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "has(.card)");
    (void)ui_css_stylesheet_append_rule(sheet, rule_has);

    struct ui_css_rule *rule_scope;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_scope);
    ui_css_rule_append_selector(rule_scope, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "scope(.card, .hole)");
    (void)ui_css_stylesheet_append_rule(sheet, rule_scope);

    struct ui_css_rule *rule_scope2;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_scope2);
    ui_css_rule_append_selector(rule_scope2, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "scope(NULL, .hole)");
    (void)ui_css_stylesheet_append_rule(sheet, rule_scope2);

    struct ui_css_computed_style *style;
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    (void)ui_dom_node_set_attribute(node, "class", "card");
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    (void)ui_dom_node_set_attribute(node, "class", "container");
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    (void)ui_dom_node_set_attribute(node, "class", "hole");
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;

      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);

      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);

      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    (void)ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }

  {
    /* Mismatches and default operators */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_INCLUDES,
                                     "v");

    struct ui_dom_node *node;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    (void)ui_dom_node_set_attribute(node, "data-test", "");

    struct ui_css_stylesheet *sheet;
    (void)ui_css_stylesheet_create(&sheet);
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    struct ui_css_computed_style *style;
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    /* Cover missing internal mock string cases */
    struct ui_css_rule *rule_mock;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_mock);
    ui_css_rule_append_selector(rule_mock, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "has(.unknown)");
    (void)ui_css_stylesheet_append_rule(sheet, rule_mock);
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;

      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);

      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);

      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    (void)ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }

  {
    /* Mismatches and default operators */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_INCLUDES,
                                     "v");
    ui_css_rule_append_selector_attr(rule, "data-test2",
                                     UI_CSS_ATTR_OP_INCLUDES, "v");

    struct ui_dom_node *node;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    (void)ui_dom_node_set_attribute(node, "data-test", "");
    (void)ui_dom_node_set_attribute(node, "data-test2", "v something");

    struct ui_css_stylesheet *sheet;
    (void)ui_css_stylesheet_create(&sheet);
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    struct ui_css_computed_style *style;
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    /* Cover missing internal mock string cases */
    struct ui_css_rule *rule_mock;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_mock);
    ui_css_rule_append_selector(rule_mock, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "has(.unknown)");
    (void)ui_css_stylesheet_append_rule(sheet, rule_mock);
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;

      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);

      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);

      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    (void)ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }

  {
    /* Mismatches and default operators */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);

    struct ui_css_stylesheet *sheet;
    (void)ui_css_stylesheet_create(&sheet);
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    struct ui_css_computed_style *style;

    struct ui_dom_node *node;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    /* Child matching coverage */
    struct ui_dom_node *child1;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
    (void)ui_dom_node_set_attribute(child1, "class", "hole");

    struct ui_dom_node *text_child;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_child);
    (void)ui_dom_node_append_child(child1, text_child);

    (void)ui_dom_node_append_child(node, child1);

    struct ui_css_rule *rule_has;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_has);
    struct ui_css_selector *has_hole =
        create_mock_selector(UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS, "has");
    struct ui_css_selector *nested_hole =
        create_mock_selector(UI_CSS_SELECTOR_TYPE_CLASS, ".hole");
    has_hole->nested_selector = nested_hole;
    rule_has->selectors = has_hole;
    (void)ui_css_stylesheet_append_rule(sheet, rule_has);
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    /* ui_dom_node_destroy(text_child); Since parent is freed, but let's make
     * sure it's not a double free. Actually node frees its children. So we just
     * need to wait for node destruction */

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;

      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);

      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);

      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      (void)ui_css_stylesheet_create(&tmp_sheet);
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      (void)ui_css_stylesheet_append_rule(tmp_sheet, rule);

      (void)ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      (void)ui_css_stylesheet_destroy(tmp_sheet);
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      (void)ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        (void)ui_css_computed_style_destroy(style);
      }
    }

    (void)ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }

  {
    struct ui_css_stylesheet *sheet;
    (void)ui_css_stylesheet_create(&sheet);
    int order1, order2, order3;
    (void)ui_css_stylesheet_register_layer(sheet, "L1", &order1);
    (void)ui_css_stylesheet_register_layer(sheet, "L2", &order2);
    (void)ui_css_stylesheet_register_layer(sheet, "L3", &order3);
    (void)ui_css_stylesheet_destroy(sheet);
  }
  {
    /* Missing pseudo classes */
    struct ui_dom_node *node;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    const char *pseudos[] = {"disabled",      "enabled",   "required",
                             "optional",      "read-only", "read-write",
                             "indeterminate", "default",   "invalid",
                             "valid",         "unknown"};

    struct ui_css_stylesheet *sheet;
    (void)ui_css_stylesheet_create(&sheet);

    for (int i = 0; i < 11; ++i) {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                  pseudos[i]);
      (void)ui_css_stylesheet_append_rule(sheet, rule);
    }

    /* Empty rule without selector */
    struct ui_css_rule *empty_rule;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &empty_rule);
    (void)ui_css_stylesheet_append_rule(sheet, empty_rule);

    struct ui_css_rule *attr_rule;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &attr_rule);
    ui_css_rule_append_selector_attr(attr_rule, "lang", UI_CSS_ATTR_OP_DASH,
                                     "en");
    (void)ui_css_stylesheet_append_rule(sheet, attr_rule);

    /* Test state 1: no attributes */
    struct ui_css_computed_style *style;
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    /* Test dash mismatch */
    (void)ui_dom_node_set_attribute(node, "lang", "en");
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);
    (void)ui_dom_node_remove_attribute(node, "lang");
    (void)ui_dom_node_set_attribute(node, "lang", "es");
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);
    (void)ui_dom_node_remove_attribute(node, "lang");
    (void)ui_dom_node_set_attribute(node, "lang", "eng");
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);
    (void)ui_dom_node_remove_attribute(node, "lang");
    (void)ui_dom_node_set_attribute(node, "lang", "en-US");
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);
    (void)ui_dom_node_remove_attribute(node, "lang");
    (void)ui_dom_node_set_attribute(node, "lang", "en-");
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    /* Test state 2: With native attributes */
    (void)ui_dom_node_set_attribute(node, "disabled", "");
    (void)ui_dom_node_set_attribute(node, "required", "");
    (void)ui_dom_node_set_attribute(node, "readonly", "");
    (void)ui_dom_node_set_attribute(node, "indeterminate", "");
    (void)ui_dom_node_set_attribute(node, "default", "");
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    /* Test state 3: With aria attributes (true) */
    (void)ui_dom_node_remove_attribute(node, "disabled");
    (void)ui_dom_node_remove_attribute(node, "required");
    (void)ui_dom_node_remove_attribute(node, "readonly");
    (void)ui_dom_node_remove_attribute(node, "indeterminate");
    (void)ui_dom_node_remove_attribute(node, "default");

    (void)ui_dom_node_set_attribute(node, "aria-disabled", "true");
    (void)ui_dom_node_set_attribute(node, "aria-required", "true");
    (void)ui_dom_node_set_attribute(node, "aria-readonly", "true");
    (void)ui_dom_node_set_attribute(node, "aria-checked", "true");
    (void)ui_dom_node_set_attribute(node, "aria-invalid", "true");
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    /* Test state 4: with aria attributes (false) */
    (void)ui_dom_node_set_attribute(node, "aria-disabled", "false");
    (void)ui_dom_node_set_attribute(node, "aria-required", "false");
    (void)ui_dom_node_set_attribute(node, "aria-readonly", "false");
    (void)ui_dom_node_set_attribute(node, "aria-checked", "false");
    (void)ui_dom_node_set_attribute(node, "aria-invalid", "false");
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    (void)ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }
  {
    /* Important rules overriding */
    struct ui_dom_node *node;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    struct ui_css_stylesheet *sheet;
    (void)ui_css_stylesheet_create(&sheet);

    int order1, order2;
    (void)ui_css_stylesheet_register_layer(sheet, "L1", &order1);
    (void)ui_css_stylesheet_register_layer(sheet, "L2", &order2);

    struct ui_css_rule *rule1, *rule2, *rule3, *rule4;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule1);
    (void)ui_css_rule_append_selector(rule1, UI_CSS_SELECTOR_TYPE_TAG, "div");
    (void)ui_css_rule_append_declaration(rule1, "color", "red", 0);
    (void)ui_css_rule_append_declaration(rule1, "background", "red", 1);

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule2);
    (void)ui_css_rule_append_selector(rule2, UI_CSS_SELECTOR_TYPE_TAG, "div");
    ui_css_rule_append_declaration(rule2, "color", "blue",
                                   1); /* overrides normal */
    ui_css_rule_append_declaration(rule2, "background", "blue",
                                   0); /* fails to override important */

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule3);
    (void)ui_css_rule_append_selector(rule3, UI_CSS_SELECTOR_TYPE_TAG, "div");
    ui_css_rule_append_declaration(rule3, "color", "green",
                                   1); /* same important, overrides */

    (void)ui_css_stylesheet_append_rule(sheet, rule1);
    (void)ui_css_stylesheet_append_rule(sheet, rule2);
    (void)ui_css_stylesheet_append_rule(sheet, rule3);

    (void)ui_dom_node_set_tag_name(node, "div");

    struct ui_css_computed_style *style;
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    (void)ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }

  {
    /* More coverage */
    struct ui_dom_node *node;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    struct ui_css_stylesheet *sheet;
    (void)ui_css_stylesheet_create(&sheet);

    struct ui_css_rule *r1, *r2, *r3, *r4, *r5;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r1);
    (void)ui_css_rule_append_selector(r1, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                      "not");
    (void)ui_css_stylesheet_append_rule(sheet, r1);

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r2);
    ui_css_rule_append_selector(r2, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "checked");
    (void)ui_css_stylesheet_append_rule(sheet, r2);

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r3);
    ui_css_rule_append_selector(r3, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "target");
    (void)ui_css_stylesheet_append_rule(sheet, r3);

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r4);
    ui_css_rule_append_selector(r4, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "target-within");
    (void)ui_css_stylesheet_append_rule(sheet, r4);

    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SCOPE, &r5);
    r5->scope_start = NULL;
    r5->scope_end = NULL;
    struct ui_css_rule *r5_nested;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r5_nested);
    (void)ui_css_rule_append_selector(r5_nested, UI_CSS_SELECTOR_TYPE_TAG,
                                      "div");
    r5->nested_rules = r5_nested;
    (void)ui_css_stylesheet_append_rule(sheet, r5);

    /* has(.container) */
    struct ui_css_rule *r6;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r6);
    struct ui_css_selector *has_container =
        create_mock_selector(UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS, "has");
    struct ui_css_selector *nested_container =
        create_mock_selector(UI_CSS_SELECTOR_TYPE_CLASS, ".container");
    has_container->nested_selector = nested_container;
    r6->selectors = has_container;
    (void)ui_css_stylesheet_append_rule(sheet, r6);

    (void)ui_dom_node_set_tag_name(node, "div");
    (void)ui_dom_node_set_attribute(node, "checked", "");

    struct ui_dom_node *container_child;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container_child);
    (void)ui_dom_node_set_attribute(container_child, "class", "container");
    (void)ui_dom_node_append_child(node, container_child);

    struct ui_css_computed_style *style;
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    (void)ui_dom_node_remove_attribute(node, "checked");
    (void)ui_dom_node_set_attribute(node, "aria-checked", "true");
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    (void)ui_dom_node_set_attribute(node, "aria-checked", "false");
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    (void)ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }

  {
    /* Missing edge cases */
    struct ui_dom_node *node;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    (void)ui_dom_node_set_tag_name(node, "div");

    struct ui_css_stylesheet *sheet;
    (void)ui_css_stylesheet_create(&sheet);

    struct ui_css_rule *r1;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r1);
    ui_css_rule_append_selector(r1, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "read-write");
    (void)ui_css_stylesheet_append_rule(sheet, r1);

    struct ui_css_computed_style *style;

    /* :read-write with disabled attribute */
    (void)ui_dom_node_set_attribute(node, "disabled", "");
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    /* :read-write with aria-disabled */
    (void)ui_dom_node_remove_attribute(node, "disabled");
    (void)ui_dom_node_set_attribute(node, "aria-disabled", "true");
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    (void)ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }

  {
    /* Text node matching */
    struct ui_dom_node *node;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &node);

    struct ui_css_stylesheet *sheet;
    (void)ui_css_stylesheet_create(&sheet);

    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    struct ui_css_computed_style *style;
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    struct ui_dom_node *parent;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &parent);

    struct ui_dom_node *wrapper;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &wrapper);
    (void)ui_dom_node_append_child(wrapper, node);
    (void)ui_dom_node_append_child(parent, wrapper);

    struct ui_css_rule *has_rule;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &has_rule);

    struct ui_css_selector *has_sel =
        create_mock_selector(UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS, "has");
    struct ui_css_selector *nested_div =
        create_mock_selector(UI_CSS_SELECTOR_TYPE_TAG, "div");
    has_sel->nested_selector = nested_div;
    has_rule->selectors = has_sel;

    (void)ui_css_stylesheet_append_rule(sheet, has_rule);

    (void)ui_css_resolve_style(sheet, parent, &style);
    (void)ui_css_computed_style_destroy(style);

    (void)ui_css_stylesheet_destroy(sheet);

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
    sel->nested_selector =
        create_mock_selector(UI_CSS_SELECTOR_TYPE_TAG, "span");
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
    (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_ID,
                                      "wrong-id");
    (void)ui_css_rule_append_declaration(rule, "color", "green", 0);
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    /* :has(div) with nested selector to cover 1100s branch */
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                      "has");
    sel = rule->selectors;
    sel->nested_selector =
        create_mock_selector(UI_CSS_SELECTOR_TYPE_TAG, "div");
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
      (void)ui_css_rule_append_declaration(rule, "prop_layer_important2", "A",
                                           1);
      layer_rule1->nested_rules = rule;

      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "span");
      (void)ui_css_rule_append_declaration(rule, "prop_layer_normal2", "B", 0);
      (void)ui_css_rule_append_declaration(rule, "prop_layer_important2", "B",
                                           1);
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
      (void)ui_css_rule_append_declaration(rule, "prop_layer_important2", "A",
                                           1);
      layer_rule1->nested_rules = rule;

      (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "span");
      (void)ui_css_rule_append_declaration(rule, "prop_layer_normal2", "B", 0);
      (void)ui_css_rule_append_declaration(rule, "prop_layer_important2", "B",
                                           1);
      layer_rule2->nested_rules = rule;

      (void)ui_css_stylesheet_append_rule(sheet, layer_rule1);
      (void)ui_css_stylesheet_append_rule(sheet, layer_rule2);
    }
    (void)ui_css_resolve_style(sheet, cov_child, &style);
    (void)ui_css_computed_style_destroy(style);
    (void)ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(cov_parent);
    (void)ui_dom_node_destroy(parent);
    /* node is destroyed when parent is destroyed */
  }
  {
    /* Nested parens in supports condition */
    struct ui_dom_node *node;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    struct ui_css_stylesheet *sheet;
    (void)ui_css_stylesheet_create(&sheet);

    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition = my_strdup("(not ((display: flex)))");
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    struct ui_css_computed_style *style;
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    (void)ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }
  {
    /* Reverts Author origin test */
    struct ui_dom_node *node;
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    struct ui_css_stylesheet *sheet;
    (void)ui_css_stylesheet_create(&sheet);

    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
    (void)ui_css_rule_append_declaration(rule, "color", "revert", 0);
    (void)ui_css_stylesheet_append_rule(sheet, rule);

    struct ui_css_computed_style *style;
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);

    (void)ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
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
        (void)ui_css_rule_append_declaration(cov_rule, "color", "red", 0);
        (void)ui_css_rule_append_declaration(cov_rule, "margin", "10px", 0);
        (void)ui_css_stylesheet_append_rule(cov_sheet, cov_rule);
      }

      if (ui_css_rule_create(UI_CSS_RULE_TYPE_MEDIA, &rule_media) ==
          UI_ERROR_NONE) {
        rule_media->media_condition = my_strdup("(max-width: 600px)");
        (void)ui_css_rule_append_declaration(rule_media, "padding", "5px", 0);
        (void)ui_css_rule_append_declaration(rule_media, "margin", "2px", 0);
        (void)ui_css_stylesheet_append_rule(cov_sheet, rule_media);
      }

      if (ui_css_rule_create(UI_CSS_RULE_TYPE_SCOPE, &rule_scope) ==
          UI_ERROR_NONE) {
        rule_scope->scope_start =
            create_mock_selector(UI_CSS_SELECTOR_TYPE_CLASS, ".card");
        rule_scope->scope_end =
            create_mock_selector(UI_CSS_SELECTOR_TYPE_CLASS, ".hole");
        (void)ui_css_stylesheet_append_rule(cov_sheet, rule_scope);
      }

      if (ui_css_rule_create(UI_CSS_RULE_TYPE_SCOPE, &rule_scope2) ==
          UI_ERROR_NONE) {
        rule_scope2->scope_start =
            create_mock_selector(UI_CSS_SELECTOR_TYPE_CLASS, ".container");
        rule_scope2->scope_end = NULL;
        (void)ui_css_stylesheet_append_rule(cov_sheet, rule_scope2);
      }

      if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container_node) ==
          UI_ERROR_NONE) {
        (void)ui_dom_node_set_attribute(container_node, "class", "container");
        (void)ui_css_resolve_style(cov_sheet, container_node, &container_style);
        (void)ui_css_computed_style_destroy(container_style);
        (void)ui_dom_node_destroy(container_node);
      }

      if (ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_supports) ==
          UI_ERROR_NONE) {
        rule_supports->supports_condition =
            my_strdup("not (display: grid) and (color: red) or selector(div)");
        (void)ui_css_stylesheet_append_rule(cov_sheet, rule_supports);
      }

      if (ui_css_rule_create(UI_CSS_RULE_TYPE_CONTAINER, &rule_container) ==
          UI_ERROR_NONE) {
        rule_container->container_condition = my_strdup("(min-width: 700px)");
        (void)ui_css_stylesheet_append_rule(cov_sheet, rule_container);
      }

      if (ui_css_rule_create(UI_CSS_RULE_TYPE_PROPERTY, &rule_property) ==
          UI_ERROR_NONE) {
        rule_property->property_name = my_strdup("--my-prop");
        rule_property->property_syntax = my_strdup("<color>");
        rule_property->property_initial_value = my_strdup("red");
        (void)ui_css_stylesheet_append_rule(cov_sheet, rule_property);
      }

      if (ui_css_rule_create(UI_CSS_RULE_TYPE_LAYER, &rule_layer) ==
          UI_ERROR_NONE) {
        rule_layer->layer_name = my_strdup("theme");
        (void)ui_css_stylesheet_append_rule(cov_sheet, rule_layer);
      }

      (void)ui_css_stylesheet_register_layer(cov_sheet, "L2", &order2);

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
        (void)ui_css_resolve_style(cov_sheet, oom_node, &oom_style);
        if (oom_style) {
          ui_css_computed_style_destroy(oom_style);
        }
        ui_dom_node_destroy(oom_node);
      }

      (void)ui_css_stylesheet_destroy(cov_sheet);
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

  /* Test @scope without scope_start on a child node */
  {
    struct ui_css_stylesheet *sheet_tmp = NULL;
    ui_css_stylesheet_create(&sheet_tmp);
    struct ui_css_rule *r5_tmp;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SCOPE, &r5_tmp);
    r5_tmp->scope_start = NULL;
    r5_tmp->scope_end = NULL;
    struct ui_css_rule *r5_nested_tmp;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r5_nested_tmp);
    ui_css_rule_append_selector(r5_nested_tmp, UI_CSS_SELECTOR_TYPE_TAG, "div");
    r5_tmp->nested_rules = r5_nested_tmp;
    ui_css_stylesheet_append_rule(sheet_tmp, r5_tmp);

    struct ui_dom_node *parent_tmp = NULL;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &parent_tmp);
    struct ui_dom_node *child_tmp = NULL;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child_tmp);
    ui_dom_node_append_child(parent_tmp, child_tmp);
    ui_dom_node_set_tag_name(child_tmp, "div");

    struct ui_css_computed_style *style_tmp;
    ui_css_resolve_style(sheet_tmp, child_tmp, &style_tmp);

    ui_css_computed_style_destroy(style_tmp);
    ui_dom_node_destroy(parent_tmp);
    ui_css_stylesheet_destroy(sheet_tmp);
  }

  /* Test @scope with start and end, plus other edge cases */
  {
    struct ui_css_stylesheet *s = NULL;
    ui_css_stylesheet_create(&s);

    /* 1. @scope (div) to (span) */
    struct ui_css_rule *scope_r;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SCOPE, &scope_r);

    struct ui_css_selector *s_start = calloc(1, sizeof(*s_start));
    s_start->type = UI_CSS_SELECTOR_TYPE_TAG;
    s_start->value = my_strdup("div");
    scope_r->scope_start = s_start;

    struct ui_css_selector *s_end = calloc(1, sizeof(*s_end));
    s_end->type = UI_CSS_SELECTOR_TYPE_TAG;
    s_end->value = my_strdup("span");
    scope_r->scope_end = s_end;

    struct ui_css_rule *scope_inner;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &scope_inner);
    ui_css_rule_append_selector(scope_inner, UI_CSS_SELECTOR_TYPE_CLASS,
                                "target");
    ui_css_rule_append_declaration(scope_inner, "color", "blue", 0);
    scope_r->nested_rules = scope_inner;
    ui_css_stylesheet_append_rule(s, scope_r);

    /* 2. :where specificity + 7. Nested layer mismatch */
    ui_css_stylesheet_register_layer(s, "layer1", NULL);
    ui_css_stylesheet_register_layer(s, "layer2", NULL);

    struct ui_css_rule *where_r;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &where_r);
    where_r->layer_name = my_strdup("layer2"); /* to test layer iteration */

    struct ui_css_selector *w_sel = calloc(1, sizeof(*w_sel));
    w_sel->type = UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS;
    w_sel->value = my_strdup("where");
    struct ui_css_selector *w_inner = calloc(1, sizeof(*w_inner));
    w_inner->type = UI_CSS_SELECTOR_TYPE_TAG;
    w_inner->value = my_strdup("article");
    w_sel->nested_selector = w_inner;

    where_r->selectors = w_sel;
    ui_css_rule_append_declaration(where_r, "font-size", "12px", 0);

    /* 6. STYLE rule with nested rules */
    struct ui_css_rule *nested_style;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_style);
    ui_css_rule_append_selector(nested_style, UI_CSS_SELECTOR_TYPE_TAG, "p");
    ui_css_rule_append_declaration(nested_style, "margin", "revert",
                                   0); /* 4. 'revert' value */
    where_r->nested_rules = nested_style;

    ui_css_stylesheet_append_rule(s, where_r);

    /* 3. ID selector on node without ID */
    struct ui_css_rule *id_r;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &id_r);
    ui_css_rule_append_selector(id_r, UI_CSS_SELECTOR_TYPE_ID,
                                "does-not-exist");
    ui_css_stylesheet_append_rule(s, id_r);

    /* 5. :checked testing */
    struct ui_css_rule *checked_r;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &checked_r);
    struct ui_css_selector *chk_sel = calloc(1, sizeof(*chk_sel));
    chk_sel->type = UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS;
    chk_sel->value = my_strdup("checked");
    checked_r->selectors = chk_sel;
    ui_css_rule_append_declaration(checked_r, "border", "1px", 0);
    ui_css_stylesheet_append_rule(s, checked_r);

    /* 8. Universal selector * */
    struct ui_css_rule *univ_r;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &univ_r);
    ui_css_rule_append_selector(univ_r, UI_CSS_SELECTOR_TYPE_UNIVERSAL, "*");
    ui_css_rule_append_declaration(univ_r, "font-weight", "bold", 0);
    ui_css_stylesheet_append_rule(s, univ_r);

    /* 9. ancestor_matched test for nested rules */
    struct ui_css_rule *ancestor_r;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &ancestor_r);
    ui_css_rule_append_selector(ancestor_r, UI_CSS_SELECTOR_TYPE_TAG,
                                "article");
    ui_css_rule_append_selector(ancestor_r, UI_CSS_SELECTOR_TYPE_TAG, "div");

    struct ui_css_rule *ancestor_nested_r;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &ancestor_nested_r);
    ui_css_rule_append_selector(ancestor_nested_r, UI_CSS_SELECTOR_TYPE_TAG,
                                "p");
    ui_css_rule_append_declaration(ancestor_nested_r, "font-style", "italic",
                                   0);
    ancestor_r->nested_rules = ancestor_nested_r;
    ui_css_stylesheet_append_rule(s, ancestor_r);

    /* Build DOM */
    struct ui_dom_node *n_div, *n_article, *n_span, *n_p;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n_div);
    ui_dom_node_set_tag_name(n_div, "div");

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n_article);
    ui_dom_node_set_tag_name(n_article, "article");
    ui_dom_node_set_attribute(n_article, "class", "target\tother\r\nclass");
    ui_dom_node_set_attribute(n_article, "aria-checked", "true");

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n_span);
    ui_dom_node_set_tag_name(n_span, "span");
    ui_dom_node_set_attribute(n_span, "class", "target");
    ui_dom_node_set_attribute(n_span, "aria-checked", "false");
    ui_dom_node_set_attribute(n_span, "checked",
                              ""); /* for the 'checked' direct attr branch */

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n_p);
    ui_dom_node_set_tag_name(n_p, "p");

    ui_dom_node_append_child(n_div, n_article);
    ui_dom_node_append_child(n_article, n_span);
    ui_dom_node_append_child(n_span, n_p);

    struct ui_css_computed_style *style_art, *style_span, *style_p;
    ui_css_resolve_style(s, n_article, &style_art);
    ui_css_resolve_style(s, n_span, &style_span);
    ui_css_resolve_style(s, n_p, &style_p);

    const char *non_exist_val = NULL;
    ui_css_computed_style_get_property(style_p, "non-existent-prop",
                                       &non_exist_val);

    ui_css_computed_style_destroy(style_art);
    ui_css_computed_style_destroy(style_span);
    ui_css_computed_style_destroy(style_p);

    ui_dom_node_destroy(n_div);
    ui_css_stylesheet_destroy(s);
  }
  printf("test_ui_cssom passed.\n");
  {
    /* Test whitespace characters in cond_skip_ws and cond_is_word */
    struct ui_css_rule *rule = NULL;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        C_MULTIPLATFORM_STRDUP("\t\r\n not \t\r\n (\n\r\tdisplay: flex)");
    struct ui_css_stylesheet *sheet = NULL;
    ui_css_stylesheet_create(&sheet);
    ui_css_stylesheet_append_rule(sheet, rule);

    struct ui_dom_node *node = NULL;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    struct ui_css_computed_style *c_style = NULL;
    ui_css_resolve_style(sheet, node, &c_style);

    ui_css_computed_style_destroy(c_style);
    ui_dom_node_destroy(node);
    ui_css_stylesheet_destroy(sheet);
  }
  {
    /* Test cond_is_word with partial match (coverage line 1298) */
    struct ui_css_rule *rule = NULL;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition = C_MULTIPLATFORM_STRDUP("notx (width)");
    struct ui_css_stylesheet *sheet = NULL;
    ui_css_stylesheet_create(&sheet);
    ui_css_stylesheet_append_rule(sheet, rule);

    struct ui_dom_node *node = NULL;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    struct ui_css_computed_style *c_style = NULL;
    ui_css_resolve_style(sheet, node, &c_style);

    ui_css_computed_style_destroy(c_style);
    ui_dom_node_destroy(node);
    ui_css_stylesheet_destroy(sheet);
  }
  {
    /* Test property value "revert" (coverage line 1633) */
    struct ui_css_stylesheet *sheet = NULL;
    ui_css_stylesheet_create(&sheet);
    struct ui_css_rule *rule = NULL;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
    ui_css_rule_append_declaration(rule, "color", "revert", 0);
    ui_css_stylesheet_append_rule(sheet, rule);

    struct ui_dom_node *node = NULL;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    ui_dom_node_set_tag_name(node, "div");

    struct ui_css_computed_style *c_style = NULL;
    ui_css_resolve_style(sheet, node, &c_style);

    const char *val = NULL;
    ui_css_computed_style_get_property(c_style, "color", &val);

    ui_css_computed_style_destroy(c_style);
    ui_dom_node_destroy(node);
    ui_css_stylesheet_destroy(sheet);
  }
  {
    /* Test setting multiple variables (coverage line 1724) */
    struct ui_css_variable_store *store = NULL;
    ui_css_variable_store_create(&store);
    ui_css_variable_store_set(store, "--var1", "1");
    ui_css_variable_store_set(store, "--var2", "2");
    ui_css_variable_store_set(store, "--var2", "3");
    ui_css_variable_store_destroy(store);
  }
  return 0;
}
