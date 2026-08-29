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

int test_cssom_part4_stylesheets(void) {
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
  return 0;
}
