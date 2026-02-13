#include "cmpc/cmp_a11y.h"

#ifdef CMP_TESTING
static CMPBool g_cmp_a11y_test_fail_clear = CMP_FALSE;

int CMP_CALL cmp_a11y_test_set_fail_clear(CMPBool enable) {
  g_cmp_a11y_test_fail_clear = enable ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}
#endif

static int cmp_a11y_clear_semantics(CMPSemantics *semantics) {
  if (semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_cmp_a11y_test_fail_clear == CMP_TRUE) {
    g_cmp_a11y_test_fail_clear = CMP_FALSE;
    return CMP_ERR_UNKNOWN;
  }
#endif
  semantics->role = CMP_SEMANTIC_NONE;
  semantics->flags = 0;
  semantics->utf8_label = NULL;
  semantics->utf8_hint = NULL;
  semantics->utf8_value = NULL;
  return CMP_OK;
}

static int cmp_a11y_find_child_index(const CMPA11yNode *parent,
                                     const CMPA11yNode *child,
                                     cmp_usize *out_index) {
  cmp_usize i;

  if (parent->child_count > 0 && parent->children == NULL) {
    return CMP_ERR_STATE;
  }

  for (i = 0; i < parent->child_count; ++i) {
    if (parent->children[i] == NULL) {
      return CMP_ERR_STATE;
    }
    if (parent->children[i] == child) {
      *out_index = i;
      return CMP_OK;
    }
  }

  return CMP_ERR_NOT_FOUND;
}

int CMP_CALL cmp_a11y_semantics_init(CMPSemantics *semantics) {
  return cmp_a11y_clear_semantics(semantics);
}

int CMP_CALL cmp_a11y_node_init(CMPA11yNode *node, CMPWidget *widget,
                                const CMPSemantics *semantics) {
  int rc;

  if (node == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (semantics != NULL) {
    node->semantics = *semantics;
  } else {
    rc = cmp_a11y_clear_semantics(&node->semantics);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  node->widget = widget;
  node->parent = NULL;
  node->children = NULL;
  node->child_count = 0;
  return CMP_OK;
}

int CMP_CALL cmp_a11y_node_set_semantics(CMPA11yNode *node,
                                         const CMPSemantics *semantics) {
  int rc;

  if (node == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (semantics != NULL) {
    node->semantics = *semantics;
    return CMP_OK;
  }

  rc = cmp_a11y_clear_semantics(&node->semantics);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

int CMP_CALL cmp_a11y_node_set_widget(CMPA11yNode *node, CMPWidget *widget) {
  if (node == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  node->widget = widget;
  return CMP_OK;
}

int CMP_CALL cmp_a11y_node_set_children(CMPA11yNode *node,
                                        CMPA11yNode **children,
                                        cmp_usize count) {
  cmp_usize i;
  cmp_usize j;

  if (node == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (count > 0 && children == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (node->child_count > 0 && node->children == NULL) {
    return CMP_ERR_STATE;
  }

  for (i = 0; i < count; ++i) {
    if (children[i] == NULL) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
    if (children[i] == node) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
    if (children[i]->parent != NULL && children[i]->parent != node) {
      return CMP_ERR_STATE;
    }
    for (j = i + 1; j < count; ++j) {
      if (children[i] == children[j]) {
        return CMP_ERR_INVALID_ARGUMENT;
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

  return CMP_OK;
}

int CMP_CALL cmp_a11y_node_get_parent(const CMPA11yNode *node,
                                      CMPA11yNode **out_parent) {
  if (node == NULL || out_parent == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_parent = node->parent;
  return CMP_OK;
}

int CMP_CALL cmp_a11y_node_get_child_count(const CMPA11yNode *node,
                                           cmp_usize *out_count) {
  if (node == NULL || out_count == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (node->child_count > 0 && node->children == NULL) {
    return CMP_ERR_STATE;
  }

  *out_count = node->child_count;
  return CMP_OK;
}

int CMP_CALL cmp_a11y_node_get_child(const CMPA11yNode *node, cmp_usize index,
                                     CMPA11yNode **out_child) {
  if (node == NULL || out_child == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (node->child_count > 0 && node->children == NULL) {
    return CMP_ERR_STATE;
  }
  if (index >= node->child_count) {
    return CMP_ERR_RANGE;
  }
  if (node->children[index] == NULL) {
    return CMP_ERR_STATE;
  }

  *out_child = node->children[index];
  return CMP_OK;
}

int CMP_CALL cmp_a11y_node_get_next_sibling(const CMPA11yNode *node,
                                            CMPA11yNode **out_sibling) {
  int rc;
  cmp_usize index;
  const CMPA11yNode *parent;

  if (node == NULL || out_sibling == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  parent = node->parent;
  if (parent == NULL) {
    *out_sibling = NULL;
    return CMP_OK;
  }

  rc = cmp_a11y_find_child_index(parent, node, &index);
  if (rc == CMP_ERR_NOT_FOUND) {
    return CMP_ERR_STATE;
  }
  if (rc != CMP_OK) {
    return rc;
  }

  if (index + 1 >= parent->child_count) {
    *out_sibling = NULL;
    return CMP_OK;
  }
  if (parent->children[index + 1] == NULL) {
    return CMP_ERR_STATE;
  }

  *out_sibling = parent->children[index + 1];
  return CMP_OK;
}

int CMP_CALL cmp_a11y_node_get_prev_sibling(const CMPA11yNode *node,
                                            CMPA11yNode **out_sibling) {
  int rc;
  cmp_usize index;
  const CMPA11yNode *parent;

  if (node == NULL || out_sibling == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  parent = node->parent;
  if (parent == NULL) {
    *out_sibling = NULL;
    return CMP_OK;
  }

  rc = cmp_a11y_find_child_index(parent, node, &index);
  if (rc == CMP_ERR_NOT_FOUND) {
    return CMP_ERR_STATE;
  }
  if (rc != CMP_OK) {
    return rc;
  }

  if (index == 0) {
    *out_sibling = NULL;
    return CMP_OK;
  }

  *out_sibling = parent->children[index - 1];
  return CMP_OK;
}
