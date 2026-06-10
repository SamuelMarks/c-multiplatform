/* clang-format off */
#include "cmp_css_selectors.h"
#include <string.h>
#include <ctype.h>
/* clang-format on */

static int cmp_stricmp(const char *a, const char *b) {
  while (*a && *b) {
    int diff = tolower((unsigned char)*a) - tolower((unsigned char)*b);
    if (diff != 0) {
      return diff;
    }
    a++;
    b++;
  }
  return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

static int cmp_strnicmp(const char *a, const char *b, size_t n) {
  size_t i;
  for (i = 0; i < n; i++) {
    int diff;
    if (!a[i] || !b[i]) {
      return tolower((unsigned char)a[i]) - tolower((unsigned char)b[i]);
    }
    diff = tolower((unsigned char)a[i]) - tolower((unsigned char)b[i]);
    if (diff != 0) {
      return diff;
    }
  }
  return 0;
}

static int cmp_str_contains_word(const char *haystack, const char *needle,
                                 int case_sensitive) {
  size_t needle_len = strlen(needle);
  const char *p = haystack;
  if (needle_len == 0)
    return 0;
  while (*p) {
    while (*p && isspace((unsigned char)*p)) {
      p++;
    }
    if (*p) {
      const char *word_start = p;
      while (*p && !isspace((unsigned char)*p)) {
        p++;
      }
      if ((size_t)(p - word_start) == needle_len) {
        if (case_sensitive) {
          if (strncmp(word_start, needle, needle_len) == 0)
            return 1;
        } else {
          if (cmp_strnicmp(word_start, needle, needle_len) == 0)
            return 1;
        }
      }
    }
  }
  return 0;
}

int cmp_sel_universal_match(const cmp_sel_universal_t *sel,
                            const cmp_dom_node_t *node, int *out_match) {
  if (!sel || !node || !out_match) {
    return -1;
  }
  *out_match = 1;
  return 0;
}

int cmp_sel_tag_match(const cmp_sel_tag_t *sel, const cmp_dom_node_t *node,
                      int *out_match) {
  const char *tag = NULL;
  int rc;

  if (!sel || !node || !out_match) {
    return -1;
  }

  rc = node->vtable->get_tag(node, &tag);
  if (rc != 0) {
    return rc;
  }

  if (tag && sel->tag_name && cmp_stricmp(tag, sel->tag_name) == 0) {
    *out_match = 1;
  } else {
    *out_match = 0;
  }

  return 0;
}

int cmp_sel_class_match(const cmp_sel_class_t *sel, const cmp_dom_node_t *node,
                        int *out_match) {
  const char *classes = NULL;
  int rc;

  if (!sel || !node || !out_match) {
    return -1;
  }

  rc = node->vtable->get_classes(node, &classes);
  if (rc != 0) {
    return rc;
  }

  if (classes && sel->class_name &&
      cmp_str_contains_word(classes, sel->class_name, 0)) {
    *out_match = 1;
  } else {
    *out_match = 0;
  }

  return 0;
}

int cmp_sel_id_match(const cmp_sel_id_t *sel, const cmp_dom_node_t *node,
                     int *out_match) {
  const char *id_name = NULL;
  int rc;

  if (!sel || !node || !out_match) {
    return -1;
  }

  rc = node->vtable->get_id(node, &id_name);
  if (rc != 0) {
    return rc;
  }

  if (id_name && sel->id_name && strcmp(id_name, sel->id_name) == 0) {
    *out_match = 1;
  } else {
    *out_match = 0;
  }

  return 0;
}

int cmp_sel_attr_match(const cmp_sel_attr_t *sel, const cmp_dom_node_t *node,
                       int *out_match) {
  const char *val = NULL;
  int rc;
  int match = 0;
  int case_sensitive;

  if (!sel || !node || !out_match) {
    return -1;
  }

  rc = node->vtable->get_attribute(node, sel->attr_name, &val);
  if (rc != 0) {
    return rc;
  }

  if (!val) {
    *out_match = 0;
    return 0;
  }

  if (sel->op == CMP_SEL_ATTR_OP_EXISTS) {
    *out_match = 1;
    return 0;
  }

  if (!sel->attr_value) {
    *out_match = 0;
    return 0;
  }

  case_sensitive = (sel->modifier == CMP_SEL_ATTR_MOD_CASE) ||
                   (sel->modifier == CMP_SEL_ATTR_MOD_NONE);

  switch (sel->op) {
  case CMP_SEL_ATTR_OP_EQUALS:
    if (case_sensitive) {
      match = (strcmp(val, sel->attr_value) == 0);
    } else {
      match = (cmp_stricmp(val, sel->attr_value) == 0);
    }
    break;
  case CMP_SEL_ATTR_OP_PREFIX: {
    size_t prefix_len = strlen(sel->attr_value);
    if (prefix_len == 0) {
      match = 0;
    } else if (case_sensitive) {
      match = (strncmp(val, sel->attr_value, prefix_len) == 0);
    } else {
      match = (cmp_strnicmp(val, sel->attr_value, prefix_len) == 0);
    }
    break;
  }
  case CMP_SEL_ATTR_OP_SUFFIX: {
    size_t val_len = strlen(val);
    size_t suffix_len = strlen(sel->attr_value);
    if (suffix_len == 0 || suffix_len > val_len) {
      match = 0;
    } else {
      const char *val_suffix = val + (val_len - suffix_len);
      if (case_sensitive) {
        match = (strcmp(val_suffix, sel->attr_value) == 0);
      } else {
        match = (cmp_stricmp(val_suffix, sel->attr_value) == 0);
      }
    }
    break;
  }
  case CMP_SEL_ATTR_OP_CONTAINS:
    if (strlen(sel->attr_value) == 0) {
      match = 0;
    } else {
      if (case_sensitive) {
        match = (strstr(val, sel->attr_value) != NULL);
      } else {
        /* basic case-insensitive substring search */
        size_t val_len = strlen(val);
        size_t sub_len = strlen(sel->attr_value);
        size_t i;
        match = 0;
        if (sub_len <= val_len && sub_len > 0) {
          for (i = 0; i <= val_len - sub_len; i++) {
            if (cmp_strnicmp(val + i, sel->attr_value, sub_len) == 0) {
              match = 1;
              break;
            }
          }
        }
      }
    }
    break;
  case CMP_SEL_ATTR_OP_WHITESPACE:
    match = cmp_str_contains_word(val, sel->attr_value, case_sensitive);
    break;
  case CMP_SEL_ATTR_OP_DASH:
    if (case_sensitive) {
      if (strcmp(val, sel->attr_value) == 0) {
        match = 1;
      } else {
        size_t prefix_len = strlen(sel->attr_value);
        if (strncmp(val, sel->attr_value, prefix_len) == 0 &&
            val[prefix_len] == '-') {
          match = 1;
        }
      }
    } else {
      if (cmp_stricmp(val, sel->attr_value) == 0) {
        match = 1;
      } else {
        size_t prefix_len = strlen(sel->attr_value);
        if (cmp_strnicmp(val, sel->attr_value, prefix_len) == 0 &&
            val[prefix_len] == '-') {
          match = 1;
        }
      }
    }
    break;
  default:
    match = 0;
    break;
  }

  *out_match = match;
  return 0;
}

int cmp_selector_match(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                       int *out_match) {
  if (!sel || !node || !out_match) {
    return -1;
  }
  if (!sel->match) {
    return -1;
  }
  return sel->match(sel, node, out_match);
}

int cmp_sel_descendant_match(const cmp_sel_descendant_t *sel,
                             const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int match = 0;
  const cmp_dom_node_t *parent = NULL;

  if (!sel || !node || !out_match)
    return -1;

  rc = cmp_selector_match(sel->right, node, &match);
  if (rc != 0)
    return rc;
  if (!match) {
    *out_match = 0;
    return 0;
  }

  rc = node->vtable->get_parent(node, &parent);
  if (rc != 0)
    return rc;

  while (parent) {
    rc = cmp_selector_match(sel->left, parent, &match);
    if (rc != 0)
      return rc;
    if (match) {
      *out_match = 1;
      return 0;
    }
    {
      const cmp_dom_node_t *next_parent = NULL;
      rc = parent->vtable->get_parent(parent, &next_parent);
      if (rc != 0)
        return rc;
      parent = next_parent;
    }
  }

  *out_match = 0;
  return 0;
}

int cmp_sel_child_match(const cmp_sel_child_t *sel, const cmp_dom_node_t *node,
                        int *out_match) {
  int rc;
  int match = 0;
  const cmp_dom_node_t *parent = NULL;

  if (!sel || !node || !out_match)
    return -1;

  rc = cmp_selector_match(sel->right, node, &match);
  if (rc != 0)
    return rc;
  if (!match) {
    *out_match = 0;
    return 0;
  }

  rc = node->vtable->get_parent(node, &parent);
  if (rc != 0)
    return rc;

  if (parent) {
    rc = cmp_selector_match(sel->left, parent, out_match);
    return rc;
  }

  *out_match = 0;
  return 0;
}

int cmp_sel_next_sibling_match(const cmp_sel_next_sibling_t *sel,
                               const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int match = 0;
  const cmp_dom_node_t *prev = NULL;

  if (!sel || !node || !out_match)
    return -1;

  rc = cmp_selector_match(sel->right, node, &match);
  if (rc != 0)
    return rc;
  if (!match) {
    *out_match = 0;
    return 0;
  }

  rc = node->vtable->get_prev_sibling(node, &prev);
  if (rc != 0)
    return rc;

  if (prev) {
    rc = cmp_selector_match(sel->left, prev, out_match);
    return rc;
  }

  *out_match = 0;
  return 0;
}

int cmp_sel_subsequent_sibling_match(const cmp_sel_subsequent_sibling_t *sel,
                                     const cmp_dom_node_t *node,
                                     int *out_match) {
  int rc;
  int match = 0;
  const cmp_dom_node_t *prev = NULL;

  if (!sel || !node || !out_match)
    return -1;

  rc = cmp_selector_match(sel->right, node, &match);
  if (rc != 0)
    return rc;
  if (!match) {
    *out_match = 0;
    return 0;
  }

  rc = node->vtable->get_prev_sibling(node, &prev);
  if (rc != 0)
    return rc;

  while (prev) {
    rc = cmp_selector_match(sel->left, prev, &match);
    if (rc != 0)
      return rc;
    if (match) {
      *out_match = 1;
      return 0;
    }
    {
      const cmp_dom_node_t *next_prev = NULL;
      rc = prev->vtable->get_prev_sibling(prev, &next_prev);
      if (rc != 0)
        return rc;
      prev = next_prev;
    }
  }

  *out_match = 0;
  return 0;
}

int cmp_sel_column_match(const cmp_sel_column_t *sel,
                         const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int match = 0;

  if (!sel || !node || !out_match)
    return -1;

  rc = cmp_selector_match(sel->right, node, &match);
  if (rc != 0)
    return rc;
  if (!match) {
    *out_match = 0;
    return 0;
  }

  if (node->vtable->match_column_selector) {
    rc = node->vtable->match_column_selector(node, sel->left, out_match);
    return rc;
  }

  *out_match = 0;
  return 0;
}

int cmp_sel_is_match(const cmp_sel_is_t *sel, const cmp_dom_node_t *node,
                     int *out_match) {
  size_t i;
  int rc;
  int match = 0;

  if (!sel || !node || !out_match)
    return -1;

  for (i = 0; i < sel->selector_count; i++) {
    rc = cmp_selector_match(sel->selectors[i], node, &match);
    if (rc != 0)
      return rc;
    if (match) {
      *out_match = 1;
      return 0;
    }
  }

  *out_match = 0;
  return 0;
}

int cmp_sel_where_match(const cmp_sel_where_t *sel, const cmp_dom_node_t *node,
                        int *out_match) {
  /* :where() has the exact same matching logic as :is(), only specificity
   * differs. */
  size_t i;
  int rc;
  int match = 0;

  if (!sel || !node || !out_match)
    return -1;

  for (i = 0; i < sel->selector_count; i++) {
    rc = cmp_selector_match(sel->selectors[i], node, &match);
    if (rc != 0)
      return rc;
    if (match) {
      *out_match = 1;
      return 0;
    }
  }

  *out_match = 0;
  return 0;
}

static int cmp_sel_has_match_subtree(const cmp_dom_node_t *current_node,
                                     const cmp_selector_t *target_sel,
                                     int *found) {
  int rc;
  int match = 0;
  const cmp_dom_node_t *child = NULL;

  if (!current_node)
    return 0;

  rc = cmp_selector_match(target_sel, current_node, &match);
  if (rc != 0)
    return rc;
  if (match) {
    *found = 1;
    return 0;
  }

  rc = current_node->vtable->get_first_child(current_node, &child);
  if (rc != 0)
    return rc;

  while (child) {
    const cmp_dom_node_t *next_child = NULL;
    rc = cmp_sel_has_match_subtree(child, target_sel, found);
    if (rc != 0)
      return rc;
    if (*found)
      return 0;

    rc = child->vtable->get_next_sibling(child, &next_child);
    if (rc != 0)
      return rc;
    child = next_child;
  }

  return 0;
}

int cmp_sel_has_match(const cmp_sel_has_t *sel, const cmp_dom_node_t *node,
                      int *out_match) {
  size_t i;
  int rc;
  int found = 0;
  const cmp_dom_node_t *child = NULL;

  if (!sel || !node || !out_match)
    return -1;

  /* A :has() selector matches an element if any of the relative selectors match
     when evaluated with the element as the anchor. For simplicity in this
     abstract model, we search the subtree of the element for any node that
     matches the selector. */

  for (i = 0; i < sel->selector_count; i++) {
    found = 0;
    rc = node->vtable->get_first_child(node, &child);
    if (rc != 0)
      return rc;

    while (child) {
      const cmp_dom_node_t *next_child = NULL;
      rc = cmp_sel_has_match_subtree(child, sel->selectors[i], &found);
      if (rc != 0)
        return rc;
      if (found) {
        *out_match = 1;
        return 0;
      }
      rc = child->vtable->get_next_sibling(child, &next_child);
      if (rc != 0)
        return rc;
      child = next_child;
    }
  }

  *out_match = 0;
  return 0;
}

int cmp_pseudo_empty_match(const cmp_pseudo_empty_t *sel,
                           const cmp_dom_node_t *node, int *out_match) {
  int rc;
  const cmp_dom_node_t *child = NULL;

  if (!sel || !node || !out_match)
    return -1;

  rc = node->vtable->get_first_child(node, &child);
  if (rc != 0)
    return rc;

  *out_match = (child == NULL) ? 1 : 0;
  return 0;
}

int cmp_pseudo_root_match(const cmp_pseudo_root_t *sel,
                          const cmp_dom_node_t *node, int *out_match) {
  int rc;
  const cmp_dom_node_t *parent = NULL;

  if (!sel || !node || !out_match)
    return -1;

  rc = node->vtable->get_parent(node, &parent);
  if (rc != 0)
    return rc;

  *out_match = (parent == NULL) ? 1 : 0;
  return 0;
}

static int check_an_plus_b(const cmp_an_plus_b_t *pattern, int index) {
  if (pattern->a == 0) {
    return index == pattern->b;
  }
  if (pattern->a > 0) {
    if (index < pattern->b)
      return 0;
    return (index - pattern->b) % pattern->a == 0;
  } else {
    if (index > pattern->b)
      return 0;
    return (index - pattern->b) % pattern->a == 0;
  }
}

int cmp_pseudo_nth_child_match(const cmp_pseudo_nth_child_t *sel,
                               const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int index = 1;
  const cmp_dom_node_t *prev = NULL;

  if (!sel || !node || !out_match)
    return -1;

  if (sel->of_selector) {
    int match = 0;
    rc = cmp_selector_match(sel->of_selector, node, &match);
    if (rc != 0)
      return rc;
    if (!match) {
      *out_match = 0;
      return 0;
    }
  }

  rc = node->vtable->get_prev_sibling(node, &prev);
  if (rc != 0)
    return rc;

  while (prev) {
    if (sel->of_selector) {
      int match = 0;
      rc = cmp_selector_match(sel->of_selector, prev, &match);
      if (rc != 0)
        return rc;
      if (match)
        index++;
    } else {
      index++;
    }

    {
      const cmp_dom_node_t *next_prev = NULL;
      rc = prev->vtable->get_prev_sibling(prev, &next_prev);
      if (rc != 0)
        return rc;
      prev = next_prev;
    }
  }

  *out_match = check_an_plus_b(&sel->pattern, index);
  return 0;
}

int cmp_pseudo_nth_last_child_match(const cmp_pseudo_nth_last_child_t *sel,
                                    const cmp_dom_node_t *node,
                                    int *out_match) {
  int rc;
  int index = 1;
  const cmp_dom_node_t *next = NULL;

  if (!sel || !node || !out_match)
    return -1;

  if (sel->of_selector) {
    int match = 0;
    rc = cmp_selector_match(sel->of_selector, node, &match);
    if (rc != 0)
      return rc;
    if (!match) {
      *out_match = 0;
      return 0;
    }
  }

  rc = node->vtable->get_next_sibling(node, &next);
  if (rc != 0)
    return rc;

  while (next) {
    if (sel->of_selector) {
      int match = 0;
      rc = cmp_selector_match(sel->of_selector, next, &match);
      if (rc != 0)
        return rc;
      if (match)
        index++;
    } else {
      index++;
    }

    {
      const cmp_dom_node_t *next_next = NULL;
      rc = next->vtable->get_next_sibling(next, &next_next);
      if (rc != 0)
        return rc;
      next = next_next;
    }
  }

  *out_match = check_an_plus_b(&sel->pattern, index);
  return 0;
}

int cmp_pseudo_first_child_match(const cmp_pseudo_first_child_t *sel,
                                 const cmp_dom_node_t *node, int *out_match) {
  int rc;
  const cmp_dom_node_t *prev = NULL;

  if (!sel || !node || !out_match)
    return -1;

  rc = node->vtable->get_prev_sibling(node, &prev);
  if (rc != 0)
    return rc;

  *out_match = (prev == NULL) ? 1 : 0;
  return 0;
}

int cmp_pseudo_last_child_match(const cmp_pseudo_last_child_t *sel,
                                const cmp_dom_node_t *node, int *out_match) {
  int rc;
  const cmp_dom_node_t *next = NULL;

  if (!sel || !node || !out_match)
    return -1;

  rc = node->vtable->get_next_sibling(node, &next);
  if (rc != 0)
    return rc;

  *out_match = (next == NULL) ? 1 : 0;
  return 0;
}

int cmp_pseudo_only_child_match(const cmp_pseudo_only_child_t *sel,
                                const cmp_dom_node_t *node, int *out_match) {
  int rc;
  const cmp_dom_node_t *prev = NULL;
  const cmp_dom_node_t *next = NULL;

  if (!sel || !node || !out_match)
    return -1;

  rc = node->vtable->get_prev_sibling(node, &prev);
  if (rc != 0)
    return rc;

  rc = node->vtable->get_next_sibling(node, &next);
  if (rc != 0)
    return rc;

  *out_match = (prev == NULL && next == NULL) ? 1 : 0;
  return 0;
}

static int get_same_type_index(const cmp_dom_node_t *node, int forward,
                               int *out_index) {
  int rc;
  int index = 1;
  const char *my_tag = NULL;
  const cmp_dom_node_t *sibling = NULL;

  rc = node->vtable->get_tag(node, &my_tag);
  if (rc != 0)
    return rc;

  if (!my_tag) {
    *out_index = 1;
    return 0;
  }

  if (forward) {
    rc = node->vtable->get_prev_sibling(node, &sibling);
  } else {
    rc = node->vtable->get_next_sibling(node, &sibling);
  }
  if (rc != 0)
    return rc;

  while (sibling) {
    const char *sib_tag = NULL;
    rc = sibling->vtable->get_tag(sibling, &sib_tag);
    if (rc != 0)
      return rc;

    if (sib_tag && cmp_stricmp(my_tag, sib_tag) == 0) {
      index++;
    }

    {
      const cmp_dom_node_t *next_sibling = NULL;
      if (forward) {
        rc = sibling->vtable->get_prev_sibling(sibling, &next_sibling);
      } else {
        rc = sibling->vtable->get_next_sibling(sibling, &next_sibling);
      }
      if (rc != 0)
        return rc;
      sibling = next_sibling;
    }
  }

  *out_index = index;
  return 0;
}

int cmp_pseudo_nth_of_type_match(const cmp_pseudo_nth_of_type_t *sel,
                                 const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int index = 1;

  if (!sel || !node || !out_match)
    return -1;

  rc = get_same_type_index(node, 1, &index);
  if (rc != 0)
    return rc;

  *out_match = check_an_plus_b(&sel->pattern, index);
  return 0;
}

int cmp_pseudo_nth_last_of_type_match(const cmp_pseudo_nth_last_of_type_t *sel,
                                      const cmp_dom_node_t *node,
                                      int *out_match) {
  int rc;
  int index = 1;

  if (!sel || !node || !out_match)
    return -1;

  rc = get_same_type_index(node, 0, &index);
  if (rc != 0)
    return rc;

  *out_match = check_an_plus_b(&sel->pattern, index);
  return 0;
}

int cmp_pseudo_first_of_type_match(const cmp_pseudo_first_of_type_t *sel,
                                   const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int index = 1;

  if (!sel || !node || !out_match)
    return -1;

  rc = get_same_type_index(node, 1, &index);
  if (rc != 0)
    return rc;

  *out_match = (index == 1) ? 1 : 0;
  return 0;
}

int cmp_pseudo_last_of_type_match(const cmp_pseudo_last_of_type_t *sel,
                                  const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int index = 1;

  if (!sel || !node || !out_match)
    return -1;

  rc = get_same_type_index(node, 0, &index);
  if (rc != 0)
    return rc;

  *out_match = (index == 1) ? 1 : 0;
  return 0;
}

int cmp_pseudo_only_of_type_match(const cmp_pseudo_only_of_type_t *sel,
                                  const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int index_fwd = 1;
  int index_bwd = 1;

  if (!sel || !node || !out_match)
    return -1;

  rc = get_same_type_index(node, 1, &index_fwd);
  if (rc != 0)
    return rc;

  rc = get_same_type_index(node, 0, &index_bwd);
  if (rc != 0)
    return rc;

  *out_match = (index_fwd == 1 && index_bwd == 1) ? 1 : 0;
  return 0;
}

int cmp_sel_not_match(const cmp_sel_not_t *sel, const cmp_dom_node_t *node,
                      int *out_match) {
  size_t i;
  int rc;
  int match = 0;

  if (!sel || !node || !out_match)
    return -1;

  /* :not() matches if none of the selectors match */
  for (i = 0; i < sel->selector_count; i++) {
    rc = cmp_selector_match(sel->selectors[i], node, &match);
    if (rc != 0)
      return rc;
    if (match) {
      *out_match = 0; /* A match was found, so :not() fails */
      return 0;
    }
  }

  *out_match = 1; /* No selectors matched, so :not() succeeds */
  return 0;
}

static int ascii_case_insensitive_char_eq(char a, char b) {
  char lower_a = (a >= 'A' && a <= 'Z') ? (char)(a + ('a' - 'A')) : a;
  char lower_b = (b >= 'A' && b <= 'Z') ? (char)(b + ('a' - 'A')) : b;
  return lower_a == lower_b;
}

static int ascii_case_insensitive_strn_eq(const char *s1, const char *s2,
                                          size_t n) {
  size_t i;
  for (i = 0; i < n; i++) {
    if (s1[i] == '\0' || s2[i] == '\0' ||
        !ascii_case_insensitive_char_eq(s1[i], s2[i])) {
      return (s1[i] == s2[i] && s1[i] == '\0' && i == n) ? 1 : 0;
    }
  }
  return 1;
}

int cmp_pseudo_dir_match(const cmp_pseudo_dir_t *sel,
                         const cmp_dom_node_t *node, int *out_match) {
  int rc;
  const char *dir = NULL;

  if (!sel || !node || !out_match)
    return -1;

  *out_match = 0;
  if (node->vtable->get_dir) {
    rc = node->vtable->get_dir(node, &dir);
    if (rc != 0)
      return rc;
    if (dir && sel->dir && strcmp(dir, sel->dir) == 0) {
      *out_match = 1;
    }
  }
  return 0;
}

int cmp_pseudo_lang_match(const cmp_pseudo_lang_t *sel,
                          const cmp_dom_node_t *node, int *out_match) {
  int rc;
  const char *lang = NULL;
  size_t i;

  if (!sel || !node || !out_match)
    return -1;

  *out_match = 0;
  if (node->vtable->get_lang) {
    rc = node->vtable->get_lang(node, &lang);
    if (rc != 0)
      return rc;
    if (lang) {
      size_t lang_len = strlen(lang);
      for (i = 0; i < sel->lang_count; i++) {
        const char *match_lang = sel->langs[i];
        size_t match_len;
        if (!match_lang)
          continue;
        match_len = strlen(match_lang);
        if (match_len == 0)
          continue;

        if (lang_len >= match_len) {
          if (ascii_case_insensitive_strn_eq(lang, match_lang, match_len)) {
            if (lang_len == match_len || lang[match_len] == '-') {
              *out_match = 1;
              break;
            }
          }
        }
      }
    }
  }
  return 0;
}

int cmp_pseudo_any_link_match(const cmp_pseudo_any_link_t *sel,
                              const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int is_any = 0, is_link = 0, is_vis = 0, is_local = 0;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->get_link_state) {
    rc = node->vtable->get_link_state(node, &is_any, &is_link, &is_vis,
                                      &is_local);
    if (rc != 0)
      return rc;
    *out_match = is_any;
  }
  return 0;
}

