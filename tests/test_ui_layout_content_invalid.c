
void test_content_negative(void) {
  struct ui_dom_node *root, *c1, *c2;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule, *r1, *r2;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "cnt-root");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c1);
  ui_dom_node_set_tag_name(c1, "div");
  ui_dom_node_set_attribute(c1, "class", "cnt-c1");
  ui_dom_node_append_child(root, c1);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c2);
  ui_dom_node_set_tag_name(c2, "div");
  ui_dom_node_set_attribute(c2, "class", "cnt-c2");
  ui_dom_node_append_child(root, c2);

  ui_css_stylesheet_create(&sheet);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "cnt-root");
  ui_css_rule_append_declaration(rule, "display", "block", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r1);
  ui_css_rule_append_selector(r1, UI_CSS_SELECTOR_TYPE_CLASS, "cnt-c1");
  ui_css_rule_append_declaration(r1, "display", "block", 0);
  ui_css_rule_append_declaration(r1, "width", "10px", 0);
  ui_css_rule_append_declaration(r1, "height", "10px", 0);
  ui_css_rule_append_declaration(r1, "padding", "20px", 0);
  ui_css_rule_append_declaration(r1, "border-width", "20px", 0);
  r1->next = sheet->rules;
  sheet->rules = r1;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r2);
  ui_css_rule_append_selector(r2, UI_CSS_SELECTOR_TYPE_CLASS, "cnt-c2");
  ui_css_rule_append_declaration(r2, "display", "flex", 0);
  ui_css_rule_append_declaration(r2, "width", "10px", 0);
  ui_css_rule_append_declaration(r2, "height", "10px", 0);
  ui_css_rule_append_declaration(r2, "padding", "20px", 0);
  ui_css_rule_append_declaration(r2, "border-width", "20px", 0);
  r2->next = sheet->rules;
  sheet->rules = r2;

  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot) {
    ui_layout_compute(lroot, 100, 100);
    ui_layout_tree_destroy(lroot);
  }

  ui_css_stylesheet_destroy(sheet);
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(root);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
}
/* clang-format off */
#include "../include/ui_cssom.h"
#include "../include/ui_dom_node.h"
#include "../include/ui_layout.h"
/* clang-format on */

static void test_flex_grow_shrink(void) {
  struct ui_dom_node *root, *c1, *c2;
  struct ui_css_stylesheet *sheet;
  struct ui_css_rule *rule;
  struct ui_layout_node *lroot;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "root");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c1);
  ui_dom_node_set_tag_name(c1, "div");
  ui_dom_node_set_attribute(c1, "class", "child1");
  ui_dom_node_append_child(root, c1);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c2);
  ui_dom_node_set_tag_name(c2, "div");
  ui_dom_node_set_attribute(c2, "class", "child2");
  ui_dom_node_append_child(root, c2);

  ui_css_stylesheet_create(&sheet);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "root");
  ui_css_rule_append_declaration(rule, "display", "flex", 0);
  ui_css_rule_append_declaration(rule, "width", "100px", 0);
  ui_css_stylesheet_append_rule(sheet, rule);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "child1");
  ui_css_rule_append_declaration(rule, "flex-grow", "1", 0);
  ui_css_rule_append_declaration(rule, "width", "10px", 0);
  ui_css_stylesheet_append_rule(sheet, rule);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "child2");
  ui_css_rule_append_declaration(rule, "flex-grow", "1", 0);
  ui_css_rule_append_declaration(rule, "width", "10px", 0);
  ui_css_stylesheet_append_rule(sheet, rule);

  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot) {
    ui_layout_compute(lroot, 100.0f, 100.0f);
    ui_layout_tree_destroy(lroot);
  }

  /* Shrink */
  ui_css_stylesheet_destroy(sheet);
  ui_css_stylesheet_create(&sheet);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "root");
  ui_css_rule_append_declaration(rule, "display", "flex", 0);
  ui_css_rule_append_declaration(rule, "width", "10px", 0);
  ui_css_stylesheet_append_rule(sheet, rule);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "child1");
  ui_css_rule_append_declaration(rule, "flex-shrink", "1", 0);
  ui_css_rule_append_declaration(rule, "width", "20px", 0);
  ui_css_stylesheet_append_rule(sheet, rule);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "child2");
  ui_css_rule_append_declaration(rule, "flex-shrink", "1", 0);
  ui_css_rule_append_declaration(rule, "width", "20px", 0);
  ui_css_stylesheet_append_rule(sheet, rule);

  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot) {
    ui_layout_compute(lroot, 100.0f, 100.0f);
    ui_layout_tree_destroy(lroot);
  }

  ui_css_stylesheet_destroy(sheet);
  ui_dom_node_destroy(root);
}

