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
    strcpy(d, s);
  return d;
}

int test_cssom_part6_memory(void) {
  struct ui_css_stylesheet *sheet = NULL;
  (void)sheet;
  struct ui_css_rule *rule = NULL, *rule2 = NULL, *rule3 = NULL;
  (void)rule;
  (void)rule2;
  (void)rule3;
  struct ui_dom_node *node = NULL, *child = NULL, *grandchild = NULL;
  (void)node;
  (void)child;
  (void)grandchild;
  ui_error_t rc = UI_ERROR_NONE;
  (void)rc;
  struct ui_css_computed_style *style = NULL;
  (void)style;
  struct ui_css_computed_style *c_style = NULL;
  (void)c_style;
  int order = 0;
  (void)order;
  struct ui_css_selector *sel = NULL;
  (void)sel;

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
  return 0;
}
