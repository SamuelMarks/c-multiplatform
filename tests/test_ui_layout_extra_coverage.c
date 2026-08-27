
static void test_extra_coverage(void) {
  struct ui_dom_node *root;
  struct ui_css_stylesheet *sheet;
  struct ui_layout_node *lroot;
  struct ui_css_rule *rule;
  struct ui_layout_node *fake_child;
  struct ui_dom_node *mix_root, *b1, *i1, *b2;
  int i, j;
  const char *displays[] = {"none",  "run-in",    "grid",
                            "table", "flow-root", "inline-block"};
  const char *justify_vals[] = {"space-evenly", "space-between",
                                "space-around"};

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  for (i = 0; i < 6; ++i) {
    struct ui_css_stylesheet *temp_sheet;
    struct ui_css_rule *temp_rule;
    ui_css_stylesheet_create(&temp_sheet);
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &temp_rule);
    ui_css_rule_append_selector(temp_rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
    ui_css_rule_append_declaration(temp_rule, "display", displays[i], 0);
    ui_css_stylesheet_append_rule(temp_sheet, temp_rule);
    ui_layout_tree_generate(root, temp_sheet, &lroot);
    ui_layout_tree_destroy(lroot);
    ui_css_stylesheet_destroy(temp_sheet);
  }
  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_stylesheet_append_rule(sheet, rule);

  ui_css_rule_append_declaration(rule, "hyphens", "manual", 0);
  ui_css_rule_append_declaration(rule, "overflow-wrap", "break-word", 0);
  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_tree_destroy(lroot);

  /* test child bounding shrinking to 0 for height */
  ui_css_rule_append_declaration(rule, "display", "block", 0);
  ui_css_rule_append_declaration(rule, "height", "-10px", 0);
  ui_css_rule_append_declaration(rule, "width", "-10px", 0);
  ui_css_rule_append_declaration(rule, "padding", "10px", 0);
  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  /* columns */
  ui_css_rule_append_declaration(rule, "columns", "3", 0);
  ui_css_rule_append_declaration(rule, "column-gap", "10px", 0);
  ui_css_rule_append_declaration(rule, "column-width", "50px", 0);
  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 10.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  /* percentage root */
  ui_css_rule_append_declaration(rule, "display", "block", 0);
  ui_css_rule_append_declaration(rule, "width", "100%", 0);
  ui_css_rule_append_declaration(rule, "height", "100%", 0);
  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  /* sanity check */
  ui_css_rule_append_declaration(rule, "display", "block", 0);
  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_tree_generate(root, sheet, &fake_child);
  fake_child->parent = NULL;
  fake_child->next_sibling = NULL;
  fake_child->previous_sibling = NULL;
  lroot->first_child = fake_child;
  ui_layout_sanity_check(lroot);
  lroot->first_child = NULL;
  ui_layout_tree_destroy(fake_child);
  ui_layout_tree_destroy(lroot);

  /* mixed inline/block children */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &mix_root);
  ui_dom_node_set_tag_name(mix_root, "div");
  ui_dom_node_set_attribute(mix_root, "class", "mixroot");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &b1);
  ui_dom_node_set_tag_name(b1, "div");
  ui_dom_node_set_attribute(b1, "class", "b1");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &i1);
  ui_dom_node_set_tag_name(i1, "span");
  ui_dom_node_set_attribute(i1, "class", "i1");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &b2);
  ui_dom_node_set_tag_name(b2, "div");
  ui_dom_node_set_attribute(b2, "class", "b2");

  struct ui_dom_node *tb1, *ti1, *tb2;
  ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &tb1);
  ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &ti1);
  ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &tb2);
  ui_dom_node_append_child(b1, tb1);
  ui_dom_node_append_child(i1, ti1);
  ui_dom_node_append_child(b2, tb2);

  ui_dom_node_append_child(mix_root, i1);
  ui_dom_node_append_child(mix_root, b1);
  ui_dom_node_append_child(mix_root, b2);

  {
    struct ui_css_stylesheet *mix_sheet;
    struct ui_css_rule *r_mixroot, *r_b1, *r_i1, *r_b2;
    ui_css_stylesheet_create(&mix_sheet);

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r_mixroot);
    ui_css_rule_append_selector(r_mixroot, UI_CSS_SELECTOR_TYPE_TAG, "mixroot");
    ui_css_rule_append_declaration(r_mixroot, "display", "block", 0);
    ui_css_stylesheet_append_rule(mix_sheet, r_mixroot);

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r_b1);
    ui_css_rule_append_selector(r_b1, UI_CSS_SELECTOR_TYPE_TAG, "b1");
    ui_css_rule_append_declaration(r_b1, "display", "block", 0);
    ui_css_stylesheet_append_rule(mix_sheet, r_b1);

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r_i1);
    ui_css_rule_append_selector(r_i1, UI_CSS_SELECTOR_TYPE_TAG, "i1");
    ui_css_rule_append_declaration(r_i1, "display", "inline", 0);
    ui_css_stylesheet_append_rule(mix_sheet, r_i1);

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r_b2);
    ui_css_rule_append_selector(r_b2, UI_CSS_SELECTOR_TYPE_TAG, "b2");
    ui_css_rule_append_declaration(r_b2, "display", "block", 0);
    ui_css_stylesheet_append_rule(mix_sheet, r_b2);

    ui_layout_tree_generate(mix_root, mix_sheet, &lroot);
    ui_layout_compute(lroot, 100.0f, 100.0f);
    ui_layout_tree_destroy(lroot);
    ui_css_stylesheet_destroy(mix_sheet);
  }

  for (j = 0; j < 3; ++j) {
    struct ui_css_stylesheet *mix_sheet;
    struct ui_css_rule *r_mixroot, *r_i1;
    ui_css_stylesheet_create(&mix_sheet);

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r_mixroot);
    ui_css_rule_append_selector(r_mixroot, UI_CSS_SELECTOR_TYPE_TAG, "mixroot");
    ui_css_rule_append_declaration(r_mixroot, "display", "flex", 0);
    ui_css_rule_append_declaration(r_mixroot, "flex-direction", "column", 0);
    ui_css_rule_append_declaration(r_mixroot, "justify-content",
                                   justify_vals[j], 0);
    ui_css_rule_append_declaration(r_mixroot, "align-items", "center", 0);
    ui_css_rule_append_declaration(r_mixroot, "wrap-through", "wrap", 0);
    ui_css_stylesheet_append_rule(mix_sheet, r_mixroot);

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r_i1);
    ui_css_rule_append_selector(r_i1, UI_CSS_SELECTOR_TYPE_TAG, "i1");
    ui_css_rule_append_declaration(r_i1, "display", "flex", 0);
    ui_css_rule_append_declaration(r_i1, "flex-shrink", "1", 0);
    ui_css_rule_append_declaration(r_i1, "height", "200px", 0);
    ui_css_stylesheet_append_rule(mix_sheet, r_i1);

    ui_layout_tree_generate(mix_root, mix_sheet, &lroot);
    ui_layout_compute(lroot, 100.0f, 100.0f);
    ui_layout_tree_destroy(lroot);

    ui_css_stylesheet_destroy(mix_sheet);
  }

  (void)ui_dom_node_destroy(mix_root);

  (void)ui_dom_node_destroy(root);
  ui_css_stylesheet_destroy(sheet);
}

