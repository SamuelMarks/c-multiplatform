
static void test_anonymous_box_cleanup(void) {
  /* To hit the cleanup loop we need OOM inside the anonymous block wrapping
   * loop */
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *child_block = NULL;
  struct ui_dom_node *child_inline1 = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule_block = NULL;
  struct ui_css_rule *rule_inline = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "blk");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child_inline1);
  ui_dom_node_set_tag_name(child_inline1, "span");
  ui_dom_node_set_attribute(child_inline1, "class", "inl");
  struct ui_dom_node *t1;
  ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &t1);
  ui_dom_node_append_child(child_inline1, t1);
  ui_dom_node_append_child(root, child_inline1);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child_block);
  ui_dom_node_set_tag_name(child_block, "div");
  ui_dom_node_set_attribute(child_block, "class", "blk");
  struct ui_dom_node *t2;
  ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &t2);
  ui_dom_node_append_child(child_block, t2);
  ui_dom_node_append_child(root, child_block);

  ui_css_stylesheet_create(&sheet);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_block);
  ui_css_rule_append_selector(rule_block, UI_CSS_SELECTOR_TYPE_CLASS, "blk");
  ui_css_rule_append_declaration(rule_block, "display", "block", 0);
  rule_block->next = sheet->rules;
  sheet->rules = rule_block;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_inline);
  ui_css_rule_append_selector(rule_inline, UI_CSS_SELECTOR_TYPE_CLASS, "inl");
  ui_css_rule_append_declaration(rule_inline, "display", "inline", 0);
  rule_inline->next = sheet->rules;
  sheet->rules = rule_inline;

  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  g_malloc_fail_countdown = 2; /* Target the anonymous node alloc */
  ui_layout_tree_generate(root, sheet, &lroot);
  g_malloc_fail_countdown = -1;
  if (lroot)
    ui_layout_tree_destroy(lroot);

  ui_css_stylesheet_destroy(sheet);
  ui_dom_node_destroy(root);
}

static void test_more_layout_coverage6(void) {
  /* Wrap node tree to trigger the cleanup anonymous loops and display edge
   * cases */
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *child1 = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "blk");
  ui_dom_node_set_attribute(root, "class", "test-class");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
  ui_dom_node_set_tag_name(child1, "div");
  ui_dom_node_set_attribute(child1, "class", "test-child");
  ui_dom_node_append_child(root, child1);

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "display", "run-in", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;

  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);

  ui_css_stylesheet_destroy(sheet);
  ui_dom_node_destroy(root);
}

static void test_more_layout_coverage5(void) {
  struct ui_dom_node *root = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "blk");
  ui_dom_node_set_attribute(root, "class", "test-class");

  const char *displays[] = {"run-in",      "grid",         "table", "flow-root",
                            "inline-grid", "inline-table", "ruby"};

  int i;
  for (i = 0; i < 7; i++) {
    ui_css_stylesheet_create(&sheet);
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
    ui_css_rule_append_declaration(rule, "display", displays[i], 0);
    rule->next = sheet->rules;
    sheet->rules = rule;

    ui_layout_tree_generate(root, sheet, &lroot);
    if (lroot)
      ui_layout_tree_destroy(lroot);
    ui_css_stylesheet_destroy(sheet);
  }
  ui_dom_node_destroy(root);
}

