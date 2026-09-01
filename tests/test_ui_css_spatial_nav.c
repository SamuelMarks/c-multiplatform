/* clang-format off */
#include "../include/ui_css_spatial_nav.h"
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

static void test_spatial_nav_cases(void) {
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_css_rule *rule = NULL;
  struct ui_dom_node *node = NULL;
  struct ui_css_computed_style *style = NULL;
  struct ui_css_spatial_nav_properties props;

  /* Invalid args */
  TEST_ASSERT(ui_css_spatial_nav_parse(NULL, &props) ==
              UI_ERROR_INVALID_ARGUMENT);

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_stylesheet_append_rule(sheet, rule);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  ui_dom_node_set_tag_name(node, "div");
  ui_css_resolve_style(sheet, node, &style);

  TEST_ASSERT(ui_css_spatial_nav_parse(style, NULL) ==
              UI_ERROR_INVALID_ARGUMENT);

  /* Empty style defaults */
  TEST_ASSERT(ui_css_spatial_nav_parse(style, &props) == UI_ERROR_NONE);
  TEST_ASSERT(props.action == UI_CSS_SPATIAL_NAV_ACTION_AUTO);
  TEST_ASSERT(props.contain == UI_CSS_SPATIAL_NAV_CONTAIN_AUTO);
  TEST_ASSERT(props.function == UI_CSS_SPATIAL_NAV_FUNCTION_NORMAL);

  ui_css_computed_style_destroy(style);
  ui_css_stylesheet_destroy(sheet);

  /* Helper to test values */
#define TEST_SPATIAL_NAV(a_str, a_val, c_str, c_val, f_str, f_val)             \
  do {                                                                         \
    struct ui_css_stylesheet *sh;                                              \
    struct ui_css_rule *r;                                                     \
    struct ui_css_computed_style *s;                                           \
    struct ui_css_spatial_nav_properties p;                                    \
    ui_css_stylesheet_create(&sh);                                             \
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r);                            \
    ui_css_rule_append_selector(r, UI_CSS_SELECTOR_TYPE_TAG, "div");           \
    if (a_str)                                                                 \
      ui_css_rule_append_declaration(r, "spatial-navigation-action", a_str,    \
                                     0);                                       \
    if (c_str)                                                                 \
      ui_css_rule_append_declaration(r, "spatial-navigation-contain", c_str,   \
                                     0);                                       \
    if (f_str)                                                                 \
      ui_css_rule_append_declaration(r, "spatial-navigation-function", f_str,  \
                                     0);                                       \
    ui_css_stylesheet_append_rule(sh, r);                                      \
    ui_css_resolve_style(sh, node, &s);                                        \
    TEST_ASSERT(ui_css_spatial_nav_parse(s, &p) == UI_ERROR_NONE);             \
    TEST_ASSERT(p.action == a_val);                                            \
    TEST_ASSERT(p.contain == c_val);                                           \
    TEST_ASSERT(p.function == f_val);                                          \
    ui_css_computed_style_destroy(s);                                          \
    ui_css_stylesheet_destroy(sh);                                             \
  } while (0)

  /* Actions */
  TEST_SPATIAL_NAV("auto", UI_CSS_SPATIAL_NAV_ACTION_AUTO, NULL,
                   UI_CSS_SPATIAL_NAV_CONTAIN_AUTO, NULL,
                   UI_CSS_SPATIAL_NAV_FUNCTION_NORMAL);
  TEST_SPATIAL_NAV("focus", UI_CSS_SPATIAL_NAV_ACTION_FOCUS, NULL,
                   UI_CSS_SPATIAL_NAV_CONTAIN_AUTO, NULL,
                   UI_CSS_SPATIAL_NAV_FUNCTION_NORMAL);
  TEST_SPATIAL_NAV("scroll", UI_CSS_SPATIAL_NAV_ACTION_SCROLL, NULL,
                   UI_CSS_SPATIAL_NAV_CONTAIN_AUTO, NULL,
                   UI_CSS_SPATIAL_NAV_FUNCTION_NORMAL);
  TEST_SPATIAL_NAV("invalid", UI_CSS_SPATIAL_NAV_ACTION_AUTO, NULL,
                   UI_CSS_SPATIAL_NAV_CONTAIN_AUTO, NULL,
                   UI_CSS_SPATIAL_NAV_FUNCTION_NORMAL);

  /* Contain */
  TEST_SPATIAL_NAV(NULL, UI_CSS_SPATIAL_NAV_ACTION_AUTO, "auto",
                   UI_CSS_SPATIAL_NAV_CONTAIN_AUTO, NULL,
                   UI_CSS_SPATIAL_NAV_FUNCTION_NORMAL);
  TEST_SPATIAL_NAV(NULL, UI_CSS_SPATIAL_NAV_ACTION_AUTO, "contain",
                   UI_CSS_SPATIAL_NAV_CONTAIN_CONTAIN, NULL,
                   UI_CSS_SPATIAL_NAV_FUNCTION_NORMAL);
  TEST_SPATIAL_NAV(NULL, UI_CSS_SPATIAL_NAV_ACTION_AUTO, "invalid",
                   UI_CSS_SPATIAL_NAV_CONTAIN_AUTO, NULL,
                   UI_CSS_SPATIAL_NAV_FUNCTION_NORMAL);

  /* Function */
  TEST_SPATIAL_NAV(NULL, UI_CSS_SPATIAL_NAV_ACTION_AUTO, NULL,
                   UI_CSS_SPATIAL_NAV_CONTAIN_AUTO, "normal",
                   UI_CSS_SPATIAL_NAV_FUNCTION_NORMAL);
  TEST_SPATIAL_NAV(NULL, UI_CSS_SPATIAL_NAV_ACTION_AUTO, NULL,
                   UI_CSS_SPATIAL_NAV_CONTAIN_AUTO, "grid",
                   UI_CSS_SPATIAL_NAV_FUNCTION_GRID);
  TEST_SPATIAL_NAV(NULL, UI_CSS_SPATIAL_NAV_ACTION_AUTO, NULL,
                   UI_CSS_SPATIAL_NAV_CONTAIN_AUTO, "invalid",
                   UI_CSS_SPATIAL_NAV_FUNCTION_NORMAL);

  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(node);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
}

int main(void) {
  test_spatial_nav_cases();
  printf("test_ui_css_spatial_nav passed.\n");
  return 0;
}
