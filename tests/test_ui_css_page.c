/* clang-format off */
#include "../include/ui_css_page.h"
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

static void test_page_parse_cases(void) {
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_css_rule *rule = NULL;
  struct ui_dom_node *node = NULL;
  struct ui_css_computed_style *style = NULL;
  struct ui_css_page_properties props;
  enum ui_error rc;

  /* Test invalid arguments */
  rc = ui_css_page_parse(NULL, &props);
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
  rc = ui_css_page_parse(style, NULL);
  TEST_ASSERT(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Test empty style (defaults) */
  rc = ui_css_page_parse(style, &props);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  TEST_ASSERT(props.size == UI_CSS_PAGE_SIZE_AUTO);
  TEST_ASSERT(props.marks == UI_CSS_PAGE_MARKS_NONE);
  TEST_ASSERT(props.page_name == NULL);
  TEST_ASSERT(props.bleed_is_auto == 1);
  ui_css_page_properties_cleanup(&props);
  ui_css_computed_style_destroy(style);
  style = NULL;
  ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* Helper to test a set of properties */
#define TEST_PAGE_PROPS(sz_str, sz_val, mk_str, mk_val, pg_str, bld_str,       \
                        bld_auto)                                              \
  do {                                                                         \
    struct ui_css_stylesheet *sh;                                              \
    struct ui_css_rule *r;                                                     \
    struct ui_css_computed_style *s;                                           \
    struct ui_css_page_properties p;                                           \
    ui_css_stylesheet_create(&sh);                                             \
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r);                            \
    ui_css_rule_append_selector(r, UI_CSS_SELECTOR_TYPE_TAG, "div");           \
    if (sz_str)                                                                \
      ui_css_rule_append_declaration(r, "size", sz_str, 0);                    \
    if (mk_str)                                                                \
      ui_css_rule_append_declaration(r, "marks", mk_str, 0);                   \
    if (pg_str)                                                                \
      ui_css_rule_append_declaration(r, "page", pg_str, 0);                    \
    if (bld_str)                                                               \
      ui_css_rule_append_declaration(r, "bleed", bld_str, 0);                  \
    ui_css_stylesheet_append_rule(sh, r);                                      \
    ui_css_resolve_style(sh, node, &s);                                        \
    ui_css_page_parse(s, &p);                                                  \
    TEST_ASSERT(p.size == sz_val);                                             \
    TEST_ASSERT(p.marks == mk_val);                                            \
    if (pg_str && strcmp(pg_str, "auto") != 0) {                               \
      TEST_ASSERT(p.page_name != NULL &&                                       \
                  strcmp(p.page_name, pg_str ? pg_str : "") == 0);             \
    } else {                                                                   \
      TEST_ASSERT(p.page_name == NULL);                                        \
    }                                                                          \
    TEST_ASSERT(p.bleed_is_auto == bld_auto);                                  \
    ui_css_page_properties_cleanup(&p);                                        \
    ui_css_computed_style_destroy(s);                                          \
    ui_css_stylesheet_destroy(sh);                                             \
  } while (0)

  /* Sizes */
  TEST_PAGE_PROPS("portrait", UI_CSS_PAGE_SIZE_PORTRAIT, NULL,
                  UI_CSS_PAGE_MARKS_NONE, NULL, NULL, 1);
  TEST_PAGE_PROPS("landscape", UI_CSS_PAGE_SIZE_LANDSCAPE, NULL,
                  UI_CSS_PAGE_MARKS_NONE, NULL, NULL, 1);
  TEST_PAGE_PROPS("a4", UI_CSS_PAGE_SIZE_A4, NULL, UI_CSS_PAGE_MARKS_NONE, NULL,
                  NULL, 1);
  TEST_PAGE_PROPS("a5", UI_CSS_PAGE_SIZE_A5, NULL, UI_CSS_PAGE_MARKS_NONE, NULL,
                  NULL, 1);
  TEST_PAGE_PROPS("a3", UI_CSS_PAGE_SIZE_A3, NULL, UI_CSS_PAGE_MARKS_NONE, NULL,
                  NULL, 1);
  TEST_PAGE_PROPS("b5", UI_CSS_PAGE_SIZE_B5, NULL, UI_CSS_PAGE_MARKS_NONE, NULL,
                  NULL, 1);
  TEST_PAGE_PROPS("b4", UI_CSS_PAGE_SIZE_B4, NULL, UI_CSS_PAGE_MARKS_NONE, NULL,
                  NULL, 1);
  TEST_PAGE_PROPS("jis-b5", UI_CSS_PAGE_SIZE_JIS_B5, NULL,
                  UI_CSS_PAGE_MARKS_NONE, NULL, NULL, 1);
  TEST_PAGE_PROPS("jis-b4", UI_CSS_PAGE_SIZE_JIS_B4, NULL,
                  UI_CSS_PAGE_MARKS_NONE, NULL, NULL, 1);
  TEST_PAGE_PROPS("letter", UI_CSS_PAGE_SIZE_LETTER, NULL,
                  UI_CSS_PAGE_MARKS_NONE, NULL, NULL, 1);
  TEST_PAGE_PROPS("legal", UI_CSS_PAGE_SIZE_LEGAL, NULL, UI_CSS_PAGE_MARKS_NONE,
                  NULL, NULL, 1);
  TEST_PAGE_PROPS("ledger", UI_CSS_PAGE_SIZE_LEDGER, NULL,
                  UI_CSS_PAGE_MARKS_NONE, NULL, NULL, 1);
  TEST_PAGE_PROPS("unknown", UI_CSS_PAGE_SIZE_AUTO, NULL,
                  UI_CSS_PAGE_MARKS_NONE, NULL, NULL, 1);
  TEST_PAGE_PROPS("auto", UI_CSS_PAGE_SIZE_AUTO, NULL, UI_CSS_PAGE_MARKS_NONE,
                  NULL, NULL, 1);

  /* Marks */
  TEST_PAGE_PROPS(NULL, UI_CSS_PAGE_SIZE_AUTO, "crop cross",
                  UI_CSS_PAGE_MARKS_BOTH, NULL, NULL, 1);
  TEST_PAGE_PROPS(NULL, UI_CSS_PAGE_SIZE_AUTO, "cross crop",
                  UI_CSS_PAGE_MARKS_BOTH, NULL, NULL, 1);
  TEST_PAGE_PROPS(NULL, UI_CSS_PAGE_SIZE_AUTO, "crop", UI_CSS_PAGE_MARKS_CROP,
                  NULL, NULL, 1);
  TEST_PAGE_PROPS(NULL, UI_CSS_PAGE_SIZE_AUTO, "cross", UI_CSS_PAGE_MARKS_CROSS,
                  NULL, NULL, 1);
  TEST_PAGE_PROPS(NULL, UI_CSS_PAGE_SIZE_AUTO, "none", UI_CSS_PAGE_MARKS_NONE,
                  NULL, NULL, 1);
  TEST_PAGE_PROPS(NULL, UI_CSS_PAGE_SIZE_AUTO, "unknown",
                  UI_CSS_PAGE_MARKS_NONE, NULL, NULL, 1);

  /* Page */
  TEST_PAGE_PROPS(NULL, UI_CSS_PAGE_SIZE_AUTO, NULL, UI_CSS_PAGE_MARKS_NONE,
                  "auto", NULL, 1);
  TEST_PAGE_PROPS(NULL, UI_CSS_PAGE_SIZE_AUTO, NULL, UI_CSS_PAGE_MARKS_NONE,
                  "custom-page", NULL, 1);

  /* Bleed */
  TEST_PAGE_PROPS(NULL, UI_CSS_PAGE_SIZE_AUTO, NULL, UI_CSS_PAGE_MARKS_NONE,
                  NULL, "auto", 1);
  TEST_PAGE_PROPS(NULL, UI_CSS_PAGE_SIZE_AUTO, NULL, UI_CSS_PAGE_MARKS_NONE,
                  NULL, "10", 0);
  TEST_PAGE_PROPS(NULL, UI_CSS_PAGE_SIZE_AUTO, NULL, UI_CSS_PAGE_MARKS_NONE,
                  NULL, "20.5", 0);

  /* OOM for page name */
  {
    struct ui_css_stylesheet *sh;
    struct ui_css_rule *r;
    struct ui_css_computed_style *s;
    struct ui_css_page_properties p;
    ui_css_stylesheet_create(&sh);
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r);
    ui_css_rule_append_selector(r, UI_CSS_SELECTOR_TYPE_TAG, "div");
    ui_css_rule_append_declaration(r, "page", "custom-page", 0);
    ui_css_stylesheet_append_rule(sh, r);
    ui_css_resolve_style(sh, node, &s);

    extern int g_malloc_fail_countdown;
    g_malloc_fail_countdown = 0;
    rc = ui_css_page_parse(s, &p);
    g_malloc_fail_countdown = -1;
    TEST_ASSERT(rc == UI_ERROR_OUT_OF_MEMORY);

    ui_css_computed_style_destroy(s);
    ui_css_stylesheet_destroy(sh);
  }

  /* ui_css_page_properties_cleanup with NULL properties or already freed name
   */
  ui_css_page_properties_cleanup(NULL);

  /* Already freed */
  {
    struct ui_css_page_properties p;
    p.page_name = NULL;
    ui_css_page_properties_cleanup(&p);
  }

  ui_dom_node_destroy(node);
}

int main(void) {
  test_page_parse_cases();
  printf("test_ui_css_page passed.\n");
  return 0;
}
