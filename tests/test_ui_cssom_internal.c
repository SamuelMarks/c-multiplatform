/* clang-format off */
#include "../src/ui_cssom.c"
#include "../src/ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

#ifndef TEST_ASSERT
#define TEST_ASSERT(cond)                                                      \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "Assert failed: %s at %d\n", #cond, __LINE__);           \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)
#endif

static void test_cascade_internal(void) {
  struct ui_css_computed_style *style;
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;

  /* Insert a dummy property to be the current one */
  TEST_ASSERT(append_computed_declaration(style, "color", "red", 0, 0, 1, 2, 3,
                                          10) == UI_ERROR_NONE);

  /* Now test all the cascade branches against it! */
  /* new has higher important */
  TEST_ASSERT(append_computed_declaration(style, "color", "red2", 1, 0, 1, 2, 3,
                                          10) == UI_ERROR_NONE);

  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  TEST_ASSERT(append_computed_declaration(style, "color", "red", 1, 0, 1, 2, 3,
                                          10) == UI_ERROR_NONE);
  TEST_ASSERT(append_computed_declaration(style, "color", "red2", 0, 0, 1, 2, 3,
                                          10) == UI_ERROR_NONE);

  /* layer order */
  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  TEST_ASSERT(append_computed_declaration(style, "color", "red", 0, 1, 1, 2, 3,
                                          10) == UI_ERROR_NONE);
  TEST_ASSERT(append_computed_declaration(style, "color", "red2", 0, 2, 1, 2, 3,
                                          10) == UI_ERROR_NONE);
  TEST_ASSERT(append_computed_declaration(style, "color", "red3", 0, 0, 1, 2, 3,
                                          10) == UI_ERROR_NONE);

  /* layer order important inverted */
  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  TEST_ASSERT(append_computed_declaration(style, "color", "red", 1, 1, 1, 2, 3,
                                          10) == UI_ERROR_NONE);
  TEST_ASSERT(append_computed_declaration(style, "color", "red2", 1, 2, 1, 2, 3,
                                          10) == UI_ERROR_NONE);
  TEST_ASSERT(append_computed_declaration(style, "color", "red3", 1, 0, 1, 2, 3,
                                          10) == UI_ERROR_NONE);

  /* spec a */
  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  TEST_ASSERT(append_computed_declaration(style, "color", "red", 0, 0, 2, 2, 3,
                                          10) == UI_ERROR_NONE);
  TEST_ASSERT(append_computed_declaration(style, "color", "red2", 0, 0, 3, 2, 3,
                                          10) == UI_ERROR_NONE);
  TEST_ASSERT(append_computed_declaration(style, "color", "red3", 0, 0, 1, 2, 3,
                                          10) == UI_ERROR_NONE);

  /* spec b */
  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  TEST_ASSERT(append_computed_declaration(style, "color", "red", 0, 0, 1, 2, 3,
                                          10) == UI_ERROR_NONE);
  TEST_ASSERT(append_computed_declaration(style, "color", "red2", 0, 0, 1, 3, 3,
                                          10) == UI_ERROR_NONE);
  TEST_ASSERT(append_computed_declaration(style, "color", "red3", 0, 0, 1, 1, 3,
                                          10) == UI_ERROR_NONE);

  /* spec c */
  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  TEST_ASSERT(append_computed_declaration(style, "color", "red", 0, 0, 1, 1, 3,
                                          10) == UI_ERROR_NONE);
  TEST_ASSERT(append_computed_declaration(style, "color", "red2", 0, 0, 1, 1, 4,
                                          10) == UI_ERROR_NONE);
  TEST_ASSERT(append_computed_declaration(style, "color", "red3", 0, 0, 1, 1, 2,
                                          10) == UI_ERROR_NONE);

  /* source order */
  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  TEST_ASSERT(append_computed_declaration(style, "color", "red", 0, 0, 1, 1, 1,
                                          10) == UI_ERROR_NONE);
  TEST_ASSERT(append_computed_declaration(style, "color", "red2", 0, 0, 1, 1, 1,
                                          11) == UI_ERROR_NONE);
  TEST_ASSERT(append_computed_declaration(style, "color", "red3", 0, 0, 1, 1, 1,
                                          9) == UI_ERROR_NONE);

  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
  }
}

static void test_node_has_class_whitespace(void) {
  int matched = 0;
  ui_error_t rc;

  rc = class_list_contains("\t\r\nfoo\t\r\nbar\t\r\n", "foo", &matched);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  TEST_ASSERT(matched == 1);
  rc = class_list_contains("\t\r\nfoo\t\r\nbar\t\r\n", "bar", &matched);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  TEST_ASSERT(matched == 1);
  rc = class_list_contains("\t\r\nfoo\t\r\nbar\t\r\n", "baz", &matched);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  TEST_ASSERT(matched == 0);
}

static void test_cond_is_word_whitespace(void) {
  int matched = 0;
  ui_error_t rc;

  rc = cond_is_word("word\t", "word", &matched);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  TEST_ASSERT(matched == 1);
  rc = cond_is_word("word\r", "word", &matched);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  TEST_ASSERT(matched == 1);
  rc = cond_is_word("word\n", "word", &matched);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  TEST_ASSERT(matched == 1);
}

static void test_specificity_extra(void) {
  struct ui_css_selector sel_attr;
  struct ui_css_selector sel_pe;
  int a, b, c;
  ui_error_t rc;

  memset(&sel_attr, 0, sizeof(sel_attr));
  sel_attr.type = UI_CSS_SELECTOR_TYPE_ATTRIBUTE;
  rc = get_selector_specificity(&sel_attr, &a, &b, &c);
  TEST_ASSERT(rc == UI_ERROR_NONE);

  memset(&sel_pe, 0, sizeof(sel_pe));
  sel_pe.type = UI_CSS_SELECTOR_TYPE_PSEUDO_ELEMENT;
  rc = get_selector_specificity(&sel_pe, &a, &b, &c);
  TEST_ASSERT(rc == UI_ERROR_NONE);
}

static void test_style_rule_selector_specificity_tie(void) {
  struct ui_dom_node *node = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_css_rule *rule = NULL;
  struct ui_css_computed_style *style = NULL;
  ui_error_t rc;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_dom_node_set_tag_name(node, "div");
  TEST_ASSERT(rc == UI_ERROR_NONE);

  rc = ui_css_stylesheet_create(&sheet);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_rule_append_declaration(rule, "color", "blue", 0);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_stylesheet_append_rule(sheet, rule);
  TEST_ASSERT(rc == UI_ERROR_NONE);

  rc = ui_css_resolve_style(sheet, node, &style);
  TEST_ASSERT(rc == UI_ERROR_NONE);

  if (style) {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
  }
  if (sheet) {
    ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
  }
  if (node) {
    ui_error_t rc_cleanup = ui_dom_node_destroy(node);
    TEST_ASSERT(rc_cleanup == UI_ERROR_NONE);
  }
}

static void test_supports_mock_branches(void) {
  int res = 1;
  ui_error_t rc;

  rc = eval_supports_condition("(display: grid)", &res);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  TEST_ASSERT(res == 0);

  rc = eval_supports_condition("selector(:invalid)", &res);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  TEST_ASSERT(res == 0);
}

int main(void) {
  test_cascade_internal();
  test_node_has_class_whitespace();
  test_cond_is_word_whitespace();
  test_specificity_extra();
  test_style_rule_selector_specificity_tie();
  test_supports_mock_branches();
  return 0;
}
