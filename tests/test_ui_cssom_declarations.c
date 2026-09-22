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
  struct ui_css_rule *rule = NULL;
  struct ui_dom_node *node = NULL;
  struct ui_css_computed_style *style = NULL;

  {
    ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
  }
  {
    ui_error_t rc_cleanup = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;

    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(tmp_sheet, rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                  rc_cleanup == UI_ERROR_PARSE_FAILED);
    }
    if (tmp_style) {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(tmp_style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      tmp_style = NULL;
    }

    {
      ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    rule->supports_condition =
        ui_mock_strdup("((display: flex) and"); /* missing end term */
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(tmp_sheet, rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                  rc_cleanup == UI_ERROR_PARSE_FAILED);
    }
    if (tmp_style) {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(tmp_style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      tmp_style = NULL;
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
  }
  {
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    rule->supports_condition =
        ui_mock_strdup("((display: flex) or"); /* missing end term */
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(tmp_sheet, rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_css_resolve_style(tmp_sheet, node, &tmp_style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                  rc_cleanup == UI_ERROR_PARSE_FAILED);
    }
    if (tmp_style) {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(tmp_style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      tmp_style = NULL;
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
  }

  {
    /* Test evaluating condition parenthesis errors */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
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
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    rule->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
    }
  }

  {
    ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
  }
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(node);
    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
  }

  {
    int match = 0;
    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    ui_css_rule_append_selector_attr(rule, "href", UI_CSS_ATTR_OP_EQUALS,
                                     "link");

    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    /* Cover class_list_contains missing branches */
    ui_css_rule_append_selector_attr(rule, "class", UI_CSS_ATTR_OP_INCLUDES,
                                     "btn");

    struct ui_css_computed_style *test_style = NULL;
    struct ui_css_stylesheet *sheet;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "class", "btn\tother");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &test_style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(test_style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "class", "btn\rother");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &test_style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(test_style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "class", "btn\nother");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &test_style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(test_style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_dom_node_set_attribute(node, "class", "btn");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &test_style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(test_style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    /* Cover strings */
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "data-test", "val");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_PREFIX,
                                     "value");
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_SUFFIX,
                                     "value");

    struct ui_css_computed_style *style;
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
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
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
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
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(tmp_sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
        ui_error_t rc_cleanup =
            ui_css_resolve_style(tmp_sheet, node, &tmp_style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                    rc_cleanup == UI_ERROR_PARSE_FAILED);
      }
      if (tmp_style) {
        ui_error_t rc_cleanup = ui_css_computed_style_destroy(tmp_style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        tmp_style = NULL;
      }

      {
        ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(tmp_sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
        ui_error_t rc_cleanup =
            ui_css_resolve_style(tmp_sheet, node, &tmp_style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                    rc_cleanup == UI_ERROR_PARSE_FAILED);
      }
      if (tmp_style) {
        ui_error_t rc_cleanup = ui_css_computed_style_destroy(tmp_style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        tmp_style = NULL;
      }
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(tmp_sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
        ui_error_t rc_cleanup =
            ui_css_resolve_style(tmp_sheet, node, &tmp_style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                    rc_cleanup == UI_ERROR_PARSE_FAILED);
      }
      if (tmp_style) {
        ui_error_t rc_cleanup = ui_css_computed_style_destroy(tmp_style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        tmp_style = NULL;
      }
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
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
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
      }
    }

    {
      ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
  }

  {
    /* Mismatches and default operators */
    struct ui_css_rule *rule;
    struct ui_css_selector *sel = NULL;
    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_css_rule_append_selector_attr(rule, "data-test", 99, "value");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
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
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "data-test", "val");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_stylesheet *sheet;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_computed_style *style;
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
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
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
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
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(tmp_sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
        ui_error_t rc_cleanup =
            ui_css_resolve_style(tmp_sheet, node, &tmp_style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                    rc_cleanup == UI_ERROR_PARSE_FAILED);
      }
      if (tmp_style) {
        ui_error_t rc_cleanup = ui_css_computed_style_destroy(tmp_style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        tmp_style = NULL;
      }

      {
        ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(tmp_sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
        ui_error_t rc_cleanup =
            ui_css_resolve_style(tmp_sheet, node, &tmp_style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                    rc_cleanup == UI_ERROR_PARSE_FAILED);
      }
      if (tmp_style) {
        ui_error_t rc_cleanup = ui_css_computed_style_destroy(tmp_style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        tmp_style = NULL;
      }
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(tmp_sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
        ui_error_t rc_cleanup =
            ui_css_resolve_style(tmp_sheet, node, &tmp_style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                    rc_cleanup == UI_ERROR_PARSE_FAILED);
      }
      if (tmp_style) {
        ui_error_t rc_cleanup = ui_css_computed_style_destroy(tmp_style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        tmp_style = NULL;
      }
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
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
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
      }
    }

    {
      ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
  }

  {
    /* Combinator checks */
    struct ui_dom_node *node;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_set_attribute(node, "class", "card");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_dom_node *child1;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(child1, "class", "container");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_append_child(node, child1);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_dom_node *child2;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child2);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(child2, "class", "hole");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_append_child(child1, child2);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    /* Child matching */
    int m = 0;

    /* Cover :has */
    struct ui_css_rule *rule_has;
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_has);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    ui_css_rule_append_selector(rule_has, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "has(.hole)");

    /* Scope checking logic */
    struct ui_css_rule *rule_scope;
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_scope);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    ui_css_rule_append_selector(rule_scope, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                "scope(.card, .container)");

    struct ui_css_stylesheet *sheet;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule_has);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule_scope);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    struct ui_css_computed_style *style;
    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, child1, &style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, child2, &style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
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
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
      }
    }

    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("(not (display: flex)"); /* missing inner end */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or )"); /* trailing paren */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
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
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(tmp_sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
        ui_error_t rc_cleanup =
            ui_css_resolve_style(tmp_sheet, node, &tmp_style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                    rc_cleanup == UI_ERROR_PARSE_FAILED);
      }
      if (tmp_style) {
        ui_error_t rc_cleanup = ui_css_computed_style_destroy(tmp_style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        tmp_style = NULL;
      }

      {
        ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) and"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(tmp_sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
        ui_error_t rc_cleanup =
            ui_css_resolve_style(tmp_sheet, node, &tmp_style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                    rc_cleanup == UI_ERROR_PARSE_FAILED);
      }
      if (tmp_style) {
        ui_error_t rc_cleanup = ui_css_computed_style_destroy(tmp_style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        tmp_style = NULL;
      }
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
    }
    {
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      struct ui_css_stylesheet *tmp_sheet = NULL;
      struct ui_css_computed_style *tmp_style = NULL;
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex) or"); /* missing end term */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(tmp_sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
        ui_error_t rc_cleanup =
            ui_css_resolve_style(tmp_sheet, node, &tmp_style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                    rc_cleanup == UI_ERROR_PARSE_FAILED);
      }
      if (tmp_style) {
        ui_error_t rc_cleanup = ui_css_computed_style_destroy(tmp_style);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        tmp_style = NULL;
      }
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
    }

    {
      /* Test evaluating condition parenthesis errors */
      struct ui_css_rule *rule;
      struct ui_css_selector *sel = NULL;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
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
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      rule->supports_condition =
          ui_mock_strdup("((display: flex"); /* missing ) */
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
      }
    }

    {
      ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
  }
  return 0;
}