void test_more_layout(void);
void test_coverage_layout(void);
static void test_flex_col_justify(void);
static void test_clamp_negative_content(void);
static void test_flex_grow_shrink(void);
static void test_oom_loop(void);

static void test_nested_overflow_violation(void) {
  struct ui_dom_node *root, *child, *grandchild;
  struct ui_css_stylesheet *sheet;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *r1, *r2, *r3;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "ov-root");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child);
  ui_dom_node_set_tag_name(child, "div");
  ui_dom_node_set_attribute(child, "class", "ov-child");
  ui_dom_node_append_child(root, child);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &grandchild);
  ui_dom_node_set_tag_name(grandchild, "div");
  ui_dom_node_set_attribute(grandchild, "class", "ov-grandchild");
  ui_dom_node_append_child(child, grandchild);

  ui_css_stylesheet_create(&sheet);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r1);
  ui_css_rule_append_selector(r1, UI_CSS_SELECTOR_TYPE_CLASS, "ov-root");
  ui_css_rule_append_declaration(r1, "width", "100px", 0);
  ui_css_rule_append_declaration(r1, "height", "100px", 0);
  ui_css_rule_append_declaration(r1, "overflow", "hidden", 0);
  ui_css_stylesheet_append_rule(sheet, r1);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r2);
  ui_css_rule_append_selector(r2, UI_CSS_SELECTOR_TYPE_CLASS, "ov-child");
  ui_css_rule_append_declaration(r2, "width", "50px", 0);
  ui_css_rule_append_declaration(r2, "height", "50px", 0);
  ui_css_rule_append_declaration(r2, "overflow", "hidden", 0);
  ui_css_stylesheet_append_rule(sheet, r2);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r3);
  ui_css_rule_append_selector(r3, UI_CSS_SELECTOR_TYPE_CLASS, "ov-grandchild");
  ui_css_rule_append_declaration(r3, "width", "200px", 0);
  ui_css_rule_append_declaration(r3, "height", "200px", 0);
  ui_css_stylesheet_append_rule(sheet, r3);

  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot) {
    ui_layout_compute(lroot, 100.0f, 100.0f);

    ui_error_t check_rc = ui_layout_sanity_check(lroot);
    if (check_rc != UI_ERROR_LAYOUT_VIOLATION) {
      printf("Expected nested layout violation\n");
    }

    ui_layout_tree_destroy(lroot);
  }

  ui_css_stylesheet_destroy(sheet);
  (void)ui_dom_node_destroy(root);
}

