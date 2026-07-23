/* clang-format off */
#include "../include/ui_css_page_floats.h"
#include "../include/ui_cssom.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#define TEST_ASSERT(cond)                                                      \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "Assertion failed: %s at %s:%d\n", #cond, __FILE__,      \
              __LINE__);                                                       \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

static void test_page_floats_cases(void) {
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_css_rule *rule = NULL;
  struct ui_dom_node *node = NULL;
  struct ui_css_computed_style *style = NULL;
  struct ui_css_page_floats_properties props;
  enum ui_error rc;

  /* Test invalid arguments */
  rc = ui_css_page_floats_parse(NULL, &props);
  TEST_ASSERT(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Need a real style object for next tests */
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_stylesheet_append_rule(sheet, rule);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  ui_dom_node_set_tag_name(node, "div");
  ui_css_resolve_style(sheet, node, &style);

  /* Test out_props == NULL */
  rc = ui_css_page_floats_parse(style, NULL);
  TEST_ASSERT(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Test empty style (defaults) */
  rc = ui_css_page_floats_parse(style, &props);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  TEST_ASSERT(props.float_reference == UI_CSS_FLOAT_REFERENCE_INLINE);
  TEST_ASSERT(props.float_defer_type == UI_CSS_FLOAT_DEFER_NONE);
  TEST_ASSERT(props.float_defer_value == 0);
  TEST_ASSERT(props.clear_after == UI_CSS_CLEAR_AFTER_NONE);

  ui_css_computed_style_destroy(style);
  style = NULL;
  ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* Helper to test a set of properties */
#define TEST_PAGE_FLOATS_PROPS(ref_str, ref_val, def_str, def_type, def_val,   \
                               clr_str, clr_val)                               \
  do {                                                                         \
    struct ui_css_stylesheet *sh;                                              \
    struct ui_css_rule *r;                                                     \
    struct ui_css_computed_style *s;                                           \
    struct ui_css_page_floats_properties p;                                    \
    ui_css_stylesheet_create(&sh);                                             \
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r);                            \
    ui_css_rule_append_selector(r, UI_CSS_SELECTOR_TYPE_TAG, "div");           \
    if (ref_str)                                                               \
      ui_css_rule_append_declaration(r, "float-reference", ref_str, 0);        \
    if (def_str)                                                               \
      ui_css_rule_append_declaration(r, "float-defer", def_str, 0);            \
    if (clr_str)                                                               \
      ui_css_rule_append_declaration(r, "clear-after", clr_str, 0);            \
    ui_css_stylesheet_append_rule(sh, r);                                      \
    ui_css_resolve_style(sh, node, &s);                                        \
    ui_css_page_floats_parse(s, &p);                                           \
    TEST_ASSERT(p.float_reference == ref_val);                                 \
    TEST_ASSERT(p.float_defer_type == def_type);                               \
    TEST_ASSERT(p.float_defer_value == def_val);                               \
    TEST_ASSERT(p.clear_after == clr_val);                                     \
    ui_css_computed_style_destroy(s);                                          \
    ui_css_stylesheet_destroy(sh);                                             \
  } while (0)

  /* float-reference */
  TEST_PAGE_FLOATS_PROPS("column", UI_CSS_FLOAT_REFERENCE_COLUMN, NULL,
                         UI_CSS_FLOAT_DEFER_NONE, 0, NULL,
                         UI_CSS_CLEAR_AFTER_NONE);
  TEST_PAGE_FLOATS_PROPS("region", UI_CSS_FLOAT_REFERENCE_REGION, NULL,
                         UI_CSS_FLOAT_DEFER_NONE, 0, NULL,
                         UI_CSS_CLEAR_AFTER_NONE);
  TEST_PAGE_FLOATS_PROPS("page", UI_CSS_FLOAT_REFERENCE_PAGE, NULL,
                         UI_CSS_FLOAT_DEFER_NONE, 0, NULL,
                         UI_CSS_CLEAR_AFTER_NONE);
  TEST_PAGE_FLOATS_PROPS("inline", UI_CSS_FLOAT_REFERENCE_INLINE, NULL,
                         UI_CSS_FLOAT_DEFER_NONE, 0, NULL,
                         UI_CSS_CLEAR_AFTER_NONE);
  TEST_PAGE_FLOATS_PROPS("unknown", UI_CSS_FLOAT_REFERENCE_INLINE, NULL,
                         UI_CSS_FLOAT_DEFER_NONE, 0, NULL,
                         UI_CSS_CLEAR_AFTER_NONE);

  /* float-defer */
  TEST_PAGE_FLOATS_PROPS(NULL, UI_CSS_FLOAT_REFERENCE_INLINE, "none",
                         UI_CSS_FLOAT_DEFER_NONE, 0, NULL,
                         UI_CSS_CLEAR_AFTER_NONE);
  TEST_PAGE_FLOATS_PROPS(NULL, UI_CSS_FLOAT_REFERENCE_INLINE, "3",
                         UI_CSS_FLOAT_DEFER_INTEGER, 3, NULL,
                         UI_CSS_CLEAR_AFTER_NONE);
  TEST_PAGE_FLOATS_PROPS(NULL, UI_CSS_FLOAT_REFERENCE_INLINE, "-2",
                         UI_CSS_FLOAT_DEFER_INTEGER, -2, NULL,
                         UI_CSS_CLEAR_AFTER_NONE);
  TEST_PAGE_FLOATS_PROPS(NULL, UI_CSS_FLOAT_REFERENCE_INLINE, "invalid",
                         UI_CSS_FLOAT_DEFER_INTEGER, 0, NULL,
                         UI_CSS_CLEAR_AFTER_NONE);

  /* clear-after */
  TEST_PAGE_FLOATS_PROPS(NULL, UI_CSS_FLOAT_REFERENCE_INLINE, NULL,
                         UI_CSS_FLOAT_DEFER_NONE, 0, "left",
                         UI_CSS_CLEAR_AFTER_LEFT);
  TEST_PAGE_FLOATS_PROPS(NULL, UI_CSS_FLOAT_REFERENCE_INLINE, NULL,
                         UI_CSS_FLOAT_DEFER_NONE, 0, "right",
                         UI_CSS_CLEAR_AFTER_RIGHT);
  TEST_PAGE_FLOATS_PROPS(NULL, UI_CSS_FLOAT_REFERENCE_INLINE, NULL,
                         UI_CSS_FLOAT_DEFER_NONE, 0, "both",
                         UI_CSS_CLEAR_AFTER_BOTH);
  TEST_PAGE_FLOATS_PROPS(NULL, UI_CSS_FLOAT_REFERENCE_INLINE, NULL,
                         UI_CSS_FLOAT_DEFER_NONE, 0, "start",
                         UI_CSS_CLEAR_AFTER_START);
  TEST_PAGE_FLOATS_PROPS(NULL, UI_CSS_FLOAT_REFERENCE_INLINE, NULL,
                         UI_CSS_FLOAT_DEFER_NONE, 0, "end",
                         UI_CSS_CLEAR_AFTER_END);
  TEST_PAGE_FLOATS_PROPS(NULL, UI_CSS_FLOAT_REFERENCE_INLINE, NULL,
                         UI_CSS_FLOAT_DEFER_NONE, 0, "descendants",
                         UI_CSS_CLEAR_AFTER_DESCENDANTS);
  TEST_PAGE_FLOATS_PROPS(NULL, UI_CSS_FLOAT_REFERENCE_INLINE, NULL,
                         UI_CSS_FLOAT_DEFER_NONE, 0, "none",
                         UI_CSS_CLEAR_AFTER_NONE);
  TEST_PAGE_FLOATS_PROPS(NULL, UI_CSS_FLOAT_REFERENCE_INLINE, NULL,
                         UI_CSS_FLOAT_DEFER_NONE, 0, "unknown",
                         UI_CSS_CLEAR_AFTER_NONE);

  ui_dom_node_destroy(node);
}

int main(void) {
  test_page_floats_cases();
  printf("test_ui_css_page_floats passed.\n");
  return 0;
}