int cmp_pseudo_link_match(const cmp_pseudo_link_t *sel,
                          const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int is_any = 0, is_link = 0, is_vis = 0, is_local = 0;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->get_link_state) {
    rc = node->vtable->get_link_state(node, &is_any, &is_link, &is_vis,
                                      &is_local);
    if (rc != 0)
      return rc;
    *out_match = is_link;
  }
  return 0;
}

int cmp_pseudo_visited_match(const cmp_pseudo_visited_t *sel,
                             const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int is_any = 0, is_link = 0, is_vis = 0, is_local = 0;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->get_link_state) {
    rc = node->vtable->get_link_state(node, &is_any, &is_link, &is_vis,
                                      &is_local);
    if (rc != 0)
      return rc;
    *out_match = is_vis;
  }
  return 0;
}

int cmp_pseudo_local_link_match(const cmp_pseudo_local_link_t *sel,
                                const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int is_any = 0, is_link = 0, is_vis = 0, is_local = 0;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->get_link_state) {
    rc = node->vtable->get_link_state(node, &is_any, &is_link, &is_vis,
                                      &is_local);
    if (rc != 0)
      return rc;
    *out_match = is_local;
  }
  return 0;
}

int cmp_pseudo_target_match(const cmp_pseudo_target_t *sel,
                            const cmp_dom_node_t *node, int *out_match) {
  int rc;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->is_target) {
    rc = node->vtable->is_target(node, out_match);
    if (rc != 0)
      return rc;
  }
  return 0;
}

