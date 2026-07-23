/* clang-format off */
#include "../include/ui_cssom.h"
#include "../include/ui_dom_node.h"
#include "../include/ui_layout.h"
#include <stdio.h>
#include <string.h>

/* clang-format on */
void test_missing_coverage(void) {
  struct ui_dom_node *root, *c1, *c2;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule, *r_inline, *r_block;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "test-root");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c1);
  ui_dom_node_set_tag_name(c1, "span");
  ui_dom_node_set_attribute(c1, "class", "test-inline");
  ui_dom_node_append_child(root, c1);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c2);
  ui_dom_node_set_tag_name(c2, "div");
  ui_dom_node_set_attribute(c2, "class", "test-block");
  ui_dom_node_append_child(root, c2);

  ui_css_stylesheet_create(&sheet);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-root");
  ui_css_rule_append_declaration(rule, "display", "block",
                                 0); /* Make it a block container! */
  ui_css_rule_append_declaration(rule, "padding", "10%",
                                 0); /* percent resolution */
  ui_css_rule_append_declaration(rule, "width", "auto", 0);
  ui_css_rule_append_declaration(rule, "wrap-through", "none", 0);
  ui_css_rule_append_declaration(rule, "hyphens", "auto", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r_inline);
  ui_css_rule_append_selector(r_inline, UI_CSS_SELECTOR_TYPE_CLASS,
                              "test-inline");
  ui_css_rule_append_declaration(r_inline, "display", "inline", 0);
  r_inline->next = sheet->rules;
  sheet->rules = r_inline;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r_block);
  ui_css_rule_append_selector(r_block, UI_CSS_SELECTOR_TYPE_CLASS,
                              "test-block");
  ui_css_rule_append_declaration(r_block, "display", "block", 0);
  r_block->next = sheet->rules;
  sheet->rules = r_block;

  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot) {
    ui_layout_compute(lroot, 100, 100);
    ui_layout_tree_destroy(lroot);
  }

  ui_css_stylesheet_destroy(sheet);
  ui_dom_node_destroy(root);
}

void test_missing_compute(void) {
  /* Line 1881: shrink-to-fit width where available_inner <= 0 */
  /* We need a block container with float or absolute, auto width, but 0 or
   * negative available width */
  struct ui_dom_node *root, *c1;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule, *r_child;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "shrink-root");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c1);
  ui_dom_node_set_tag_name(c1, "div");
  ui_dom_node_set_attribute(c1, "class", "shrink-child");
  ui_dom_node_append_child(root, c1);

  ui_css_stylesheet_create(&sheet);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "shrink-root");
  ui_css_rule_append_declaration(rule, "display", "block", 0);
  ui_css_rule_append_declaration(rule, "width", "0px", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r_child);
  ui_css_rule_append_selector(r_child, UI_CSS_SELECTOR_TYPE_CLASS,
                              "shrink-child");
  ui_css_rule_append_declaration(r_child, "display", "block", 0);
  ui_css_rule_append_declaration(r_child, "position", "absolute",
                                 0); /* Causes shrink-to-fit width */
  ui_css_rule_append_declaration(r_child, "width", "auto", 0);
  r_child->next = sheet->rules;
  sheet->rules = r_child;

  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot) {
    ui_layout_compute(lroot, 0, 100);
    ui_layout_tree_destroy(lroot);
  }

  ui_css_stylesheet_destroy(sheet);
  ui_dom_node_destroy(root);
}
