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

int test_cssom_part5_serialization(void) {

  {
    /* Mismatches and default operators */
    struct ui_css_rule *rule;
    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    ui_css_rule_append_selector_attr(rule, "data-test", UI_CSS_ATTR_OP_INCLUDES,
                                     "v");
    ui_css_rule_append_selector_attr(rule, "data-test2",
                                     UI_CSS_ATTR_OP_INCLUDES, "v");

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
        ui_error_t rc_cleanup =
            ui_dom_node_set_attribute(node, "data-test2", "v something");
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

  {
    /* Mismatches and default operators */
    struct ui_css_rule *rule;
    {
      ui_error_t rc_cleanup = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
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

        struct ui_dom_node *node;
        {
          ui_error_t rc_cleanup =
              ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
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
                ui_dom_node_set_attribute(child1, "class", "hole");
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }

          {
            struct ui_dom_node *text_child;
            {
              ui_error_t rc_cleanup =
                  ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_child);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            {
              ui_error_t rc_cleanup =
                  ui_dom_node_append_child(child1, text_child);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }

            {
              ui_error_t rc_cleanup = ui_dom_node_append_child(node, child1);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }

            {
              struct ui_css_rule *rule_has;
              {
                ui_error_t rc_cleanup =
                    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_has);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                struct ui_css_selector *has_hole = create_mock_selector(
                    UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS, "has");
                struct ui_css_selector *nested_hole =
                    create_mock_selector(UI_CSS_SELECTOR_TYPE_CLASS, ".hole");
                has_hole->nested_selector = nested_hole;
                rule_has->selectors = has_hole;
                {
                  ui_error_t rc_cleanup =
                      ui_css_stylesheet_append_rule(sheet, rule_has);
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

                /* ui_dom_node_destroy(text_child); Since parent is freed, but
                 * let's make sure it's not a double free. Actually node frees
                 * its children. So we just need to wait for node destruction */

                {
                  /* Test evaluating condition parenthesis errors */
                  struct ui_css_rule *rule_sub12;
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub12);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub12->supports_condition =
                      ui_mock_strdup("((display: flex"); /* missing ) */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(sheet, rule_sub12);
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
                  struct ui_css_rule *rule_sub13;
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub13);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub13->supports_condition =
                      ui_mock_strdup("((display: flex"); /* missing ) */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(sheet, rule_sub13);
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
                  struct ui_css_rule *rule_sub14;
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub14);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub14->supports_condition = ui_mock_strdup(
                      "((display: flex) and"); /* missing end term */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(sheet, rule_sub14);
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
                  struct ui_css_rule *rule_sub15;
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub15);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub15->supports_condition = ui_mock_strdup(
                      "((display: flex) or"); /* missing end term */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(sheet, rule_sub15);
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
                  struct ui_css_rule *rule_sub16;
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub16);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub16->supports_condition = ui_mock_strdup(
                      "(not (display: flex)"); /* missing inner end */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(sheet, rule_sub16);
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
                  struct ui_css_rule *rule_sub17;
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub17);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub17->supports_condition = ui_mock_strdup(
                      "((display: flex) or )"); /* trailing paren */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(sheet, rule_sub17);
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
                  struct ui_css_rule *rule_sub18;

                  struct ui_css_stylesheet *tmp_sheet = NULL;
                  struct ui_css_computed_style *tmp_style = NULL;
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_create(&tmp_sheet);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }

                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub18);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub18->supports_condition =
                      ui_mock_strdup("((display: flex"); /* missing ) */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(tmp_sheet, rule_sub18);
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
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_destroy(tmp_sheet);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                }
                {
                  struct ui_css_rule *rule_sub19;
                  struct ui_css_stylesheet *tmp_sheet = NULL;
                  struct ui_css_computed_style *tmp_style = NULL;
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_create(&tmp_sheet);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub19);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub19->supports_condition = ui_mock_strdup(
                      "((display: flex) and"); /* missing end term */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(tmp_sheet, rule_sub19);
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
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_destroy(tmp_sheet);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                }
                {
                  struct ui_css_rule *rule_sub20;
                  struct ui_css_stylesheet *tmp_sheet = NULL;
                  struct ui_css_computed_style *tmp_style = NULL;
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_create(&tmp_sheet);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub20);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub20->supports_condition = ui_mock_strdup(
                      "((display: flex) or"); /* missing end term */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(tmp_sheet, rule_sub20);
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
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_destroy(tmp_sheet);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                }

                {
                  /* Test evaluating condition parenthesis errors */
                  struct ui_css_rule *rule_sub21;
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub21);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub21->supports_condition =
                      ui_mock_strdup("((display: flex"); /* missing ) */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(sheet, rule_sub21);
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
                  struct ui_css_rule *rule_sub22;
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub22);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub22->supports_condition =
                      ui_mock_strdup("((display: flex"); /* missing ) */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(sheet, rule_sub22);
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
  }

  {
    struct ui_css_stylesheet *sheet;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      int order1, order2, order3;
      {
        ui_error_t rc_cleanup =
            ui_css_stylesheet_register_layer(sheet, "L1", &order1);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_stylesheet_register_layer(sheet, "L2", &order2);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_stylesheet_register_layer(sheet, "L3", &order3);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
    }
  }
  {
    /* Missing pseudo classes */
    struct ui_dom_node *node;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      const char *pseudos[] = {"disabled",      "enabled",   "required",
                               "optional",      "read-only", "read-write",
                               "indeterminate", "default",   "invalid",
                               "valid",         "unknown"};

      struct ui_css_stylesheet *sheet;
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
        int i;
        for (i = 0; i < 11; ++i) {
          struct ui_css_rule *rule;
          {
            ui_error_t rc_cleanup =
                ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                      pseudos[i]);
          {
            ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
        }

        /* Empty rule without selector */
        {
          struct ui_css_rule *empty_rule;
          {
            ui_error_t rc_cleanup =
                ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &empty_rule);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          {
            ui_error_t rc_cleanup =
                ui_css_stylesheet_append_rule(sheet, empty_rule);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }

          {
            struct ui_css_rule *attr_rule;
            {
              ui_error_t rc_cleanup =
                  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &attr_rule);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            ui_css_rule_append_selector_attr(attr_rule, "lang",
                                             UI_CSS_ATTR_OP_DASH, "en");
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(sheet, attr_rule);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }

            /* Test state 1: no attributes */
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

              /* Test dash mismatch */
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "lang", "en");
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
                    ui_dom_node_remove_attribute(node, "lang");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "lang", "es");
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
                    ui_dom_node_remove_attribute(node, "lang");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "lang", "eng");
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
                    ui_dom_node_remove_attribute(node, "lang");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "lang", "en-US");
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
                    ui_dom_node_remove_attribute(node, "lang");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "lang", "en-");
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

              /* Test state 2: With native attributes */
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "disabled", "");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "required", "");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "readonly", "");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "indeterminate", "");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "default", "");
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

              /* Test state 3: With aria attributes (true) */
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_remove_attribute(node, "disabled");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_remove_attribute(node, "required");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_remove_attribute(node, "readonly");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_remove_attribute(node, "indeterminate");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_remove_attribute(node, "default");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }

              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "aria-disabled", "true");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "aria-required", "true");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "aria-readonly", "true");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "aria-checked", "true");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "aria-invalid", "true");
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

              /* Test state 4: with aria attributes (false) */
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "aria-disabled", "false");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "aria-required", "false");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "aria-readonly", "false");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "aria-checked", "false");
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_dom_node_set_attribute(node, "aria-invalid", "false");
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
    /* Important rules overriding */
    struct ui_dom_node *node;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      struct ui_css_stylesheet *sheet;
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
        int order1, order2;
        {
          ui_error_t rc_cleanup =
              ui_css_stylesheet_register_layer(sheet, "L1", &order1);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
        {
          ui_error_t rc_cleanup =
              ui_css_stylesheet_register_layer(sheet, "L2", &order2);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }

        {
          struct ui_css_rule *rule1, *rule2, *rule3;
          {
            ui_error_t rc_cleanup =
                ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule1);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          {
            ui_error_t rc_cleanup = ui_css_rule_append_selector(
                rule1, UI_CSS_SELECTOR_TYPE_TAG, "div");
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          {
            ui_error_t rc_cleanup =
                ui_css_rule_append_declaration(rule1, "color", "red", 0);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          {
            ui_error_t rc_cleanup =
                ui_css_rule_append_declaration(rule1, "background", "red", 1);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }

          {
            ui_error_t rc_cleanup =
                ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule2);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          {
            ui_error_t rc_cleanup = ui_css_rule_append_selector(
                rule2, UI_CSS_SELECTOR_TYPE_TAG, "div");
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          ui_css_rule_append_declaration(rule2, "color", "blue",
                                         1); /* overrides normal */
          ui_css_rule_append_declaration(rule2, "background", "blue",
                                         0); /* fails to override important */

          {
            ui_error_t rc_cleanup =
                ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule3);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          {
            ui_error_t rc_cleanup = ui_css_rule_append_selector(
                rule3, UI_CSS_SELECTOR_TYPE_TAG, "div");
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          ui_css_rule_append_declaration(rule3, "color", "green",
                                         1); /* same important, overrides */

          {
            ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule1);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          {
            ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule2);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          {
            ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule3);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }

          {
            ui_error_t rc_cleanup = ui_dom_node_set_tag_name(node, "div");
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