int cmp_pseudo_target_within_match(const cmp_pseudo_target_within_t *sel,
                                   const cmp_dom_node_t *node, int *out_match) {
  int rc;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->is_target_within) {
    rc = node->vtable->is_target_within(node, out_match);
    if (rc != 0)
      return rc;
  }
  return 0;
}

int cmp_pseudo_scope_match(const cmp_pseudo_scope_t *sel,
                           const cmp_dom_node_t *node, int *out_match) {
  int rc;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->is_scope) {
    rc = node->vtable->is_scope(node, out_match);
    if (rc != 0)
      return rc;
  }
  return 0;
}

int cmp_pseudo_hover_match(const cmp_pseudo_hover_t *sel,
                           const cmp_dom_node_t *node, int *out_match) {
  int rc, is_hover = 0, is_active = 0, is_focus = 0, is_focus_vis = 0,
          is_focus_within = 0;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->get_user_action_state) {
    rc = node->vtable->get_user_action_state(node, &is_hover, &is_active,
                                             &is_focus, &is_focus_vis,
                                             &is_focus_within);
    if (rc != 0)
      return rc;
    *out_match = is_hover;
  }
  return 0;
}

int cmp_pseudo_active_match(const cmp_pseudo_active_t *sel,
                            const cmp_dom_node_t *node, int *out_match) {
  int rc, is_hover = 0, is_active = 0, is_focus = 0, is_focus_vis = 0,
          is_focus_within = 0;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->get_user_action_state) {
    rc = node->vtable->get_user_action_state(node, &is_hover, &is_active,
                                             &is_focus, &is_focus_vis,
                                             &is_focus_within);
    if (rc != 0)
      return rc;
    *out_match = is_active;
  }
  return 0;
}

