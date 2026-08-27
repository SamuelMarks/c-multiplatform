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

int test_cssom_part5_serialization(void) {
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

    int i;
    for (i = 0; i < 11; ++i) {
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
  return 0;
}
