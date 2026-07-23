/* clang-format off */
#include "../include/ui_css_scroll_snap.h"
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

static void test_scroll_snap_cases(void) {
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_css_rule *rule = NULL;
  struct ui_dom_node *node = NULL;
  struct ui_css_computed_style *style = NULL;
  struct ui_css_scroll_snap_properties props;

  /* Null arguments */
  TEST_ASSERT(ui_css_scroll_snap_parse(NULL, &props) ==
              UI_ERROR_INVALID_ARGUMENT);

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);

  /* long length truncation */
  char long_str[200];
  memset(long_str, 'a', 190);
  long_str[190] = '\0';
  ui_css_rule_append_declaration(rule, "scroll-snap-type", long_str, 0);
  ui_css_rule_append_declaration(rule, "scroll-snap-align", long_str, 0);
  ui_css_rule_append_declaration(rule, "scroll-margin", long_str, 0);

  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_stylesheet_append_rule(sheet, rule);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  ui_dom_node_set_tag_name(node, "div");
  ui_css_resolve_style(sheet, node, &style);

  TEST_ASSERT(ui_css_scroll_snap_parse(style, NULL) ==
              UI_ERROR_INVALID_ARGUMENT);

  /* Empty style -> defaults */
  TEST_ASSERT(ui_css_scroll_snap_parse(style, &props) == UI_ERROR_NONE);
  TEST_ASSERT(props.type.axis == UI_CSS_SCROLL_SNAP_AXIS_NONE);
  TEST_ASSERT(props.type.strictness == UI_CSS_SCROLL_SNAP_STRICTNESS_NONE);
  TEST_ASSERT(props.align.block == UI_CSS_SCROLL_SNAP_ALIGN_NONE);
  TEST_ASSERT(props.align.inline_axis == UI_CSS_SCROLL_SNAP_ALIGN_NONE);
  TEST_ASSERT(props.stop == UI_CSS_SCROLL_SNAP_STOP_NORMAL);
  TEST_ASSERT(props.padding.top.value == 0.0f);
  TEST_ASSERT(props.margin.top.value == 0.0f);

  ui_css_computed_style_destroy(style);
  ui_css_stylesheet_destroy(sheet);

  /* Helper to test values */