int cmp_pseudo_focus_match(const cmp_pseudo_focus_t *sel,
                           const cmp_dom_node_t *node, int *out_match) {
  int rc, is_hover = 0, is_active = 0, is_focus = 0, is_focus_vis = 0,
          is_focus_within = 0;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->get_user_action_state) {
    rc = node->vtable->get_user_action_state(node, &is_hover, &is_active,
                                             &is_focus, &is_focus_vis,
                                             &is_focus_within);
    if (rc != 0)
      return rc;
    *out_match = is_focus;
  }
  return 0;
}

int cmp_pseudo_focus_visible_match(const cmp_pseudo_focus_visible_t *sel,
                                   const cmp_dom_node_t *node, int *out_match) {
  int rc, is_hover = 0, is_active = 0, is_focus = 0, is_focus_vis = 0,
          is_focus_within = 0;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->get_user_action_state) {
    rc = node->vtable->get_user_action_state(node, &is_hover, &is_active,
                                             &is_focus, &is_focus_vis,
                                             &is_focus_within);
    if (rc != 0)
      return rc;
    *out_match = is_focus_vis;
  }
  return 0;
}

int cmp_pseudo_focus_within_match(const cmp_pseudo_focus_within_t *sel,
                                  const cmp_dom_node_t *node, int *out_match) {
  int rc, is_hover = 0, is_active = 0, is_focus = 0, is_focus_vis = 0,
          is_focus_within = 0;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->get_user_action_state) {
    rc = node->vtable->get_user_action_state(node, &is_hover, &is_active,
                                             &is_focus, &is_focus_vis,
                                             &is_focus_within);
    if (rc != 0)
      return rc;
    *out_match = is_focus_within;
  }
  return 0;
}

