
static void test_extra_coverage_3(void) {
  struct ui_dom_node *root = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_css_rule *rule1 = NULL;
  struct ui_layout_node *lroot = NULL;

  /* ui_layout_compute(NULL) */
  ui_layout_compute(NULL, 100.0f, 100.0f);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_css_stylesheet_create(&sheet);

  /* root with various edge cases */
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule1);
  ui_css_rule_append_selector(rule1, UI_CSS_SELECTOR_TYPE_TAG, "div");
  /* width < min-width */
  ui_css_rule_append_declaration(rule1, "width", "10px", 0);
  ui_css_rule_append_declaration(rule1, "min-width", "50px", 0);
  /* aspect-ratio with large padding -> content_height < 0 */
  ui_css_rule_append_declaration(rule1, "aspect-ratio", "1", 0);
  ui_css_rule_append_declaration(rule1, "padding", "100px", 0);
  /* column-width and column-count clipping */
  ui_css_rule_append_declaration(rule1, "column-width", "10px", 0);
  ui_css_rule_append_declaration(rule1, "column-count", "2", 0);
  /* max-height */
  ui_css_rule_append_declaration(rule1, "max-height", "5px", 0);
  /* overflow */
  ui_css_rule_append_declaration(rule1, "overflow", "hidden", 0);

  ui_css_stylesheet_append_rule(sheet, rule1);
  ui_dom_node_set_tag_name(root, "div");

  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);

  /* ui_layout_solve_viewport with an explicit dom node that isn't html or body
   */
  ui_layout_solve_viewport(lroot, 800.0f, 600.0f);
  ui_layout_sanity_check(lroot);

  /* ui_layout_solve_viewport with anonymous root */
  struct ui_layout_node anon_root;
  memset(&anon_root, 0, sizeof(anon_root));
  ui_layout_solve_viewport(&anon_root, 800.0f, 600.0f);

  /* empty block with padding -> content_height < 0 without children */
  /* since it has width 10px, aspect-ratio will compute height, so let's
   * override height to 0 to test implicit height clamping */
  /* wait, aspect-ratio already overrides height if it was 0. */
  ui_layout_tree_destroy(lroot);

  /* Test implicit height < 0 */
  ui_css_rule_append_declaration(rule1, "aspect-ratio", "auto", 0);
  ui_css_rule_append_declaration(rule1, "height", "auto", 0);
  ui_layout_tree_generate(root, sheet, &lroot);
  ui_layout_compute(lroot, 100.0f, 100.0f);
  ui_layout_tree_destroy(lroot);

  ui_dom_node_destroy(root);
  ui_css_stylesheet_destroy(sheet);
}

