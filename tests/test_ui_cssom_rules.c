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

int test_cssom_part2_rules(void) {
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

  (void)ui_dom_node_destroy(node);
  (void)ui_css_stylesheet_destroy(sheet);

  /* OOM Mocks - Very high coverage */
  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_stylesheet_create(&sheet) == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  rc = ui_css_stylesheet_create(&sheet);
  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_stylesheet_register_namespace(sheet, "a", "b") ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  TEST_ASSERT(ui_css_stylesheet_register_namespace(sheet, "a", "b") ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 2;
  TEST_ASSERT(ui_css_stylesheet_register_namespace(sheet, "a", "b") ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_stylesheet_register_namespace(sheet, NULL, "b") ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  TEST_ASSERT(ui_css_stylesheet_register_namespace(sheet, NULL, "b") ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_stylesheet_register_layer(sheet, "a", &order) ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  TEST_ASSERT(ui_css_stylesheet_register_layer(sheet, "a", &order) ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule) ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);

  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS,
                                          "a") == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  TEST_ASSERT(ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS,
                                          "a") == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_UNIVERSAL,
                                          NULL) == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_rule_append_selector_attr(rule, "a", UI_CSS_ATTR_OP_EQUALS,
                                               "b") == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  TEST_ASSERT(ui_css_rule_append_selector_attr(rule, "a", UI_CSS_ATTR_OP_EQUALS,
                                               "b") == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 2;
  ui_error_t rc_debug =
      ui_css_rule_append_selector_attr(rule, "a", UI_CSS_ATTR_OP_EQUALS, "b");
  printf("DEBUG: rc_debug=%d\n", rc_debug);
  TEST_ASSERT(rc_debug == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_rule_append_selector_attr(rule, "a", UI_CSS_ATTR_OP_NONE,
                                               NULL) == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  TEST_ASSERT(ui_css_rule_append_selector_attr(rule, "a", UI_CSS_ATTR_OP_NONE,
                                               NULL) == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  TEST_ASSERT(ui_css_rule_append_declaration(rule, "a", "b", 0) ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  TEST_ASSERT(ui_css_rule_append_declaration(rule, "a", "b", 0) ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 2;
  TEST_ASSERT(ui_css_rule_append_declaration(rule, "a", "b", 0) ==
              UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  (void)ui_css_rule_append_declaration(rule, "prop1", "val1", 0);
  (void)ui_css_rule_append_declaration(rule, "prop2", "val2", 0);

  (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  (void)ui_css_stylesheet_append_rule(sheet, rule);
  (void)ui_css_rule_append_declaration(rule, "a", "b", 0);

  {
    int oom_cnt = 0;
    while (1) {
      g_malloc_fail_countdown = oom_cnt;
      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_OUT_OF_MEMORY) {
        oom_cnt++;
      } else {
        break;
      }
    }
  }
  (void)ui_css_computed_style_destroy(style);
  g_malloc_fail_countdown = -1;

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

  /* Eval condition paths */
  /* These are static functions but reached via supports/media rules */
  rc = ui_css_stylesheet_create(&sheet);
  struct ui_css_rule *r;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SUPPORTS, &r);
  struct ui_css_rule *nested_r;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_r);
  (void)ui_css_rule_append_selector(nested_r, UI_CSS_SELECTOR_TYPE_TAG, "div");
  (void)ui_css_rule_append_declaration(nested_r, "display", "flex", 0);
  r->nested_rules = nested_r;
  (void)ui_css_stylesheet_append_rule(sheet, r);

  struct ui_css_rule *media_r;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_MEDIA, &media_r);
  media_r->media_condition = my_strdup("screen");
  struct ui_css_rule *nested_mr;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_mr);
  (void)ui_css_rule_append_selector(nested_mr, UI_CSS_SELECTOR_TYPE_TAG, "div");
  (void)ui_css_rule_append_declaration(nested_mr, "color", "red", 0);
  media_r->nested_rules = nested_mr;
  (void)ui_css_stylesheet_append_rule(sheet, media_r);

  struct ui_css_rule *cont_r;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_CONTAINER, &cont_r);
  cont_r->container_condition = my_strdup("(min-width: 100px)");
  struct ui_css_rule *nested_cr;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_cr);
  (void)ui_css_rule_append_selector(nested_cr, UI_CSS_SELECTOR_TYPE_TAG, "div");
  (void)ui_css_rule_append_declaration(nested_cr, "color", "blue", 0);
  cont_r->nested_rules = nested_cr;
  (void)ui_css_stylesheet_append_rule(sheet, cont_r);

  struct ui_css_rule *scope_r;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_SCOPE, &scope_r);
  scope_r->scope_start =
      create_mock_selector(UI_CSS_SELECTOR_TYPE_CLASS, ".card");
  struct ui_css_rule *nested_scope_r;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_scope_r);
  (void)ui_css_rule_append_selector(nested_scope_r, UI_CSS_SELECTOR_TYPE_TAG,
                                    "div");
  (void)ui_css_rule_append_declaration(nested_scope_r, "background", "yellow",
                                       0);
  scope_r->nested_rules = nested_scope_r;
  (void)ui_css_stylesheet_append_rule(sheet, scope_r);

  struct ui_css_rule *style_r;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &style_r);
  (void)ui_css_rule_append_selector(style_r, UI_CSS_SELECTOR_TYPE_TAG, "div");
  struct ui_css_rule *nested_sr;
  (void)ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &nested_sr);
  (void)ui_css_rule_append_selector(nested_sr, UI_CSS_SELECTOR_TYPE_CLASS,
                                    "nested");
  (void)ui_css_rule_append_declaration(nested_sr, "margin", "0", 0);
  style_r->nested_rules = nested_sr;
  (void)ui_css_stylesheet_append_rule(sheet, style_r);

  /* Condition variations */
  const char *conds[] = {"not (display: flex)",
                         "(display: grid) or (display: flex)",
                         "(display: flex) and (display: none)",
                         "selector(:has(.a))",
                         "selector(:invalid)",
                         "(((a)))",
                         "not not",
                         "(not (display: flex))",
                         "(not display: flex)",
                         "(display: flex) and (not (display: grid))",
                         "(display: flex) or (not (display: grid))",
                         "and and or or not not",
                         "and (display: flex)",
                         "or (display: flex)",
                         "not display: flex",
                         "(((",
                         "selector",
                         "selector(",
                         "selector(:has",
                         "selector(:has()",
                         "not",
                         "not (",
                         "not )",
                         "(a) and",
                         "(a) or"};
  size_t i;
  for (i = 0; i < sizeof(conds) / sizeof(conds[0]); i++) {
    if (r->supports_condition) {
      C_MULTIPLATFORM_FREE(r->supports_condition);
    }
    r->supports_condition = my_strdup(conds[i]);
    (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    (void)ui_css_resolve_style(sheet, node, &style);
    (void)ui_css_computed_style_destroy(style);
    (void)ui_dom_node_destroy(node);
  }

  (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  (void)ui_dom_node_set_tag_name(node, "div");
  (void)ui_dom_node_set_attribute(node, "class", "nested");
  {
    int oom_cnt = 0;
    while (1) {
      g_malloc_fail_countdown = oom_cnt;
      if (ui_css_resolve_style(sheet, node, &style) == UI_ERROR_OUT_OF_MEMORY) {
        oom_cnt++;
      } else {
        break;
      }
    }
  }
  (void)ui_css_computed_style_destroy(style);
  g_malloc_fail_countdown = -1;
  (void)ui_dom_node_destroy(node);

  /* Check supports evaluation failure */
  if (r->supports_condition) {
    C_MULTIPLATFORM_FREE(r->supports_condition);
  }
  r->supports_condition = my_strdup("(a) and"); /* parse error */
  (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  rc = ui_css_resolve_style(sheet, node, &style);
  if (rc != UI_ERROR_PARSE_FAILED) {
    printf("Expected PARSE_FAILED, got %d\n", rc);
    TEST_ASSERT(rc == UI_ERROR_PARSE_FAILED);
  }
  (void)ui_dom_node_destroy(node);

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
  return 0;
}