int cmp_pseudo_current_match(const cmp_pseudo_current_t *sel,
                             const cmp_dom_node_t *node, int *out_match) {
  int rc, is_current = 0, is_past = 0, is_future = 0;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->get_time_state) {
    rc = node->vtable->get_time_state(node, &is_current, &is_past, &is_future);
    if (rc != 0)
      return rc;
    *out_match = is_current;
  }
  return 0;
}

int cmp_pseudo_past_match(const cmp_pseudo_past_t *sel,
                          const cmp_dom_node_t *node, int *out_match) {
  int rc, is_current = 0, is_past = 0, is_future = 0;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->get_time_state) {
    rc = node->vtable->get_time_state(node, &is_current, &is_past, &is_future);
    if (rc != 0)
      return rc;
    *out_match = is_past;
  }
  return 0;
}

int cmp_pseudo_future_match(const cmp_pseudo_future_t *sel,
                            const cmp_dom_node_t *node, int *out_match) {
  int rc, is_current = 0, is_past = 0, is_future = 0;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->get_time_state) {
    rc = node->vtable->get_time_state(node, &is_current, &is_past, &is_future);
    if (rc != 0)
      return rc;
    *out_match = is_future;
  }
  return 0;
}

int cmp_pseudo_enabled_disabled_match(const cmp_pseudo_enabled_disabled_t *sel,
                                      const cmp_dom_node_t *node,
                                      int *out_match) {
  int rc;
  cmp_form_state_t state;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  memset(&state, 0, sizeof(state));
  if (node->vtable->get_form_state) {
    rc = node->vtable->get_form_state(node, &state);
    if (rc != 0)
      return rc;
    *out_match = sel->is_disabled ? state.is_disabled : state.is_enabled;
  }
  return 0;
}

