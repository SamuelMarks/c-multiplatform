#include "cmpc/cmp_a11y.h"
#include "test_utils.h"

int CMP_CALL cmp_a11y_test_set_fail_clear(CMPBool enable);

static int test_semantics_init_defaults(void) {
  CMPSemantics semantics;

  CMP_TEST_EXPECT(cmp_a11y_semantics_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_a11y_semantics_init(&semantics));
  CMP_TEST_ASSERT(semantics.role == CMP_SEMANTIC_NONE);
  CMP_TEST_ASSERT(semantics.flags == 0);
  CMP_TEST_ASSERT(semantics.utf8_label == NULL);
  CMP_TEST_ASSERT(semantics.utf8_hint == NULL);
  CMP_TEST_ASSERT(semantics.utf8_value == NULL);
  return CMP_OK;
}

static int test_node_init_and_setters(void) {
  CMPA11yNode node;
  CMPSemantics semantics;
  CMPWidget widget;

  CMP_TEST_EXPECT(cmp_a11y_node_init(NULL, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_a11y_test_set_fail_clear(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_a11y_node_init(&node, &widget, NULL), CMP_ERR_UNKNOWN);

  CMP_TEST_OK(cmp_a11y_semantics_init(&semantics));
  semantics.role = CMP_SEMANTIC_BUTTON;
  semantics.flags = CMP_SEMANTIC_FLAG_FOCUSABLE;
  semantics.utf8_label = "Button";
  semantics.utf8_hint = "Hint";
  semantics.utf8_value = "Value";

  CMP_TEST_OK(cmp_a11y_node_init(&node, &widget, &semantics));
  CMP_TEST_ASSERT(node.widget == &widget);
  CMP_TEST_ASSERT(node.parent == NULL);
  CMP_TEST_ASSERT(node.children == NULL);
  CMP_TEST_ASSERT(node.child_count == 0);
  CMP_TEST_ASSERT(node.semantics.role == semantics.role);
  CMP_TEST_ASSERT(node.semantics.flags == semantics.flags);
  CMP_TEST_ASSERT(node.semantics.utf8_label == semantics.utf8_label);
  CMP_TEST_ASSERT(node.semantics.utf8_hint == semantics.utf8_hint);
  CMP_TEST_ASSERT(node.semantics.utf8_value == semantics.utf8_value);

  CMP_TEST_OK(cmp_a11y_node_set_widget(&node, NULL));
  CMP_TEST_ASSERT(node.widget == NULL);
  CMP_TEST_EXPECT(cmp_a11y_node_set_widget(NULL, &widget),
                  CMP_ERR_INVALID_ARGUMENT);

  semantics.role = CMP_SEMANTIC_TEXT;
  semantics.flags = CMP_SEMANTIC_FLAG_SELECTED;
  semantics.utf8_label = "Label";
  semantics.utf8_hint = NULL;
  semantics.utf8_value = NULL;
  CMP_TEST_OK(cmp_a11y_node_set_semantics(&node, &semantics));
  CMP_TEST_ASSERT(node.semantics.role == CMP_SEMANTIC_TEXT);
  CMP_TEST_ASSERT(node.semantics.flags == CMP_SEMANTIC_FLAG_SELECTED);
  CMP_TEST_ASSERT(node.semantics.utf8_label == semantics.utf8_label);
  CMP_TEST_ASSERT(node.semantics.utf8_hint == NULL);
  CMP_TEST_ASSERT(node.semantics.utf8_value == NULL);

  CMP_TEST_OK(cmp_a11y_test_set_fail_clear(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_a11y_node_set_semantics(&node, NULL), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_a11y_node_set_semantics(&node, NULL));
  CMP_TEST_ASSERT(node.semantics.role == CMP_SEMANTIC_NONE);
  CMP_TEST_ASSERT(node.semantics.flags == 0);
  CMP_TEST_ASSERT(node.semantics.utf8_label == NULL);
  CMP_TEST_ASSERT(node.semantics.utf8_hint == NULL);
  CMP_TEST_ASSERT(node.semantics.utf8_value == NULL);

  CMP_TEST_EXPECT(cmp_a11y_node_set_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  return CMP_OK;
}

static int test_node_set_children(void) {
  CMPA11yNode parent;
  CMPA11yNode other_parent;
  CMPA11yNode child_a;
  CMPA11yNode child_b;
  CMPA11yNode child_c;
  CMPA11yNode *children[2];
  CMPA11yNode *single_child[1];
  CMPA11yNode *duplicate_children[2];
  int rc;

  CMP_TEST_OK(cmp_a11y_node_init(&parent, NULL, NULL));
  CMP_TEST_OK(cmp_a11y_node_init(&other_parent, NULL, NULL));
  CMP_TEST_OK(cmp_a11y_node_init(&child_a, NULL, NULL));
  CMP_TEST_OK(cmp_a11y_node_init(&child_b, NULL, NULL));
  CMP_TEST_OK(cmp_a11y_node_init(&child_c, NULL, NULL));

  children[0] = &child_a;
  children[1] = &child_b;
  single_child[0] = &child_c;
  duplicate_children[0] = &child_a;
  duplicate_children[1] = &child_a;

  CMP_TEST_EXPECT(cmp_a11y_node_set_children(NULL, children, 2),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_a11y_node_set_children(&parent, NULL, 2),
                  CMP_ERR_INVALID_ARGUMENT);

  children[1] = NULL;
  CMP_TEST_EXPECT(cmp_a11y_node_set_children(&parent, children, 2),
                  CMP_ERR_INVALID_ARGUMENT);
  children[1] = &child_b;

  children[0] = &parent;
  CMP_TEST_EXPECT(cmp_a11y_node_set_children(&parent, children, 2),
                  CMP_ERR_INVALID_ARGUMENT);
  children[0] = &child_a;

  CMP_TEST_EXPECT(cmp_a11y_node_set_children(&parent, duplicate_children, 2),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_a11y_node_set_children(&other_parent, children, 1));
  rc = cmp_a11y_node_set_children(&parent, children, 1);
  CMP_TEST_EXPECT(rc, CMP_ERR_STATE);
  CMP_TEST_OK(cmp_a11y_node_set_children(&other_parent, NULL, 0));

  CMP_TEST_OK(cmp_a11y_node_set_children(&parent, children, 2));
  CMP_TEST_ASSERT(child_a.parent == &parent);
  CMP_TEST_ASSERT(child_b.parent == &parent);

  CMP_TEST_OK(cmp_a11y_node_set_children(&parent, single_child, 1));
  CMP_TEST_ASSERT(child_a.parent == NULL);
  CMP_TEST_ASSERT(child_b.parent == NULL);
  CMP_TEST_ASSERT(child_c.parent == &parent);

  {
    CMPA11yNode *old_children[2];

    old_children[0] = &child_a;
    old_children[1] = NULL;
    child_a.parent = &other_parent;
    parent.children = old_children;
    parent.child_count = 2;
    CMP_TEST_OK(cmp_a11y_node_set_children(&parent, single_child, 1));
  }

  parent.child_count = 1;
  parent.children = NULL;
  CMP_TEST_EXPECT(cmp_a11y_node_set_children(&parent, single_child, 1),
                  CMP_ERR_STATE);
  return CMP_OK;
}

static int test_node_queries(void) {
  CMPA11yNode parent;
  CMPA11yNode child_a;
  CMPA11yNode child_b;
  CMPA11yNode *children[2];
  CMPA11yNode *out_node;
  cmp_usize count;
  int rc;

  CMP_TEST_OK(cmp_a11y_node_init(&parent, NULL, NULL));
  CMP_TEST_OK(cmp_a11y_node_init(&child_a, NULL, NULL));
  CMP_TEST_OK(cmp_a11y_node_init(&child_b, NULL, NULL));

  children[0] = &child_a;
  children[1] = &child_b;
  CMP_TEST_OK(cmp_a11y_node_set_children(&parent, children, 2));

  CMP_TEST_EXPECT(cmp_a11y_node_get_parent(NULL, &out_node),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_a11y_node_get_parent(&child_a, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_a11y_node_get_parent(&child_a, &out_node));
  CMP_TEST_ASSERT(out_node == &parent);

  CMP_TEST_EXPECT(cmp_a11y_node_get_child_count(NULL, &count),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_a11y_node_get_child_count(&parent, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_a11y_node_get_child_count(&parent, &count));
  CMP_TEST_ASSERT(count == 2);

  parent.child_count = 0;
  parent.children = NULL;
  CMP_TEST_OK(cmp_a11y_node_get_child_count(&parent, &count));
  CMP_TEST_ASSERT(count == 0);
  parent.child_count = 2;
  parent.children = children;

  CMP_TEST_EXPECT(cmp_a11y_node_get_child(NULL, 0, &out_node),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_a11y_node_get_child(&parent, 0, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_a11y_node_get_child(&parent, 2, &out_node),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_a11y_node_get_child(&parent, 1, &out_node));
  CMP_TEST_ASSERT(out_node == &child_b);

  parent.child_count = 1;
  parent.children = NULL;
  CMP_TEST_EXPECT(cmp_a11y_node_get_child_count(&parent, &count),
                  CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_a11y_node_get_child(&parent, 0, &out_node),
                  CMP_ERR_STATE);
  parent.child_count = 2;
  parent.children = children;

  parent.children[1] = NULL;
  rc = cmp_a11y_node_get_child(&parent, 1, &out_node);
  CMP_TEST_EXPECT(rc, CMP_ERR_STATE);
  parent.children[1] = &child_b;

  return CMP_OK;
}

static int test_sibling_queries(void) {
  CMPA11yNode parent;
  CMPA11yNode child_a;
  CMPA11yNode child_b;
  CMPA11yNode child_c;
  CMPA11yNode orphan;
  CMPA11yNode *children[3];
  CMPA11yNode *out_node;
  int rc;

  CMP_TEST_OK(cmp_a11y_node_init(&parent, NULL, NULL));
  CMP_TEST_OK(cmp_a11y_node_init(&child_a, NULL, NULL));
  CMP_TEST_OK(cmp_a11y_node_init(&child_b, NULL, NULL));
  CMP_TEST_OK(cmp_a11y_node_init(&child_c, NULL, NULL));
  CMP_TEST_OK(cmp_a11y_node_init(&orphan, NULL, NULL));

  children[0] = &child_a;
  children[1] = &child_b;
  children[2] = &child_c;
  CMP_TEST_OK(cmp_a11y_node_set_children(&parent, children, 3));

  CMP_TEST_EXPECT(cmp_a11y_node_get_next_sibling(NULL, &out_node),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_a11y_node_get_next_sibling(&child_a, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_a11y_node_get_next_sibling(&child_a, &out_node));
  CMP_TEST_ASSERT(out_node == &child_b);
  CMP_TEST_OK(cmp_a11y_node_get_next_sibling(&child_c, &out_node));
  CMP_TEST_ASSERT(out_node == NULL);

  CMP_TEST_EXPECT(cmp_a11y_node_get_prev_sibling(NULL, &out_node),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_a11y_node_get_prev_sibling(&child_a, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_a11y_node_get_prev_sibling(&child_b, &out_node));
  CMP_TEST_ASSERT(out_node == &child_a);

  parent.child_count = 1;
  parent.children = NULL;
  CMP_TEST_EXPECT(cmp_a11y_node_get_next_sibling(&child_a, &out_node),
                  CMP_ERR_STATE);
  parent.child_count = 3;
  parent.children = children;
  CMP_TEST_OK(cmp_a11y_node_get_prev_sibling(&child_a, &out_node));
  CMP_TEST_ASSERT(out_node == NULL);

  CMP_TEST_OK(cmp_a11y_node_get_next_sibling(&orphan, &out_node));
  CMP_TEST_ASSERT(out_node == NULL);
  CMP_TEST_OK(cmp_a11y_node_get_prev_sibling(&orphan, &out_node));
  CMP_TEST_ASSERT(out_node == NULL);

  orphan.parent = &parent;
  rc = cmp_a11y_node_get_next_sibling(&orphan, &out_node);
  CMP_TEST_EXPECT(rc, CMP_ERR_STATE);
  rc = cmp_a11y_node_get_prev_sibling(&orphan, &out_node);
  CMP_TEST_EXPECT(rc, CMP_ERR_STATE);

  parent.children[0] = NULL;
  rc = cmp_a11y_node_get_next_sibling(&child_b, &out_node);
  CMP_TEST_EXPECT(rc, CMP_ERR_STATE);
  parent.children[0] = &child_a;

  parent.child_count = 2;
  parent.children = NULL;
  rc = cmp_a11y_node_get_next_sibling(&child_a, &out_node);
  CMP_TEST_EXPECT(rc, CMP_ERR_STATE);
  rc = cmp_a11y_node_get_prev_sibling(&child_a, &out_node);
  CMP_TEST_EXPECT(rc, CMP_ERR_STATE);
  parent.child_count = 3;
  parent.children = children;

  parent.children[1] = NULL;
  rc = cmp_a11y_node_get_next_sibling(&child_a, &out_node);
  CMP_TEST_EXPECT(rc, CMP_ERR_STATE);
  parent.children[1] = &child_b;

  return CMP_OK;
}

int main(void) {
  CMP_TEST_OK(test_semantics_init_defaults());
  CMP_TEST_OK(test_node_init_and_setters());
  CMP_TEST_OK(test_node_set_children());
  CMP_TEST_OK(test_node_queries());
  CMP_TEST_OK(test_sibling_queries());
  return 0;
}
