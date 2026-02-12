#include "m3/m3_a11y.h"
#include "test_utils.h"

int M3_CALL m3_a11y_test_set_fail_clear(M3Bool enable);

static int test_semantics_init_defaults(void) {
  M3Semantics semantics;

  M3_TEST_EXPECT(m3_a11y_semantics_init(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_a11y_semantics_init(&semantics));
  M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_NONE);
  M3_TEST_ASSERT(semantics.flags == 0);
  M3_TEST_ASSERT(semantics.utf8_label == NULL);
  M3_TEST_ASSERT(semantics.utf8_hint == NULL);
  M3_TEST_ASSERT(semantics.utf8_value == NULL);
  return M3_OK;
}

static int test_node_init_and_setters(void) {
  M3A11yNode node;
  M3Semantics semantics;
  M3Widget widget;

  M3_TEST_EXPECT(m3_a11y_node_init(NULL, NULL, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_a11y_test_set_fail_clear(M3_TRUE));
  M3_TEST_EXPECT(m3_a11y_node_init(&node, &widget, NULL), M3_ERR_UNKNOWN);

  M3_TEST_OK(m3_a11y_semantics_init(&semantics));
  semantics.role = M3_SEMANTIC_BUTTON;
  semantics.flags = M3_SEMANTIC_FLAG_FOCUSABLE;
  semantics.utf8_label = "Button";
  semantics.utf8_hint = "Hint";
  semantics.utf8_value = "Value";

  M3_TEST_OK(m3_a11y_node_init(&node, &widget, &semantics));
  M3_TEST_ASSERT(node.widget == &widget);
  M3_TEST_ASSERT(node.parent == NULL);
  M3_TEST_ASSERT(node.children == NULL);
  M3_TEST_ASSERT(node.child_count == 0);
  M3_TEST_ASSERT(node.semantics.role == semantics.role);
  M3_TEST_ASSERT(node.semantics.flags == semantics.flags);
  M3_TEST_ASSERT(node.semantics.utf8_label == semantics.utf8_label);
  M3_TEST_ASSERT(node.semantics.utf8_hint == semantics.utf8_hint);
  M3_TEST_ASSERT(node.semantics.utf8_value == semantics.utf8_value);

  M3_TEST_OK(m3_a11y_node_set_widget(&node, NULL));
  M3_TEST_ASSERT(node.widget == NULL);
  M3_TEST_EXPECT(m3_a11y_node_set_widget(NULL, &widget),
                 M3_ERR_INVALID_ARGUMENT);

  semantics.role = M3_SEMANTIC_TEXT;
  semantics.flags = M3_SEMANTIC_FLAG_SELECTED;
  semantics.utf8_label = "Label";
  semantics.utf8_hint = NULL;
  semantics.utf8_value = NULL;
  M3_TEST_OK(m3_a11y_node_set_semantics(&node, &semantics));
  M3_TEST_ASSERT(node.semantics.role == M3_SEMANTIC_TEXT);
  M3_TEST_ASSERT(node.semantics.flags == M3_SEMANTIC_FLAG_SELECTED);
  M3_TEST_ASSERT(node.semantics.utf8_label == semantics.utf8_label);
  M3_TEST_ASSERT(node.semantics.utf8_hint == NULL);
  M3_TEST_ASSERT(node.semantics.utf8_value == NULL);

  M3_TEST_OK(m3_a11y_test_set_fail_clear(M3_TRUE));
  M3_TEST_EXPECT(m3_a11y_node_set_semantics(&node, NULL), M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_a11y_node_set_semantics(&node, NULL));
  M3_TEST_ASSERT(node.semantics.role == M3_SEMANTIC_NONE);
  M3_TEST_ASSERT(node.semantics.flags == 0);
  M3_TEST_ASSERT(node.semantics.utf8_label == NULL);
  M3_TEST_ASSERT(node.semantics.utf8_hint == NULL);
  M3_TEST_ASSERT(node.semantics.utf8_value == NULL);

  M3_TEST_EXPECT(m3_a11y_node_set_semantics(NULL, &semantics),
                 M3_ERR_INVALID_ARGUMENT);
  return M3_OK;
}

static int test_node_set_children(void) {
  M3A11yNode parent;
  M3A11yNode other_parent;
  M3A11yNode child_a;
  M3A11yNode child_b;
  M3A11yNode child_c;
  M3A11yNode *children[2];
  M3A11yNode *single_child[1];
  M3A11yNode *duplicate_children[2];
  int rc;

  M3_TEST_OK(m3_a11y_node_init(&parent, NULL, NULL));
  M3_TEST_OK(m3_a11y_node_init(&other_parent, NULL, NULL));
  M3_TEST_OK(m3_a11y_node_init(&child_a, NULL, NULL));
  M3_TEST_OK(m3_a11y_node_init(&child_b, NULL, NULL));
  M3_TEST_OK(m3_a11y_node_init(&child_c, NULL, NULL));

  children[0] = &child_a;
  children[1] = &child_b;
  single_child[0] = &child_c;
  duplicate_children[0] = &child_a;
  duplicate_children[1] = &child_a;

  M3_TEST_EXPECT(m3_a11y_node_set_children(NULL, children, 2),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_a11y_node_set_children(&parent, NULL, 2),
                 M3_ERR_INVALID_ARGUMENT);

  children[1] = NULL;
  M3_TEST_EXPECT(m3_a11y_node_set_children(&parent, children, 2),
                 M3_ERR_INVALID_ARGUMENT);
  children[1] = &child_b;

  children[0] = &parent;
  M3_TEST_EXPECT(m3_a11y_node_set_children(&parent, children, 2),
                 M3_ERR_INVALID_ARGUMENT);
  children[0] = &child_a;

  M3_TEST_EXPECT(m3_a11y_node_set_children(&parent, duplicate_children, 2),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_a11y_node_set_children(&other_parent, children, 1));
  rc = m3_a11y_node_set_children(&parent, children, 1);
  M3_TEST_EXPECT(rc, M3_ERR_STATE);
  M3_TEST_OK(m3_a11y_node_set_children(&other_parent, NULL, 0));

  M3_TEST_OK(m3_a11y_node_set_children(&parent, children, 2));
  M3_TEST_ASSERT(child_a.parent == &parent);
  M3_TEST_ASSERT(child_b.parent == &parent);

  M3_TEST_OK(m3_a11y_node_set_children(&parent, single_child, 1));
  M3_TEST_ASSERT(child_a.parent == NULL);
  M3_TEST_ASSERT(child_b.parent == NULL);
  M3_TEST_ASSERT(child_c.parent == &parent);

  parent.child_count = 1;
  parent.children = NULL;
  M3_TEST_EXPECT(m3_a11y_node_set_children(&parent, single_child, 1),
                 M3_ERR_STATE);
  return M3_OK;
}

static int test_node_queries(void) {
  M3A11yNode parent;
  M3A11yNode child_a;
  M3A11yNode child_b;
  M3A11yNode *children[2];
  M3A11yNode *out_node;
  m3_usize count;
  int rc;

  M3_TEST_OK(m3_a11y_node_init(&parent, NULL, NULL));
  M3_TEST_OK(m3_a11y_node_init(&child_a, NULL, NULL));
  M3_TEST_OK(m3_a11y_node_init(&child_b, NULL, NULL));

  children[0] = &child_a;
  children[1] = &child_b;
  M3_TEST_OK(m3_a11y_node_set_children(&parent, children, 2));

  M3_TEST_EXPECT(m3_a11y_node_get_parent(NULL, &out_node),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_a11y_node_get_parent(&child_a, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_a11y_node_get_parent(&child_a, &out_node));
  M3_TEST_ASSERT(out_node == &parent);

  M3_TEST_EXPECT(m3_a11y_node_get_child_count(NULL, &count),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_a11y_node_get_child_count(&parent, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_a11y_node_get_child_count(&parent, &count));
  M3_TEST_ASSERT(count == 2);

  M3_TEST_EXPECT(m3_a11y_node_get_child(NULL, 0, &out_node),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_a11y_node_get_child(&parent, 0, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_a11y_node_get_child(&parent, 2, &out_node), M3_ERR_RANGE);
  M3_TEST_OK(m3_a11y_node_get_child(&parent, 1, &out_node));
  M3_TEST_ASSERT(out_node == &child_b);

  parent.child_count = 1;
  parent.children = NULL;
  M3_TEST_EXPECT(m3_a11y_node_get_child_count(&parent, &count), M3_ERR_STATE);
  M3_TEST_EXPECT(m3_a11y_node_get_child(&parent, 0, &out_node), M3_ERR_STATE);
  parent.child_count = 2;
  parent.children = children;

  parent.children[1] = NULL;
  rc = m3_a11y_node_get_child(&parent, 1, &out_node);
  M3_TEST_EXPECT(rc, M3_ERR_STATE);
  parent.children[1] = &child_b;

  return M3_OK;
}

static int test_sibling_queries(void) {
  M3A11yNode parent;
  M3A11yNode child_a;
  M3A11yNode child_b;
  M3A11yNode child_c;
  M3A11yNode orphan;
  M3A11yNode *children[3];
  M3A11yNode *out_node;
  int rc;

  M3_TEST_OK(m3_a11y_node_init(&parent, NULL, NULL));
  M3_TEST_OK(m3_a11y_node_init(&child_a, NULL, NULL));
  M3_TEST_OK(m3_a11y_node_init(&child_b, NULL, NULL));
  M3_TEST_OK(m3_a11y_node_init(&child_c, NULL, NULL));
  M3_TEST_OK(m3_a11y_node_init(&orphan, NULL, NULL));

  children[0] = &child_a;
  children[1] = &child_b;
  children[2] = &child_c;
  M3_TEST_OK(m3_a11y_node_set_children(&parent, children, 3));

  M3_TEST_EXPECT(m3_a11y_node_get_next_sibling(NULL, &out_node),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_a11y_node_get_next_sibling(&child_a, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_a11y_node_get_next_sibling(&child_a, &out_node));
  M3_TEST_ASSERT(out_node == &child_b);
  M3_TEST_OK(m3_a11y_node_get_next_sibling(&child_c, &out_node));
  M3_TEST_ASSERT(out_node == NULL);

  M3_TEST_EXPECT(m3_a11y_node_get_prev_sibling(NULL, &out_node),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_a11y_node_get_prev_sibling(&child_a, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_a11y_node_get_prev_sibling(&child_b, &out_node));
  M3_TEST_ASSERT(out_node == &child_a);
  M3_TEST_OK(m3_a11y_node_get_prev_sibling(&child_a, &out_node));
  M3_TEST_ASSERT(out_node == NULL);

  M3_TEST_OK(m3_a11y_node_get_next_sibling(&orphan, &out_node));
  M3_TEST_ASSERT(out_node == NULL);
  M3_TEST_OK(m3_a11y_node_get_prev_sibling(&orphan, &out_node));
  M3_TEST_ASSERT(out_node == NULL);

  orphan.parent = &parent;
  rc = m3_a11y_node_get_next_sibling(&orphan, &out_node);
  M3_TEST_EXPECT(rc, M3_ERR_STATE);
  rc = m3_a11y_node_get_prev_sibling(&orphan, &out_node);
  M3_TEST_EXPECT(rc, M3_ERR_STATE);

  parent.children[0] = NULL;
  rc = m3_a11y_node_get_next_sibling(&child_b, &out_node);
  M3_TEST_EXPECT(rc, M3_ERR_STATE);
  parent.children[0] = &child_a;

  parent.child_count = 2;
  parent.children = NULL;
  rc = m3_a11y_node_get_next_sibling(&child_a, &out_node);
  M3_TEST_EXPECT(rc, M3_ERR_STATE);
  rc = m3_a11y_node_get_prev_sibling(&child_a, &out_node);
  M3_TEST_EXPECT(rc, M3_ERR_STATE);
  parent.child_count = 3;
  parent.children = children;

  parent.children[1] = NULL;
  rc = m3_a11y_node_get_next_sibling(&child_a, &out_node);
  M3_TEST_EXPECT(rc, M3_ERR_STATE);
  parent.children[1] = &child_b;

  return M3_OK;
}

int main(void) {
  M3_TEST_OK(test_semantics_init_defaults());
  M3_TEST_OK(test_node_init_and_setters());
  M3_TEST_OK(test_node_set_children());
  M3_TEST_OK(test_node_queries());
  M3_TEST_OK(test_sibling_queries());
  return 0;
}