int cmp_pseudo_read_only_write_match(const cmp_pseudo_read_only_write_t *sel,
                                     const cmp_dom_node_t *node,
                                     int *out_match) {
  int rc;
  cmp_form_state_t state;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  memset(&state, 0, sizeof(state));
  if (node->vtable->get_form_state) {
    rc = node->vtable->get_form_state(node, &state);
    if (rc != 0)
      return rc;
    *out_match = sel->is_read_write ? state.is_read_write : state.is_read_only;
  }
  return 0;
}

int cmp_pseudo_placeholder_shown_match(
    const cmp_pseudo_placeholder_shown_t *sel, const cmp_dom_node_t *node,
    int *out_match) {
  int rc;
  cmp_form_state_t state;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  memset(&state, 0, sizeof(state));
  if (node->vtable->get_form_state) {
    rc = node->vtable->get_form_state(node, &state);
    if (rc != 0)
      return rc;
    *out_match = state.is_placeholder_shown;
  }
  return 0;
}

int cmp_pseudo_default_match(const cmp_pseudo_default_t *sel,
                             const cmp_dom_node_t *node, int *out_match) {
  int rc;
  cmp_form_state_t state;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  memset(&state, 0, sizeof(state));
  if (node->vtable->get_form_state) {
    rc = node->vtable->get_form_state(node, &state);
    if (rc != 0)
      return rc;
    *out_match = state.is_default;
  }
  return 0;
}

int cmp_pseudo_checked_match(const cmp_pseudo_checked_t *sel,
                             const cmp_dom_node_t *node, int *out_match) {
  int rc;
  cmp_form_state_t state;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  memset(&state, 0, sizeof(state));
  if (node->vtable->get_form_state) {
    rc = node->vtable->get_form_state(node, &state);
    if (rc != 0)
      return rc;
    *out_match = state.is_checked;
  }
  return 0;
}

int cmp_pseudo_indeterminate_match(const cmp_pseudo_indeterminate_t *sel,
                                   const cmp_dom_node_t *node, int *out_match) {
  int rc;
  cmp_form_state_t state;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  memset(&state, 0, sizeof(state));
  if (node->vtable->get_form_state) {
    rc = node->vtable->get_form_state(node, &state);
    if (rc != 0)
      return rc;
    *out_match = state.is_indeterminate;
  }
  return 0;
}

int cmp_pseudo_blank_match(const cmp_pseudo_blank_t *sel,
                           const cmp_dom_node_t *node, int *out_match) {
  int rc;
  cmp_form_state_t state;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  memset(&state, 0, sizeof(state));
  if (node->vtable->get_form_state) {
    rc = node->vtable->get_form_state(node, &state);
    if (rc != 0)
      return rc;
    *out_match = state.is_blank;
  }
  return 0;
}

int cmp_pseudo_valid_invalid_match(const cmp_pseudo_valid_invalid_t *sel,
                                   const cmp_dom_node_t *node, int *out_match) {
  int rc;
  cmp_form_state_t state;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  memset(&state, 0, sizeof(state));
  if (node->vtable->get_form_state) {
    rc = node->vtable->get_form_state(node, &state);
    if (rc != 0)
      return rc;
    *out_match = sel->is_invalid ? state.is_invalid : state.is_valid;
  }
  return 0;
}