int main(void) {
  test_extra_coverage_3();
  test_clamp_negative_content();
  test_flex_col_justify();
  test_flex_grow_shrink();
  test_oom_loop();
  test_extra_coverage_2();

  test_nested_overflow_violation();
  if (test_overflow_violation() != 0)
    return 1;
  test_all_invalid_properties();
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *child1 = NULL;
  struct ui_dom_node *child2 = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_css_rule *rule1 = NULL;
  struct ui_css_rule *rule2 = NULL;
  struct ui_css_rule *rule3 = NULL;
  struct ui_layout_node *lroot = NULL;
  ui_error_t rc;

  struct ui_dom_node *child3 = NULL;
  struct ui_dom_node *child3_inner = NULL;
  struct ui_css_rule *rule4 = NULL;
  struct ui_css_rule *rule5 = NULL;

  struct ui_dom_node *child4 = NULL;
  struct ui_css_rule *rule6 = NULL;

  struct ui_dom_node *child5 = NULL;
  struct ui_css_rule *rule7 = NULL;
  struct ui_dom_node *child6 = NULL;
  struct ui_css_rule *rule8 = NULL;
  struct ui_dom_node *child7 = NULL;
  struct ui_css_rule *rule9 = NULL;

  struct ui_dom_node *inner_text1 = NULL;
  struct ui_dom_node *inner_text2 = NULL;
  struct ui_dom_node *inner_text3 = NULL;

  printf("Starting test_ui_layout...\n");
  test_css_parsers();
  test_css_parsers2();
  test_layout_flex_and_shorthands();
  test_null_parsers();
  test_border_image();
  test_numeric_parsers();
  test_margin_3();
  test_lengths();
  test_more_aspect_ratios();
  test_flex_coverage();
  test_anonymous_box();
  test_anonymous_box_oom();
  test_display_none();
  test_sanity_check_violation();
  test_viewport_edge_cases();
  test_sanity_recursive();
  test_flex_shrink();
  test_flex_grow();

  /* 1. Setup DOM */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "container");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
  ui_dom_node_set_tag_name(child1, "span");
  ui_dom_node_set_attribute(child1, "class", "hidden");
  ui_dom_node_append_child(root, child1);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child2);
  ui_dom_node_set_tag_name(child2, "p");
  ui_dom_node_append_child(root, child2);

  /* Add overflow test nodes */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child3);
  ui_dom_node_set_tag_name(child3, "div");
  ui_dom_node_set_attribute(child3, "class", "scroller");
  ui_dom_node_append_child(root, child3);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child3_inner);
  ui_dom_node_set_tag_name(child3_inner, "div");
  ui_dom_node_set_attribute(child3_inner, "class", "tall-content");
  ui_dom_node_append_child(child3, child3_inner);

  /* Add min/max clamp test nodes */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child4);
  ui_dom_node_set_tag_name(child4, "div");
  ui_dom_node_set_attribute(child4, "class", "clamper");
  ui_dom_node_append_child(root, child4);

  /* Add intrinsic size test nodes */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child5);
  ui_dom_node_set_tag_name(child5, "div");
  ui_dom_node_set_attribute(child5, "class", "intrinsic-min");
  ui_dom_node_append_child(root, child5);

  /* Mock an inner block to simulate text content size */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &inner_text1);
  ui_dom_node_set_tag_name(inner_text1, "span");
  ui_dom_node_append_child(child5, inner_text1);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child6);
  ui_dom_node_set_tag_name(child6, "div");
  ui_dom_node_set_attribute(child6, "class", "intrinsic-max");
  ui_dom_node_append_child(root, child6);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &inner_text2);
  ui_dom_node_set_tag_name(inner_text2, "span");
  ui_dom_node_append_child(child6, inner_text2);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child7);
  ui_dom_node_set_tag_name(child7, "div");
  ui_dom_node_set_attribute(child7, "class", "intrinsic-fit");
  ui_dom_node_append_child(root, child7);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &inner_text3);
  ui_dom_node_set_tag_name(inner_text3, "span");
  ui_dom_node_append_child(child7, inner_text3);

  /* 2. Setup CSSOM */
  ui_css_stylesheet_create(&sheet);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule1);
  ui_css_rule_append_selector(rule1, UI_CSS_SELECTOR_TYPE_CLASS, "container");
  ui_css_rule_append_declaration(rule1, "display", "block", 0);
  ui_css_rule_append_declaration(rule1, "margin", "10px 20px 30px 40px", 0);
  ui_css_rule_append_declaration(rule1, "padding-top", "5px", 0);
  ui_css_rule_append_declaration(rule1, "border-width", "2px", 0);
  ui_css_rule_append_declaration(rule1, "width", "100px", 0);
  ui_css_rule_append_declaration(rule1, "height", "50px", 0);
  ui_css_stylesheet_append_rule(sheet, rule1);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule2);
  ui_css_rule_append_selector(rule2, UI_CSS_SELECTOR_TYPE_CLASS, "hidden");
  ui_css_rule_append_declaration(rule2, "display", "none", 0);
  ui_css_stylesheet_append_rule(sheet, rule2);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule3);
  ui_css_rule_append_selector(rule3, UI_CSS_SELECTOR_TYPE_TAG, "p");
  ui_css_rule_append_declaration(rule3, "display", "block", 0);
  ui_css_rule_append_declaration(rule3, "box-sizing", "border-box", 0);
  ui_css_rule_append_declaration(rule3, "width", "100px", 0);
  ui_css_rule_append_declaration(rule3, "height", "50px", 0);
  ui_css_rule_append_declaration(rule3, "padding", "10px", 0);
  ui_css_rule_append_declaration(rule3, "border-width", "5px", 0);
  ui_css_stylesheet_append_rule(sheet, rule3);

  /* Scroll rules */
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule4);
  ui_css_rule_append_selector(rule4, UI_CSS_SELECTOR_TYPE_CLASS, "scroller");
  ui_css_rule_append_declaration(rule4, "display", "block", 0);
  ui_css_rule_append_declaration(rule4, "overflow-y", "auto", 0);
  ui_css_rule_append_declaration(rule4, "height", "100px", 0);
  ui_css_stylesheet_append_rule(sheet, rule4);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule5);
  ui_css_rule_append_selector(rule5, UI_CSS_SELECTOR_TYPE_CLASS,
                              "tall-content");
  ui_css_rule_append_declaration(rule5, "display", "block", 0);
  ui_css_rule_append_declaration(rule5, "height", "200px", 0);
  ui_css_stylesheet_append_rule(sheet, rule5);

  /* Clamping rules */
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule6);
  ui_css_rule_append_selector(rule6, UI_CSS_SELECTOR_TYPE_CLASS, "clamper");
  ui_css_rule_append_declaration(rule6, "display", "block", 0);
  ui_css_rule_append_declaration(rule6, "width", "10px", 0); /* Too small */
  ui_css_rule_append_declaration(rule6, "min-width", "50px", 0);
  ui_css_rule_append_declaration(rule6, "height", "300px", 0); /* Too tall */
  ui_css_rule_append_declaration(rule6, "max-height", "80px", 0);
  ui_css_stylesheet_append_rule(sheet, rule6);

  /* Intrinsic Rules */
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule7);
  ui_css_rule_append_selector(rule7, UI_CSS_SELECTOR_TYPE_CLASS,
                              "intrinsic-min");
  ui_css_rule_append_declaration(rule7, "display", "block", 0);
  ui_css_rule_append_declaration(rule7, "width", "min-content", 0);
  ui_css_stylesheet_append_rule(sheet, rule7);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule8);
  ui_css_rule_append_selector(rule8, UI_CSS_SELECTOR_TYPE_CLASS,
                              "intrinsic-max");
  ui_css_rule_append_declaration(rule8, "display", "block", 0);
  ui_css_rule_append_declaration(rule8, "width", "max-content", 0);
  ui_css_stylesheet_append_rule(sheet, rule8);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule9);
  ui_css_rule_append_selector(rule9, UI_CSS_SELECTOR_TYPE_CLASS,
                              "intrinsic-fit");
  ui_css_rule_append_declaration(rule9, "display", "block", 0);
  ui_css_rule_append_declaration(rule9, "width", "fit-content", 0);
  ui_css_stylesheet_append_rule(sheet, rule9);

  /* 3. Generate Layout Tree */
  rc = ui_layout_tree_generate(root, sheet, &lroot);
  if (rc != UI_ERROR_NONE || !lroot) {
    printf("Failed to generate layout tree\n");
    return 1;
  }

  /* 4. Validate Layout Tree (child1 should be dropped, child2 should exist) */
  if (lroot->dom_node != root) {
    printf("Layout root does not match DOM root\n");
    return 1;
  }

  /* Validate Box Model metrics on lroot */
  if (lroot->margin[0] != 10.0f || lroot->margin[1] != 20.0f ||
      lroot->margin[2] != 30.0f || lroot->margin[3] != 40.0f) {
    printf("Layout margin metrics incorrect\n");
    return 1;
  }
  if (lroot->padding[0] != 5.0f || lroot->padding[1] != 0.0f) {
    printf("Layout padding metrics incorrect\n");
    return 1;
  }
  if (lroot->border[0] != 2.0f || lroot->border[1] != 2.0f) {
    printf("Layout border metrics incorrect\n");
    return 1;
  }
  if (lroot->content_width != 100.0f || lroot->content_height != 50.0f) {
    printf("Layout dimensions incorrect\n");
    return 1;
  }
  /* width = content (100) + pad L/R (0) + border L/R (4) = 104 */
  if (lroot->width != 104.0f ||
      lroot->height != 59.0f) { /* height = 50 + 5 + 0 + 4 = 59 */
    printf("Layout total width/height incorrect: w=%f, h=%f\n", lroot->width,
           lroot->height);
    return 1;
  }

  if (!lroot->first_child) {
    printf("Layout root has no children\n");
    return 1;
  }

  if (lroot->first_child->dom_node != child2) {
    printf("Layout child is not child2. Expected 'display: none' node to be "
           "dropped.\n");
    return 1;
  }

  /* Validate border-box metrics on lroot->first_child (child2) */
  if (lroot->first_child->width != 100.0f ||
      lroot->first_child->height != 50.0f) {
    printf("Layout border-box total width/height incorrect: w=%f, h=%f\n",
           lroot->first_child->width, lroot->first_child->height);
    return 1;
  }
  /* content = width (100) - pad L/R (20) - border L/R (10) = 70 */
  if (lroot->first_child->content_width != 70.0f ||
      lroot->first_child->content_height != 20.0f) {
    printf("Layout border-box content width/height incorrect: w=%f, h=%f\n",
           lroot->first_child->content_width,
           lroot->first_child->content_height);
    return 1;
  }

  if (lroot->first_child->next_sibling == NULL) {
    printf("Layout root is missing child3 (scroller)\n");
    return 1;
  }

  /* Test Viewport Root Solver (this triggers ui_layout_compute on the whole
   * tree) */
  /* Re-setup the root to simulate an html -> body tree */
  ui_dom_node_set_tag_name(root, "html");
  ui_dom_node_set_tag_name(child2, "body");

  rc = ui_layout_solve_viewport(lroot, 800.0f, 600.0f);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to solve viewport layout\n");
    return 1;
  }

  if (lroot->width != 800.0f || lroot->height != 600.0f) {
    printf("Viewport solver failed on root HTML. width=%f, height=%f\n",
           lroot->width, lroot->height);
    return 1;
  }

  if (lroot->first_child->width != 800.0f ||
      lroot->first_child->height != 600.0f) {
    printf("Viewport solver failed on child BODY. width=%f, height=%f\n",
           lroot->first_child->width, lroot->first_child->height);
    return 1;
  }

  /* Validate scrollbar track allocation on child3 (now that layout is computed)
   */
  {
    struct ui_layout_node *scroller = lroot->first_child->next_sibling;
    if (scroller->dom_node != child3) {
      printf("Expected child3 (scroller)\n");
      return 1;
    }

    if (scroller->overflow_y != UI_LAYOUT_OVERFLOW_AUTO) {
      printf("Scroller overflow_y parsed incorrectly\n");
      return 1;
    }

    if (scroller->scrollbar_width != 16.0f) {
      printf("Scroller failed to allocate scrollbar track, got %f\n",
             scroller->scrollbar_width);
      printf("Debug: scroller height=%f, content_height=%f, actual inner child "
             "height=%f\n",
             scroller->height, scroller->content_height,
             scroller->first_child ? scroller->first_child->height : -1.0f);
      return 1;
    }
  }

  /* Validate clamper constraints on child4 */
  {
    struct ui_layout_node *clamper =
        lroot->first_child->next_sibling->next_sibling;
    if (clamper->dom_node != child4) {
      printf("Expected child4 (clamper)\n");
      return 1;
    }

    if (clamper->width != 50.0f) {
      printf("Clamper failed to clamp min-width, expected 50, got %f\n",
             clamper->width);
      return 1;
    }

    if (clamper->height != 80.0f) {
      printf("Clamper failed to clamp max-height, expected 80, got %f\n",
             clamper->height);
      return 1;
    }
  }

  /* Validate intrinsic constraints on child5, child6, child7 */
  {
    struct ui_layout_node *imin =
        lroot->first_child->next_sibling->next_sibling->next_sibling;
    struct ui_layout_node *imax = imin->next_sibling;
    struct ui_layout_node *ifit = imax->next_sibling;

    if (imin->dom_node != child5 ||
        imin->width_type != UI_LAYOUT_SIZE_MIN_CONTENT) {
      printf("Expected child5 (intrinsic-min)\n");
      return 1;
    }
    if (imax->dom_node != child6 ||
        imax->width_type != UI_LAYOUT_SIZE_MAX_CONTENT) {
      printf("Expected child6 (intrinsic-max)\n");
      return 1;
    }
    if (ifit->dom_node != child7 ||
        ifit->width_type != UI_LAYOUT_SIZE_FIT_CONTENT) {
      printf("Expected child7 (intrinsic-fit)\n");
      return 1;
    }

    /* Mock sizes used in layout.c: min=20, max=100. */
    if (imin->width != 20.0f) {
      printf("min-content sizing failed, expected 20, got %f\n", imin->width);
      return 1;
    }
    if (imax->width != 100.0f) {
      printf("max-content sizing failed, expected 100, got %f\n", imax->width);
      return 1;
    }
    /* Fit content inside an 800px window is going to just be max-content (100)
     */
    if (ifit->width != 100.0f) {
      printf("fit-content sizing failed, expected 100, got %f\n", ifit->width);
      return 1;
    }
  }

  /* Run Sanity Checker */
  rc = ui_layout_sanity_check(lroot);
  if (rc == UI_ERROR_LAYOUT_VIOLATION) {
    printf(
        "Layout containment sanity check failed: bounds bleeding detected!\n");
    return 1;
  } else if (rc != UI_ERROR_NONE) {
    printf("Sanity checker failed with code %d\n", rc);
    return 1;
  }

  /* 4.5 Test Aspect Ratio & Margin Trim */
  {
    struct ui_dom_node *box_node = NULL;
    struct ui_dom_node *inner1 = NULL;
    struct ui_dom_node *inner2 = NULL;
    struct ui_layout_node *box_layout = NULL;
    struct ui_css_rule *ar_rule = NULL;
    struct ui_css_rule *inner_rule = NULL;

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &box_node);
    ui_dom_node_set_tag_name(box_node, "div");
    ui_dom_node_set_attribute(box_node, "class", "ar-box");

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &inner1);
    ui_dom_node_set_tag_name(inner1, "div");
    ui_dom_node_set_attribute(inner1, "class", "ar-inner");
    ui_dom_node_append_child(box_node, inner1);

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &inner2);
    ui_dom_node_set_tag_name(inner2, "div");
    ui_dom_node_set_attribute(inner2, "class", "ar-inner");
    ui_dom_node_append_child(box_node, inner2);

    /* width 100px, aspect-ratio 2/1 => height 50px */
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &ar_rule);
    ui_css_rule_append_selector(ar_rule, UI_CSS_SELECTOR_TYPE_CLASS, "ar-box");
    ui_css_rule_append_declaration(ar_rule, "width", "100px", 0);
    ui_css_rule_append_declaration(ar_rule, "aspect-ratio", "2/1", 0);
    ui_css_rule_append_declaration(ar_rule, "margin-trim", "all", 0);
    ar_rule->next = sheet->rules;
    sheet->rules = ar_rule;

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &inner_rule);
    ui_css_rule_append_selector(inner_rule, UI_CSS_SELECTOR_TYPE_CLASS,
                                "ar-inner");
    ui_css_rule_append_declaration(inner_rule, "margin", "20px", 0);
    inner_rule->next = sheet->rules;
    sheet->rules = inner_rule;

    rc = ui_layout_tree_generate(box_node, sheet, &box_layout);
    if (rc != UI_ERROR_NONE) {
      printf("Failed to generate layout for aspect-ratio test\n");
      return 1;
    }

    rc = ui_layout_solve_viewport(box_layout, 800.0f, 600.0f);

    /* Box should have width 100 and height 50 based on aspect ratio 2/1 */
    if (box_layout->width != 100.0f) {
      printf("Aspect ratio test failed: width %f != 100\n", box_layout->width);
      return 1;
    }
    if (box_layout->height != 50.0f) {
      printf("Aspect ratio test failed: height %f != 50\n", box_layout->height);
      return 1;
    }

    /* Inner nodes should have trimmed margins. First child top margin should be
     * 0. Last child bottom margin should be 0. */
    if (box_layout->first_child && box_layout->first_child->margin[0] != 0.0f) {
      printf("Margin trim test failed: top margin not trimmed, got %f\n",
             box_layout->first_child->margin[0]);
      return 1;
    }
    if (box_layout->last_child && box_layout->last_child->margin[2] != 0.0f) {
      printf("Margin trim test failed: bottom margin not trimmed, got %f\n",
             box_layout->last_child->margin[2]);
      return 1;
    }

    ui_layout_tree_destroy(box_layout);
    (void)ui_dom_node_destroy(box_node);
  }

  /* 4.12 Test Fragmentation Layout */
  {
    struct ui_dom_node *container = NULL;
    struct ui_dom_node *child1 = NULL;
    struct ui_dom_node *child2 = NULL;
    struct ui_dom_node *child3 = NULL;

    struct ui_layout_node *layout = NULL;

    struct ui_css_rule *container_rule = NULL;
    struct ui_css_rule *child2_rule = NULL;

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container);
    ui_dom_node_set_tag_name(container, "div");
    ui_dom_node_set_attribute(container, "class", "frag-container");

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
    ui_dom_node_set_tag_name(child1, "div");
    ui_dom_node_set_attribute(child1, "style", "height: 50px;");
    ui_dom_node_append_child(container, child1);

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child2);
    ui_dom_node_set_tag_name(child2, "div");
    ui_dom_node_set_attribute(child2, "class", "frag-child2");
    ui_dom_node_append_child(container, child2);

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child3);
    ui_dom_node_set_tag_name(child3, "div");
    ui_dom_node_set_attribute(child3, "style", "height: 50px;");
    ui_dom_node_append_child(container, child3);

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &container_rule);
    ui_css_rule_append_selector(container_rule, UI_CSS_SELECTOR_TYPE_CLASS,
                                "frag-container");
    ui_css_rule_append_declaration(container_rule, "column-count", "3", 0);
    ui_css_rule_append_declaration(container_rule, "width", "300px", 0);
    ui_css_rule_append_declaration(container_rule, "column-gap", "0px", 0);
    container_rule->next = sheet->rules;
    sheet->rules = container_rule;

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &child2_rule);
    ui_css_rule_append_selector(child2_rule, UI_CSS_SELECTOR_TYPE_CLASS,
                                "frag-child2");
    ui_css_rule_append_declaration(child2_rule, "break-before", "column", 0);
    ui_css_rule_append_declaration(child2_rule, "break-after", "column", 0);
    ui_css_rule_append_declaration(child2_rule, "height", "50px", 0);
    child2_rule->next = sheet->rules;
    sheet->rules = child2_rule;

    rc = ui_layout_tree_generate(container, sheet, &layout);
    if (rc != UI_ERROR_NONE) {
      printf("Failed to generate layout for fragmentation test\n");
      return 1;
    }

    rc = ui_layout_solve_viewport(layout, 800.0f, 600.0f);

    if (layout->first_child) {
      struct ui_layout_node *l_c1 = layout->first_child;
      struct ui_layout_node *l_c2 = l_c1->next_sibling;
      struct ui_layout_node *l_c3 = l_c2->next_sibling;

      /* Columns: width 300, 3 cols -> col width 100 */
      /* Child 1 is in col 1: x = 0 */
      if (l_c1->x != 0.0f || l_c1->y != 0.0f) {
        printf("Frag c1 pos failed: x=%f, y=%f\n", l_c1->x, l_c1->y);
        return 1;
      }
      /* Child 2 has break-before: column -> moves to col 2: x = 100 */
      if (l_c2->x != 100.0f || l_c2->y != 0.0f) {
        printf("Frag c2 pos failed: x=%f, y=%f\n", l_c2->x, l_c2->y);
        return 1;
      }
      /* Child 2 has break-after: column -> child 3 moves to col 3: x = 200 */
      if (l_c3->x != 200.0f || l_c3->y != 0.0f) {
        printf("Frag c3 pos failed: x=%f, y=%f\n", l_c3->x, l_c3->y);
        return 1;
      }
    }

    ui_layout_tree_destroy(layout);
    (void)ui_dom_node_destroy(container);
  }

  /* 5. Cleanup */
  {
    /* 4.13 Positioning and Z-Index Tests */
    struct ui_dom_node *z_root;
    struct ui_layout_node *z_layout;
    struct ui_css_rule *rule_static, *rule_rel, *rule_abs, *rule_fixed,
        *rule_sticky;
    struct ui_dom_node *n_static, *n_rel, *n_abs, *n_fixed, *n_sticky;
    struct ui_layout_node *c1, *c2, *c3, *c4, *c5;

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &z_root);
    ui_dom_node_set_tag_name(z_root, "div");

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n_static);
    ui_dom_node_set_attribute(n_static, "class", "pos-static");
    ui_dom_node_append_child(z_root, n_static);

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n_rel);
    ui_dom_node_set_attribute(n_rel, "class", "pos-rel");
    ui_dom_node_append_child(z_root, n_rel);

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n_abs);
    ui_dom_node_set_attribute(n_abs, "class", "pos-abs");
    ui_dom_node_append_child(z_root, n_abs);

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n_fixed);
    ui_dom_node_set_attribute(n_fixed, "class", "pos-fixed");
    ui_dom_node_append_child(z_root, n_fixed);

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n_sticky);
    ui_dom_node_set_attribute(n_sticky, "class", "pos-sticky");
    ui_dom_node_append_child(z_root, n_sticky);

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_static);
    ui_css_rule_append_selector(rule_static, UI_CSS_SELECTOR_TYPE_CLASS,
                                "pos-static");
    ui_css_rule_append_declaration(rule_static, "position", "static", 0);
    ui_css_rule_append_declaration(rule_static, "z-index", "auto", 0);
    rule_static->next = sheet->rules;
    sheet->rules = rule_static;

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_rel);
    ui_css_rule_append_selector(rule_rel, UI_CSS_SELECTOR_TYPE_CLASS,
                                "pos-rel");
    ui_css_rule_append_declaration(rule_rel, "position", "relative", 0);
    ui_css_rule_append_declaration(rule_rel, "z-index", "10", 0);
    rule_rel->next = sheet->rules;
    sheet->rules = rule_rel;

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_abs);
    ui_css_rule_append_selector(rule_abs, UI_CSS_SELECTOR_TYPE_CLASS,
                                "pos-abs");
    ui_css_rule_append_declaration(rule_abs, "position", "absolute", 0);
    ui_css_rule_append_declaration(rule_abs, "z-index", "5", 0);
    rule_abs->next = sheet->rules;
    sheet->rules = rule_abs;

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_fixed);
    ui_css_rule_append_selector(rule_fixed, UI_CSS_SELECTOR_TYPE_CLASS,
                                "pos-fixed");
    ui_css_rule_append_declaration(rule_fixed, "position", "fixed", 0);
    ui_css_rule_append_declaration(rule_fixed, "z-index", "999", 0);
    rule_fixed->next = sheet->rules;
    sheet->rules = rule_fixed;

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_sticky);
    ui_css_rule_append_selector(rule_sticky, UI_CSS_SELECTOR_TYPE_CLASS,
                                "pos-sticky");
    ui_css_rule_append_declaration(rule_sticky, "position", "sticky", 0);
    ui_css_rule_append_declaration(rule_sticky, "z-index", "-1", 0);
    rule_sticky->next = sheet->rules;
    sheet->rules = rule_sticky;

    rc = ui_layout_tree_generate(z_root, sheet, &z_layout);
    if (rc != UI_ERROR_NONE) {
      printf("Failed to generate layout for z-index test\n");
      return 1;
    }

    c1 = z_layout->first_child;
    c2 = c1->next_sibling;
    c3 = c2->next_sibling;
    c4 = c3->next_sibling;
    c5 = c4->next_sibling;

    if (c1->position != UI_LAYOUT_POSITION_STATIC || c1->z_index != 0) {
      printf("Static positioning failed\n");
      return 1;
    }
    if (c2->position != UI_LAYOUT_POSITION_RELATIVE || c2->z_index != 10) {
      printf("Relative positioning failed\n");
      return 1;
    }
    if (c3->position != UI_LAYOUT_POSITION_ABSOLUTE || c3->z_index != 5) {
      printf("Absolute positioning failed\n");
      return 1;
    }
    if (c4->position != UI_LAYOUT_POSITION_FIXED || c4->z_index != 999) {
      printf("Fixed positioning failed\n");
      return 1;
    }
    if (c5->position != UI_LAYOUT_POSITION_STICKY || c5->z_index != -1) {
      printf("Sticky positioning failed\n");
      return 1;
    }

    ui_layout_tree_destroy(z_layout);
    (void)ui_dom_node_destroy(z_root);
  }
  /* 5. Cleanup */
  {
    /* 4.14 Stacking Context Tests */
    struct ui_dom_node *s_root;
    struct ui_layout_node *s_layout;
    struct ui_css_rule *rule_op, *rule_tx, *rule_z;

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &s_root);
    ui_dom_node_set_tag_name(s_root, "div");

    struct ui_dom_node *n_op, *n_tx, *n_z, *n_no;
    struct ui_layout_node *s1, *s2, *s3, *s4;

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n_op);
    ui_dom_node_set_attribute(n_op, "class", "stack-op");
    ui_dom_node_append_child(s_root, n_op);

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n_tx);
    ui_dom_node_set_attribute(n_tx, "class", "stack-tx");
    ui_dom_node_append_child(s_root, n_tx);

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n_z);
    ui_dom_node_set_attribute(n_z, "class", "stack-z");
    ui_dom_node_append_child(s_root, n_z);

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &n_no);
    ui_dom_node_set_attribute(n_no, "class", "stack-no");
    ui_dom_node_append_child(s_root, n_no);

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_op);
    ui_css_rule_append_selector(rule_op, UI_CSS_SELECTOR_TYPE_CLASS,
                                "stack-op");
    ui_css_rule_append_declaration(rule_op, "opacity", "0.5", 0);
    rule_op->next = sheet->rules;
    sheet->rules = rule_op;

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_tx);
    ui_css_rule_append_selector(rule_tx, UI_CSS_SELECTOR_TYPE_CLASS,
                                "stack-tx");
    ui_css_rule_append_declaration(rule_tx, "transform", "scale(2)", 0);
    rule_tx->next = sheet->rules;
    sheet->rules = rule_tx;

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule_z);
    ui_css_rule_append_selector(rule_z, UI_CSS_SELECTOR_TYPE_CLASS, "stack-z");
    ui_css_rule_append_declaration(rule_z, "position", "relative", 0);
    ui_css_rule_append_declaration(rule_z, "z-index", "1", 0);
    rule_z->next = sheet->rules;
    sheet->rules = rule_z;

    rc = ui_layout_tree_generate(s_root, sheet, &s_layout);
    if (rc != UI_ERROR_NONE) {
      printf("Failed to generate layout for stacking test\n");
      return 1;
    }

    s1 = s_layout->first_child;
    s2 = s1->next_sibling;
    s3 = s2->next_sibling;
    s4 = s3->next_sibling;

    if (!s1->is_stacking_context) {
      printf("Opacity stacking context failed\n");
      return 1;
    }
    if (!s2->is_stacking_context) {
      printf("Transform stacking context failed\n");
      return 1;
    }
    if (!s3->is_stacking_context) {
      printf("Z-index stacking context failed\n");
      return 1;
    }
    if (s4->is_stacking_context) {
      printf("Non-stacking context failed\n");
      return 1;
    }

    ui_layout_tree_destroy(s_layout);
    (void)ui_dom_node_destroy(s_root);
  }
  /* 5. Cleanup */

  ui_layout_tree_destroy(lroot);

  /* Test Error Percolation (Mock Memory Failures) */
  g_malloc_fail_countdown = 0;
  rc = ui_layout_tree_generate(root, sheet, &lroot);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Expected OUT_OF_MEMORY on generate (allocating style)\n");
    return 1;
  }

  /* Try forcing failure deeper in the tree */
  g_malloc_fail_countdown = 1;
  rc = ui_layout_tree_generate(root, sheet, &lroot);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Expected OUT_OF_MEMORY on generate (deep 1)\n");
    return 1;
  }

  g_malloc_fail_countdown = 2;
  rc = ui_layout_tree_generate(root, sheet, &lroot);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Expected OUT_OF_MEMORY on generate (deep 2)\n");
    return 1;
  }

  g_malloc_fail_countdown = 3;
  rc = ui_layout_tree_generate(root, sheet, &lroot);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Expected OUT_OF_MEMORY on generate (deep 3)\n");
    return 1;
  }

  (void)ui_dom_node_destroy(root);
  ui_css_stylesheet_destroy(sheet);

  test_extra_coverage();
  test_more_layout();
  test_coverage_layout();
  test_all_invalid_properties();

  /* Explicit missing branches */
  {
    struct ui_dom_node *root;
    struct ui_css_stylesheet *sheet;
    struct ui_layout_node *lroot = NULL;
    struct ui_css_rule *rule;

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
    ui_dom_node_set_tag_name(root, "div");

    ui_css_stylesheet_create(&sheet);
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");

    ui_css_rule_append_declaration(rule, "margin", "1px 2px 3px 4px 5px", 0);
    ui_css_rule_append_declaration(rule, "padding", "", 0);
    ui_css_rule_append_declaration(rule, "width", "1rem", 0);
    ui_css_rule_append_declaration(rule, "max-width", "none", 0);
    ui_css_rule_append_declaration(rule, "max-height", "none", 0);

    ui_css_stylesheet_append_rule(sheet, rule);

    ui_layout_tree_generate(root, sheet, &lroot);
    if (lroot) {
      ui_layout_compute(lroot, 100.0f, 100.0f);
      ui_layout_tree_destroy(lroot);
    }
    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(root);
  }

  {
    struct ui_dom_node *root;
    struct ui_css_stylesheet *sheet;
    struct ui_layout_node *lroot = NULL;
    struct ui_css_rule *rule;

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
    ui_dom_node_set_tag_name(root, "div");

    ui_css_stylesheet_create(&sheet);
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");

    ui_css_rule_append_declaration(rule, "transform", "none", 0);
    ui_css_rule_append_declaration(rule, "width", "100px", 0);
    ui_css_rule_append_declaration(rule, "max-width", "50px", 0);
    ui_css_rule_append_declaration(rule, "height", "100px", 0);
    ui_css_rule_append_declaration(rule, "max-height", "50px", 0);

    ui_css_stylesheet_append_rule(sheet, rule);

    ui_layout_tree_generate(root, sheet, &lroot);
    if (lroot) {
      ui_layout_compute(lroot, 100.0f, 100.0f);
      ui_layout_tree_destroy(lroot);
    }
    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(root);
  }

  {
    struct ui_dom_node *root;
    struct ui_css_stylesheet *sheet;
    struct ui_layout_node *lroot = NULL;
    struct ui_css_rule *rule;

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
    ui_dom_node_set_tag_name(root, "div");

    ui_css_stylesheet_create(&sheet);
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");

    ui_css_rule_append_declaration(rule, "width", "10px", 0);
    ui_css_rule_append_declaration(rule, "max-width", "50px", 0);
    ui_css_rule_append_declaration(rule, "height", "10px", 0);
    ui_css_rule_append_declaration(rule, "max-height", "50px", 0);

    ui_css_stylesheet_append_rule(sheet, rule);

    struct ui_dom_node *t1, *t2;
    ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &t1);
    ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &t2);
    ui_dom_node_append_child(root, t1);
    ui_dom_node_append_child(root, t2);

    ui_layout_tree_generate(root, sheet, &lroot);
    if (lroot) {
      ui_layout_compute(lroot, 100.0f, 100.0f);
      ui_layout_tree_destroy(lroot);
    }
    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(root);
  }
  printf("All layout tree tests passed.\n");
  return 0;
}
/* clang-format off */
#include "../include/ui_cssom.h"
#include "../include/ui_dom_node.h"
#include "../include/ui_layout.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */
