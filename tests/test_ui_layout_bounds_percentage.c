
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

  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(root);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  ui_css_stylesheet_destroy(sheet);
}
void test_coverage_layout(void) {
  struct ui_dom_node *root = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule = NULL;

  ui_layout_tree_generate(NULL, NULL, NULL); /* covers early exit */
  ui_layout_tree_generate(root, NULL, NULL);
  ui_layout_tree_generate(root, sheet, NULL);

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

  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(root);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  ui_css_stylesheet_destroy(sheet);
}
/* clang-format off */
#include "../include/ui_cssom.h"
#include "../include/ui_dom_node.h"
#include "../include/ui_layout.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

void test_bounds(void) {
  struct ui_dom_node *root, *c1, *c2, *c3;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule, *r1, *r2, *r3;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "root");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c1);
  ui_dom_node_set_tag_name(c1, "div");
  ui_dom_node_set_attribute(c1, "class", "c1");
  ui_dom_node_append_child(root, c1);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c2);
  ui_dom_node_set_tag_name(c2, "div");
  ui_dom_node_set_attribute(c2, "class", "c2");
  ui_dom_node_append_child(root, c2);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c3);
  ui_dom_node_set_tag_name(c3, "div");
  ui_dom_node_set_attribute(c3, "class", "c3");
  ui_dom_node_append_child(root, c3);

  ui_css_stylesheet_create(&sheet);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "root");
  ui_css_rule_append_declaration(rule, "display", "block", 0);
  ui_css_rule_append_declaration(rule, "wrap-through", "invalid-value", 0);
  ui_css_rule_append_declaration(rule, "hyphens", "invalid-value", 0);
  ui_css_rule_append_declaration(rule, "aspect-ratio", "1/0",
                                 0); /* hit div by 0 */
  rule->next = sheet->rules;
  sheet->rules = rule;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r1);
  ui_css_rule_append_selector(r1, UI_CSS_SELECTOR_TYPE_CLASS, "c1");
  ui_css_rule_append_declaration(r1, "display", "block", 0);
  ui_css_rule_append_declaration(r1, "width", "10px", 0);
  ui_css_rule_append_declaration(r1, "padding", "20px", 0); /* 10 - 40 < 0 */
  ui_css_rule_append_declaration(r1, "height", "10px", 0);
  ui_css_rule_append_declaration(r1, "min-width", "50px", 0);
  ui_css_rule_append_declaration(r1, "max-width", "5px", 0); /* max < min */
  ui_css_rule_append_declaration(r1, "min-height", "50px", 0);
  ui_css_rule_append_declaration(r1, "max-height", "5px", 0);
  r1->next = sheet->rules;
  sheet->rules = r1;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r2);
  ui_css_rule_append_selector(r2, UI_CSS_SELECTOR_TYPE_CLASS, "c2");
  ui_css_rule_append_declaration(r2, "display", "flex", 0);
  ui_css_rule_append_declaration(r2, "flex-direction", "column", 0);
  ui_css_rule_append_declaration(r2, "align-items", "center", 0);
  ui_css_rule_append_declaration(r2, "justify-content", "space-around", 0);
  ui_css_rule_append_declaration(r2, "width", "10px", 0);
  ui_css_rule_append_declaration(r2, "padding", "20px", 0);
  ui_css_rule_append_declaration(r2, "height", "10px", 0);
  ui_css_rule_append_declaration(r2, "overflow-x", "scroll", 0);
  r2->next = sheet->rules;
  sheet->rules = r2;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r3);
  ui_css_rule_append_selector(r3, UI_CSS_SELECTOR_TYPE_CLASS, "c3");
  ui_css_rule_append_declaration(r3, "display", "flex", 0);
  ui_css_rule_append_declaration(r3, "flex-direction", "column", 0);
  ui_css_rule_append_declaration(r3, "justify-content", "space-between", 0);
  ui_css_rule_append_declaration(r3, "height", "100px", 0);
  ui_css_rule_append_declaration(r3, "padding", "100px", 0);
  r3->next = sheet->rules;
  sheet->rules = r3;

  /* Trigger NULL args */
  ui_layout_compute(NULL, 100, 100);
  ui_layout_solve_viewport(NULL, 100, 100);

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