int cmp_pseudo_range_match(const cmp_pseudo_range_t *sel,
                           const cmp_dom_node_t *node, int *out_match) {
  int rc;
  cmp_form_state_t state;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  memset(&state, 0, sizeof(state));
  if (node->vtable->get_form_state) {
    rc = node->vtable->get_form_state(node, &state);
    if (rc != 0)
      return rc;
    *out_match =
        sel->is_out_of_range ? state.is_out_of_range : state.is_in_range;
  }
  return 0;
}

int cmp_pseudo_required_optional_match(
    const cmp_pseudo_required_optional_t *sel, const cmp_dom_node_t *node,
    int *out_match) {
  int rc;
  cmp_form_state_t state;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  memset(&state, 0, sizeof(state));
  if (node->vtable->get_form_state) {
    rc = node->vtable->get_form_state(node, &state);
    if (rc != 0)
      return rc;
    *out_match = sel->is_optional ? state.is_optional : state.is_required;
  }
  return 0;
}

int cmp_pseudo_user_validity_match(const cmp_pseudo_user_validity_t *sel,
                                   const cmp_dom_node_t *node, int *out_match) {
  int rc;
  cmp_form_state_t state;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  memset(&state, 0, sizeof(state));
  if (node->vtable->get_form_state) {
    rc = node->vtable->get_form_state(node, &state);
    if (rc != 0)
      return rc;
    *out_match = sel->is_invalid ? state.is_user_invalid : state.is_user_valid;
  }
  return 0;
}

int cmp_pseudo_autofill_match(const cmp_pseudo_autofill_t *sel,
                              const cmp_dom_node_t *node, int *out_match) {
  int rc;
  cmp_form_state_t state;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  memset(&state, 0, sizeof(state));
  if (node->vtable->get_form_state) {
    rc = node->vtable->get_form_state(node, &state);
    if (rc != 0)
      return rc;
    *out_match = state.is_autofill;
  }
  return 0;
}

int cmp_pseudo_host_match(const cmp_pseudo_host_t *sel,
                          const cmp_dom_node_t *node, int *out_match) {
  int rc;
  const cmp_dom_node_t *host = NULL;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  /* :host matches if the node IS a shadow host, which we check by seeing if it
     has a shadow root. For a simpler generic model, we will check if
     get_shadow_host returns itself, or we can check a specific flag. Wait,
     :host matches the *shadow host* itself, but only from within the shadow
     tree's stylesheets. Actually, if node->vtable->get_shadow_host returns
     `node`, then it is the host. */
  if (node->vtable->get_shadow_host) {
    rc = node->vtable->get_shadow_host(node, &host);
    if (rc != 0)
      return rc;
    if (host == node) {
      *out_match = 1;
    }
  }
  return 0;
}

int cmp_pseudo_host_func_match(const cmp_pseudo_host_func_t *sel,
                               const cmp_dom_node_t *node, int *out_match) {
  int rc;
  const cmp_dom_node_t *host = NULL;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->get_shadow_host) {
    rc = node->vtable->get_shadow_host(node, &host);
    if (rc != 0)
      return rc;
    if (host == node) {
      if (sel->selector) {
        rc = cmp_selector_match(sel->selector, node, out_match);
        return rc;
      } else {
        *out_match = 1;
      }
    }
  }
  return 0;
}

int cmp_pseudo_host_context_match(const cmp_pseudo_host_context_t *sel,
                                  const cmp_dom_node_t *node, int *out_match) {
  int rc;
  const cmp_dom_node_t *host = NULL;
  const cmp_dom_node_t *ancestor = NULL;

  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;

  if (node->vtable->get_shadow_host) {
    rc = node->vtable->get_shadow_host(node, &host);
    if (rc != 0)
      return rc;

    if (host == node) {
      if (!sel->selector) {
        *out_match = 1;
        return 0;
      }

      ancestor = host;
      while (ancestor) {
        int match = 0;
        rc = cmp_selector_match(sel->selector, ancestor, &match);
        if (rc != 0)
          return rc;
        if (match) {
          *out_match = 1;
          return 0;
        }

        if (ancestor->vtable->get_parent) {
          const cmp_dom_node_t *next_ancestor = NULL;
          rc = ancestor->vtable->get_parent(ancestor, &next_ancestor);
          if (rc != 0)
            return rc;
          ancestor = next_ancestor;
        } else {
          break;
        }
      }
    }
  }
  return 0;
}

int cmp_pseudo_modal_match(const cmp_pseudo_modal_t *sel,
                           const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int is_modal = 0, is_fullscreen = 0, is_pip = 0, is_popover_open = 0,
      is_defined = 0;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->get_dom_state) {
    rc = node->vtable->get_dom_state(node, &is_modal, &is_fullscreen, &is_pip,
                                     &is_popover_open, &is_defined);
    if (rc != 0)
      return rc;
    *out_match = is_modal;
  }
  return 0;
}

int cmp_pseudo_fullscreen_match(const cmp_pseudo_fullscreen_t *sel,
                                const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int is_modal = 0, is_fullscreen = 0, is_pip = 0, is_popover_open = 0,
      is_defined = 0;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->get_dom_state) {
    rc = node->vtable->get_dom_state(node, &is_modal, &is_fullscreen, &is_pip,
                                     &is_popover_open, &is_defined);
    if (rc != 0)
      return rc;
    *out_match = is_fullscreen;
  }
  return 0;
}

int cmp_pseudo_pip_match(const cmp_pseudo_pip_t *sel,
                         const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int is_modal = 0, is_fullscreen = 0, is_pip = 0, is_popover_open = 0,
      is_defined = 0;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->get_dom_state) {
    rc = node->vtable->get_dom_state(node, &is_modal, &is_fullscreen, &is_pip,
                                     &is_popover_open, &is_defined);
    if (rc != 0)
      return rc;
    *out_match = is_pip;
  }
  return 0;
}

int cmp_pseudo_popover_open_match(const cmp_pseudo_popover_open_t *sel,
                                  const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int is_modal = 0, is_fullscreen = 0, is_pip = 0, is_popover_open = 0,
      is_defined = 0;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->get_dom_state) {
    rc = node->vtable->get_dom_state(node, &is_modal, &is_fullscreen, &is_pip,
                                     &is_popover_open, &is_defined);
    if (rc != 0)
      return rc;
    *out_match = is_popover_open;
  }
  return 0;
}

