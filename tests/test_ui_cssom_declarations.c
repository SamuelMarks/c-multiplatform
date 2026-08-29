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

int test_cssom_part3_declarations(void) {
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
  return 0;
}
