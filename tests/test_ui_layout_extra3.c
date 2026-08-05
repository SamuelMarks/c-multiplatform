void test_coverage_layout(void) {
  struct ui_dom_node *root;
  struct ui_css_stylesheet *sheet;
  struct ui_layout_node *lroot;
  struct ui_css_rule *rule;

  ui_layout_tree_generate(NULL, NULL, NULL); /* covers early exit */

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_stylesheet_append_rule(sheet, rule);

  ui_css_rule_append_declaration(rule, "width", "100px", 0);
  ui_css_rule_append_declaration(rule, "max-width", "50px", 0);
  ui_css_rule_append_declaration(rule, "height", "10px", 0);
  ui_css_rule_append_declaration(rule, "min-height", "50px", 0);
  ui_css_rule_append_declaration(rule, "margin-trim", "inline", 0);
  ui_css_rule_append_declaration(
      rule, "overflow", "scroll",
      0); /* will test ui_layout_compute's overflow_x */
  ui_css_rule_append_declaration(
      rule, "overflow", "hidden",
      0); /* will test ui_layout_compute's overflow_x */
  ui_css_rule_append_declaration(rule, "box-sizing", "border-box", 0);

  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  /* Also test "none" display parsing */
  ui_css_rule_append_declaration(rule, "display", "none block", 0);
  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_tree_destroy(lroot);

  /* Test inline anon block */
  {
    struct ui_dom_node *child1, *child2, *child3;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
    ui_dom_node_set_tag_name(child1, "span");
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child2);
    ui_dom_node_set_tag_name(child2, "div");
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child3);
    ui_dom_node_set_tag_name(child3, "span");

    ui_dom_node_append_child(root, child1);
    ui_dom_node_append_child(root, child2);
    ui_dom_node_append_child(root, child3);

    struct ui_css_rule *rule2;
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule2);
    ui_css_rule_append_selector(rule2, UI_CSS_SELECTOR_TYPE_TAG, "span");
    ui_css_rule_append_declaration(rule2, "display", "inline", 0);
    ui_css_stylesheet_append_rule(sheet, rule2);

    ui_layout_tree_generate(root, sheet, &lroot);
    ui_layout_compute(lroot, 100.0f, 100.0f);
    ui_layout_tree_destroy(lroot);
  }

  (void)ui_dom_node_destroy(root);
  ui_css_stylesheet_destroy(sheet);
}
