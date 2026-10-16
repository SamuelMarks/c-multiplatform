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

int test_cssom_part4_stylesheets(void) {

  {
    /* Combinator checks */
    struct ui_dom_node *node;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node, "class", "card container hole");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      struct ui_css_rule *rule_has;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_has);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      /* Child matching coverage */
      {
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

        /* We can't directly manipulate selector easily so we just parse or set
         * it
         */

        {
          ui_error_t rc_cleanup = ui_css_rule_destroy(rule_has);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
        {
          ui_error_t rc_cleanup = ui_dom_node_destroy(node);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
      }
    }
  }

  {
    /* Combinator checks for child matching */
    struct ui_dom_node *node;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      struct ui_css_rule *rule_has;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_has);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      /* check_mock_string_selector edge cases */
      {
        struct ui_css_stylesheet *sheet;
        {
          ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
        ui_css_rule_append_selector(rule_has, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                    "has");
        {
          ui_error_t rc_cleanup =
              ui_css_stylesheet_append_rule(sheet, rule_has);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }

        {
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
            {
              ui_error_t rc_cleanup =
                  ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            rule->supports_condition =
                ui_mock_strdup("((display: flex"); /* missing ) */
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(sheet, rule);
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
            {
              ui_error_t rc_cleanup =
                  ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            rule->supports_condition =
                ui_mock_strdup("((display: flex"); /* missing ) */
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(sheet, rule);
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
            {
              ui_error_t rc_cleanup =
                  ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            rule->supports_condition =
                ui_mock_strdup("((display: flex) and"); /* missing end term */
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(sheet, rule);
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
            {
              ui_error_t rc_cleanup =
                  ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            rule->supports_condition =
                ui_mock_strdup("((display: flex) or"); /* missing end term */
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(sheet, rule);
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
            {
              ui_error_t rc_cleanup =
                  ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            rule->supports_condition =
                ui_mock_strdup("(not (display: flex)"); /* missing inner end */
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(sheet, rule);
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
            {
              ui_error_t rc_cleanup =
                  ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            rule->supports_condition =
                ui_mock_strdup("((display: flex) or )"); /* trailing paren */
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(sheet, rule);
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
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(tmp_sheet, rule);
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
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(tmp_sheet, rule);
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
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(tmp_sheet, rule);
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
            {
              ui_error_t rc_cleanup =
                  ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            rule->supports_condition =
                ui_mock_strdup("((display: flex"); /* missing ) */
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(sheet, rule);
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
            {
              ui_error_t rc_cleanup =
                  ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            rule->supports_condition =
                ui_mock_strdup("((display: flex"); /* missing ) */
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(sheet, rule);
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
      }
    }
  }

  {
    /* Combinator checks for child matching and scope boundary */
    struct ui_dom_node *node;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      struct ui_css_rule *rule_has;
      {
        ui_error_t rc_cleanup =
            ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_has);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      /* check_mock_string_selector edge cases */
      {
        struct ui_css_stylesheet *sheet;
        {
          ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
        ui_css_rule_append_selector(rule_has, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                    "has(.card)");
        {
          ui_error_t rc_cleanup =
              ui_css_stylesheet_append_rule(sheet, rule_has);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }

        {
          struct ui_css_rule *rule_scope;
          {
            ui_error_t rc_cleanup =
                ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_scope);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          ui_css_rule_append_selector(rule_scope,
                                      UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                      "scope(.card, .hole)");
          {
            ui_error_t rc_cleanup =
                ui_css_stylesheet_append_rule(sheet, rule_scope);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }

          {
            struct ui_css_rule *rule_scope2;
            {
              ui_error_t rc_cleanup =
                  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_scope2);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            ui_css_rule_append_selector(rule_scope2,
                                        UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                        "scope(NULL, .hole)");
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(sheet, rule_scope2);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }

            {
              struct ui_css_computed_style *style;
              {
                ui_error_t rc_cleanup =
                    ui_css_resolve_style(sheet, node, &style);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }

              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "class", "card");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_css_resolve_style(sheet, node, &style);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }

              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "class", "container");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_css_resolve_style(sheet, node, &style);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }

              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "class", "hole");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_css_resolve_style(sheet, node, &style);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }

              {
                /* Test evaluating condition parenthesis errors */
                struct ui_css_rule *rule;
                {
                  ui_error_t rc_cleanup =
                      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }
                rule->supports_condition =
                    ui_mock_strdup("((display: flex"); /* missing ) */
                {
                  ui_error_t rc_cleanup =
                      ui_css_stylesheet_append_rule(sheet, rule);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }

                if (ui_css_resolve_style(sheet, node, &style) ==
                    UI_ERROR_NONE) {
                  {
                    ui_error_t rc_cleanup =
                        ui_css_computed_style_destroy(style);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                }
              }

              {
                /* Test evaluating condition parenthesis errors */
                struct ui_css_rule *rule;
                {
                  ui_error_t rc_cleanup =
                      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }
                rule->supports_condition =
                    ui_mock_strdup("((display: flex"); /* missing ) */
                {
                  ui_error_t rc_cleanup =
                      ui_css_stylesheet_append_rule(sheet, rule);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }

                if (ui_css_resolve_style(sheet, node, &style) ==
                    UI_ERROR_NONE) {
                  {
                    ui_error_t rc_cleanup =
                        ui_css_computed_style_destroy(style);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                }
              }

              {
                struct ui_css_rule *rule;
                {
                  ui_error_t rc_cleanup =
                      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }
                rule->supports_condition = ui_mock_strdup(
                    "((display: flex) and"); /* missing end term */
                {
                  ui_error_t rc_cleanup =
                      ui_css_stylesheet_append_rule(sheet, rule);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }

                if (ui_css_resolve_style(sheet, node, &style) ==
                    UI_ERROR_NONE) {
                  {
                    ui_error_t rc_cleanup =
                        ui_css_computed_style_destroy(style);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                }
              }

              {
                struct ui_css_rule *rule;
                {
                  ui_error_t rc_cleanup =
                      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }
                rule->supports_condition = ui_mock_strdup(
                    "((display: flex) or"); /* missing end term */
                {
                  ui_error_t rc_cleanup =
                      ui_css_stylesheet_append_rule(sheet, rule);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }

                if (ui_css_resolve_style(sheet, node, &style) ==
                    UI_ERROR_NONE) {
                  {
                    ui_error_t rc_cleanup =
                        ui_css_computed_style_destroy(style);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                }
              }
              {
                struct ui_css_rule *rule;
                {
                  ui_error_t rc_cleanup =
                      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }
                rule->supports_condition = ui_mock_strdup(
                    "(not (display: flex)"); /* missing inner end */
                {
                  ui_error_t rc_cleanup =
                      ui_css_stylesheet_append_rule(sheet, rule);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }

                if (ui_css_resolve_style(sheet, node, &style) ==
                    UI_ERROR_NONE) {
                  {
                    ui_error_t rc_cleanup =
                        ui_css_computed_style_destroy(style);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                }
              }
              {
                struct ui_css_rule *rule;
                {
                  ui_error_t rc_cleanup =
                      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }
                rule->supports_condition = ui_mock_strdup(
                    "((display: flex) or )"); /* trailing paren */
                {
                  ui_error_t rc_cleanup =
                      ui_css_stylesheet_append_rule(sheet, rule);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }

                if (ui_css_resolve_style(sheet, node, &style) ==
                    UI_ERROR_NONE) {
                  {
                    ui_error_t rc_cleanup =
                        ui_css_computed_style_destroy(style);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                }
              }

              {
                /* Test evaluating condition parenthesis errors */
                struct ui_css_rule *rule;

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
                  ui_error_t rc_cleanup =
                      ui_css_stylesheet_append_rule(tmp_sheet, rule);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }

                {
                  ui_error_t rc_cleanup =
                      ui_css_resolve_style(tmp_sheet, node, &tmp_style);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                              rc_cleanup == UI_ERROR_PARSE_FAILED);
                }
                if (tmp_style) {
                  ui_error_t rc_cleanup =
                      ui_css_computed_style_destroy(tmp_style);
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
                rule->supports_condition = ui_mock_strdup(
                    "((display: flex) and"); /* missing end term */
                {
                  ui_error_t rc_cleanup =
                      ui_css_stylesheet_append_rule(tmp_sheet, rule);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }

                {
                  ui_error_t rc_cleanup =
                      ui_css_resolve_style(tmp_sheet, node, &tmp_style);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                              rc_cleanup == UI_ERROR_PARSE_FAILED);
                }
                if (tmp_style) {
                  ui_error_t rc_cleanup =
                      ui_css_computed_style_destroy(tmp_style);
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
                rule->supports_condition = ui_mock_strdup(
                    "((display: flex) or"); /* missing end term */
                {
                  ui_error_t rc_cleanup =
                      ui_css_stylesheet_append_rule(tmp_sheet, rule);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }

                {
                  ui_error_t rc_cleanup =
                      ui_css_resolve_style(tmp_sheet, node, &tmp_style);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                              rc_cleanup == UI_ERROR_PARSE_FAILED);
                }
                if (tmp_style) {
                  ui_error_t rc_cleanup =
                      ui_css_computed_style_destroy(tmp_style);
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
                {
                  ui_error_t rc_cleanup =
                      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }
                rule->supports_condition =
                    ui_mock_strdup("((display: flex"); /* missing ) */
                {
                  ui_error_t rc_cleanup =
                      ui_css_stylesheet_append_rule(sheet, rule);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }

                if (ui_css_resolve_style(sheet, node, &style) ==
                    UI_ERROR_NONE) {
                  {
                    ui_error_t rc_cleanup =
                        ui_css_computed_style_destroy(style);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                }
              }

              {
                /* Test evaluating condition parenthesis errors */
                struct ui_css_rule *rule;
                {
                  ui_error_t rc_cleanup =
                      ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }
                rule->supports_condition =
                    ui_mock_strdup("((display: flex"); /* missing ) */
                {
                  ui_error_t rc_cleanup =
                      ui_css_stylesheet_append_rule(sheet, rule);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }

                if (ui_css_resolve_style(sheet, node, &style) ==
                    UI_ERROR_NONE) {
                  {
                    ui_error_t rc_cleanup =
                        ui_css_computed_style_destroy(style);
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
          }
        }
      }
    }
  }

  {
    /* Mismatches and default operators */
    struct ui_css_rule *rule;
    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_INCLUDES,
                                     "v");

    {
      struct ui_dom_node *node;
      {
        ui_error_t rc_cleanup =
            ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_dom_node_set_attribute(node, "data-test", "");
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
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
          struct ui_css_computed_style *style;
          {
            ui_error_t rc_cleanup = ui_css_resolve_style(sheet, node, &style);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          {
            ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }

          /* Cover missing internal mock string cases */
          {
            struct ui_css_rule *rule_mock;
            {
              ui_error_t rc_cleanup =
                  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_mock);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            ui_css_rule_append_selector(
                rule_mock, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS, "has(.unknown)");
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(sheet, rule_mock);
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
              struct ui_css_rule *rule_sub1;
              {
                ui_error_t rc_cleanup =
                    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub1);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              rule_sub1->supports_condition =
                  ui_mock_strdup("((display: flex"); /* missing ) */
              {
                ui_error_t rc_cleanup =
                    ui_css_stylesheet_append_rule(sheet, rule_sub1);
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
              struct ui_css_rule *rule_sub2;
              {
                ui_error_t rc_cleanup =
                    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub2);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              rule_sub2->supports_condition =
                  ui_mock_strdup("((display: flex"); /* missing ) */
              {
                ui_error_t rc_cleanup =
                    ui_css_stylesheet_append_rule(sheet, rule_sub2);
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
              struct ui_css_rule *rule_sub3;
              {
                ui_error_t rc_cleanup =
                    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub3);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              rule_sub3->supports_condition =
                  ui_mock_strdup("((display: flex) and"); /* missing end term */
              {
                ui_error_t rc_cleanup =
                    ui_css_stylesheet_append_rule(sheet, rule_sub3);
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
              struct ui_css_rule *rule_sub4;
              {
                ui_error_t rc_cleanup =
                    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub4);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              rule_sub4->supports_condition =
                  ui_mock_strdup("((display: flex) or"); /* missing end term */
              {
                ui_error_t rc_cleanup =
                    ui_css_stylesheet_append_rule(sheet, rule_sub4);
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
              struct ui_css_rule *rule_sub5;
              {
                ui_error_t rc_cleanup =
                    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub5);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              rule_sub5->supports_condition = ui_mock_strdup(
                  "(not (display: flex)"); /* missing inner end */
              {
                ui_error_t rc_cleanup =
                    ui_css_stylesheet_append_rule(sheet, rule_sub5);
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
              struct ui_css_rule *rule_sub6;
              {
                ui_error_t rc_cleanup =
                    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub6);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              rule_sub6->supports_condition =
                  ui_mock_strdup("((display: flex) or )"); /* trailing paren */
              {
                ui_error_t rc_cleanup =
                    ui_css_stylesheet_append_rule(sheet, rule_sub6);
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
              struct ui_css_rule *rule_sub7;

              struct ui_css_stylesheet *tmp_sheet = NULL;
              struct ui_css_computed_style *tmp_style = NULL;
              {
                ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }

              {
                ui_error_t rc_cleanup =
                    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub7);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              rule_sub7->supports_condition =
                  ui_mock_strdup("((display: flex"); /* missing ) */
              {
                ui_error_t rc_cleanup =
                    ui_css_stylesheet_append_rule(tmp_sheet, rule_sub7);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }

              {
                ui_error_t rc_cleanup =
                    ui_css_resolve_style(tmp_sheet, node, &tmp_style);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                            rc_cleanup == UI_ERROR_PARSE_FAILED);
              }
              if (tmp_style) {
                ui_error_t rc_cleanup =
                    ui_css_computed_style_destroy(tmp_style);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                tmp_style = NULL;
              }

              {
                ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
            }
            {
              struct ui_css_rule *rule_sub8;
              struct ui_css_stylesheet *tmp_sheet = NULL;
              struct ui_css_computed_style *tmp_style = NULL;
              {
                ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub8);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              rule_sub8->supports_condition =
                  ui_mock_strdup("((display: flex) and"); /* missing end term */
              {
                ui_error_t rc_cleanup =
                    ui_css_stylesheet_append_rule(tmp_sheet, rule_sub8);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }

              {
                ui_error_t rc_cleanup =
                    ui_css_resolve_style(tmp_sheet, node, &tmp_style);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                            rc_cleanup == UI_ERROR_PARSE_FAILED);
              }
              if (tmp_style) {
                ui_error_t rc_cleanup =
                    ui_css_computed_style_destroy(tmp_style);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                tmp_style = NULL;
              }
              {
                ui_error_t rc_cleanup = ui_css_stylesheet_destroy(tmp_sheet);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
            }
            {
              struct ui_css_rule *rule_sub9;
              struct ui_css_stylesheet *tmp_sheet = NULL;
              struct ui_css_computed_style *tmp_style = NULL;
              {
                ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub9);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              rule_sub9->supports_condition =
                  ui_mock_strdup("((display: flex) or"); /* missing end term */
              {
                ui_error_t rc_cleanup =
                    ui_css_stylesheet_append_rule(tmp_sheet, rule_sub9);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }

              {
                ui_error_t rc_cleanup =
                    ui_css_resolve_style(tmp_sheet, node, &tmp_style);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE ||
                            rc_cleanup == UI_ERROR_PARSE_FAILED);
              }
              if (tmp_style) {
                ui_error_t rc_cleanup =
                    ui_css_computed_style_destroy(tmp_style);
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
              struct ui_css_rule *rule_sub10;
              {
                ui_error_t rc_cleanup =
                    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub10);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              rule_sub10->supports_condition =
                  ui_mock_strdup("((display: flex"); /* missing ) */
              {
                ui_error_t rc_cleanup =
                    ui_css_stylesheet_append_rule(sheet, rule_sub10);
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
              struct ui_css_rule *rule_sub11;
              {
                ui_error_t rc_cleanup =
                    ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub11);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              rule_sub11->supports_condition =
                  ui_mock_strdup("((display: flex"); /* missing ) */
              {
                ui_error_t rc_cleanup =
                    ui_css_stylesheet_append_rule(sheet, rule_sub11);
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
        }
      }
    }
  }
  return 0;
}