static int test_overflow_violation(void) {
  struct ui_dom_node *root, *child;
  struct ui_css_stylesheet *sheet;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *r1, *r2;
  ui_error_t rc;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "ov-root");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child);
  ui_dom_node_set_tag_name(child, "div");
  ui_dom_node_set_attribute(child, "class", "ov-child");
  ui_dom_node_append_child(root, child);

  ui_css_stylesheet_create(&sheet);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r1);
  ui_css_rule_append_selector(r1, UI_CSS_SELECTOR_TYPE_CLASS, "ov-root");
  ui_css_rule_append_declaration(r1, "display", "block", 0);
  ui_css_rule_append_declaration(r1, "width", "100px", 0);
  ui_css_rule_append_declaration(r1, "height", "100px", 0);
  ui_css_rule_append_declaration(r1, "overflow", "hidden", 0);
  ui_css_stylesheet_append_rule(sheet, r1);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r2);
  ui_css_rule_append_selector(r2, UI_CSS_SELECTOR_TYPE_CLASS, "ov-child");
  ui_css_rule_append_declaration(r2, "display", "block", 0);
  ui_css_rule_append_declaration(r2, "width", "200px", 0); /* bleeds! */
  ui_css_rule_append_declaration(r2, "height", "200px", 0);
  ui_css_stylesheet_append_rule(sheet, r2);

  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 800, 600);

  rc = ui_layout_sanity_check(lroot);
  if (rc != UI_ERROR_LAYOUT_VIOLATION) {
    printf("Expected UI_ERROR_LAYOUT_VIOLATION for bleeding overflow hidden\n");
    return 1;
  }

  /* Now fix X and bleed Y */
  ui_css_rule_append_declaration(r2, "width", "50px", 0);
  ui_css_rule_append_declaration(r2, "height", "200px", 0);
  ui_layout_compute(lroot, 800, 600);
  rc = ui_layout_sanity_check(lroot);
  if (rc != UI_ERROR_LAYOUT_VIOLATION) {
    printf("Expected UI_ERROR_LAYOUT_VIOLATION for Y bleeding\n");
    return 1;
  }

  ui_layout_tree_destroy(lroot);
  ui_css_stylesheet_destroy(sheet);
  ui_dom_node_destroy(root);
  return 0;
}

