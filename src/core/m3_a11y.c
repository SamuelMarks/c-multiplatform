#include "m3/m3_a11y.h"

#ifdef M3_TESTING
static M3Bool g_m3_a11y_test_fail_clear = M3_FALSE;

int M3_CALL m3_a11y_test_set_fail_clear(M3Bool enable) {
  g_m3_a11y_test_fail_clear = enable ? M3_TRUE : M3_FALSE;
  return M3_OK;
}
#endif

static int m3_a11y_clear_semantics(M3Semantics *semantics) {
  if (semantics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#ifdef M3_TESTING
  if (g_m3_a11y_test_fail_clear == M3_TRUE) {
    g_m3_a11y_test_fail_clear = M3_FALSE;
    return M3_ERR_UNKNOWN;
  }
#endif
  semantics->role = M3_SEMANTIC_NONE;
  semantics->flags = 0;
  semantics->utf8_label = NULL;
  semantics->utf8_hint = NULL;
  semantics->utf8_value = NULL;
  return M3_OK;
}

static int m3_a11y_find_child_index(const M3A11yNode *parent,
                                    const M3A11yNode *child,
                                    m3_usize *out_index) {
  m3_usize i;

  if (parent->child_count > 0 && parent->children == NULL) {
    return M3_ERR_STATE;
  }

  for (i = 0; i < parent->child_count; ++i) {
    if (parent->children[i] == NULL) {
      return M3_ERR_STATE;
    }
    if (parent->children[i] == child) {
      *out_index = i;
      return M3_OK;
    }
  }

  return M3_ERR_NOT_FOUND;
}

int M3_CALL m3_a11y_semantics_init(M3Semantics *semantics) {
  return m3_a11y_clear_semantics(semantics);
}

int M3_CALL m3_a11y_node_init(M3A11yNode *node, M3Widget *widget,
                              const M3Semantics *semantics) {
  int rc;

  if (node == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (semantics != NULL) {
    node->semantics = *semantics;
  } else {
    rc = m3_a11y_clear_semantics(&node->semantics);
    if (rc != M3_OK) {
      return rc;
    }
  }

  node->widget = widget;
  node->parent = NULL;
  node->children = NULL;
  node->child_count = 0;
  return M3_OK;
}

int M3_CALL m3_a11y_node_set_semantics(M3A11yNode *node,
                                       const M3Semantics *semantics) {
  int rc;

  if (node == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (semantics != NULL) {
    node->semantics = *semantics;
    return M3_OK;
  }

  rc = m3_a11y_clear_semantics(&node->semantics);
  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

int M3_CALL m3_a11y_node_set_widget(M3A11yNode *node, M3Widget *widget) {
  if (node == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  node->widget = widget;
  return M3_OK;
}

int M3_CALL m3_a11y_node_set_children(M3A11yNode *node, M3A11yNode **children,
                                      m3_usize count) {
  m3_usize i;
  m3_usize j;

  if (node == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (count > 0 && children == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (node->child_count > 0 && node->children == NULL) {
    return M3_ERR_STATE;
  }

  for (i = 0; i < count; ++i) {
    if (children[i] == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
    if (children[i] == node) {
      return M3_ERR_INVALID_ARGUMENT;
    }
    if (children[i]->parent != NULL && children[i]->parent != node) {
      return M3_ERR_STATE;
    }
    for (j = i + 1; j < count; ++j) {
      if (children[i] == children[j]) {
        return M3_ERR_INVALID_ARGUMENT;
      }
    }
  }

  for (i = 0; i < node->child_count; ++i) {
    if (node->children[i] != NULL && node->children[i]->parent == node) {
      node->children[i]->parent = NULL;
    }
  }

  node->children = children;
  node->child_count = count;

  for (i = 0; i < count; ++i) {
    children[i]->parent = node;
  }

  return M3_OK;
}

int M3_CALL m3_a11y_node_get_parent(const M3A11yNode *node,
                                    M3A11yNode **out_parent) {
  if (node == NULL || out_parent == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_parent = node->parent;
  return M3_OK;
}

int M3_CALL m3_a11y_node_get_child_count(const M3A11yNode *node,
                                         m3_usize *out_count) {
  if (node == NULL || out_count == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (node->child_count > 0 && node->children == NULL) {
    return M3_ERR_STATE;
  }

  *out_count = node->child_count;
  return M3_OK;
}

int M3_CALL m3_a11y_node_get_child(const M3A11yNode *node, m3_usize index,
                                   M3A11yNode **out_child) {
  if (node == NULL || out_child == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (node->child_count > 0 && node->children == NULL) {
    return M3_ERR_STATE;
  }
  if (index >= node->child_count) {
    return M3_ERR_RANGE;
  }
  if (node->children[index] == NULL) {
    return M3_ERR_STATE;
  }

  *out_child = node->children[index];
  return M3_OK;
}

int M3_CALL m3_a11y_node_get_next_sibling(const M3A11yNode *node,
                                          M3A11yNode **out_sibling) {
  int rc;
  m3_usize index;
  const M3A11yNode *parent;

  if (node == NULL || out_sibling == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  parent = node->parent;
  if (parent == NULL) {
    *out_sibling = NULL;
    return M3_OK;
  }

  rc = m3_a11y_find_child_index(parent, node, &index);
  if (rc == M3_ERR_NOT_FOUND) {
    return M3_ERR_STATE;
  }
  if (rc != M3_OK) {
    return rc;
  }

  if (index + 1 >= parent->child_count) {
    *out_sibling = NULL;
    return M3_OK;
  }
  if (parent->children[index + 1] == NULL) {
    return M3_ERR_STATE;
  }

  *out_sibling = parent->children[index + 1];
  return M3_OK;
}

int M3_CALL m3_a11y_node_get_prev_sibling(const M3A11yNode *node,
                                          M3A11yNode **out_sibling) {
  int rc;
  m3_usize index;
  const M3A11yNode *parent;

  if (node == NULL || out_sibling == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  parent = node->parent;
  if (parent == NULL) {
    *out_sibling = NULL;
    return M3_OK;
  }

  rc = m3_a11y_find_child_index(parent, node, &index);
  if (rc == M3_ERR_NOT_FOUND) {
    return M3_ERR_STATE;
  }
  if (rc != M3_OK) {
    return rc;
  }

  if (index == 0) {
    *out_sibling = NULL;
    return M3_OK;
  }

  *out_sibling = parent->children[index - 1];
  return M3_OK;
}
