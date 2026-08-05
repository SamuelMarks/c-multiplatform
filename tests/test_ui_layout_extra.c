#include "../include/ui_cssom.h"
#include "../include/ui_dom_node.h"
#include "../include/ui_layout.h"
#include <stdio.h>
#include <stdlib.h>

void test_more_layout(void) {
  struct ui_dom_node *root;
  struct ui_css_stylesheet *sheet;
  struct ui_layout_node *lroot;
  struct ui_css_rule *rule;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_stylesheet_append_rule(sheet, rule);

  ui_css_rule_append_declaration(rule, "width", "auto", 0);
  ui_css_rule_append_declaration(rule, "height", "auto", 0);
  ui_css_rule_append_declaration(rule, "wrap-through", "bogus", 0);
  ui_css_rule_append_declaration(rule, "hyphens", "bogus", 0);
  ui_css_rule_append_declaration(rule, "aspect-ratio", "0/0", 0);
  ui_css_rule_append_declaration(rule, "padding", "-10px", 0);
  ui_css_rule_append_declaration(rule, "min-height", "50px", 0);
  ui_css_rule_append_declaration(rule, "max-width", "10px", 0);

  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  ui_css_rule_append_declaration(rule, "width", "-100%", 0);
  ui_css_rule_append_declaration(rule, "display", "block", 0);
  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  ui_css_rule_append_declaration(rule, "width", "100%", 0);
  ui_css_rule_append_declaration(rule, "height", "-100%", 0);
  ui_css_rule_append_declaration(rule, "display", "flex", 0);
  ui_css_rule_append_declaration(rule, "flex-direction", "column", 0);
  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  /* More flex test */
  ui_css_rule_append_declaration(rule, "justify-content", "end", 0);
  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  ui_css_rule_append_declaration(rule, "justify-content", "space-between", 0);
  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  ui_css_rule_append_declaration(rule, "justify-content", "space-around", 0);
  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  (void)ui_dom_node_destroy(root);
  ui_css_stylesheet_destroy(sheet);
}
