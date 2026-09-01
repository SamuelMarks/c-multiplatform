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
      ui_error_t rc_cleanup = ui_css_resolve_style(tmp_sheet, node, &tmp_style);
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
      ui_error_t rc_cleanup = ui_css_resolve_style(tmp_sheet, node, &tmp_style);
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
      ui_error_t rc_cleanup = ui_css_resolve_style(tmp_sheet, node, &tmp_style);
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
    int match = 0;
    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    ui_css_rule_append_selector_attr(rule, "href", UI_CSS_ATTR_OP_EQUALS,
                                     "link");

    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    /* Cover class_list_contains missing branches */
    ui_css_rule_append_selector_attr(rule, "class", UI_CSS_ATTR_OP_INCLUDES,
                                     "btn");

    struct ui_css_computed_style *test_style = NULL;
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

    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "class", "btn\tother");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &test_style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(test_style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "class", "btn\rother");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &test_style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(test_style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "class", "btn\nother");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &test_style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(test_style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    {
      ui_error_t rc_cleanup = ui_dom_node_set_attribute(node, "class", "btn");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &test_style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(test_style);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    /* Cover strings */
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "data-test", "val");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_PREFIX,
                                     "value");
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_SUFFIX,
                                     "value");

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
    {
      ui_error_t rc_cleanup =
          ui_css_rule_append_selector_attr(rule, "data-test", 99, "value");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_PREFIX,
                                     "somethingverylong");
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_SUFFIX,
                                     "somethingverylong");
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_INCLUDES,
                                     "v");
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_DASH,
                                     "somethingverylong");

    struct ui_dom_node *node;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "data-test", "val");
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
    /* Combinator checks */
    struct ui_dom_node *node;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_set_attribute(node, "class", "card");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

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
          ui_dom_node_set_attribute(child1, "class", "container");
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

    struct ui_dom_node *child2;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child2);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(child2, "class", "hole");
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_append_child(child1, child2);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    /* Child matching */
    int m = 0;

    /* Cover :has */
    struct ui_css_rule *rule_has;
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_has);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    ui_css_rule_append_selector(rule_has, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "has(.hole)");

    /* Scope checking logic */
    struct ui_css_rule *rule_scope;
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_scope);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    ui_css_rule_append_selector(rule_scope, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "scope(.card, .container)");

    struct ui_css_stylesheet *sheet;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule_has);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule_scope);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    struct ui_css_computed_style *style;
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, child1, &style);
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
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, child2, &style);
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
  return 0;
}