static void test_oom_loop(void) {
  struct ui_dom_node *root, *c1, *c2, *c3;
  struct ui_css_stylesheet *sheet;
  struct ui_css_rule *rule;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "root");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c1);
  ui_dom_node_set_tag_name(c1, "div");
  ui_dom_node_set_attribute(c1, "class", "child1");
  ui_dom_node_append_child(root, c1);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c2);
  ui_dom_node_set_tag_name(c2, "div");
  ui_dom_node_set_attribute(c2, "class", "child2");
  ui_dom_node_append_child(root, c2);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c3);
  ui_dom_node_set_tag_name(c3, "div");
  ui_dom_node_set_attribute(c3, "class", "child3");
  ui_dom_node_append_child(c2, c3);

  ui_css_stylesheet_create(&sheet);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "root");
  ui_css_rule_append_declaration(rule, "display", "flex", 0);
  ui_css_rule_append_declaration(rule, "width", "100px", 0);
  ui_css_stylesheet_append_rule(sheet, rule);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "child1");
  ui_css_rule_append_declaration(rule, "flex-grow", "1", 0);
  ui_css_rule_append_declaration(rule, "width", "10px", 0);
  ui_css_stylesheet_append_rule(sheet, rule);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "child2");
  ui_css_rule_append_declaration(rule, "display", "flex", 0);
  ui_css_rule_append_declaration(rule, "flex-shrink", "1", 0);
  ui_css_rule_append_declaration(rule, "width", "10px", 0);
  ui_css_stylesheet_append_rule(sheet, rule);

  int oom_cnt;
  for (oom_cnt = 0; oom_cnt < 200; oom_cnt++) {
    struct ui_layout_node *lroot = NULL;

    ui_error_t rc = ui_layout_tree_generate(root, sheet, &lroot);
    if (rc == UI_ERROR_NONE && lroot) {
      g_malloc_fail_countdown = oom_cnt;
      ui_layout_compute(lroot, 100.0f, 100.0f);
      ui_layout_sanity_check(lroot);
      g_malloc_fail_countdown = -1;
      ui_layout_tree_destroy(lroot);
    } else if (lroot) {
      ui_layout_tree_destroy(lroot);
    }
  }
  g_malloc_fail_countdown = -1;

  ui_css_stylesheet_destroy(sheet);
  ui_dom_node_destroy(root);
}

static void test_clamp_negative_content(void) {
  struct ui_dom_node *root;
  struct ui_css_stylesheet *sheet;
  struct ui_css_rule *rule;
  struct ui_layout_node *lroot;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "html");
  ui_dom_node_set_attribute(root, "class", "root");

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "root");
  ui_css_rule_append_declaration(rule, "padding", "10px", 0);
  ui_css_rule_append_declaration(rule, "border-width", "5px", 0);
  ui_css_stylesheet_append_rule(sheet, rule);

  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot) {
    ui_layout_solve_viewport(lroot, 0.0f,
                             0.0f); /* Window smaller than padding+border */
    ui_layout_compute(lroot, 0.0f, 0.0f);
    ui_layout_tree_destroy(lroot);
  }

  ui_css_stylesheet_destroy(sheet);
  ui_dom_node_destroy(root);
}

static void test_flex_col_justify(void) {
  struct ui_dom_node *root;
  struct ui_dom_node *c1, *c2;
  struct ui_css_stylesheet *sheet;
  struct ui_css_rule *rule;
  struct ui_layout_node *lroot;

  const char *aligns[] = {"flex-end", "center", "space-between", "space-around",
                          "space-evenly"};
  int i;
  for (i = 0; i < 5; i++) {
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
    ui_dom_node_set_tag_name(root, "div");
    ui_dom_node_set_attribute(root, "class", "root");

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c1);
    ui_dom_node_set_tag_name(c1, "div");
    ui_dom_node_set_attribute(c1, "class", "child");
    ui_dom_node_append_child(root, c1);

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c2);
    ui_dom_node_set_tag_name(c2, "div");
    ui_dom_node_set_attribute(c2, "class", "child");
    ui_dom_node_append_child(root, c2);

    ui_css_stylesheet_create(&sheet);
    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "root");
    ui_css_rule_append_declaration(rule, "display", "flex", 0);
    ui_css_rule_append_declaration(rule, "flex-direction", "column", 0);
    ui_css_rule_append_declaration(rule, "justify-content", aligns[i], 0);
    ui_css_rule_append_declaration(rule, "align-items", "center", 0);
    ui_css_rule_append_declaration(rule, "height", "100px", 0);
    ui_css_rule_append_declaration(rule, "width", "100px", 0);
    ui_css_stylesheet_append_rule(sheet, rule);

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
    ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "child");
    ui_css_rule_append_declaration(rule, "height", "10px", 0);
    ui_css_stylesheet_append_rule(sheet, rule);

    ui_layout_tree_generate(root, sheet, &lroot);
    if (lroot) {
      ui_layout_compute(lroot, 50, 100);
      ui_layout_tree_destroy(lroot);
    }
    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(root);
  }
}

