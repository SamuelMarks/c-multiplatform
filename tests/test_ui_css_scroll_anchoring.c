/* clang-format off */
#include "greatest.h"
#include "../include/ui_css_scroll_anchoring.h"
#include "../include/ui_cssom.h"
#include "ui_test_mock_mem.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

TEST test_scroll_anchoring_invalid_args(void) {
  enum ui_css_overflow_anchor anchor;
  ui_error_t rc;

  /* Null arguments */
  rc = ui_css_scroll_anchoring_parse(NULL, &anchor);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  {
    struct ui_css_stylesheet *sheet = NULL;
    struct ui_css_rule *rule = NULL;
    struct ui_dom_node *node = NULL;
    struct ui_css_computed_style *style = NULL;

    ui_css_stylesheet_create(&sheet);
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
    ui_css_stylesheet_append_rule(sheet, rule);
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    ui_dom_node_set_tag_name(node, "div");
    ui_css_resolve_style(sheet, node, &style);

    rc = ui_css_scroll_anchoring_parse(style, NULL);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

    ui_css_computed_style_destroy(style);
    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(node);
  }

  PASS();
}

TEST test_scroll_anchoring_parse_values(void) {
  struct ui_dom_node *node = NULL;
  ui_error_t rc;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ui_dom_node_set_tag_name(node, "div");

  /* Helper to test values */
#define TEST_SCROLL_ANCHORING(val_str, expected_anchor)                        \
  do {                                                                         \
    struct ui_css_stylesheet *sh = NULL;                                       \
    struct ui_css_rule *r = NULL;                                              \
    struct ui_css_computed_style *s = NULL;                                    \
    enum ui_css_overflow_anchor a;                                             \
    ui_error_t inner_rc;                                                       \
    inner_rc = ui_css_stylesheet_create(&sh);                                  \
    ASSERT_EQ(UI_ERROR_NONE, inner_rc);                                        \
    inner_rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r);                 \
    ASSERT_EQ(UI_ERROR_NONE, inner_rc);                                        \
    inner_rc =                                                                 \
        ui_css_rule_append_selector(r, UI_CSS_SELECTOR_TYPE_TAG, "div");       \
    ASSERT_EQ(UI_ERROR_NONE, inner_rc);                                        \
    if (val_str) {                                                             \
      inner_rc =                                                               \
          ui_css_rule_append_declaration(r, "overflow-anchor", val_str, 0);    \
      ASSERT_EQ(UI_ERROR_NONE, inner_rc);                                      \
    }                                                                          \
    inner_rc = ui_css_stylesheet_append_rule(sh, r);                           \
    ASSERT_EQ(UI_ERROR_NONE, inner_rc);                                        \
    inner_rc = ui_css_resolve_style(sh, node, &s);                             \
    ASSERT_EQ(UI_ERROR_NONE, inner_rc);                                        \
    inner_rc = ui_css_scroll_anchoring_parse(s, &a);                           \
    ASSERT_EQ(UI_ERROR_NONE, inner_rc);                                        \
    ASSERT_EQ(expected_anchor, a);                                             \
    ui_css_computed_style_destroy(s);                                          \
    ui_css_stylesheet_destroy(sh);                                             \
  } while (0)

  /* Empty style -> defaults */
  TEST_SCROLL_ANCHORING(NULL, UI_CSS_OVERFLOW_ANCHOR_AUTO);

  /* Valid values */
  TEST_SCROLL_ANCHORING("auto", UI_CSS_OVERFLOW_ANCHOR_AUTO);
  TEST_SCROLL_ANCHORING("none", UI_CSS_OVERFLOW_ANCHOR_NONE);

  /* Invalid string -> fallback to auto */
  TEST_SCROLL_ANCHORING("invalid", UI_CSS_OVERFLOW_ANCHOR_AUTO);

  rc = ui_dom_node_destroy(node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_scroll_anchoring_oom(void) {
  struct ui_dom_node *node = NULL;
  struct ui_css_stylesheet *sh = NULL;
  struct ui_css_rule *r = NULL;
  struct ui_css_computed_style *s = NULL;
  enum ui_css_overflow_anchor a;
  ui_error_t rc;

  /* Setup a stylesheet and compute style to pass to parse */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ui_dom_node_set_tag_name(node, "div");

  rc = ui_css_stylesheet_create(&sh);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_rule_append_selector(r, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_rule_append_declaration(r, "overflow-anchor", "none", 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_stylesheet_append_rule(sh, r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_resolve_style(sh, node, &s);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* No OOM mocking inside ui_css_scroll_anchoring_parse since it allocates
     nothing. But we'll run it once under OOM countdown = 0 to ensure it handles
     it cleanly if something internal allocates, although it shouldn't. */
  g_malloc_fail_countdown = 0;
  rc = ui_css_scroll_anchoring_parse(s, &a);
  g_malloc_fail_countdown = -1;
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_CSS_OVERFLOW_ANCHOR_NONE, a);

  ui_css_computed_style_destroy(s);
  ui_css_stylesheet_destroy(sh);
  ui_dom_node_destroy(node);

  PASS();
}

SUITE(scroll_anchoring_suite) {
  RUN_TEST(test_scroll_anchoring_invalid_args);
  RUN_TEST(test_scroll_anchoring_parse_values);
  RUN_TEST(test_scroll_anchoring_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(scroll_anchoring_suite);
  GREATEST_MAIN_END();
}
