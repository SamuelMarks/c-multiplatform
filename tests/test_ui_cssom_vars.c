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

int test_cssom_part7_vars(void) {
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

  return 0;
}
