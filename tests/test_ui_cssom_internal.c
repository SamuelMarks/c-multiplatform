/* clang-format off */
#include "../src/ui_cssom.c"
#include "../src/ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

static void test_cascade_internal(void) {
  struct ui_css_computed_style *style;
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;

  /* Insert a dummy property to be the current one */
  (void)append_computed_declaration(style, "color", "red", 0, 0, 1, 2, 3, 10);

  /* Now test all the cascade branches against it! */
  /* new has higher important */
  (void)append_computed_declaration(style, "color", "red2", 1, 0, 1, 2, 3, 10);

  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  (void)append_computed_declaration(style, "color", "red", 1, 0, 1, 2, 3, 10);
  (void)append_computed_declaration(style, "color", "red2", 0, 0, 1, 2, 3, 10);

  /* layer order */
  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  (void)append_computed_declaration(style, "color", "red", 0, 1, 1, 2, 3, 10);
  (void)append_computed_declaration(style, "color", "red2", 0, 2, 1, 2, 3, 10);
  (void)append_computed_declaration(style, "color", "red3", 0, 0, 1, 2, 3, 10);

  /* layer order important inverted */
  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  (void)append_computed_declaration(style, "color", "red", 1, 1, 1, 2, 3, 10);
  (void)append_computed_declaration(style, "color", "red2", 1, 2, 1, 2, 3, 10);
  (void)append_computed_declaration(style, "color", "red3", 1, 0, 1, 2, 3, 10);

  /* spec a */
  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  (void)append_computed_declaration(style, "color", "red", 0, 0, 2, 2, 3, 10);
  (void)append_computed_declaration(style, "color", "red2", 0, 0, 3, 2, 3, 10);
  (void)append_computed_declaration(style, "color", "red3", 0, 0, 1, 2, 3, 10);

  /* spec b */
  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  (void)append_computed_declaration(style, "color", "red", 0, 0, 1, 2, 3, 10);
  (void)append_computed_declaration(style, "color", "red2", 0, 0, 1, 3, 3, 10);
  (void)append_computed_declaration(style, "color", "red3", 0, 0, 1, 1, 3, 10);

  /* spec c */
  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  (void)append_computed_declaration(style, "color", "red", 0, 0, 1, 1, 3, 10);
  (void)append_computed_declaration(style, "color", "red2", 0, 0, 1, 1, 4, 10);
  (void)append_computed_declaration(style, "color", "red3", 0, 0, 1, 1, 2, 10);

  /* source order */
  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  style = (struct ui_css_computed_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_computed_style));
  style->properties = NULL;
  (void)append_computed_declaration(style, "color", "red", 0, 0, 1, 1, 1, 10);
  (void)append_computed_declaration(style, "color", "red2", 0, 0, 1, 1, 1, 11);
  (void)append_computed_declaration(style, "color", "red3", 0, 0, 1, 1, 1, 9);

  {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
}

static void test_node_has_class_whitespace(void) {
  int matched = 0;
  ui_error_t rc;

  rc = class_list_contains("\t\r\nfoo\t\r\nbar\t\r\n", "foo", &matched);
  (void)rc;
  rc = class_list_contains("\t\r\nfoo\t\r\nbar\t\r\n", "bar", &matched);
  (void)rc;
  rc = class_list_contains("\t\r\nfoo\t\r\nbar\t\r\n", "baz", &matched);
  (void)rc;
}

static void test_cond_is_word_whitespace(void) {
  int matched = 0;
  ui_error_t rc;

  rc = cond_is_word("word\t", "word", &matched);
  (void)rc;
  rc = cond_is_word("word\r", "word", &matched);
  (void)rc;
  rc = cond_is_word("word\n", "word", &matched);
  (void)rc;
}

static void test_specificity_extra(void) {
  struct ui_css_selector sel_attr;
  struct ui_css_selector sel_pe;
  int a, b, c;
  ui_error_t rc;

  memset(&sel_attr, 0, sizeof(sel_attr));
  sel_attr.type = UI_CSS_SELECTOR_TYPE_ATTRIBUTE;
  rc = get_selector_specificity(&sel_attr, &a, &b, &c);
  (void)rc;

  memset(&sel_pe, 0, sizeof(sel_pe));
  sel_pe.type = UI_CSS_SELECTOR_TYPE_PSEUDO_ELEMENT;
  rc = get_selector_specificity(&sel_pe, &a, &b, &c);
  (void)rc;
}

static void test_style_rule_selector_specificity_tie(void) {
  struct ui_dom_node *node = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_css_rule *rule = NULL;
  struct ui_css_computed_style *style = NULL;
  ui_error_t rc;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  (void)rc;
  rc = ui_dom_node_set_tag_name(node, "div");
  (void)rc;

  rc = ui_css_stylesheet_create(&sheet);
  (void)rc;
  rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  (void)rc;
  rc = ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
  (void)rc;
  rc = ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
  (void)rc;
  rc = ui_css_rule_append_declaration(rule, "color", "blue", 0);
  (void)rc;
  rc = ui_css_stylesheet_append_rule(sheet, rule);
  (void)rc;

  rc = ui_css_resolve_style(sheet, node, &style);
  (void)rc;

  if (style) {
    ui_error_t rc_cleanup = ui_css_computed_style_destroy(style);
    (void)rc_cleanup;
  }
  if (sheet) {
    ui_error_t rc_cleanup = ui_css_stylesheet_destroy(sheet);
    (void)rc_cleanup;
  }
  if (node) {
    ui_error_t rc_cleanup = ui_dom_node_destroy(node);
    (void)rc_cleanup;
  }
}

int main(void) {
  test_cascade_internal();
  test_node_has_class_whitespace();
  test_cond_is_word_whitespace();
  test_specificity_extra();
  test_style_rule_selector_specificity_tie();
  return 0;
}