static void test_extra_coverage_2(void) {
  struct ui_dom_node *root = NULL, *child1 = NULL, *child2 = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_css_rule *rule1 = NULL, *rule2 = NULL, *rule3 = NULL;
  struct ui_layout_node *lroot = NULL;

  /* ui_layout_solve_viewport(NULL) */
  ui_layout_solve_viewport(NULL, 100.0f, 100.0f);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child2);
  ui_dom_node_append_child(root, child1);
  ui_dom_node_append_child(root, child2);

  ui_css_stylesheet_create(&sheet);

  /* root */
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule1);
  ui_css_rule_append_selector(rule1, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_rule_append_declaration(rule1, "width", "100px", 0);
  ui_css_stylesheet_append_rule(sheet, rule1);
  ui_dom_node_set_tag_name(root, "div");

  /* child1: inline */
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule2);
  ui_css_rule_append_selector(rule2, UI_CSS_SELECTOR_TYPE_TAG, "span");
  ui_css_rule_append_declaration(rule2, "display", "inline", 0);
  ui_css_stylesheet_append_rule(sheet, rule2);
  ui_dom_node_set_tag_name(child1, "span");

  /* child2: block */
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule3);
  ui_css_rule_append_selector(rule3, UI_CSS_SELECTOR_TYPE_TAG, "p");
  ui_css_rule_append_declaration(rule3, "display", "block", 0);
  ui_css_stylesheet_append_rule(sheet, rule3);
  ui_dom_node_set_tag_name(child2, "p");

  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_solve_viewport(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);
  ui_dom_node_destroy(root);
  ui_css_stylesheet_destroy(sheet);

  /* fit-content test */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
  struct ui_dom_node *grandchild = NULL;
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &grandchild);
  ui_dom_node_append_child(child1, grandchild);
  ui_dom_node_append_child(root, child1);
  ui_css_stylesheet_create(&sheet);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule1);
  ui_css_rule_append_selector(rule1, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_rule_append_declaration(rule1, "width", "fit-content", 0);
  ui_css_stylesheet_append_rule(sheet, rule1);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_tag_name(child1, "div");

  ui_layout_tree_generate(root, sheet, &lroot);
  /* Compute layout explicitly on the root to trigger child1 layout */
  ui_layout_compute(lroot, 5.0f, 100.0f);

  ui_layout_tree_destroy(lroot);
  ui_dom_node_destroy(root);
  ui_css_stylesheet_destroy(sheet);

  /* break-before / break-after false branches test */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
  ui_dom_node_append_child(root, child1);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_tag_name(child1, "div");
  ui_css_stylesheet_create(&sheet);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule1);
  ui_css_rule_append_selector(rule1, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_rule_append_declaration(rule1, "break-before", "page", 0);
  ui_css_rule_append_declaration(rule1, "break-after", "page", 0);
  ui_css_rule_append_declaration(rule1, "margin-trim", "block", 0);
  ui_css_stylesheet_append_rule(sheet, rule1);

  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  /* more margin-trim coverage */
  rule2 = NULL;
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule2);
  ui_css_rule_append_selector(rule2, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_rule_append_declaration(rule2, "margin-trim", "block-start", 0);
  ui_css_stylesheet_append_rule(sheet, rule2);
  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  ui_css_rule_append_declaration(rule2, "margin-trim", "block-end", 0);
  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  ui_css_rule_append_declaration(rule2, "margin-trim", "inline-start", 0);
  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  ui_css_rule_append_declaration(rule2, "margin-trim", "inline-end", 0);
  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  /* overflow-y scroll test */
  ui_css_rule_append_declaration(rule2, "overflow-y", "scroll", 0);
  ui_css_rule_append_declaration(rule2, "height", "10px",
                                 0); /* explicit height needed */
  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_sanity_check(lroot);
  ui_layout_tree_destroy(lroot);

  /* overflow-y auto without overflow test */
  ui_css_rule_append_declaration(rule2, "overflow-y", "auto", 0);
  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  /* inline-flex test */
  ui_css_rule_append_declaration(rule2, "display", "inline-flex", 0);
  ui_css_rule_append_declaration(rule2, "width", "auto", 0);

  rule3 = NULL;
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule3);
  ui_css_rule_append_selector(rule3, UI_CSS_SELECTOR_TYPE_TAG, "span");
  ui_css_rule_append_declaration(rule3, "flex-basis", "50%", 0);
  ui_css_stylesheet_append_rule(sheet, rule3);

  struct ui_dom_node *flex_child = NULL;
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &flex_child);
  ui_dom_node_set_tag_name(flex_child, "span");
  ui_dom_node_append_child(root, flex_child);

  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  /* flex wrap > 64 lines test */
  ui_css_rule_append_declaration(rule2, "display", "flex", 0);
  ui_css_rule_append_declaration(rule2, "flex-wrap", "wrap", 0);
  ui_css_rule_append_declaration(rule2, "width", "10px",
                                 0); /* very narrow container */

  struct ui_css_rule *span_rule = NULL;
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &span_rule);
  ui_css_rule_append_selector(span_rule, UI_CSS_SELECTOR_TYPE_TAG, "span");
  ui_css_rule_append_declaration(span_rule, "width", "20px", 0);
  ui_css_stylesheet_append_rule(sheet, span_rule);

  struct ui_dom_node *wrap_children[65];
  int i;
  for (i = 0; i < 65; i++) {
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &wrap_children[i]);
    ui_dom_node_set_tag_name(wrap_children[i], "span");
    ui_dom_node_append_child(root, wrap_children[i]);
  }

  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  /* justify-content: space-between with 1 child test */
  ui_css_rule_append_declaration(rule2, "display", "flex", 0);
  ui_css_rule_append_declaration(rule2, "justify-content", "space-between", 0);
  ui_css_rule_append_declaration(rule2, "width", "100px", 0);

  struct ui_dom_node *flex_child2 = NULL;
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &flex_child2);
  ui_dom_node_set_tag_name(flex_child2, "span");
  ui_dom_node_append_child(root, flex_child2);

  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  ui_dom_node_destroy(root);
  ui_css_stylesheet_destroy(sheet);
}
