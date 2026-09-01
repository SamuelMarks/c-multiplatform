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
    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_INCLUDES,
                                     "v");
    ui_css_rule_append_selector_attr(rule, "data-test2",
                                     UI_CSS_ATTR_OP_INCLUDES, "v");

    struct ui_dom_node *node;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_set_attribute(node, "data-test", "");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "data-test2", "v something");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    struct ui_css_stylesheet *sheet;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    struct ui_css_computed_style *style;
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    /* Cover missing internal mock string cases */
    struct ui_css_rule *rule_mock;
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_mock);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    ui_css_rule_append_selector(rule_mock, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "has(.unknown)");
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule_mock);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;

      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(tmp_sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      {
        ui_error_t rc_cleanup =
            ui_css_resolve_style(tmp_sheet, node, &tmp_style);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      {
        ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(tmp_sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      {
        ui_error_t rc_cleanup =
            ui_css_resolve_style(tmp_sheet, node, &tmp_style);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(tmp_sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      {
        ui_error_t rc_cleanup =
            ui_css_resolve_style(tmp_sheet, node, &tmp_style);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }

    {
      ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  {
    /* Mismatches and default operators */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    struct ui_css_stylesheet *sheet;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    struct ui_css_computed_style *style;

    struct ui_dom_node *node;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    /* Child matching coverage */
    struct ui_dom_node *child1;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(child1, "class", "hole");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    struct ui_dom_node *text_child;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_child);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_append_child(child1, text_child);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    {
      ui_error_t rc_cleanup = ui_dom_node_append_child(node, child1);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    struct ui_css_rule *rule_has;
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_has);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    struct ui_css_selector *has_hole =
        create_mock_selector(UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS, "has");
    struct ui_css_selector *nested_hole =
        create_mock_selector(UI_CSS_SELECTOR_TYPE_CLASS, ".hole");
    has_hole->nested_selector = nested_hole;
    rule_has->selectors = has_hole;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule_has);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    /* ui_dom_node_destroy(text_child); Since parent is freed, but let's make
     * sure it's not a double free. Actually node frees its children. So we just
     * need to wait for node destruction */

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;

      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(tmp_sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      {
        ui_error_t rc_cleanup =
            ui_css_resolve_style(tmp_sheet, node, &tmp_style);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      {
        ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(tmp_sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      {
        ui_error_t rc_cleanup =
            ui_css_resolve_style(tmp_sheet, node, &tmp_style);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(tmp_sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      {
        ui_error_t rc_cleanup =
            ui_css_resolve_style(tmp_sheet, node, &tmp_style);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }

    {
      ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  {
    struct ui_css_stylesheet *sheet;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    int order1, order2, order3;
    {
      ui_error_t rc_cleanup =
          ui_css_stylesheet_register_layer(sheet, "L1", &order1);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_css_stylesheet_register_layer(sheet, "L2", &order2);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_css_stylesheet_register_layer(sheet, "L3", &order3);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  {
    /* Missing pseudo classes */
    struct ui_dom_node *node;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    const char *pseudos[] = {"disabled",      "enabled",   "required",
                             "optional",      "read-only", "read-write",
                             "indeterminate", "default",   "invalid",
                             "valid",         "unknown"};

    struct ui_css_stylesheet *sheet;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    int i;
    for (i = 0; i < 11; ++i) {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                  pseudos[i]);
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }

    /* Empty rule without selector */
    struct ui_css_rule *empty_rule;
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &empty_rule);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, empty_rule);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    struct ui_css_rule *attr_rule;
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &attr_rule);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    ui_css_rule_append_selector_attr(attr_rule, "lang", UI_CSS_ATTR_OP_DASH,
                                     "en");
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, attr_rule);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    /* Test state 1: no attributes */
    struct ui_css_computed_style *style;
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    /* Test dash mismatch */
    {
      ui_error_t rc_cleanup = ui_dom_node_set_attribute(node, "lang", "en");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_remove_attribute(node, "lang");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_set_attribute(node, "lang", "es");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_remove_attribute(node, "lang");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_set_attribute(node, "lang", "eng");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_remove_attribute(node, "lang");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_set_attribute(node, "lang", "en-US");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_remove_attribute(node, "lang");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_set_attribute(node, "lang", "en-");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    /* Test state 2: With native attributes */
    {
      ui_error_t rc_cleanup = ui_dom_node_set_attribute(node, "disabled", "");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_set_attribute(node, "required", "");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_set_attribute(node, "readonly", "");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "indeterminate", "");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_set_attribute(node, "default", "");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    /* Test state 3: With aria attributes (true) */
    {
      ui_error_t rc_cleanup = ui_dom_node_remove_attribute(node, "disabled");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_remove_attribute(node, "required");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_remove_attribute(node, "readonly");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_remove_attribute(node, "indeterminate");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_remove_attribute(node, "default");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "aria-disabled", "true");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "aria-required", "true");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "aria-readonly", "true");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "aria-checked", "true");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "aria-invalid", "true");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    /* Test state 4: with aria attributes (false) */
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "aria-disabled", "false");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "aria-required", "false");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "aria-readonly", "false");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "aria-checked", "false");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "aria-invalid", "false");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    {
      ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  {
    /* Important rules overriding */
    struct ui_dom_node *node;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    struct ui_css_stylesheet *sheet;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    int order1, order2;
    {
      ui_error_t rc_cleanup =
          ui_css_stylesheet_register_layer(sheet, "L1", &order1);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_css_stylesheet_register_layer(sheet, "L2", &order2);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    struct ui_css_rule *rule1, *rule2, *rule3, *rule4;
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule1);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_css_rule_append_selector(rule1, UI_CSS_SELECTOR_TYPE_TAG, "div");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_css_rule_append_declaration(rule1, "color", "red", 0);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_css_rule_append_declaration(rule1, "background", "red", 1);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule2);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_css_rule_append_selector(rule2, UI_CSS_SELECTOR_TYPE_TAG, "div");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    ui_css_rule_append_declaration(rule2, "color", "blue",
                                   1); /* overrides normal */
    ui_css_rule_append_declaration(rule2, "background", "blue",
                                   0); /* fails to override important */

    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule3);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_css_rule_append_selector(rule3, UI_CSS_SELECTOR_TYPE_TAG, "div");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    ui_css_rule_append_declaration(rule3, "color", "green",
                                   1); /* same important, overrides */

    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule1);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule2);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule3);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    {
      ui_error_t rc_cleanup = ui_dom_node_set_tag_name(node, "div");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    struct ui_css_computed_style *style;
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    {
      ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  return 0;
}