#define TEST_SCROLL_SNAP(decls, chk)                                           \
  do {                                                                         \
    struct ui_css_stylesheet *sh;                                              \
    struct ui_css_rule *r;                                                     \
    struct ui_css_computed_style *s;                                           \
    struct ui_css_scroll_snap_properties p;                                    \
    ui_css_stylesheet_create(&sh);                                             \
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r);                            \
    ui_css_rule_append_selector(r, UI_CSS_SELECTOR_TYPE_TAG, "div");           \
    decls ui_css_stylesheet_append_rule(sh, r);                                \
    ui_css_resolve_style(sh, node, &s);                                        \
    TEST_ASSERT(ui_css_scroll_snap_parse(s, &p) == UI_ERROR_NONE);             \
    chk ui_css_computed_style_destroy(s);                                      \
    ui_css_stylesheet_destroy(sh);                                             \
  } while (0)

  /* snap-type: none */
  TEST_SCROLL_SNAP(
      { ui_css_rule_append_declaration(r, "scroll-snap-type", "none", 0); },
      {
        TEST_ASSERT(p.type.axis == UI_CSS_SCROLL_SNAP_AXIS_NONE);
        TEST_ASSERT(p.type.strictness == UI_CSS_SCROLL_SNAP_STRICTNESS_NONE);
      });

  /* snap-type: x mandatory */
  TEST_SCROLL_SNAP(
      {
        ui_css_rule_append_declaration(r, "scroll-snap-type", "x mandatory", 0);
      },
      {
        TEST_ASSERT(p.type.axis == UI_CSS_SCROLL_SNAP_AXIS_X);
        TEST_ASSERT(p.type.strictness ==
                    UI_CSS_SCROLL_SNAP_STRICTNESS_MANDATORY);
      });

  /* snap-type: y proximity */
  TEST_SCROLL_SNAP(
      {
        ui_css_rule_append_declaration(r, "scroll-snap-type", "y proximity", 0);
      },
      {
        TEST_ASSERT(p.type.axis == UI_CSS_SCROLL_SNAP_AXIS_Y);
        TEST_ASSERT(p.type.strictness ==
                    UI_CSS_SCROLL_SNAP_STRICTNESS_PROXIMITY);
      });

  /* snap-type: x invalid */
  TEST_SCROLL_SNAP(
      {
        ui_css_rule_append_declaration(r, "scroll-snap-type", "x invalid", 0);
      },
      {
        TEST_ASSERT(p.type.axis == UI_CSS_SCROLL_SNAP_AXIS_X);
        TEST_ASSERT(p.type.strictness == UI_CSS_SCROLL_SNAP_STRICTNESS_NONE);
      });

  /* snap-type: block */
  TEST_SCROLL_SNAP(
      { ui_css_rule_append_declaration(r, "scroll-snap-type", "block", 0); },
      {
        TEST_ASSERT(p.type.axis == UI_CSS_SCROLL_SNAP_AXIS_BLOCK);
        TEST_ASSERT(p.type.strictness ==
                    UI_CSS_SCROLL_SNAP_STRICTNESS_PROXIMITY); /* default */
      });

  /* snap-type: inline */
  TEST_SCROLL_SNAP(
      { ui_css_rule_append_declaration(r, "scroll-snap-type", "inline", 0); },
      { TEST_ASSERT(p.type.axis == UI_CSS_SCROLL_SNAP_AXIS_INLINE); });

  /* snap-type: both */
  TEST_SCROLL_SNAP(
      { ui_css_rule_append_declaration(r, "scroll-snap-type", "both", 0); },
      { TEST_ASSERT(p.type.axis == UI_CSS_SCROLL_SNAP_AXIS_BOTH); });

  /* snap-align: none */
  TEST_SCROLL_SNAP(
      { ui_css_rule_append_declaration(r, "scroll-snap-align", "none", 0); },
      {
        TEST_ASSERT(p.align.block == UI_CSS_SCROLL_SNAP_ALIGN_NONE);
        TEST_ASSERT(p.align.inline_axis == UI_CSS_SCROLL_SNAP_ALIGN_NONE);
      });

  /* snap-align: start center */
  TEST_SCROLL_SNAP(
      {
        ui_css_rule_append_declaration(r, "scroll-snap-align", "start center",
                                       0);
      },
      {
        TEST_ASSERT(p.align.block == UI_CSS_SCROLL_SNAP_ALIGN_START);
        TEST_ASSERT(p.align.inline_axis == UI_CSS_SCROLL_SNAP_ALIGN_CENTER);
      });

  /* snap-align: end */
  TEST_SCROLL_SNAP(
      { ui_css_rule_append_declaration(r, "scroll-snap-align", "end", 0); },
      {
        TEST_ASSERT(p.align.block == UI_CSS_SCROLL_SNAP_ALIGN_END);
        TEST_ASSERT(p.align.inline_axis == UI_CSS_SCROLL_SNAP_ALIGN_END);
      });

  /* snap-stop: normal */
  TEST_SCROLL_SNAP(
      { ui_css_rule_append_declaration(r, "scroll-snap-stop", "normal", 0); },
      { TEST_ASSERT(p.stop == UI_CSS_SCROLL_SNAP_STOP_NORMAL); });

  /* snap-stop: always */
  TEST_SCROLL_SNAP(
      { ui_css_rule_append_declaration(r, "scroll-snap-stop", "always", 0); },
      { TEST_ASSERT(p.stop == UI_CSS_SCROLL_SNAP_STOP_ALWAYS); });

  /* scroll-padding shorthand: 1 value */
  TEST_SCROLL_SNAP(
      { ui_css_rule_append_declaration(r, "scroll-padding", "10px", 0); },
      {
        TEST_ASSERT(p.padding.top.value == 10.0f);
        TEST_ASSERT(p.padding.right.value == 10.0f);
        TEST_ASSERT(p.padding.bottom.value == 10.0f);
        TEST_ASSERT(p.padding.left.value == 10.0f);
      });

  /* scroll-padding shorthand: 2 values */
  TEST_SCROLL_SNAP(
      { ui_css_rule_append_declaration(r, "scroll-padding", "10px 20px", 0); },
      {
        TEST_ASSERT(p.padding.top.value == 10.0f);
        TEST_ASSERT(p.padding.right.value == 20.0f);
        TEST_ASSERT(p.padding.bottom.value == 10.0f);
        TEST_ASSERT(p.padding.left.value == 20.0f);
      });

  /* scroll-padding shorthand: 3 values */
  TEST_SCROLL_SNAP(
      {
        ui_css_rule_append_declaration(r, "scroll-padding", "10px 20px 30px",
                                       0);
      },
      {
        TEST_ASSERT(p.padding.top.value == 10.0f);
        TEST_ASSERT(p.padding.right.value == 20.0f);
        TEST_ASSERT(p.padding.bottom.value == 30.0f);
        TEST_ASSERT(p.padding.left.value == 20.0f);
      });

  /* scroll-padding shorthand: 4 values */
  TEST_SCROLL_SNAP(
      {
        ui_css_rule_append_declaration(r, "scroll-padding",
                                       "10px 20px 30px 40px", 0);
      },
      {
        TEST_ASSERT(p.padding.top.value == 10.0f);
        TEST_ASSERT(p.padding.right.value == 20.0f);
        TEST_ASSERT(p.padding.bottom.value == 30.0f);
        TEST_ASSERT(p.padding.left.value == 40.0f);
      });

  /* scroll-padding shorthand: 5 values (tests count < 4 branch) */
  TEST_SCROLL_SNAP(
      {
        ui_css_rule_append_declaration(r, "scroll-padding",
                                       "10px 20px 30px 40px 50px", 0);
      },
      {
        TEST_ASSERT(p.padding.top.value == 10.0f);
        TEST_ASSERT(p.padding.right.value == 20.0f);
        TEST_ASSERT(p.padding.bottom.value == 30.0f);
        TEST_ASSERT(p.padding.left.value == 40.0f);
      });

  /* Invalid scroll-padding */
  TEST_SCROLL_SNAP(
      { ui_css_rule_append_declaration(r, "scroll-padding", "invalid", 0); },
      { TEST_ASSERT(p.padding.top.value == 0.0f); });

  /* scroll-margin shorthand: 2 values */
  TEST_SCROLL_SNAP(
      { ui_css_rule_append_declaration(r, "scroll-margin", "5px 15px", 0); },
      {
        TEST_ASSERT(p.margin.top.value == 5.0f);
        TEST_ASSERT(p.margin.right.value == 15.0f);
        TEST_ASSERT(p.margin.bottom.value == 5.0f);
        TEST_ASSERT(p.margin.left.value == 15.0f);
      });

  /* scroll-margin longhands */
  TEST_SCROLL_SNAP(
      {
        ui_css_rule_append_declaration(r, "scroll-margin-top", "1px", 0);
        ui_css_rule_append_declaration(r, "scroll-margin-right", "2px", 0);
        ui_css_rule_append_declaration(r, "scroll-margin-bottom", "3px", 0);
        ui_css_rule_append_declaration(r, "scroll-margin-left", "4px", 0);
      },
      {
        TEST_ASSERT(p.margin.top.value == 1.0f);
        TEST_ASSERT(p.margin.right.value == 2.0f);
        TEST_ASSERT(p.margin.bottom.value == 3.0f);
        TEST_ASSERT(p.margin.left.value == 4.0f);
      });

  /* scroll-padding longhands */
  TEST_SCROLL_SNAP(
      {
        ui_css_rule_append_declaration(r, "scroll-padding-top", "5px", 0);
        ui_css_rule_append_declaration(r, "scroll-padding-right", "6px", 0);
        ui_css_rule_append_declaration(r, "scroll-padding-bottom", "7px", 0);
        ui_css_rule_append_declaration(r, "scroll-padding-left", "8px", 0);
      },
      {
        TEST_ASSERT(p.padding.top.value == 5.0f);
        TEST_ASSERT(p.padding.right.value == 6.0f);
        TEST_ASSERT(p.padding.bottom.value == 7.0f);
        TEST_ASSERT(p.padding.left.value == 8.0f);
      });

  /* Empty strings to trigger token == NULL on first call */
  TEST_SCROLL_SNAP(
      {
        ui_css_rule_append_declaration(r, "scroll-snap-type", "   ", 0);
        ui_css_rule_append_declaration(r, "scroll-snap-align", "   ", 0);
        ui_css_rule_append_declaration(r, "scroll-padding", "   ", 0);
      },
      {
        TEST_ASSERT(p.type.axis == UI_CSS_SCROLL_SNAP_AXIS_NONE);
        TEST_ASSERT(p.align.block == UI_CSS_SCROLL_SNAP_ALIGN_NONE);
        TEST_ASSERT(p.padding.top.value == 0.0f);
      });

  ui_dom_node_destroy(node);
}

int main(void) {
  test_scroll_snap_cases();
  printf("test_ui_css_scroll_snap passed.\n");
  return 0;
}