static void test_css_parsers2(void) {
  struct ui_dom_node *root = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "blk");
  ui_dom_node_set_attribute(root, "class", "test-class");

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-size-adjust", "10%", 0);
  ui_css_rule_append_declaration(rule, "background-size", "contain", 0);
  ui_css_rule_append_declaration(rule, "block-step-size", "none", 0);
  ui_css_rule_append_declaration(rule, "text-decoration-color", "red", 0);
  ui_css_rule_append_declaration(rule, "text-shadow", "1px 1px black", 0);
  ui_css_rule_append_declaration(rule, "text-emphasis-style", "filled", 0);
  ui_css_rule_append_declaration(rule, "text-emphasis-color", "blue", 0);
  ui_css_rule_append_declaration(rule, "color", "green", 0);
  ui_css_rule_append_declaration(rule, "background-color", "yellow", 0);
  ui_css_rule_append_declaration(rule, "background-image", "url(test)", 0);
  ui_css_rule_append_declaration(rule, "width", "50%", 0);
  ui_css_rule_append_declaration(rule, "margin", "  10px    20px  ", 0);
  ui_css_rule_append_declaration(rule, "wrap-through", "wrap", 0);
  ui_css_rule_append_declaration(rule, "hyphens", "manual", 0);
  ui_css_rule_append_declaration(rule, "forced-color-adjust", "auto", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;

  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);

  /* Reset sheet */
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-size-adjust", "none", 0);

  ui_css_rule_append_declaration(rule, "border-top-left-radius", "10px", 0);
  ui_css_rule_append_declaration(rule, "border-top-right-radius", "10px", 0);
  ui_css_rule_append_declaration(rule, "border-bottom-right-radius", "10px", 0);
  ui_css_rule_append_declaration(rule, "border-bottom-left-radius", "10px", 0);
  ui_css_rule_append_declaration(rule, "border-top-color", "red", 0);
  ui_css_rule_append_declaration(rule, "border-right-color", "red", 0);
  ui_css_rule_append_declaration(rule, "border-bottom-color", "red", 0);
  ui_css_rule_append_declaration(rule, "border-left-color", "red", 0);
  ui_css_rule_append_declaration(rule, "text-emphasis-position", "over right",
                                 0);
  ui_css_rule_append_declaration(rule, "font-family", "Arial", 0);

  ui_css_rule_append_declaration(rule, "font-size", "10px", 0);
  ui_css_rule_append_declaration(rule, "font-variant", "small-caps", 0);
  ui_css_rule_append_declaration(rule, "font-feature-settings", "smcp", 0);
  ui_css_rule_append_declaration(rule, "font-variation-settings", "wght 100",
                                 0);
  ui_css_rule_append_declaration(rule, "column-width", "10", 0);
  ui_css_rule_append_declaration(rule, "margin-top", "10px", 0);
  ui_css_rule_append_declaration(rule, "margin-right", "10px", 0);
  ui_css_rule_append_declaration(rule, "margin-bottom", "10px", 0);
  ui_css_rule_append_declaration(rule, "margin-left", "10px", 0);

  ui_css_rule_append_declaration(rule, "padding-right", "10px", 0);
  ui_css_rule_append_declaration(rule, "padding-bottom", "10px", 0);
  ui_css_rule_append_declaration(rule, "padding-left", "10px", 0);
  ui_css_rule_append_declaration(rule, "border-top-width", "10px", 0);
  ui_css_rule_append_declaration(rule, "border-right-width", "10px", 0);
  ui_css_rule_append_declaration(rule, "border-bottom-width", "10px", 0);
  ui_css_rule_append_declaration(rule, "border-left-width", "10px", 0);
  ui_css_rule_append_declaration(rule, "gap", "10px", 0);
  ui_css_rule_append_declaration(rule, "row-gap", "10px", 0);

  ui_css_rule_append_declaration(rule, "max-width", "100px", 0);
  ui_css_rule_append_declaration(rule, "min-height", "10px", 0);
  ui_css_rule_append_declaration(rule, "flow-into", "flow1", 0);
  ui_css_rule_append_declaration(rule, "flow-from", "flow1", 0);
  ui_css_rule_append_declaration(rule, "top", "10px", 0);
  ui_css_rule_append_declaration(rule, "right", "10px", 0);
  ui_css_rule_append_declaration(rule, "bottom", "10px", 0);
  ui_css_rule_append_declaration(rule, "left", "10px", 0);

  ui_css_rule_append_declaration(rule, "display", "none", 0);
  ui_css_rule_append_declaration(rule, "display", "run-in", 0);
  ui_css_rule_append_declaration(rule, "display", "grid", 0);
  ui_css_rule_append_declaration(rule, "display", "table", 0);
  ui_css_rule_append_declaration(rule, "display", "flow-root", 0);
  ui_css_rule_append_declaration(rule, "display", "inline-grid", 0);
  ui_css_rule_append_declaration(rule, "display", "inline-table", 0);
  ui_css_rule_append_declaration(rule, "display", "ruby", 0);

  ui_css_rule_append_declaration(rule, "display", "none", 0);
  ui_css_rule_append_declaration(rule, "display", "run-in", 0);
  ui_css_rule_append_declaration(rule, "display", "grid", 0);
  ui_css_rule_append_declaration(rule, "display", "table", 0);
  ui_css_rule_append_declaration(rule, "display", "flow-root", 0);
  ui_css_rule_append_declaration(rule, "display", "inline-grid", 0);
  ui_css_rule_append_declaration(rule, "display", "inline-table", 0);
  ui_css_rule_append_declaration(rule, "display", "ruby", 0);

  ui_css_rule_append_declaration(rule, "block-step-size", "10px", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;

  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_dom_node_destroy(root);
}