void test_sequences_and_overflow(void) {
  struct ui_dom_node *root, *i1, *b1, *i2, *b2, *i3;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule, *r_inline, *r_block;

  /* "block", "inline" sequence -> hits new_first = curr; */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "seq-root");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &b1);
  ui_dom_node_set_tag_name(b1, "div");
  ui_dom_node_set_attribute(b1, "class", "blk");
  ui_dom_node_append_child(root, b1);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &i1);
  ui_dom_node_set_tag_name(i1, "span");
  ui_dom_node_set_attribute(i1, "class", "inl");
  ui_dom_node_append_child(root, i1);

  /* "inline", "block", "inline" sequence -> hits else block of anon */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &i2);
  ui_dom_node_set_tag_name(i2, "span");
  ui_dom_node_set_attribute(i2, "class", "inl");
  ui_dom_node_append_child(root, i2);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &b2);
  ui_dom_node_set_tag_name(b2, "div");
  ui_dom_node_set_attribute(b2, "class", "blk");
  ui_dom_node_append_child(root, b2);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &i3);
  ui_dom_node_set_tag_name(i3, "span");
  ui_dom_node_set_attribute(i3, "class", "inl");
  ui_dom_node_append_child(root, i3);

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "seq-root");
  ui_css_rule_append_declaration(rule, "display", "block", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r_inline);
  ui_css_rule_append_selector(r_inline, UI_CSS_SELECTOR_TYPE_CLASS, "inl");
  ui_css_rule_append_declaration(r_inline, "display", "inline", 0);
  r_inline->next = sheet->rules;
  sheet->rules = r_inline;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r_block);
  ui_css_rule_append_selector(r_block, UI_CSS_SELECTOR_TYPE_CLASS, "blk");
  ui_css_rule_append_declaration(r_block, "display", "block", 0);
  r_block->next = sheet->rules;
  sheet->rules = r_block;

  ui_layout_tree_generate(root, sheet, &lroot);
  if (lroot) {
    ui_layout_compute(lroot, 800, 600); /* compute anon boxes to hit line 766 */
    ui_layout_tree_destroy(lroot);
  }

  /* Horizontal Scrollbar Overflow */
  struct ui_dom_node *oroot, *oc1;
  struct ui_css_rule *orule, *orc1;
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &oroot);
  ui_dom_node_set_tag_name(oroot, "div");
  ui_dom_node_set_attribute(oroot, "class", "oroot");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &oc1);
  ui_dom_node_set_tag_name(oc1, "div");
  ui_dom_node_set_attribute(oc1, "class", "oc1");
  ui_dom_node_append_child(oroot, oc1);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &orule);
  ui_css_rule_append_selector(orule, UI_CSS_SELECTOR_TYPE_CLASS, "oroot");
  ui_css_rule_append_declaration(orule, "display", "block", 0);
  ui_css_rule_append_declaration(orule, "width", "100px", 0);
  ui_css_rule_append_declaration(orule, "overflow-x", "scroll", 0);
  orule->next = sheet->rules;
  sheet->rules = orule;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &orc1);
  ui_css_rule_append_selector(orc1, UI_CSS_SELECTOR_TYPE_CLASS, "oc1");
  ui_css_rule_append_declaration(orc1, "display", "block", 0);
  ui_css_rule_append_declaration(orc1, "width", "200px",
                                 0); /* Force overflow */
  orc1->next = sheet->rules;
  sheet->rules = orc1;

  ui_layout_tree_generate(oroot, sheet, &lroot);
  if (lroot) {
    ui_layout_compute(lroot, 800, 600);
    ui_layout_tree_destroy(lroot);
  }

  ui_css_stylesheet_destroy(sheet);
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(root);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(oroot);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
}

void test_null_api_calls(void) {
  struct ui_layout_node lnode;
  memset(&lnode, 0, sizeof(lnode));
  ui_layout_tree_generate(NULL, NULL, NULL);

  /* NULL tests for compute_box_model */
  ui_layout_compute(NULL, 100, 100);
  ui_layout_solve_viewport(NULL, 100, 100);
}

void test_percentage_negative(void) {
  struct ui_dom_node *root, *c1;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule, *r1;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "pct-root");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c1);
  ui_dom_node_set_tag_name(c1, "div");
  ui_dom_node_set_attribute(c1, "class", "pct-c1");
  ui_dom_node_append_child(root, c1);

  ui_css_stylesheet_create(&sheet);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "pct-root");
  ui_css_rule_append_declaration(rule, "display", "flex", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r1);
  ui_css_rule_append_selector(r1, UI_CSS_SELECTOR_TYPE_CLASS, "pct-c1");
  ui_css_rule_append_declaration(r1, "display", "flex", 0);
  ui_css_rule_append_declaration(r1, "width", "10%", 0);
  ui_css_rule_append_declaration(r1, "margin", "100px", 0); /* Force < 0 */
  r1->next = sheet->rules;
  sheet->rules = r1;

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

void test_percentage_negative_block(void) {
  struct ui_dom_node *root, *c1;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *lroot = NULL;
  struct ui_css_rule *rule, *r1;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  ui_dom_node_set_attribute(root, "class", "pct-root2");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c1);
  ui_dom_node_set_tag_name(c1, "div");
  ui_dom_node_set_attribute(c1, "class", "pct-c2");
  ui_dom_node_append_child(root, c1);

  ui_css_stylesheet_create(&sheet);

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_CLASS, "pct-root2");
  ui_css_rule_append_declaration(rule, "display", "block", 0);
  rule->next = sheet->rules;
  sheet->rules = rule;

  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &r1);
  ui_css_rule_append_selector(r1, UI_CSS_SELECTOR_TYPE_CLASS, "pct-c2");
  ui_css_rule_append_declaration(r1, "display", "block", 0);
  ui_css_rule_append_declaration(r1, "width", "10%", 0);
  ui_css_rule_append_declaration(r1, "margin", "20px", 0);
  r1->next = sheet->rules;
  sheet->rules = r1;

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