int cmp_pseudo_defined_match(const cmp_pseudo_defined_t *sel,
                             const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int is_modal = 0, is_fullscreen = 0, is_pip = 0, is_popover_open = 0,
      is_defined = 0;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;
  if (node->vtable->get_dom_state) {
    rc = node->vtable->get_dom_state(node, &is_modal, &is_fullscreen, &is_pip,
                                     &is_popover_open, &is_defined);
    if (rc != 0)
      return rc;
    *out_match = is_defined;
  }
  return 0;
}

int cmp_pseudo_media_state_match(const cmp_pseudo_media_state_t *sel,
                                 const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int is_playing = 0, is_paused = 0, is_muted = 0, is_volume_locked = 0;

  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;

  if (node->vtable->get_media_state) {
    rc = node->vtable->get_media_state(node, &is_playing, &is_paused, &is_muted,
                                       &is_volume_locked);
    if (rc != 0)
      return rc;
    *out_match = sel->is_paused ? is_paused : is_playing;
  }
  return 0;
}

int cmp_pseudo_media_volume_match(const cmp_pseudo_media_volume_t *sel,
                                  const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int is_playing = 0, is_paused = 0, is_muted = 0, is_volume_locked = 0;

  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;

  if (node->vtable->get_media_state) {
    rc = node->vtable->get_media_state(node, &is_playing, &is_paused, &is_muted,
                                       &is_volume_locked);
    if (rc != 0)
      return rc;
    *out_match = sel->is_volume_locked ? is_volume_locked : is_muted;
  }
  return 0;
}

int cmp_pseudo_elem_before_after_match(
    const cmp_pseudo_elem_before_after_t *sel, const cmp_dom_node_t *node,
    int *out_match) {
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 1;
  return 0;
}

int cmp_pseudo_elem_first_line_letter_match(
    const cmp_pseudo_elem_first_line_letter_t *sel, const cmp_dom_node_t *node,
    int *out_match) {
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 1;
  return 0;
}

int cmp_pseudo_elem_selection_match(const cmp_pseudo_elem_selection_t *sel,
                                    const cmp_dom_node_t *node,
                                    int *out_match) {
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 1;
  return 0;
}

int cmp_pseudo_elem_target_text_match(const cmp_pseudo_elem_target_text_t *sel,
                                      const cmp_dom_node_t *node,
                                      int *out_match) {
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 1;
  return 0;
}

int cmp_pseudo_elem_spelling_grammar_match(
    const cmp_pseudo_elem_spelling_grammar_t *sel, const cmp_dom_node_t *node,
    int *out_match) {
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 1;
  return 0;
}

int cmp_pseudo_elem_marker_match(const cmp_pseudo_elem_marker_t *sel,
                                 const cmp_dom_node_t *node, int *out_match) {
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 1;
  return 0;
}

int cmp_pseudo_elem_placeholder_match(const cmp_pseudo_elem_placeholder_t *sel,
                                      const cmp_dom_node_t *node,
                                      int *out_match) {
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 1;
  return 0;
}

int cmp_pseudo_elem_backdrop_match(const cmp_pseudo_elem_backdrop_t *sel,
                                   const cmp_dom_node_t *node, int *out_match) {
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 1;
  return 0;
}

int cmp_pseudo_elem_file_selector_match(
    const cmp_pseudo_elem_file_selector_t *sel, const cmp_dom_node_t *node,
    int *out_match) {
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 1;
  return 0;
}

int cmp_pseudo_elem_cue_match(const cmp_pseudo_elem_cue_t *sel,
                              const cmp_dom_node_t *node, int *out_match) {
  int rc;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 1;
  if (sel->selector) {
    rc = cmp_selector_match(sel->selector, node, out_match);
    if (rc != 0)
      return rc;
  }
  return 0;
}

int cmp_pseudo_elem_slotted_match(const cmp_pseudo_elem_slotted_t *sel,
                                  const cmp_dom_node_t *node, int *out_match) {
  int rc;
  int is_slotted = 0;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;

  if (node->vtable->is_slotted) {
    rc = node->vtable->is_slotted(node, &is_slotted);
    if (rc != 0)
      return rc;
    if (is_slotted) {
      if (sel->selector) {
        rc = cmp_selector_match(sel->selector, node, out_match);
        if (rc != 0)
          return rc;
      } else {
        *out_match = 1;
      }
    }
  }
  return 0;
}

int cmp_pseudo_elem_part_match(const cmp_pseudo_elem_part_t *sel,
                               const cmp_dom_node_t *node, int *out_match) {
  int rc;
  size_t i;
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 0;

  if (node->vtable->has_part && sel->part_count > 0) {
    for (i = 0; i < sel->part_count; i++) {
      int has_part = 0;
      rc = node->vtable->has_part(node, sel->parts[i], &has_part);
      if (rc != 0)
        return rc;
      if (!has_part) {
        return 0;
      }
    }
    *out_match = 1;
  }
  return 0;
}

int cmp_pseudo_elem_view_transition_match(
    const cmp_pseudo_elem_view_transition_t *sel, const cmp_dom_node_t *node,
    int *out_match) {
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 1;
  return 0;
}

int cmp_pseudo_elem_vt_group_match(const cmp_pseudo_elem_vt_group_t *sel,
                                   const cmp_dom_node_t *node, int *out_match) {
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 1;
  return 0;
}

int cmp_pseudo_elem_vt_image_pair_match(
    const cmp_pseudo_elem_vt_image_pair_t *sel, const cmp_dom_node_t *node,
    int *out_match) {
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 1;
  return 0;
}

int cmp_pseudo_elem_vt_old_new_match(const cmp_pseudo_elem_vt_old_new_t *sel,
                                     const cmp_dom_node_t *node,
                                     int *out_match) {
  if (!sel || !node || !out_match)
    return -1;
  *out_match = 1;
  return 0;
}
