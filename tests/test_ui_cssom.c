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
  ui_css_stylesheet_destroy(NULL);

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
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_anon_layer);
  ui_css_rule_append_selector(nested_anon_layer, UI_CSS_SELECTOR_TYPE_TAG,
                              "div");
  ui_css_rule_append_declaration(nested_anon_layer, "color", "green", 0);
  anon_layer->nested_rules = nested_anon_layer;

  /* Nested rule inside LAYER */
  struct ui_css_rule *nested_layer;
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_layer);
  ui_css_rule_append_selector(nested_layer, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_rule_append_declaration(nested_layer, "color", "red", 0);
  rule3->nested_rules = nested_layer;

  /* MEDIA rule */
  struct ui_css_rule *media_rule;
  ui_css_rule_create(UI_CSS_RULE_TYPE_MEDIA, &media_rule);
  media_rule->media_condition = my_strdup("screen and (min-width: 900px)");
  ui_css_stylesheet_append_rule(sheet, media_rule);
  struct ui_css_rule *nested_media;
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_media);
  ui_css_rule_append_selector(nested_media, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_rule_append_declaration(nested_media, "margin", "10px", 0);
  media_rule->nested_rules = nested_media;

  /* SUPPORTS rule */
  struct ui_css_rule *supp_rule;
  ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &supp_rule);
  supp_rule->supports_condition = my_strdup("display: flex");
  ui_css_stylesheet_append_rule(sheet, supp_rule);
  struct ui_css_rule *nested_supp;
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_supp);
  ui_css_rule_append_selector(nested_supp, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_rule_append_declaration(nested_supp, "display", "flex", 0);
  supp_rule->nested_rules = nested_supp;

  /* CONTAINER rule */
  struct ui_css_rule *cont_rule;
  ui_css_rule_create(UI_CSS_RULE_TYPE_CONTAINER, &cont_rule);
  cont_rule->container_condition = my_strdup("(min-width: 500px)");
  ui_css_stylesheet_append_rule(sheet, cont_rule);
  struct ui_css_rule *nested_cont;
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_cont);
  ui_css_rule_append_selector(nested_cont, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_rule_append_declaration(nested_cont, "width", "100%", 0);
  cont_rule->nested_rules = nested_cont;

  /* SCOPE rule */
  struct ui_css_rule *scope_rule;
  ui_css_rule_create(UI_CSS_RULE_TYPE_SCOPE, &scope_rule);
  scope_rule->scope_start = my_strdup(".card");
  scope_rule->scope_end = my_strdup(".hole");
  ui_css_stylesheet_append_rule(sheet, scope_rule);
  struct ui_css_rule *nested_scope;
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_scope);
  ui_css_rule_append_selector(nested_scope, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_rule_append_declaration(nested_scope, "background", "blue", 0);
  scope_rule->nested_rules = nested_scope;

  /* PROPERTY rule */
  struct ui_css_rule *prop_rule;
  ui_css_rule_create(UI_CSS_RULE_TYPE_PROPERTY, &prop_rule);
  prop_rule->property_name = my_strdup("--my-var");
  prop_rule->property_syntax = my_strdup("<color>");
  prop_rule->property_initial_value = my_strdup("red");
  ui_css_stylesheet_append_rule(sheet, prop_rule);

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
  TEST_ASSERT(ui_css_rule_append_selector_attr(NULL, NULL,
                                               UI_CSS_ATTR_OP_EQUALS, "text") ==
              UI_ERROR_INVALID_ARGUMENT);

  rc = ui_css_rule_append_selector_attr(rule, "data-val", UI_CSS_ATTR_OP_NONE,
                                        NULL);
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
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  ui_dom_node_set_tag_name(node, "div");
  ui_dom_node_set_attribute(node, "id", "main");
  ui_dom_node_set_attribute(node, "class", "btn active card");
  ui_dom_node_set_attribute(node, "data-val", "yes");
  ui_dom_node_set_attribute(node, "type", "text");
  ui_dom_node_set_attribute(node, "lang", "en-US");
  ui_dom_node_set_attribute(node, "href", "https://example.pdf");
  ui_dom_node_set_attribute(node, "title", "say hello there");

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
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &pr);
    ui_css_rule_append_selector(pr, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                pseudos[i]);
    ui_css_rule_append_declaration(pr, pseudos[i], "true", 0);
    ui_css_stylesheet_append_rule(sheet, pr);
  }

  /* Structural/State pseudo matches */
  node->state_flags |=
      UI_DOM_NODE_STATE_HOVER | UI_DOM_NODE_STATE_ACTIVE |
      UI_DOM_NODE_STATE_FOCUS | UI_DOM_NODE_STATE_FOCUS_VISIBLE |
      UI_DOM_NODE_STATE_FOCUS_WITHIN | UI_DOM_NODE_STATE_TARGET |
      UI_DOM_NODE_STATE_TARGET_WITHIN;
  ui_dom_node_set_attribute(node, "checked", "");
  ui_dom_node_set_attribute(node, "disabled", "");
  ui_dom_node_set_attribute(node, "required", "");
  ui_dom_node_set_attribute(node, "readonly", "");
  ui_dom_node_set_attribute(node, "indeterminate", "");
  ui_dom_node_set_attribute(node, "default", "");
  ui_dom_node_set_attribute(node, "aria-invalid", "true");

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

  ui_css_computed_style_destroy(style);

  /* Children and ancestry logic */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child);
  ui_dom_node_set_tag_name(child, "span");
  ui_dom_node_append_child(node, child);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &grandchild);
  ui_dom_node_set_tag_name(grandchild, "p");
  ui_dom_node_append_child(child, grandchild);

  /* Add :has and :is, :where, :not */
  struct ui_css_rule *func_rule;
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &func_rule);

  struct ui_css_selector *is_sel = calloc(1, sizeof(*is_sel));
  is_sel->type = UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS;
  is_sel->value = my_strdup("is");
  struct ui_css_selector *nested1 = calloc(1, sizeof(*nested1));
  nested1->type = UI_CSS_SELECTOR_TYPE_TAG;
  nested1->value = my_strdup("div");
  is_sel->nested_selector = nested1;
  func_rule->selectors = is_sel;
  ui_css_rule_append_declaration(func_rule, "is-matched", "yes", 0);
  ui_css_stylesheet_append_rule(sheet, func_rule);

  struct ui_css_rule *has_rule;
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &has_rule);
  struct ui_css_selector *has_sel = calloc(1, sizeof(*has_sel));
  has_sel->type = UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS;
  has_sel->value = my_strdup("has");
  struct ui_css_selector *nested2 = calloc(1, sizeof(*nested2));
  nested2->type = UI_CSS_SELECTOR_TYPE_TAG;
  nested2->value = my_strdup("p");
  has_sel->nested_selector = nested2;
  has_rule->selectors = has_sel;
  ui_css_rule_append_declaration(has_rule, "has-matched", "yes", 0);
  ui_css_stylesheet_append_rule(sheet, has_rule);

  rc = ui_css_resolve_style(sheet, node, &style);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_computed_style_get_property(style, "is-matched", &val);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_computed_style_get_property(style, "has-matched", &val);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  ui_css_computed_style_destroy(style);

  /* CSS Variables */
  TEST_ASSERT(ui_css_variable_store_create(NULL) == UI_ERROR_INVALID_ARGUMENT);
  ui_css_variable_store_destroy(NULL);
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

  ui_css_variable_store_destroy(store);
  ui_dom_node_destroy(node); /* Recursively frees child, grandchild */

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }

  {
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) and"); /* missing end term */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }

  {
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) or"); /* missing end term */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }
  {
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("(not (display: flex)"); /* missing inner end */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }
  {
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) or )"); /* trailing paren */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;

    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    ui_css_stylesheet_create(&tmp_sheet);

    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    ui_css_stylesheet_append_rule(tmp_sheet, rule);

    ui_css_resolve_style(tmp_sheet, node, &tmp_style);

    ui_css_stylesheet_destroy(tmp_sheet);
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    ui_css_stylesheet_create(&tmp_sheet);
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) and"); /* missing end term */
    ui_css_stylesheet_append_rule(tmp_sheet, rule);

    ui_css_resolve_style(tmp_sheet, node, &tmp_style);
    ui_css_stylesheet_destroy(tmp_sheet);
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    ui_css_stylesheet_create(&tmp_sheet);
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) or"); /* missing end term */
    ui_css_stylesheet_append_rule(tmp_sheet, rule);

    ui_css_resolve_style(tmp_sheet, node, &tmp_style);
    ui_css_stylesheet_destroy(tmp_sheet);
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }

  ui_css_stylesheet_destroy(sheet);

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
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);

  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS,
                                          "a") == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  TEST_ASSERT(ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS,
                                          "a") == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_rule_append_selector_attr(rule, "a", UI_CSS_ATTR_OP_EQUALS,
                                               "b") == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  TEST_ASSERT(ui_css_rule_append_selector_attr(rule, "a", UI_CSS_ATTR_OP_EQUALS,
                                               "b") == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 2;
  TEST_ASSERT(ui_css_rule_append_selector_attr(rule, "a", UI_CSS_ATTR_OP_EQUALS,
                                               "b") == UI_ERROR_OUT_OF_MEMORY);
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

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  ui_css_stylesheet_append_rule(sheet, rule);
  ui_css_rule_append_declaration(rule, "a", "b", 0);

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
  ui_css_computed_style_destroy(style);
  g_malloc_fail_countdown = -1;

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }

  {
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) and"); /* missing end term */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }

  {
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) or"); /* missing end term */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }
  {
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("(not (display: flex)"); /* missing inner end */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }
  {
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) or )"); /* trailing paren */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;

    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    ui_css_stylesheet_create(&tmp_sheet);

    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    ui_css_stylesheet_append_rule(tmp_sheet, rule);

    ui_css_resolve_style(tmp_sheet, node, &tmp_style);

    ui_css_stylesheet_destroy(tmp_sheet);
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    ui_css_stylesheet_create(&tmp_sheet);
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) and"); /* missing end term */
    ui_css_stylesheet_append_rule(tmp_sheet, rule);

    ui_css_resolve_style(tmp_sheet, node, &tmp_style);
    ui_css_stylesheet_destroy(tmp_sheet);
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    ui_css_stylesheet_create(&tmp_sheet);
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) or"); /* missing end term */
    ui_css_stylesheet_append_rule(tmp_sheet, rule);

    ui_css_resolve_style(tmp_sheet, node, &tmp_style);
    ui_css_stylesheet_destroy(tmp_sheet);
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }

  ui_css_stylesheet_destroy(sheet);
  (void)ui_dom_node_destroy(node);

  /* Eval condition paths */
  /* These are static functions but reached via supports/media rules */
  rc = ui_css_stylesheet_create(&sheet);
  struct ui_css_rule *r;
  ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &r);
  struct ui_css_rule *nested_r;
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_r);
  ui_css_rule_append_selector(nested_r, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_rule_append_declaration(nested_r, "display", "flex", 0);
  r->nested_rules = nested_r;
  ui_css_stylesheet_append_rule(sheet, r);

  struct ui_css_rule *media_r;
  ui_css_rule_create(UI_CSS_RULE_TYPE_MEDIA, &media_r);
  media_r->media_condition = my_strdup("screen");
  struct ui_css_rule *nested_mr;
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_mr);
  ui_css_rule_append_selector(nested_mr, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_rule_append_declaration(nested_mr, "color", "red", 0);
  media_r->nested_rules = nested_mr;
  ui_css_stylesheet_append_rule(sheet, media_r);

  struct ui_css_rule *cont_r;
  ui_css_rule_create(UI_CSS_RULE_TYPE_CONTAINER, &cont_r);
  cont_r->container_condition = my_strdup("(min-width: 100px)");
  struct ui_css_rule *nested_cr;
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_cr);
  ui_css_rule_append_selector(nested_cr, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_rule_append_declaration(nested_cr, "color", "blue", 0);
  cont_r->nested_rules = nested_cr;
  ui_css_stylesheet_append_rule(sheet, cont_r);

  struct ui_css_rule *scope_r;
  ui_css_rule_create(UI_CSS_RULE_TYPE_SCOPE, &scope_r);
  scope_r->scope_start = my_strdup(".card");
  struct ui_css_rule *nested_scope_r;
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_scope_r);
  ui_css_rule_append_selector(nested_scope_r, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_rule_append_declaration(nested_scope_r, "background", "yellow", 0);
  scope_r->nested_rules = nested_scope_r;
  ui_css_stylesheet_append_rule(sheet, scope_r);

  struct ui_css_rule *style_r;
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &style_r);
  ui_css_rule_append_selector(style_r, UI_CSS_SELECTOR_TYPE_TAG, "div");
  struct ui_css_rule *nested_sr;
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_sr);
  ui_css_rule_append_selector(nested_sr, UI_CSS_SELECTOR_TYPE_CLASS, "nested");
  ui_css_rule_append_declaration(nested_sr, "margin", "0", 0);
  style_r->nested_rules = nested_sr;
  ui_css_stylesheet_append_rule(sheet, style_r);

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
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);
    (void)ui_dom_node_destroy(node);
  }

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  ui_dom_node_set_tag_name(node, "div");
  ui_dom_node_set_attribute(node, "class", "nested");
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
  ui_css_computed_style_destroy(style);
  g_malloc_fail_countdown = -1;
  (void)ui_dom_node_destroy(node);

  /* Check supports evaluation failure */
  if (r->supports_condition) {
    C_MULTIPLATFORM_FREE(r->supports_condition);
  }
  r->supports_condition = my_strdup("(a) and"); /* parse error */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  rc = ui_css_resolve_style(sheet, node, &style);
  if (rc != UI_ERROR_PARSE_FAILED) {
    printf("Expected PARSE_FAILED, got %d\n", rc);
    TEST_ASSERT(rc == UI_ERROR_PARSE_FAILED);
  }
  (void)ui_dom_node_destroy(node);

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }

  {
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) and"); /* missing end term */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }

  {
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) or"); /* missing end term */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }
  {
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("(not (display: flex)"); /* missing inner end */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }
  {
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) or )"); /* trailing paren */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;

    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    ui_css_stylesheet_create(&tmp_sheet);

    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    ui_css_stylesheet_append_rule(tmp_sheet, rule);

    ui_css_resolve_style(tmp_sheet, node, &tmp_style);

    ui_css_stylesheet_destroy(tmp_sheet);
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    ui_css_stylesheet_create(&tmp_sheet);
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) and"); /* missing end term */
    ui_css_stylesheet_append_rule(tmp_sheet, rule);

    ui_css_resolve_style(tmp_sheet, node, &tmp_style);
    ui_css_stylesheet_destroy(tmp_sheet);
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    ui_css_stylesheet_create(&tmp_sheet);
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex) or"); /* missing end term */
    ui_css_stylesheet_append_rule(tmp_sheet, rule);

    ui_css_resolve_style(tmp_sheet, node, &tmp_style);
    ui_css_stylesheet_destroy(tmp_sheet);
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    ui_css_stylesheet_append_rule(sheet, rule);

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      ui_css_computed_style_destroy(style);
    }
  }

  ui_css_stylesheet_destroy(sheet);

  {
    int match = 0;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);

    ui_css_rule_append_selector_attr(rule, "href", UI_CSS_ATTR_OP_EQUALS,
                                     "link");

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    /* Cover class_list_contains missing branches */
    ui_dom_node_set_attribute(node, "class", "  btn   \t\n");
    ui_css_rule_append_selector_attr(rule, "class", UI_CSS_ATTR_OP_INCLUDES,
                                     "btn");

    /* Cover strings */
    ui_dom_node_set_attribute(node, "data-test", "val");
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_PREFIX,
                                     "value");
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_SUFFIX,
                                     "value");

    struct ui_css_stylesheet *sheet;
    ui_css_stylesheet_create(&sheet);
    ui_css_stylesheet_append_rule(sheet, rule);

    struct ui_css_computed_style *style;
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;

      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);

      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);

      ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      ui_css_stylesheet_destroy(tmp_sheet);
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }

  {
    /* Mismatches and default operators */
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    ui_css_rule_append_selector_attr(rule, "data-test", 99, "value");
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_PREFIX,
                                     "somethingverylong");
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_SUFFIX,
                                     "somethingverylong");
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_INCLUDES,
                                     "v");
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_DASH,
                                     "somethingverylong");

    struct ui_dom_node *node;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    ui_dom_node_set_attribute(node, "data-test", "val");

    struct ui_css_stylesheet *sheet;
    ui_css_stylesheet_create(&sheet);
    ui_css_stylesheet_append_rule(sheet, rule);

    struct ui_css_computed_style *style;
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;

      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);

      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);

      ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      ui_css_stylesheet_destroy(tmp_sheet);
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }

  {
    /* Combinator checks */
    struct ui_dom_node *node;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    ui_dom_node_set_attribute(node, "class", "card");

    struct ui_dom_node *child1;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
    ui_dom_node_set_attribute(child1, "class", "container");
    ui_dom_node_append_child(node, child1);

    struct ui_dom_node *child2;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child2);
    ui_dom_node_set_attribute(child2, "class", "hole");
    ui_dom_node_append_child(child1, child2);

    /* Child matching */
    int m = 0;

    /* Cover :has */
    struct ui_css_rule *rule_has;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_has);
    ui_css_rule_append_selector(rule_has, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "has(.hole)");

    /* Scope checking logic */
    struct ui_css_rule *rule_scope;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_scope);
    ui_css_rule_append_selector(rule_scope, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "scope(.card, .container)");

    struct ui_css_stylesheet *sheet;
    ui_css_stylesheet_create(&sheet);
    ui_css_stylesheet_append_rule(sheet, rule_has);
    ui_css_stylesheet_append_rule(sheet, rule_scope);

    struct ui_css_computed_style *style;
    ui_css_resolve_style(sheet, child1, &style);
    ui_css_computed_style_destroy(style);

    ui_css_resolve_style(sheet, child2, &style);
    ui_css_computed_style_destroy(style);

    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;

      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);

      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);

      ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      ui_css_stylesheet_destroy(tmp_sheet);
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }

  {
    /* Combinator checks */
    struct ui_dom_node *node;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    ui_dom_node_set_attribute(node, "class", "card container hole");

    struct ui_css_rule *rule_has;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_has);

    /* Child matching coverage */
    struct ui_dom_node *child1;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
    ui_dom_node_set_attribute(child1, "class", "container");
    ui_dom_node_append_child(node, child1);

    struct ui_css_selector *sel1;
    /* We can't directly manipulate selector easily so we just parse or set it
     */

    ui_css_rule_destroy(rule_has);
    (void)ui_dom_node_destroy(node);
  }

  {
    /* Combinator checks for child matching */
    struct ui_dom_node *node;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    struct ui_css_rule *rule_has;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_has);

    /* check_mock_string_selector edge cases */
    struct ui_css_stylesheet *sheet;
    ui_css_stylesheet_create(&sheet);
    ui_css_rule_append_selector(rule_has, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "has");
    ui_css_stylesheet_append_rule(sheet, rule_has);

    struct ui_css_computed_style *style;
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;

      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);

      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);

      ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      ui_css_stylesheet_destroy(tmp_sheet);
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }

  {
    /* Combinator checks for child matching and scope boundary */
    struct ui_dom_node *node;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    struct ui_css_rule *rule_has;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_has);

    /* check_mock_string_selector edge cases */
    struct ui_css_stylesheet *sheet;
    ui_css_stylesheet_create(&sheet);
    ui_css_rule_append_selector(rule_has, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "has(.card)");
    ui_css_stylesheet_append_rule(sheet, rule_has);

    struct ui_css_rule *rule_scope;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_scope);
    ui_css_rule_append_selector(rule_scope, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "scope(.card, .hole)");
    ui_css_stylesheet_append_rule(sheet, rule_scope);

    struct ui_css_rule *rule_scope2;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_scope2);
    ui_css_rule_append_selector(rule_scope2, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "scope(NULL, .hole)");
    ui_css_stylesheet_append_rule(sheet, rule_scope2);

    struct ui_css_computed_style *style;
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    ui_dom_node_set_attribute(node, "class", "card");
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    ui_dom_node_set_attribute(node, "class", "container");
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    ui_dom_node_set_attribute(node, "class", "hole");
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;

      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);

      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);

      ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      ui_css_stylesheet_destroy(tmp_sheet);
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }

  {
    /* Mismatches and default operators */
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_INCLUDES,
                                     "v");

    struct ui_dom_node *node;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    ui_dom_node_set_attribute(node, "data-test", "");

    struct ui_css_stylesheet *sheet;
    ui_css_stylesheet_create(&sheet);
    ui_css_stylesheet_append_rule(sheet, rule);

    struct ui_css_computed_style *style;
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    /* Cover missing internal mock string cases */
    struct ui_css_rule *rule_mock;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_mock);
    ui_css_rule_append_selector(rule_mock, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "has(.unknown)");
    ui_css_stylesheet_append_rule(sheet, rule_mock);
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;

      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);

      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);

      ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      ui_css_stylesheet_destroy(tmp_sheet);
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }

  {
    /* Mismatches and default operators */
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_INCLUDES,
                                     "v");
    ui_css_rule_append_selector_attr(rule, "data-test2",
                                     UI_CSS_ATTR_OP_INCLUDES, "v");

    struct ui_dom_node *node;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    ui_dom_node_set_attribute(node, "data-test", "");
    ui_dom_node_set_attribute(node, "data-test2", "v something");

    struct ui_css_stylesheet *sheet;
    ui_css_stylesheet_create(&sheet);
    ui_css_stylesheet_append_rule(sheet, rule);

    struct ui_css_computed_style *style;
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    /* Cover missing internal mock string cases */
    struct ui_css_rule *rule_mock;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_mock);
    ui_css_rule_append_selector(rule_mock, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "has(.unknown)");
    ui_css_stylesheet_append_rule(sheet, rule_mock);
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;

      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);

      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);

      ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      ui_css_stylesheet_destroy(tmp_sheet);
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }

  {
    /* Mismatches and default operators */
    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);

    struct ui_css_stylesheet *sheet;
    ui_css_stylesheet_create(&sheet);
    ui_css_stylesheet_append_rule(sheet, rule);

    struct ui_css_computed_style *style;

    struct ui_dom_node *node;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    /* Child matching coverage */
    struct ui_dom_node *child1;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
    ui_dom_node_set_attribute(child1, "class", "hole");
    ui_dom_node_append_child(node, child1);

    struct ui_css_rule *rule_has;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_has);
    ui_css_rule_append_selector(rule_has, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "has(.hole)");
    ui_css_stylesheet_append_rule(sheet, rule_has);
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }
    {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;

      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);

      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);

      ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      ui_css_stylesheet_destroy(tmp_sheet);
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      ui_css_stylesheet_create(&tmp_sheet);
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      ui_css_stylesheet_append_rule(tmp_sheet, rule);

      ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      ui_css_stylesheet_destroy(tmp_sheet);
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      ui_css_stylesheet_append_rule(sheet, rule);

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        ui_css_computed_style_destroy(style);
      }
    }

    ui_css_stylesheet_destroy(sheet);
    (void)ui_dom_node_destroy(node);
  }

  {
    struct ui_css_stylesheet *sheet;
    ui_css_stylesheet_create(&sheet);
    int order1, order2, order3;
    ui_css_stylesheet_register_layer(sheet, "L1", &order1);
    ui_css_stylesheet_register_layer(sheet, "L2", &order2);
    ui_css_stylesheet_register_layer(sheet, "L3", &order3);
    ui_css_stylesheet_destroy(sheet);
  }
  {
    /* Missing pseudo classes */
    struct ui_dom_node *node;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    const char *pseudos[] = {"disabled",      "enabled",   "required",
                             "optional",      "read-only", "read-write",
                             "indeterminate", "default",   "invalid",
                             "valid",         "unknown"};

    struct ui_css_stylesheet *sheet;
    ui_css_stylesheet_create(&sheet);

    for (int i = 0; i < 11; ++i) {
      struct ui_css_rule *rule;
      ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                  pseudos[i]);
      ui_css_stylesheet_append_rule(sheet, rule);
    }

    /* Empty rule without selector */
    struct ui_css_rule *empty_rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &empty_rule);
    ui_css_stylesheet_append_rule(sheet, empty_rule);

    struct ui_css_rule *attr_rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &attr_rule);
    ui_css_rule_append_selector_attr(attr_rule, "lang", UI_CSS_ATTR_OP_DASH,
                                     "en");
    ui_css_stylesheet_append_rule(sheet, attr_rule);

    /* Test state 1: no attributes */
    struct ui_css_computed_style *style;
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    /* Test dash mismatch */
    ui_dom_node_set_attribute(node, "lang", "es");
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);
    ui_dom_node_remove_attribute(node, "lang");
    ui_dom_node_set_attribute(node, "lang", "eng");
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    /* Test state 2: With native attributes */
    ui_dom_node_set_attribute(node, "disabled", "");
    ui_dom_node_set_attribute(node, "required", "");
    ui_dom_node_set_attribute(node, "readonly", "");
    ui_dom_node_set_attribute(node, "indeterminate", "");
    ui_dom_node_set_attribute(node, "default", "");
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    /* Test state 3: With aria attributes (true) */
    ui_dom_node_remove_attribute(node, "disabled");
    ui_dom_node_remove_attribute(node, "required");
    ui_dom_node_remove_attribute(node, "readonly");
    ui_dom_node_remove_attribute(node, "indeterminate");
    ui_dom_node_remove_attribute(node, "default");

    ui_dom_node_set_attribute(node, "aria-disabled", "true");
    ui_dom_node_set_attribute(node, "aria-required", "true");
    ui_dom_node_set_attribute(node, "aria-readonly", "true");
    ui_dom_node_set_attribute(node, "aria-checked", "true");
    ui_dom_node_set_attribute(node, "aria-invalid", "true");
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    /* Test state 4: with aria attributes (false) */
    ui_dom_node_set_attribute(node, "aria-disabled", "false");
    ui_dom_node_set_attribute(node, "aria-required", "false");
    ui_dom_node_set_attribute(node, "aria-readonly", "false");
    ui_dom_node_set_attribute(node, "aria-checked", "false");
    ui_dom_node_set_attribute(node, "aria-invalid", "false");
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(node);
  }
  {
    /* Important rules overriding */
    struct ui_dom_node *node;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    struct ui_css_stylesheet *sheet;
    ui_css_stylesheet_create(&sheet);

    int order1, order2;
    ui_css_stylesheet_register_layer(sheet, "L1", &order1);
    ui_css_stylesheet_register_layer(sheet, "L2", &order2);

    struct ui_css_rule *rule1, *rule2, *rule3, *rule4;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule1);
    ui_css_rule_append_selector(rule1, UI_CSS_SELECTOR_TYPE_TAG, "div");
    ui_css_rule_append_declaration(rule1, "color", "red", 0);
    ui_css_rule_append_declaration(rule1, "background", "red", 1);

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule2);
    ui_css_rule_append_selector(rule2, UI_CSS_SELECTOR_TYPE_TAG, "div");
    ui_css_rule_append_declaration(rule2, "color", "blue",
                                   1); /* overrides normal */
    ui_css_rule_append_declaration(rule2, "background", "blue",
                                   0); /* fails to override important */

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule3);
    ui_css_rule_append_selector(rule3, UI_CSS_SELECTOR_TYPE_TAG, "div");
    ui_css_rule_append_declaration(rule3, "color", "green",
                                   1); /* same important, overrides */

    ui_css_stylesheet_append_rule(sheet, rule1);
    ui_css_stylesheet_append_rule(sheet, rule2);
    ui_css_stylesheet_append_rule(sheet, rule3);

    ui_dom_node_set_tag_name(node, "div");

    struct ui_css_computed_style *style;
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(node);
  }

  {
    /* More coverage */
    struct ui_dom_node *node;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    struct ui_css_stylesheet *sheet;
    ui_css_stylesheet_create(&sheet);

    struct ui_css_rule *r1, *r2, *r3, *r4, *r5;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r1);
    ui_css_rule_append_selector(r1, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS, "not");
    ui_css_stylesheet_append_rule(sheet, r1);

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r2);
    ui_css_rule_append_selector(r2, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "checked");
    ui_css_stylesheet_append_rule(sheet, r2);

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r3);
    ui_css_rule_append_selector(r3, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "target");
    ui_css_stylesheet_append_rule(sheet, r3);

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r4);
    ui_css_rule_append_selector(r4, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "target-within");
    ui_css_stylesheet_append_rule(sheet, r4);

    ui_css_rule_create(UI_CSS_RULE_TYPE_SCOPE, &r5);
    r5->scope_start = NULL;
    r5->scope_end = NULL;
    struct ui_css_rule *r5_nested;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r5_nested);
    ui_css_rule_append_selector(r5_nested, UI_CSS_SELECTOR_TYPE_TAG, "div");
    r5->nested_rules = r5_nested;
    ui_css_stylesheet_append_rule(sheet, r5);

    /* has(.container) */
    struct ui_css_rule *r6;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r6);
    ui_css_rule_append_selector(r6, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "has(.container)");
    ui_css_stylesheet_append_rule(sheet, r6);

    ui_dom_node_set_tag_name(node, "div");
    ui_dom_node_set_attribute(node, "checked", "");
    ui_dom_node_set_attribute(node, "class", "container");

    struct ui_css_computed_style *style;
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    ui_dom_node_remove_attribute(node, "checked");
    ui_dom_node_set_attribute(node, "aria-checked", "true");
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    ui_dom_node_set_attribute(node, "aria-checked", "false");
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(node);
  }

  {
    /* Missing edge cases */
    struct ui_dom_node *node;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    ui_dom_node_set_tag_name(node, "div");

    struct ui_css_stylesheet *sheet;
    ui_css_stylesheet_create(&sheet);

    struct ui_css_rule *r1;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r1);
    ui_css_rule_append_selector(r1, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "read-write");
    ui_css_stylesheet_append_rule(sheet, r1);

    struct ui_css_computed_style *style;

    /* :read-write with disabled attribute */
    ui_dom_node_set_attribute(node, "disabled", "");
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    /* :read-write with aria-disabled */
    ui_dom_node_remove_attribute(node, "disabled");
    ui_dom_node_set_attribute(node, "aria-disabled", "true");
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(node);
  }

  {
    /* Text node matching */
    struct ui_dom_node *node;
    ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &node);

    struct ui_css_stylesheet *sheet;
    ui_css_stylesheet_create(&sheet);

    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
    ui_css_stylesheet_append_rule(sheet, rule);

    struct ui_css_computed_style *style;
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(node);
  }
  {
    /* Nested parens in supports condition */
    struct ui_dom_node *node;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    struct ui_css_stylesheet *sheet;
    ui_css_stylesheet_create(&sheet);

    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
    rule->supports_condition = my_strdup("(not ((display: flex)))");
    ui_css_stylesheet_append_rule(sheet, rule);

    struct ui_css_computed_style *style;
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(node);
  }
  {
    /* Reverts Author origin test */
    struct ui_dom_node *node;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

    struct ui_css_stylesheet *sheet;
    ui_css_stylesheet_create(&sheet);

    struct ui_css_rule *rule;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
    ui_css_rule_append_declaration(rule, "color", "revert", 0);
    ui_css_stylesheet_append_rule(sheet, rule);

    struct ui_css_computed_style *style;
    ui_css_resolve_style(sheet, node, &style);
    ui_css_computed_style_destroy(style);

    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(node);
  }
  int cssom_oom_cnt = 0;
  while (1) {
    g_malloc_fail_countdown = cssom_oom_cnt;
    cssom_oom_cnt++;

    struct ui_css_stylesheet *cov_sheet = NULL;
    ui_error_t rc_sheet = ui_css_stylesheet_create(&cov_sheet);

    if (rc_sheet == UI_ERROR_NONE && cov_sheet != NULL) {
      struct ui_css_rule *cov_rule = NULL;
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
        ui_css_rule_append_declaration(cov_rule, "color", "red", 0);
        ui_css_rule_append_declaration(cov_rule, "margin", "10px", 0);
        ui_css_stylesheet_append_rule(cov_sheet, cov_rule);
      }

      struct ui_css_rule *rule_media = NULL;
      if (ui_css_rule_create(UI_CSS_RULE_TYPE_MEDIA, &rule_media) ==
          UI_ERROR_NONE) {
        rule_media->media_condition = my_strdup("(max-width: 600px)");
        ui_css_rule_append_declaration(rule_media, "padding", "5px", 0);
        ui_css_stylesheet_append_rule(cov_sheet, rule_media);
      }

      struct ui_css_rule *rule_scope = NULL;
      if (ui_css_rule_create(UI_CSS_RULE_TYPE_SCOPE, &rule_scope) ==
          UI_ERROR_NONE) {
        rule_scope->scope_start = my_strdup(".card");
        rule_scope->scope_end = my_strdup(".hole");
        ui_css_stylesheet_append_rule(cov_sheet, rule_scope);
      }

      struct ui_css_rule *rule_scope2 = NULL;
      if (ui_css_rule_create(UI_CSS_RULE_TYPE_SCOPE, &rule_scope2) ==
          UI_ERROR_NONE) {
        rule_scope2->scope_start = my_strdup(".container");
        rule_scope2->scope_end = NULL;
        ui_css_stylesheet_append_rule(cov_sheet, rule_scope2);
      }

      struct ui_css_rule *rule_supports = NULL;
      if (ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_supports) ==
          UI_ERROR_NONE) {
        rule_supports->supports_condition = my_strdup("(display: grid)");
        ui_css_stylesheet_append_rule(cov_sheet, rule_supports);
      }

      struct ui_css_rule *rule_container = NULL;
      if (ui_css_rule_create(UI_CSS_RULE_TYPE_CONTAINER, &rule_container) ==
          UI_ERROR_NONE) {
        rule_container->container_condition = my_strdup("(min-width: 700px)");
        ui_css_stylesheet_append_rule(cov_sheet, rule_container);
      }

      struct ui_css_rule *rule_property = NULL;
      if (ui_css_rule_create(UI_CSS_RULE_TYPE_PROPERTY, &rule_property) ==
          UI_ERROR_NONE) {
        rule_property->property_name = my_strdup("--my-prop");
        rule_property->property_syntax = my_strdup("<color>");
        rule_property->property_initial_value = my_strdup("red");
        ui_css_stylesheet_append_rule(cov_sheet, rule_property);
      }

      struct ui_css_rule *rule_layer = NULL;
      if (ui_css_rule_create(UI_CSS_RULE_TYPE_LAYER, &rule_layer) ==
          UI_ERROR_NONE) {
        rule_layer->layer_name = my_strdup("theme");
        ui_css_stylesheet_append_rule(cov_sheet, rule_layer);
      }

      int order2 = 2;
      ui_css_stylesheet_register_layer(cov_sheet, "L2", &order2);

      ui_css_stylesheet_destroy(cov_sheet);
    }

    if (g_malloc_fail_countdown > 0) {
      g_malloc_fail_countdown = -1;
      break;
    }
  }

  printf("test_ui_cssom passed.\n");
  return 0;
}