static void test_css_parsers(void) {
  struct ui_dom_node *root = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "blk");
  ui_dom_node_set_attribute(root, "class", "test-class");

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-before", "avoid", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-before", "always", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-before", "all", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-before", "avoid-page", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-before", "page", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-before", "left", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-before", "right", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-before", "recto", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-before", "verso", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-before", "avoid-column", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-before", "column", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-before", "avoid-region", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-before", "region", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-before", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-after", "avoid", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-after", "always", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-after", "all", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-after", "avoid-page", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-after", "page", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-after", "left", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-after", "right", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-after", "recto", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-after", "verso", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-after", "avoid-column", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-after", "column", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-after", "avoid-region", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-after", "region", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-after", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-inside", "avoid", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-inside", "always", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-inside", "all", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-inside", "avoid-page", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-inside", "page", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-inside", "left", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-inside", "right", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-inside", "recto", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-inside", "verso", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-inside", "avoid-column", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-inside", "column", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-inside", "avoid-region", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-inside", "region", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "break-inside", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-before", "avoid", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-before", "always", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-before", "all", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-before", "avoid-page", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-before", "page", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-before", "left", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-before", "right", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-before", "recto", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-before", "verso", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-before", "avoid-column", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-before", "column", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-before", "avoid-region", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-before", "region", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-before", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-after", "avoid", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-after", "always", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-after", "all", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-after", "avoid-page", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-after", "page", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-after", "left", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-after", "right", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-after", "recto", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-after", "verso", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-after", "avoid-column", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-after", "column", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-after", "avoid-region", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-after", "region", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-after", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-inside", "avoid", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-inside", "always", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-inside", "all", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-inside", "avoid-page", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-inside", "page", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-inside", "left", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-inside", "right", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-inside", "recto", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-inside", "verso", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-inside", "avoid-column", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-inside", "column", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-inside", "avoid-region", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-inside", "region", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "page-break-inside", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "overflow", "hidden", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "overflow", "scroll", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "overflow", "auto", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "overflow", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "overflow-x", "hidden", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "overflow-x", "scroll", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "overflow-x", "auto", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "overflow-x", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "overflow-y", "hidden", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "overflow-y", "scroll", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "overflow-y", "auto", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "overflow-y", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "wrap-flow", "auto", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "wrap-flow", "both", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "wrap-flow", "start", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "wrap-flow", "end", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "wrap-flow", "minimum", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "wrap-flow", "maximum", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "wrap-flow", "clear", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "wrap-flow", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "wrap-through", "wrap", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "wrap-through", "none", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "white-space", "normal", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "white-space", "nowrap", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "white-space", "pre", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "white-space", "pre-wrap", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "white-space", "pre-line", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "white-space", "break-spaces", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "white-space", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-align", "start", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-align", "end", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-align", "left", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-align", "right", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-align", "center", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-align", "justify", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-align", "match-parent", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-align", "justify-all", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-align", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "word-break", "normal", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "word-break", "keep-all", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "word-break", "break-all", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "word-break", "break-word", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "word-break", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "hyphens", "none", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "hyphens", "manual", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "hyphens", "auto", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "writing-mode", "horizontal-tb", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "writing-mode", "vertical-rl", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "writing-mode", "vertical-lr", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "writing-mode", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "direction", "ltr", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "direction", "rtl", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "direction", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "unicode-bidi", "normal", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "unicode-bidi", "embed", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "unicode-bidi", "bidi-override", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "unicode-bidi", "isolate", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "unicode-bidi", "isolate-override", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "unicode-bidi", "plaintext", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "unicode-bidi", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "line-grid", "create", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "line-grid", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "line-snap", "baseline", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "line-snap", "contain", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "line-snap", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "box-snap", "block-start", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "box-snap", "block-end", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "box-snap", "center", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "box-snap", "baseline", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "box-snap", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "block-step-insert", "padding", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "block-step-insert", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "block-step-align", "center", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "block-step-align", "start", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "block-step-align", "end", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "block-step-align", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "block-step-round", "down", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "block-step-round", "nearest", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "block-step-round", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "color-scheme", "normal", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "color-scheme", "light", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "color-scheme", "dark", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "color-scheme", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "print-color-adjust", "exact", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "print-color-adjust", "invalid-value",
                                 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "forced-color-adjust", "none", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "forced-color-adjust",
                                 "preserve-parent-color", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-orientation", "mixed", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-orientation", "upright", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-orientation", "sideways", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-orientation", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-decoration-style", "double", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-decoration-style", "dotted", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-decoration-style", "dashed", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-decoration-style", "wavy", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-decoration-style", "invalid-value",
                                 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "font-style", "italic", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "font-style", "oblique", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "font-style", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "font-stretch", "ultra-condensed", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "font-stretch", "extra-condensed", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "font-stretch", "condensed", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "font-stretch", "semi-condensed", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "font-stretch", "semi-expanded", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "font-stretch", "expanded", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "font-stretch", "extra-expanded", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "font-stretch", "ultra-expanded", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "font-stretch", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-size-adjust", "auto", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-size-adjust", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "background-repeat", "repeat", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "background-repeat", "space", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "background-repeat", "round", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "background-repeat", "no-repeat", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "background-repeat", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "background-size", "cover", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "background-size", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "box-decoration-break", "clone", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "box-decoration-break", "invalid-value",
                                 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-self", "auto", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-self", "normal", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-self", "start", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-self", "flex-start", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-self", "end", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-self", "flex-end", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-self", "center", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-self", "stretch", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-self", "space-between", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-self", "space-around", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-self", "space-evenly", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-self", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-items", "auto", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-items", "normal", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-items", "start", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-items", "flex-start", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-items", "end", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-items", "flex-end", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-items", "center", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-items", "stretch", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-items", "space-between", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-items", "space-around", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-items", "space-evenly", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-items", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "justify-content", "auto", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "justify-content", "normal", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "justify-content", "start", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "justify-content", "flex-start", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "justify-content", "end", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "justify-content", "flex-end", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "justify-content", "center", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "justify-content", "stretch", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "justify-content", "space-between", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "justify-content", "space-around", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "justify-content", "space-evenly", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "justify-content", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-content", "auto", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-content", "normal", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-content", "start", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-content", "flex-start", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-content", "end", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-content", "flex-end", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-content", "center", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-content", "stretch", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-content", "space-between", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-content", "space-around", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-content", "space-evenly", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "align-content", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "flex-direction", "row-reverse", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "flex-direction", "column", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "flex-direction", "column-reverse", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "flex-direction", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "flex-wrap", "wrap", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "flex-wrap", "wrap-reverse", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "flex-wrap", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "margin-trim", "block", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "margin-trim", "block-start", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "margin-trim", "block-end", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "margin-trim", "inline", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "margin-trim", "inline-start", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "margin-trim", "inline-end", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "margin-trim", "all", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "margin-trim", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-decoration-line", "none", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-decoration-line", "underline", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-decoration-line", "overline", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-decoration-line", "line-through",
                                 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-decoration-line", "blink", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-decoration", "none", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-decoration", "underline", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-decoration", "overline", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-decoration", "line-through", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "text-decoration", "blink", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "font-weight", "normal", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "font-weight", "bold", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "font-weight", "bolder", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "font-weight", "lighter", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "font-weight", "invalid-value", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_dom_node_destroy(root);
}
static void test_layout_flex_and_shorthands(void) {
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *child1 = NULL;
  struct ui_dom_node *child2 = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule = NULL;
  struct ui_css_rule *rule1 = NULL;
  struct ui_css_rule *rule2 = NULL;
  struct ui_css_rule *rule_extra = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "blk");
  ui_dom_node_set_attribute(root, "class", "flex-container");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
  ui_dom_node_set_tag_name(child1, "div");
  ui_dom_node_set_attribute(child1, "class", "flex-child1");
  ui_dom_node_append_child(root, child1);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child2);
  ui_dom_node_set_tag_name(child2, "div");
  ui_dom_node_set_attribute(child2, "class", "flex-child2");
  ui_dom_node_append_child(root, child2);

  ui_css_stylesheet_create(&sheet);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS,
                              "flex-container");
  ui_css_rule_append_declaration(rule, "display", "flex", 0);
  ui_css_rule_append_declaration(rule, "flex-direction", "column", 0);
  ui_css_rule_append_declaration(rule, "flex-wrap", "wrap", 0);
  ui_css_rule_append_declaration(rule, "align-items", "center", 0);
  ui_css_rule_append_declaration(rule, "justify-content", "space-between", 0);
  ui_css_rule_append_declaration(rule, "margin", "10px 20px 30px 40px", 0);
  ui_css_rule_append_declaration(rule, "padding", "1px 2px", 0);
  ui_css_rule_append_declaration(rule, "border-width", "5px", 0);
  ui_css_rule_append_declaration(rule, "border-radius", "10px", 0);
  ui_css_rule_append_declaration(rule, "box-shadow", "inset 2px 2px 5px black",
                                 0);
  ui_css_rule_append_declaration(rule, "box-sizing", "border-box", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule1);
  ui_css_rule_append_selector(rule1, UI_CSS_SELECTOR_TYPE_CLASS, "flex-child1");
  ui_css_rule_append_declaration(rule1, "flex-grow", "1", 0);
  ui_css_rule_append_declaration(rule1, "flex-shrink", "0", 0);
  ui_css_rule_append_declaration(rule1, "flex-basis", "100px", 0);
  ui_css_rule_append_declaration(rule1, "align-self", "flex-end", 0);
  ui_css_rule_append_declaration(rule1, "box-shadow", "none", 0);
  rule1->next = sheet->rules;
  sheet->rules = rule1;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule2);
  ui_css_rule_append_selector(rule2, UI_CSS_SELECTOR_TYPE_CLASS, "flex-child2");
  ui_css_rule_append_declaration(rule2, "flex-grow", "2", 0);
  ui_css_rule_append_declaration(rule2, "align-self", "stretch", 0);
  ui_css_rule_append_declaration(rule2, "box-sizing", "content-box", 0);
  rule2->next = sheet->rules;
  sheet->rules = rule2;

  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 800, 600);
  ui_layout_tree_destroy(lroot);

  /* Additional flex direction tests */
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_extra);
  ui_css_rule_append_selector(rule_extra, UI_CSS_SELECTOR_TYPE_CLASS,
                              "flex-container");
  ui_css_rule_append_declaration(rule_extra, "flex-direction", "row-reverse",
                                 0);
  rule_extra->next = sheet->rules;
  sheet->rules = rule_extra;

  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 800, 600);
  ui_layout_tree_destroy(lroot);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_extra);
  ui_css_rule_append_selector(rule_extra, UI_CSS_SELECTOR_TYPE_CLASS,
                              "flex-container");
  ui_css_rule_append_declaration(rule_extra, "flex-direction", "column-reverse",
                                 0);
  rule_extra->next = sheet->rules;
  sheet->rules = rule_extra;

  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 800, 600);
  ui_layout_tree_destroy(lroot);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_extra);
  ui_css_rule_append_selector(rule_extra, UI_CSS_SELECTOR_TYPE_CLASS,
                              "flex-container");
  ui_css_rule_append_declaration(rule_extra, "flex-wrap", "wrap-reverse", 0);
  rule_extra->next = sheet->rules;
  sheet->rules = rule_extra;

  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 800, 600);
  ui_layout_tree_destroy(lroot);

  ui_css_stylesheet_destroy(sheet);
  ui_dom_node_destroy(root);
}
static void test_null_parsers(void) {
  struct ui_dom_node *root = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "blk");
  ui_dom_node_set_attribute(root, "class", "test-null");

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-null");

  ui_css_rule_append_declaration(rule, "break-before", NULL, 0);
  ui_css_rule_append_declaration(rule, "overflow", NULL, 0);
  ui_css_rule_append_declaration(rule, "wrap-flow", NULL, 0);
  ui_css_rule_append_declaration(rule, "wrap-through", NULL, 0);
  ui_css_rule_append_declaration(rule, "white-space", NULL, 0);
  ui_css_rule_append_declaration(rule, "text-align", NULL, 0);
  ui_css_rule_append_declaration(rule, "word-break", NULL, 0);
  ui_css_rule_append_declaration(rule, "hyphens", NULL, 0);
  ui_css_rule_append_declaration(rule, "writing-mode", NULL, 0);
  ui_css_rule_append_declaration(rule, "direction", NULL, 0);
  ui_css_rule_append_declaration(rule, "unicode-bidi", NULL, 0);
  ui_css_rule_append_declaration(rule, "line-grid", NULL, 0);
  ui_css_rule_append_declaration(rule, "line-snap", NULL, 0);
  ui_css_rule_append_declaration(rule, "box-snap", NULL, 0);
  ui_css_rule_append_declaration(rule, "block-step-insert", NULL, 0);
  ui_css_rule_append_declaration(rule, "block-step-align", NULL, 0);
  ui_css_rule_append_declaration(rule, "block-step-round", NULL, 0);
  ui_css_rule_append_declaration(rule, "color-scheme", NULL, 0);
  ui_css_rule_append_declaration(rule, "print-color-adjust", NULL, 0);
  ui_css_rule_append_declaration(rule, "forced-color-adjust", NULL, 0);
  ui_css_rule_append_declaration(rule, "text-orientation", NULL, 0);
  ui_css_rule_append_declaration(rule, "text-decoration-style", NULL, 0);
  ui_css_rule_append_declaration(rule, "text-decoration-line", NULL, 0);
  ui_css_rule_append_declaration(rule, "text-decoration", NULL, 0);
  ui_css_rule_append_declaration(rule, "font-style", NULL, 0);
  ui_css_rule_append_declaration(rule, "font-stretch", NULL, 0);
  ui_css_rule_append_declaration(rule, "font-weight", NULL, 0);
  ui_css_rule_append_declaration(rule, "text-size-adjust", NULL, 0);
  ui_css_rule_append_declaration(rule, "background-repeat", NULL, 0);
  ui_css_rule_append_declaration(rule, "background-size", NULL, 0);
  ui_css_rule_append_declaration(rule, "box-decoration-break", NULL, 0);
  ui_css_rule_append_declaration(rule, "flex-direction", NULL, 0);
  ui_css_rule_append_declaration(rule, "flex-wrap", NULL, 0);
  ui_css_rule_append_declaration(rule, "margin-trim", NULL, 0);
  ui_css_rule_append_declaration(rule, "align-self", NULL, 0);
  ui_css_rule_append_declaration(rule, "padding", NULL, 0);
  ui_css_rule_append_declaration(rule, "aspect-ratio", NULL, 0);
  ui_css_rule_append_declaration(rule, "width", NULL, 0);

  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_dom_node_destroy(root);
}
static void test_border_image(void) {
  struct ui_dom_node *root = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "blk");
  ui_dom_node_set_attribute(root, "class", "test-class");

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "border-image", "url(test.png)", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "border-image-source", "url(test2.png)",
                                 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);

  ui_dom_node_destroy(root);
}
static void test_numeric_parsers(void) {
  struct ui_dom_node *root = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "blk");
  ui_dom_node_set_attribute(root, "class", "test-class");

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "orphans", "3", 0);
  ui_css_rule_append_declaration(rule, "widows", "4", 0);
  ui_css_rule_append_declaration(rule, "column-count", "auto", 0);
  ui_css_rule_append_declaration(rule, "column-width", "auto", 0);
  ui_css_rule_append_declaration(rule, "column-gap", "normal", 0);
  ui_css_rule_append_declaration(rule, "z-index", "10", 0);
  ui_css_rule_append_declaration(rule, "opacity", "0.5", 0);
  ui_css_rule_append_declaration(rule, "flex-grow", "1.5", 0);
  ui_css_rule_append_declaration(rule, "flex-shrink", "0.5", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);

  ui_dom_node_destroy(root);
}
static void test_more_aspect_ratios(void) {
  struct ui_dom_node *root = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "blk");
  ui_dom_node_set_attribute(root, "class", "test-class");

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "aspect-ratio", "auto", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "aspect-ratio", "16", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);

  ui_dom_node_destroy(root);
}
static void test_margin_3(void) {
  struct ui_dom_node *root = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "blk");
  ui_dom_node_set_attribute(root, "class", "test-class");

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
  ui_css_rule_append_declaration(rule, "margin", "10px 20px 30px", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);

  ui_dom_node_destroy(root);
}
static void test_lengths(void) {
  struct ui_dom_node *root = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "blk");
  ui_dom_node_set_attribute(root, "class", "test-class");

  const char *lengths[] = {"1em", "1rem", "50vw", "50vh", "50vmin", "50vmax",
                           "5cm", "10mm", "1in",  "10pt", "10pc"};
  int i;
  for (i = 0; i < (int)(sizeof(lengths) / sizeof(lengths[0])); i++) {
    ui_css_stylesheet_create(&sheet);
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "test-class");
    ui_css_rule_append_declaration(rule, "width", lengths[i], 0);
    rule->next = sheet->rules;
    sheet->rules = rule;
    ui_layout_tree_generate(root, sheet, &lroot);
    if (lroot)
      ui_layout_tree_destroy(lroot);
    ui_css_stylesheet_destroy(sheet);
  }
  ui_dom_node_destroy(root);
}
static void test_flex_coverage(void) {
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *child1 = NULL;
  struct ui_dom_node *child2 = NULL;
  struct ui_dom_node *child3 = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule = NULL;
  struct ui_css_rule *rule_c1 = NULL;
  struct ui_css_rule *rule_c2 = NULL;
  struct ui_css_rule *rule_c3 = NULL;

  const char *justify[] = {"center", "flex-end", "space-around",
                           "space-evenly"};
  const char *align_items[] = {"center", "flex-end", "baseline", "stretch"};
  int i;

  for (i = 0; i < 4; i++) {
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
    ui_dom_node_set_tag_name(root, "div");
    ui_dom_node_set_attribute(root, "class", "blk");
    ui_dom_node_set_attribute(root, "class", "flex-container");

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
    ui_dom_node_set_tag_name(child1, "div");
    ui_dom_node_set_attribute(child1, "class", "flex-child1");
    ui_dom_node_append_child(root, child1);

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child2);
    ui_dom_node_set_tag_name(child2, "div");
    ui_dom_node_set_attribute(child2, "class", "flex-child2");
    ui_dom_node_append_child(root, child2);

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child3);
    ui_dom_node_set_tag_name(child3, "div");
    ui_dom_node_set_attribute(child3, "class", "flex-child3");
    ui_dom_node_append_child(root, child3);

    ui_css_stylesheet_create(&sheet);
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS,
                                "flex-container");
    ui_css_rule_append_declaration(rule, "display", "flex", 0);
    ui_css_rule_append_declaration(rule, "width", "500px", 0);
    ui_css_rule_append_declaration(rule, "height", "200px", 0);
    ui_css_rule_append_declaration(rule, "justify-content", justify[i], 0);
    ui_css_rule_append_declaration(rule, "align-items", align_items[i], 0);
    ui_css_rule_append_declaration(rule, "flex-wrap", "wrap", 0);
    rule->next = sheet->rules;
    sheet->rules = rule;

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_c1);
    ui_css_rule_append_selector(rule_c1, UI_CSS_SELECTOR_TYPE_CLASS,
                                "flex-child1");
    ui_css_rule_append_declaration(rule_c1, "width", "100px", 0);
    ui_css_rule_append_declaration(rule_c1, "height", "50px", 0);
    ui_css_rule_append_declaration(rule_c1, "flex-shrink", "1", 0);
    rule_c1->next = sheet->rules;
    sheet->rules = rule_c1;

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_c2);
    ui_css_rule_append_selector(rule_c2, UI_CSS_SELECTOR_TYPE_CLASS,
                                "flex-child2");
    ui_css_rule_append_declaration(rule_c2, "width", "100%",
                                   0); /* percent test */
    ui_css_rule_append_declaration(rule_c2, "height", "100%", 0);
    ui_css_rule_append_declaration(rule_c2, "flex-grow", "1", 0);
    rule_c2->next = sheet->rules;
    sheet->rules = rule_c2;

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_c3);
    ui_css_rule_append_selector(rule_c3, UI_CSS_SELECTOR_TYPE_CLASS,
                                "flex-child3");
    ui_css_rule_append_declaration(rule_c3, "flex-basis", "300px",
                                   0); /* Wrap test */
    rule_c3->next = sheet->rules;
    sheet->rules = rule_c3;

    ui_layout_tree_generate(root, sheet, &lroot);
    if (lroot) {
      ui_layout_compute(lroot, 800, 600);
      ui_layout_tree_destroy(lroot);
    }
    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(root);
  }
}
static void test_anonymous_box(void) {
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *child_block = NULL;
  struct ui_dom_node *child_inline1 = NULL;
  struct ui_dom_node *child_inline2 = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule_block = NULL;
  struct ui_css_rule *rule_inline = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "blk");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child_inline1);
  ui_dom_node_set_tag_name(child_inline1, "span");
  ui_dom_node_set_attribute(child_inline1, "class", "inl");
  struct ui_dom_node *t1;
  ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &t1);
  ui_dom_node_append_child(child_inline1, t1);
  ui_dom_node_append_child(root, child_inline1);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child_block);
  ui_dom_node_set_tag_name(child_block, "div");
  ui_dom_node_set_attribute(child_block, "class", "blk");
  struct ui_dom_node *t2;
  ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &t2);
  ui_dom_node_append_child(child_block, t2);
  ui_dom_node_append_child(root, child_block);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child_inline2);
  ui_dom_node_set_tag_name(child_inline2, "span");
  ui_dom_node_set_attribute(child_inline2, "class", "inl");
  struct ui_dom_node *t3;
  ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &t3);
  ui_dom_node_append_child(child_inline2, t3);
  ui_dom_node_append_child(root, child_inline2);

  ui_css_stylesheet_create(&sheet);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_block);
  ui_css_rule_append_selector(rule_block, UI_CSS_SELECTOR_TYPE_CLASS, "blk");
  ui_css_rule_append_declaration(rule_block, "display", "block", 0);
  rule_block->next = sheet->rules;
  sheet->rules = rule_block;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_inline);
  ui_css_rule_append_selector(rule_inline, UI_CSS_SELECTOR_TYPE_CLASS, "inl");
  ui_css_rule_append_declaration(rule_inline, "display", "inline", 0);
  rule_inline->next = sheet->rules;
  sheet->rules = rule_inline;

  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot) {
    ui_layout_compute(lroot, 800, 600);
    ui_layout_tree_destroy(lroot);
  }

  /* Anon box starting with block child */
  {
    struct ui_dom_node *root2 = NULL;
    struct ui_dom_node *c_block = NULL;
    struct ui_dom_node *c_inline = NULL;
    struct ui_layout_node *lroot2 = NULL;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root2);
    ui_dom_node_set_tag_name(root2, "div");
    ui_dom_node_set_attribute(root2, "class", "blk");

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c_block);
    ui_dom_node_set_tag_name(c_block, "div");
    ui_dom_node_set_attribute(c_block, "class", "blk");
    ui_dom_node_append_child(root2, c_block);

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c_inline);
    ui_dom_node_set_tag_name(c_inline, "span");
    ui_dom_node_set_attribute(c_inline, "class", "inl");
    ui_dom_node_append_child(root2, c_inline);

    ui_layout_tree_generate(root2, sheet, &lroot2);
    if (lroot2)
      ui_layout_tree_destroy(lroot2);
    ui_dom_node_destroy(root2);
  }

  ui_css_stylesheet_destroy(sheet);
  ui_dom_node_destroy(root);
}