void test_all_invalid_properties(void) {
  const char *props[] = {"align-content",
                         "align-items",
                         "align-self",
                         "aspect-ratio",
                         "background-color",
                         "background-image",
                         "background-repeat",
                         "background-size",
                         "block-step-align",
                         "block-step-insert",
                         "block-step-round",
                         "block-step-size",
                         "border-bottom-color",
                         "border-bottom-left-radius",
                         "border-bottom-right-radius",
                         "border-bottom-width",
                         "border-image",
                         "border-image-source",
                         "border-left-color",
                         "border-left-width",
                         "border-radius",
                         "border-right-color",
                         "border-right-width",
                         "border-top-color",
                         "border-top-left-radius",
                         "border-top-right-radius",
                         "border-top-width",
                         "border-width",
                         "bottom",
                         "box-decoration-break",
                         "box-shadow",
                         "box-sizing",
                         "box-snap",
                         "break-after",
                         "break-before",
                         "break-inside",
                         "color",
                         "color-scheme",
                         "column-count",
                         "column-gap",
                         "column-width",
                         "direction",
                         "display",
                         "flex-basis",
                         "flex-direction",
                         "flex-grow",
                         "flex-shrink",
                         "flex-wrap",
                         "flow-from",
                         "flow-into",
                         "font-family",
                         "font-feature-settings",
                         "font-size",
                         "font-stretch",
                         "font-style",
                         "font-variant",
                         "font-variation-settings",
                         "font-weight",
                         "forced-color-adjust",
                         "height",
                         "hyphens",
                         "justify-content",
                         "left",
                         "line-grid",
                         "line-snap",
                         "margin",
                         "margin-bottom",
                         "margin-left",
                         "margin-right",
                         "margin-top",
                         "margin-trim",
                         "max-height",
                         "max-width",
                         "min-height",
                         "min-width",
                         "opacity",
                         "orphans",
                         "overflow",
                         "overflow-x",
                         "overflow-y",
                         "padding",
                         "padding-bottom",
                         "padding-left",
                         "padding-right",
                         "padding-top",
                         "page-break-after",
                         "page-break-before",
                         "page-break-inside",
                         "position",
                         "print-color-adjust",
                         "right",
                         "text-align",
                         "text-decoration",
                         "text-decoration-color",
                         "text-decoration-line",
                         "text-decoration-style",
                         "text-emphasis-color",
                         "text-emphasis-position",
                         "text-emphasis-style",
                         "text-orientation",
                         "text-shadow",
                         "text-size-adjust",
                         "top",
                         "transform",
                         "unicode-bidi",
                         "white-space",
                         "widows",
                         "width",
                         "word-break",
                         "wrap-flow",
                         "wrap-through",
                         "writing-mode",
                         "z-index"};

  size_t i;
  for (i = 0; i < sizeof(props) / sizeof(props[0]); i++) {
    const char *edge_cases[] = {
        "invalid-value", "10px", "10px ",       "10%",         "contain",
        "cover",         "auto", "min-content", "max-content", "fit-content"};
    size_t j;
    for (j = 0; j < sizeof(edge_cases) / sizeof(edge_cases[0]); j++) {
      struct ui_dom_node *root;
      struct ui_css_stylesheet *sheet;
      struct ui_layout_node *lroot = NULL;
      struct ui_css_rule *rule;

      ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
      ui_dom_node_set_tag_name(root, "div");

      ui_css_stylesheet_create(&sheet);
      ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
      ui_css_stylesheet_append_rule(sheet, rule);

      ui_css_rule_append_declaration(rule, props[i], edge_cases[j], 0);
      ui_dom_node_set_attribute(root, props[i], edge_cases[j]);

      ui_layout_tree_generate(root, sheet, &lroot);

      if (lroot) {
        ui_layout_tree_destroy(lroot);
      }
      ui_css_stylesheet_destroy(sheet);
      ui_dom_node_destroy(root);

      /* Negative auto sizes test */
      ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
      ui_dom_node_set_tag_name(root, "div");
      ui_dom_node_set_attribute(root, "class", "neg-auto");

      ui_css_stylesheet_create(&sheet);
      ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "neg-auto");
      ui_css_rule_append_declaration(rule, "display", "block", 0);
      ui_css_rule_append_declaration(rule, "width", "auto", 0);
      ui_css_rule_append_declaration(rule, "height", "auto", 0);
      ui_css_rule_append_declaration(rule, "margin", "100px", 0);
      ui_css_stylesheet_append_rule(sheet, rule);

      ui_layout_tree_generate(root, sheet, &lroot);
      if (lroot) {
        ui_layout_compute(lroot, 50, 50);
        ui_layout_tree_destroy(lroot);
      }
      ui_css_stylesheet_destroy(sheet);
      ui_dom_node_destroy(root);
    }
  }
}
