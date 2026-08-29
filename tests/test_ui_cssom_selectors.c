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
    UI_STRCPY(d, 256, s);
  return d;
}

int test_cssom_part1_selectors(void) {
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

  struct ui_css_variable_store *store = NULL;
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
  return 0;
}