static void test_display_none(void) {
  struct ui_dom_node *root = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule = NULL;
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "blk");
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_rule_append_declaration(rule, "display", "none", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;
  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot)
    ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_dom_node_destroy(root);
}
static void test_anonymous_box_oom(void) {
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *child_block = NULL;
  struct ui_dom_node *child_inline1 = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule_block = NULL;
  struct ui_css_rule *rule_inline = NULL;
  int i;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "blk");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child_inline1);
  ui_dom_node_set_tag_name(child_inline1, "span");
  ui_dom_node_set_attribute(child_inline1, "class", "inl");
  struct ui_dom_node *t1;
  ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &t1);
  ui_dom_node_append_child(child_inline1, t1);
  ui_dom_node_append_child(root, child_inline1);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child_block);
  ui_dom_node_set_tag_name(child_block, "div");
  ui_dom_node_set_attribute(child_block, "class", "blk");
  struct ui_dom_node *t2;
  ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &t2);
  ui_dom_node_append_child(child_block, t2);
  ui_dom_node_append_child(root, child_block);

  ui_css_stylesheet_create(&sheet);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_block);
  ui_css_rule_append_selector(rule_block, UI_CSS_SELECTOR_TYPE_CLASS, "blk");
  ui_css_rule_append_declaration(rule_block, "display", "block", 0);
  rule_block->next = sheet->rules;
  sheet->rules = rule_block;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_inline);
  ui_css_rule_append_selector(rule_inline, UI_CSS_SELECTOR_TYPE_CLASS, "inl");
  ui_css_rule_append_declaration(rule_inline, "display", "inline", 0);
  rule_inline->next = sheet->rules;
  sheet->rules = rule_inline;

  for (i = 1; i < 50; i++) {
    g_malloc_fail_countdown = i;
    ui_layout_tree_generate(root, sheet, &lroot);
    g_malloc_fail_countdown = -1;
    if (lroot)
      ui_layout_tree_destroy(lroot);
  }

  ui_css_stylesheet_destroy(sheet);
  ui_dom_node_destroy(root);
}
static void test_sanity_check_violation(void) {
  struct ui_layout_node parent;
  struct ui_layout_node child;

  memset(&parent, 0, sizeof(parent));
  memset(&child, 0, sizeof(child));

  parent.width = 100.0f;
  parent.height = 100.0f;
  parent.overflow_x = UI_LAYOUT_OVERFLOW_HIDDEN;
  parent.overflow_y = UI_LAYOUT_OVERFLOW_HIDDEN;
  parent.first_child = &child;

  child.x = 0.0f;
  child.y = 0.0f;
  child.width = 200.0f; /* Bleed */
  child.height = 50.0f;

  if (ui_layout_sanity_check(&parent) != UI_ERROR_LAYOUT_VIOLATION) {
    printf("Sanity check failed to catch X bleed!\n");
  }

  child.width = 50.0f;
  child.height = 200.0f; /* Bleed Y */

  if (ui_layout_sanity_check(&parent) != UI_ERROR_LAYOUT_VIOLATION) {
    printf("Sanity check failed to catch Y bleed!\n");
  }
}
static void test_viewport_edge_cases(void) {
  struct ui_layout_node lnode;
  memset(&lnode, 0, sizeof(lnode));

  if (ui_layout_solve_viewport(NULL, 800, 600) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected UI_ERROR_INVALID_ARGUMENT for NULL root\n");
  }

  /* Anonymous root */
  lnode.dom_node = NULL;
  ui_layout_solve_viewport(&lnode, 800, 600);

  /* NULL node compute */
  ui_layout_compute(NULL, 100, 100);
}
static void test_sanity_recursive(void) {
  struct ui_layout_node parent;
  struct ui_layout_node child;
  struct ui_layout_node grand_child;

  memset(&parent, 0, sizeof(parent));
  memset(&child, 0, sizeof(child));
  memset(&grand_child, 0, sizeof(grand_child));

  parent.width = 100.0f;
  parent.height = 100.0f;
  parent.overflow_x = UI_LAYOUT_OVERFLOW_VISIBLE;
  parent.overflow_y = UI_LAYOUT_OVERFLOW_VISIBLE;
  parent.first_child = &child;

  child.width = 50.0f;
  child.height = 50.0f;
  child.first_child = &grand_child;

  grand_child.width = 300.0f;
  grand_child.height = 300.0f;
  grand_child.overflow_x = UI_LAYOUT_OVERFLOW_HIDDEN;

  ui_layout_sanity_check(NULL);

  /* The grandchild bleeds relative to its parent (child), so sanity check on
   * parent should fail because it recurses */
  if (ui_layout_sanity_check(&parent) != UI_ERROR_LAYOUT_VIOLATION) {
    printf("Sanity check recursive failed!\n");
  }
}
static void test_flex_shrink(void) {
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *child1 = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule = NULL;
  struct ui_css_rule *rule_c1 = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "blk");
  ui_dom_node_set_attribute(root, "class", "flex-container-s");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
  ui_dom_node_set_tag_name(child1, "div");
  ui_dom_node_set_attribute(child1, "class", "flex-child-s");
  ui_dom_node_append_child(root, child1);

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS,
                              "flex-container-s");
  ui_css_rule_append_declaration(rule, "display", "flex", 0);
  ui_css_rule_append_declaration(rule, "flex-wrap", "nowrap", 0);
  ui_css_rule_append_declaration(rule, "width", "100px", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_c1);
  ui_css_rule_append_selector(rule_c1, UI_CSS_SELECTOR_TYPE_CLASS,
                              "flex-child-s");
  ui_css_rule_append_declaration(rule_c1, "width", "200px", 0);
  ui_css_rule_append_declaration(rule_c1, "flex-shrink", "1", 0);
  rule_c1->next = sheet->rules;
  sheet->rules = rule_c1;

  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot) {
    ui_layout_compute(lroot, 800, 600);
    ui_layout_tree_destroy(lroot);
  }
  ui_css_stylesheet_destroy(sheet);

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS,
                              "flex-container-s");
  ui_css_rule_append_declaration(rule, "display", "flex", 0);
  ui_css_rule_append_declaration(rule, "flex-wrap", "nowrap", 0);
  ui_css_rule_append_declaration(rule, "height", "100px", 0);
  ui_css_rule_append_declaration(rule, "flex-direction", "column", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_c1);
  ui_css_rule_append_selector(rule_c1, UI_CSS_SELECTOR_TYPE_CLASS,
                              "flex-child-s");
  ui_css_rule_append_declaration(rule_c1, "height", "200px", 0);
  ui_css_rule_append_declaration(rule_c1, "flex-shrink", "1", 0);
  rule_c1->next = sheet->rules;
  sheet->rules = rule_c1;

  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot) {
    ui_layout_compute(lroot, 800, 600);
    ui_layout_tree_destroy(lroot);
  }
  ui_css_stylesheet_destroy(sheet);

  ui_dom_node_destroy(root);
}

