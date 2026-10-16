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
    struct ui_css_rule *rule_sub1;

    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub1);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    rule_sub1->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    {
      ui_error_t rc_cleanup =
          ui_css_stylesheet_append_rule(tmp_sheet, rule_sub1);
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
    struct ui_css_rule *rule_sub2;
    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub2);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    rule_sub2->supports_condition =
        ui_mock_strdup("((display: flex) and"); /* missing end term */
    {
      ui_error_t rc_cleanup =
          ui_css_stylesheet_append_rule(tmp_sheet, rule_sub2);
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
    struct ui_css_rule *rule_sub3;
    struct ui_css_stylesheet *tmp_sheet = NULL;
    struct ui_css_computed_style *tmp_style = NULL;
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub3);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    rule_sub3->supports_condition =
        ui_mock_strdup("((display: flex) or"); /* missing end term */
    {
      ui_error_t rc_cleanup =
          ui_css_stylesheet_append_rule(tmp_sheet, rule_sub3);
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
    struct ui_css_rule *rule_sub4;
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub4);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    rule_sub4->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule_sub4);
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
    struct ui_css_rule *rule_sub5;
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub5);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    rule_sub5->supports_condition =
        ui_mock_strdup("((display: flex"); /* missing ) */
    {
      ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet, rule_sub5);
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

    {
      struct ui_css_computed_style *test_style = NULL;
      struct ui_css_stylesheet *sheet_sub6;
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet_sub6);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup = ui_css_stylesheet_append_rule(sheet_sub6, rule);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
        ui_error_t rc_cleanup =
            ui_dom_node_set_attribute(node, "class", "btn\tother");
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_css_resolve_style(sheet_sub6, node, &test_style);
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
        ui_error_t rc_cleanup =
            ui_css_resolve_style(sheet_sub6, node, &test_style);
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
        ui_error_t rc_cleanup =
            ui_css_resolve_style(sheet_sub6, node, &test_style);
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
        ui_error_t rc_cleanup =
            ui_css_resolve_style(sheet_sub6, node, &test_style);
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

      {
        struct ui_css_computed_style *style_sub7;
        {
          ui_error_t rc_cleanup =
              ui_css_resolve_style(sheet_sub6, node, &style_sub7);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
        {
          ui_error_t rc_cleanup = ui_css_computed_style_destroy(style_sub7);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }

        {
          /* Test evaluating condition parenthesis errors */
          struct ui_css_rule *rule_sub8;
          {
            ui_error_t rc_cleanup =
                ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub8);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          rule_sub8->supports_condition =
              ui_mock_strdup("((display: flex"); /* missing ) */
          {
            ui_error_t rc_cleanup =
                ui_css_stylesheet_append_rule(sheet_sub6, rule_sub8);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }

          if (ui_css_resolve_style(sheet_sub6, node, &style_sub7) ==
              UI_ERROR_NONE) {
            {
              ui_error_t rc_cleanup = ui_css_computed_style_destroy(style_sub7);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
          }
        }

        {
          /* Test evaluating condition parenthesis errors */
          struct ui_css_rule *rule_sub9;
          {
            ui_error_t rc_cleanup =
                ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub9);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          rule_sub9->supports_condition =
              ui_mock_strdup("((display: flex"); /* missing ) */
          {
            ui_error_t rc_cleanup =
                ui_css_stylesheet_append_rule(sheet_sub6, rule_sub9);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }

          if (ui_css_resolve_style(sheet_sub6, node, &style_sub7) ==
              UI_ERROR_NONE) {
            {
              ui_error_t rc_cleanup = ui_css_computed_style_destroy(style_sub7);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
          }
        }

        {
          struct ui_css_rule *rule_sub10;
          {
            ui_error_t rc_cleanup =
                ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub10);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          rule_sub10->supports_condition =
              ui_mock_strdup("((display: flex) and"); /* missing end term */
          {
            ui_error_t rc_cleanup =
                ui_css_stylesheet_append_rule(sheet_sub6, rule_sub10);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }

          if (ui_css_resolve_style(sheet_sub6, node, &style_sub7) ==
              UI_ERROR_NONE) {
            {
              ui_error_t rc_cleanup = ui_css_computed_style_destroy(style_sub7);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
          }
        }

        {
          struct ui_css_rule *rule_sub11;
          {
            ui_error_t rc_cleanup =
                ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub11);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          rule_sub11->supports_condition =
              ui_mock_strdup("((display: flex) or"); /* missing end term */
          {
            ui_error_t rc_cleanup =
                ui_css_stylesheet_append_rule(sheet_sub6, rule_sub11);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }

          if (ui_css_resolve_style(sheet_sub6, node, &style_sub7) ==
              UI_ERROR_NONE) {
            {
              ui_error_t rc_cleanup = ui_css_computed_style_destroy(style_sub7);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
          }
        }
        {
          struct ui_css_rule *rule_sub12;
          {
            ui_error_t rc_cleanup =
                ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub12);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          rule_sub12->supports_condition =
              ui_mock_strdup("(not (display: flex)"); /* missing inner end */
          {
            ui_error_t rc_cleanup =
                ui_css_stylesheet_append_rule(sheet_sub6, rule_sub12);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }

          if (ui_css_resolve_style(sheet_sub6, node, &style_sub7) ==
              UI_ERROR_NONE) {
            {
              ui_error_t rc_cleanup = ui_css_computed_style_destroy(style_sub7);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
          }
        }
        {
          struct ui_css_rule *rule_sub13;
          {
            ui_error_t rc_cleanup =
                ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub13);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          rule_sub13->supports_condition =
              ui_mock_strdup("((display: flex) or )"); /* trailing paren */
          {
            ui_error_t rc_cleanup =
                ui_css_stylesheet_append_rule(sheet_sub6, rule_sub13);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }

          if (ui_css_resolve_style(sheet_sub6, node, &style_sub7) ==
              UI_ERROR_NONE) {
            {
              ui_error_t rc_cleanup = ui_css_computed_style_destroy(style_sub7);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
          }
        }

        {
          /* Test evaluating condition parenthesis errors */
          struct ui_css_rule *rule_sub14;

          struct ui_css_stylesheet *tmp_sheet = NULL;
          struct ui_css_computed_style *tmp_style = NULL;
          {
            ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }

          {
            ui_error_t rc_cleanup =
                ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub14);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          rule_sub14->supports_condition =
              ui_mock_strdup("((display: flex"); /* missing ) */
          {
            ui_error_t rc_cleanup =
                ui_css_stylesheet_append_rule(tmp_sheet, rule_sub14);
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
          struct ui_css_rule *rule_sub15;
          struct ui_css_stylesheet *tmp_sheet = NULL;
          struct ui_css_computed_style *tmp_style = NULL;
          {
            ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          {
            ui_error_t rc_cleanup =
                ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub15);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          rule_sub15->supports_condition =
              ui_mock_strdup("((display: flex) and"); /* missing end term */
          {
            ui_error_t rc_cleanup =
                ui_css_stylesheet_append_rule(tmp_sheet, rule_sub15);
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
          struct ui_css_rule *rule_sub16;
          struct ui_css_stylesheet *tmp_sheet = NULL;
          struct ui_css_computed_style *tmp_style = NULL;
          {
            ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          {
            ui_error_t rc_cleanup =
                ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub16);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          rule_sub16->supports_condition =
              ui_mock_strdup("((display: flex) or"); /* missing end term */
          {
            ui_error_t rc_cleanup =
                ui_css_stylesheet_append_rule(tmp_sheet, rule_sub16);
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
          struct ui_css_rule *rule_sub17;
          {
            ui_error_t rc_cleanup =
                ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub17);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          rule_sub17->supports_condition =
              ui_mock_strdup("((display: flex"); /* missing ) */
          {
            ui_error_t rc_cleanup =
                ui_css_stylesheet_append_rule(sheet_sub6, rule_sub17);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }

          if (ui_css_resolve_style(sheet_sub6, node, &style_sub7) ==
              UI_ERROR_NONE) {
            {
              ui_error_t rc_cleanup = ui_css_computed_style_destroy(style_sub7);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
          }
        }

        {
          /* Test evaluating condition parenthesis errors */
          struct ui_css_rule *rule_sub18;
          {
            ui_error_t rc_cleanup =
                ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub18);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          rule_sub18->supports_condition =
              ui_mock_strdup("((display: flex"); /* missing ) */
          {
            ui_error_t rc_cleanup =
                ui_css_stylesheet_append_rule(sheet_sub6, rule_sub18);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }

          if (ui_css_resolve_style(sheet_sub6, node, &style_sub7) ==
              UI_ERROR_NONE) {
            {
              ui_error_t rc_cleanup = ui_css_computed_style_destroy(style_sub7);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
          }
        }

        {
          ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet_sub6);
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
    /* Mismatches and default operators */
    struct ui_css_rule *rule_sub19;
    {
      ui_error_t rc_cleanup =
          ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_sub19);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_css_rule_append_selector_attr(
          rule_sub19, "data-test", 99, "value");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    ui_css_rule_append_selector_attr(
        rule_sub19, "data-test", UI_CSS_ATTR_OP_PREFIX, "somethingverylong");
    ui_css_rule_append_selector_attr(
        rule_sub19, "data-test", UI_CSS_ATTR_OP_SUFFIX, "somethingverylong");
    ui_css_rule_append_selector_attr(rule_sub19, "data-test",
                                     UI_CSS_ATTR_OP_INCLUDES, "v");
    ui_css_rule_append_selector_attr(rule_sub19, "data-test",
                                     UI_CSS_ATTR_OP_DASH, "somethingverylong");

    {
      struct ui_dom_node *node_sub20;
      {
        ui_error_t rc_cleanup =
            ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node_sub20);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }
      {
        ui_error_t rc_cleanup =
            ui_dom_node_set_attribute(node_sub20, "data-test", "val");
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
        struct ui_css_stylesheet *sheet_sub21;
        {
          ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet_sub21);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }
        {
          ui_error_t rc_cleanup =
              ui_css_stylesheet_append_rule(sheet_sub21, rule_sub19);
          TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
        }

        {
          struct ui_css_computed_style *style_sub22;
          {
            ui_error_t rc_cleanup =
                ui_css_resolve_style(sheet_sub21, node_sub20, &style_sub22);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          {
            ui_error_t rc_cleanup = ui_css_computed_style_destroy(style_sub22);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }

          {
            /* Test evaluating condition parenthesis errors */
            struct ui_css_rule *rule_sub19_sub23;
            {
              ui_error_t rc_cleanup = ui_css_rule_create(
                  UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub19_sub23);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            rule_sub19_sub23->supports_condition =
                ui_mock_strdup("((display: flex"); /* missing ) */
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(sheet_sub21, rule_sub19_sub23);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }

            if (ui_css_resolve_style(sheet_sub21, node_sub20, &style_sub22) ==
                UI_ERROR_NONE) {
              {
                ui_error_t rc_cleanup =
                    ui_css_computed_style_destroy(style_sub22);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
            }
          }

          {
            /* Test evaluating condition parenthesis errors */
            struct ui_css_rule *rule_sub19_sub24;
            {
              ui_error_t rc_cleanup = ui_css_rule_create(
                  UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub19_sub24);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            rule_sub19_sub24->supports_condition =
                ui_mock_strdup("((display: flex"); /* missing ) */
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(sheet_sub21, rule_sub19_sub24);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }

            if (ui_css_resolve_style(sheet_sub21, node_sub20, &style_sub22) ==
                UI_ERROR_NONE) {
              {
                ui_error_t rc_cleanup =
                    ui_css_computed_style_destroy(style_sub22);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
            }
          }

          {
            struct ui_css_rule *rule_sub19_sub25;
            {
              ui_error_t rc_cleanup = ui_css_rule_create(
                  UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub19_sub25);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            rule_sub19_sub25->supports_condition =
                ui_mock_strdup("((display: flex) and"); /* missing end term */
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(sheet_sub21, rule_sub19_sub25);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }

            if (ui_css_resolve_style(sheet_sub21, node_sub20, &style_sub22) ==
                UI_ERROR_NONE) {
              {
                ui_error_t rc_cleanup =
                    ui_css_computed_style_destroy(style_sub22);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
            }
          }

          {
            struct ui_css_rule *rule_sub19_sub26;
            {
              ui_error_t rc_cleanup = ui_css_rule_create(
                  UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub19_sub26);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            rule_sub19_sub26->supports_condition =
                ui_mock_strdup("((display: flex) or"); /* missing end term */
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(sheet_sub21, rule_sub19_sub26);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }

            if (ui_css_resolve_style(sheet_sub21, node_sub20, &style_sub22) ==
                UI_ERROR_NONE) {
              {
                ui_error_t rc_cleanup =
                    ui_css_computed_style_destroy(style_sub22);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
            }
          }
          {
            struct ui_css_rule *rule_sub19_sub27;
            {
              ui_error_t rc_cleanup = ui_css_rule_create(
                  UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub19_sub27);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            rule_sub19_sub27->supports_condition =
                ui_mock_strdup("(not (display: flex)"); /* missing inner end */
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(sheet_sub21, rule_sub19_sub27);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }

            if (ui_css_resolve_style(sheet_sub21, node_sub20, &style_sub22) ==
                UI_ERROR_NONE) {
              {
                ui_error_t rc_cleanup =
                    ui_css_computed_style_destroy(style_sub22);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
            }
          }
          {
            struct ui_css_rule *rule_sub19_sub28;
            {
              ui_error_t rc_cleanup = ui_css_rule_create(
                  UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub19_sub28);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            rule_sub19_sub28->supports_condition =
                ui_mock_strdup("((display: flex) or )"); /* trailing paren */
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(sheet_sub21, rule_sub19_sub28);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }

            if (ui_css_resolve_style(sheet_sub21, node_sub20, &style_sub22) ==
                UI_ERROR_NONE) {
              {
                ui_error_t rc_cleanup =
                    ui_css_computed_style_destroy(style_sub22);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
            }
          }

          {
            /* Test evaluating condition parenthesis errors */
            struct ui_css_rule *rule_sub19_sub29;

            struct ui_css_stylesheet *tmp_sheet = NULL;
            struct ui_css_computed_style *tmp_style = NULL;
            {
              ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }

            {
              ui_error_t rc_cleanup = ui_css_rule_create(
                  UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub19_sub29);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            rule_sub19_sub29->supports_condition =
                ui_mock_strdup("((display: flex"); /* missing ) */
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(tmp_sheet, rule_sub19_sub29);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }

            {
              ui_error_t rc_cleanup =
                  ui_css_resolve_style(tmp_sheet, node_sub20, &tmp_style);
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
            struct ui_css_rule *rule_sub19_sub30;
            struct ui_css_stylesheet *tmp_sheet = NULL;
            struct ui_css_computed_style *tmp_style = NULL;
            {
              ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            {
              ui_error_t rc_cleanup = ui_css_rule_create(
                  UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub19_sub30);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            rule_sub19_sub30->supports_condition =
                ui_mock_strdup("((display: flex) and"); /* missing end term */
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(tmp_sheet, rule_sub19_sub30);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }

            {
              ui_error_t rc_cleanup =
                  ui_css_resolve_style(tmp_sheet, node_sub20, &tmp_style);
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
            struct ui_css_rule *rule_sub19_sub31;
            struct ui_css_stylesheet *tmp_sheet = NULL;
            struct ui_css_computed_style *tmp_style = NULL;
            {
              ui_error_t rc_cleanup = ui_css_stylesheet_create(&tmp_sheet);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            {
              ui_error_t rc_cleanup = ui_css_rule_create(
                  UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub19_sub31);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            rule_sub19_sub31->supports_condition =
                ui_mock_strdup("((display: flex) or"); /* missing end term */
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(tmp_sheet, rule_sub19_sub31);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }

            {
              ui_error_t rc_cleanup =
                  ui_css_resolve_style(tmp_sheet, node_sub20, &tmp_style);
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
            struct ui_css_rule *rule_sub19_sub32;
            {
              ui_error_t rc_cleanup = ui_css_rule_create(
                  UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub19_sub32);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            rule_sub19_sub32->supports_condition =
                ui_mock_strdup("((display: flex"); /* missing ) */
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(sheet_sub21, rule_sub19_sub32);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }

            if (ui_css_resolve_style(sheet_sub21, node_sub20, &style_sub22) ==
                UI_ERROR_NONE) {
              {
                ui_error_t rc_cleanup =
                    ui_css_computed_style_destroy(style_sub22);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
            }
          }

          {
            /* Test evaluating condition parenthesis errors */
            struct ui_css_rule *rule_sub19_sub33;
            {
              ui_error_t rc_cleanup = ui_css_rule_create(
                  UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub19_sub33);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            rule_sub19_sub33->supports_condition =
                ui_mock_strdup("((display: flex"); /* missing ) */
            {
              ui_error_t rc_cleanup =
                  ui_css_stylesheet_append_rule(sheet_sub21, rule_sub19_sub33);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }

            if (ui_css_resolve_style(sheet_sub21, node_sub20, &style_sub22) ==
                UI_ERROR_NONE) {
              {
                ui_error_t rc_cleanup =
                    ui_css_computed_style_destroy(style_sub22);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
            }
          }

          {
            ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet_sub21);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          {
            ui_error_t rc_cleanup = ui_dom_node_destroy(node_sub20);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
        }
      }
    }
  }

  {
    /* Combinator checks */
    struct ui_dom_node *node_sub34;
    {
      ui_error_t rc_cleanup =
          ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node_sub34);
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup =
          ui_dom_node_set_attribute(node_sub34, "class", "card");
      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
    }

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
        ui_error_t rc_cleanup = ui_dom_node_append_child(node_sub34, child1);
        TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
      }

      {
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

        /* Cover :has */
        {
          struct ui_css_rule *rule_has;
          {
            ui_error_t rc_cleanup =
                ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_has);
            TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
          }
          ui_css_rule_append_selector(
              rule_has, UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS, "has(.hole)");

          /* Scope checking logic */
          {
            struct ui_css_rule *rule_scope;
            {
              ui_error_t rc_cleanup =
                  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_scope);
              TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
            }
            ui_css_rule_append_selector(rule_scope,
                                        UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
                                        "scope(.card, .container)");

            {
              struct ui_css_stylesheet *sheet_sub35;
              {
                ui_error_t rc_cleanup = ui_css_stylesheet_create(&sheet_sub35);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_css_stylesheet_append_rule(sheet_sub35, rule_has);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }
              {
                ui_error_t rc_cleanup =
                    ui_css_stylesheet_append_rule(sheet_sub35, rule_scope);
                TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
              }

              {
                struct ui_css_computed_style *style_sub36;
                {
                  ui_error_t rc_cleanup =
                      ui_css_resolve_style(sheet_sub35, child1, &style_sub36);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }
                {
                  ui_error_t rc_cleanup =
                      ui_css_computed_style_destroy(style_sub36);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }

                {
                  ui_error_t rc_cleanup =
                      ui_css_resolve_style(sheet_sub35, child2, &style_sub36);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }
                {
                  ui_error_t rc_cleanup =
                      ui_css_computed_style_destroy(style_sub36);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }

                {
                  ui_error_t rc_cleanup = ui_css_resolve_style(
                      sheet_sub35, node_sub34, &style_sub36);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }
                {
                  ui_error_t rc_cleanup =
                      ui_css_computed_style_destroy(style_sub36);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }

                {
                  /* Test evaluating condition parenthesis errors */
                  struct ui_css_rule *rule_sub37;
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub37);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub37->supports_condition =
                      ui_mock_strdup("((display: flex"); /* missing ) */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(sheet_sub35, rule_sub37);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }

                  if (ui_css_resolve_style(sheet_sub35, node_sub34,
                                           &style_sub36) == UI_ERROR_NONE) {
                    {
                      ui_error_t rc_cleanup =
                          ui_css_computed_style_destroy(style_sub36);
                      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                    }
                  }
                }

                {
                  /* Test evaluating condition parenthesis errors */
                  struct ui_css_rule *rule_sub38;
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub38);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub38->supports_condition =
                      ui_mock_strdup("((display: flex"); /* missing ) */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(sheet_sub35, rule_sub38);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }

                  if (ui_css_resolve_style(sheet_sub35, node_sub34,
                                           &style_sub36) == UI_ERROR_NONE) {
                    {
                      ui_error_t rc_cleanup =
                          ui_css_computed_style_destroy(style_sub36);
                      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                    }
                  }
                }

                {
                  struct ui_css_rule *rule_sub39;
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub39);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub39->supports_condition = ui_mock_strdup(
                      "((display: flex) and"); /* missing end term */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(sheet_sub35, rule_sub39);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }

                  if (ui_css_resolve_style(sheet_sub35, node_sub34,
                                           &style_sub36) == UI_ERROR_NONE) {
                    {
                      ui_error_t rc_cleanup =
                          ui_css_computed_style_destroy(style_sub36);
                      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                    }
                  }
                }

                {
                  struct ui_css_rule *rule_sub40;
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub40);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub40->supports_condition = ui_mock_strdup(
                      "((display: flex) or"); /* missing end term */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(sheet_sub35, rule_sub40);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }

                  if (ui_css_resolve_style(sheet_sub35, node_sub34,
                                           &style_sub36) == UI_ERROR_NONE) {
                    {
                      ui_error_t rc_cleanup =
                          ui_css_computed_style_destroy(style_sub36);
                      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                    }
                  }
                }
                {
                  struct ui_css_rule *rule_sub41;
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub41);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub41->supports_condition = ui_mock_strdup(
                      "(not (display: flex)"); /* missing inner end */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(sheet_sub35, rule_sub41);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }

                  if (ui_css_resolve_style(sheet_sub35, node_sub34,
                                           &style_sub36) == UI_ERROR_NONE) {
                    {
                      ui_error_t rc_cleanup =
                          ui_css_computed_style_destroy(style_sub36);
                      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                    }
                  }
                }
                {
                  struct ui_css_rule *rule_sub42;
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub42);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub42->supports_condition = ui_mock_strdup(
                      "((display: flex) or )"); /* trailing paren */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(sheet_sub35, rule_sub42);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }

                  if (ui_css_resolve_style(sheet_sub35, node_sub34,
                                           &style_sub36) == UI_ERROR_NONE) {
                    {
                      ui_error_t rc_cleanup =
                          ui_css_computed_style_destroy(style_sub36);
                      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                    }
                  }
                }

                {
                  /* Test evaluating condition parenthesis errors */
                  struct ui_css_rule *rule_sub43;

                  struct ui_css_stylesheet *tmp_sheet = NULL;
                  struct ui_css_computed_style *tmp_style = NULL;
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_create(&tmp_sheet);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }

                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub43);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub43->supports_condition =
                      ui_mock_strdup("((display: flex"); /* missing ) */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(tmp_sheet, rule_sub43);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }

                  {
                    ui_error_t rc_cleanup =
                        ui_css_resolve_style(tmp_sheet, node_sub34, &tmp_style);
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
                  struct ui_css_rule *rule_sub44;
                  struct ui_css_stylesheet *tmp_sheet = NULL;
                  struct ui_css_computed_style *tmp_style = NULL;
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_create(&tmp_sheet);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub44);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub44->supports_condition = ui_mock_strdup(
                      "((display: flex) and"); /* missing end term */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(tmp_sheet, rule_sub44);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }

                  {
                    ui_error_t rc_cleanup =
                        ui_css_resolve_style(tmp_sheet, node_sub34, &tmp_style);
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
                  struct ui_css_rule *rule_sub45;
                  struct ui_css_stylesheet *tmp_sheet = NULL;
                  struct ui_css_computed_style *tmp_style = NULL;
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_create(&tmp_sheet);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub45);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub45->supports_condition = ui_mock_strdup(
                      "((display: flex) or"); /* missing end term */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(tmp_sheet, rule_sub45);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }

                  {
                    ui_error_t rc_cleanup =
                        ui_css_resolve_style(tmp_sheet, node_sub34, &tmp_style);
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
                  struct ui_css_rule *rule_sub46;
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub46);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub46->supports_condition =
                      ui_mock_strdup("((display: flex"); /* missing ) */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(sheet_sub35, rule_sub46);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }

                  if (ui_css_resolve_style(sheet_sub35, node_sub34,
                                           &style_sub36) == UI_ERROR_NONE) {
                    {
                      ui_error_t rc_cleanup =
                          ui_css_computed_style_destroy(style_sub36);
                      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                    }
                  }
                }

                {
                  /* Test evaluating condition parenthesis errors */
                  struct ui_css_rule *rule_sub47;
                  {
                    ui_error_t rc_cleanup = ui_css_rule_create(
                        UI_CSS_RULE_TYPE_SUPPORTS, &rule_sub47);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }
                  rule_sub47->supports_condition =
                      ui_mock_strdup("((display: flex"); /* missing ) */
                  {
                    ui_error_t rc_cleanup =
                        ui_css_stylesheet_append_rule(sheet_sub35, rule_sub47);
                    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                  }

                  if (ui_css_resolve_style(sheet_sub35, node_sub34,
                                           &style_sub36) == UI_ERROR_NONE) {
                    {
                      ui_error_t rc_cleanup =
                          ui_css_computed_style_destroy(style_sub36);
                      TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                    }
                  }
                }

                {
                  ui_error_t rc_cleanup =
                      ui_css_stylesheet_destroy(sheet_sub35);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }
                {
                  ui_error_t rc_cleanup = ui_dom_node_destroy(node_sub34);
                  TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
                }
              }
            }
          }
        }
      }
    }
  }
  return 0;
}
