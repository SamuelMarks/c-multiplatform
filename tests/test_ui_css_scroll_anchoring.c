/* clang-format off */
#include "../include/ui_css_scroll_anchoring.h"
#include "../include/ui_cssom.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

#define TEST_ASSERT(cond)                                                      \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "Assertion failed: %s at %s:%d\n", #cond, __FILE__,      \
              __LINE__);                                                       \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

static void test_scroll_anchoring_cases(void) {
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_css_rule *rule = NULL;
  struct ui_dom_node *node = NULL;
  struct ui_css_computed_style *style = NULL;
  enum ui_css_overflow_anchor anchor;
  ui_error_t rc;

  /* Null arguments */
  TEST_ASSERT(ui_css_scroll_anchoring_parse(NULL, &anchor) ==
              UI_ERROR_INVALID_ARGUMENT);

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_stylesheet_append_rule(sheet, rule);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  ui_dom_node_set_tag_name(node, "div");
  ui_css_resolve_style(sheet, node, &style);

  TEST_ASSERT(ui_css_scroll_anchoring_parse(style, NULL) ==
              UI_ERROR_INVALID_ARGUMENT);

  /* Empty style -> defaults */
  TEST_ASSERT(ui_css_scroll_anchoring_parse(style, &anchor) == UI_ERROR_NONE);
  TEST_ASSERT(anchor == UI_CSS_OVERFLOW_ANCHOR_AUTO);

  ui_css_computed_style_destroy(style);
  ui_css_stylesheet_destroy(sheet);

  /* Helper to test values */
#define TEST_SCROLL_ANCHORING(val_str, expected_anchor)                        \
  do {                                                                         \
    struct ui_css_stylesheet *sh;                                              \
    struct ui_css_rule *r;                                                     \
    struct ui_css_computed_style *s;                                           \
    enum ui_css_overflow_anchor a;                                             \
    ui_css_stylesheet_create(&sh);                                             \
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r);                            \
    ui_css_rule_append_selector(r, UI_CSS_SELECTOR_TYPE_TAG, "div");           \
    if (val_str)                                                               \
      ui_css_rule_append_declaration(r, "overflow-anchor", val_str, 0);        \
    ui_css_stylesheet_append_rule(sh, r);                                      \
    ui_css_resolve_style(sh, node, &s);                                        \
    TEST_ASSERT(ui_css_scroll_anchoring_parse(s, &a) == UI_ERROR_NONE);        \
    TEST_ASSERT(a == expected_anchor);                                         \
    ui_css_computed_style_destroy(s);                                          \
    ui_css_stylesheet_destroy(sh);                                             \
  } while (0)

  TEST_SCROLL_ANCHORING("auto", UI_CSS_OVERFLOW_ANCHOR_AUTO);
  TEST_SCROLL_ANCHORING("none", UI_CSS_OVERFLOW_ANCHOR_NONE);
  TEST_SCROLL_ANCHORING(
      "invalid", UI_CSS_OVERFLOW_ANCHOR_AUTO); /* fallback to auto on
                                                  invalid/unknown string */

  (void)ui_dom_node_destroy(node);
}

int main(void) {
  test_scroll_anchoring_cases();
  printf("test_ui_css_scroll_anchoring passed\n");
  return 0;
}