static void test_flex_grow(void) {
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *child1 = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule = NULL;
  struct ui_css_rule *rule_c1 = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "blk");
  ui_dom_node_set_attribute(root, "class", "flex-container-g");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
  ui_dom_node_set_tag_name(child1, "div");
  ui_dom_node_set_attribute(child1, "class", "flex-child-g");
  ui_dom_node_append_child(root, child1);

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS,
                              "flex-container-g");
  ui_css_rule_append_declaration(rule, "display", "flex", 0);
  ui_css_rule_append_declaration(rule, "width", "500px", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_c1);
  ui_css_rule_append_selector(rule_c1, UI_CSS_SELECTOR_TYPE_CLASS,
                              "flex-child-g");
  ui_css_rule_append_declaration(rule_c1, "width", "100px", 0);
  ui_css_rule_append_declaration(rule_c1, "flex-grow", "1", 0);
  rule_c1->next = sheet->rules;
  sheet->rules = rule_c1;

  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot) {
    ui_layout_compute(lroot, 800, 600);
    ui_layout_tree_destroy(lroot);
  }
  ui_css_stylesheet_destroy(sheet);

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS,
                              "flex-container-g");
  ui_css_rule_append_declaration(rule, "display", "flex", 0);
  ui_css_rule_append_declaration(rule, "height", "500px", 0);
  ui_css_rule_append_declaration(rule, "flex-direction", "column", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_c1);
  ui_css_rule_append_selector(rule_c1, UI_CSS_SELECTOR_TYPE_CLASS,
                              "flex-child-g");
  ui_css_rule_append_declaration(rule_c1, "height", "100px", 0);
  ui_css_rule_append_declaration(rule_c1, "flex-grow", "1", 0);
  rule_c1->next = sheet->rules;
  sheet->rules = rule_c1;

  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot) {
    ui_layout_compute(lroot, 800, 600);
    ui_layout_tree_destroy(lroot);
  }
  ui_css_stylesheet_destroy(sheet);

  ui_dom_node_destroy(root);
}
void test_all_invalid_properties(void);
