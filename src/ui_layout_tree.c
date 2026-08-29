/**
 * @file ui_layout_tree.c
 * @brief ui_layout_tree.c implementation.
 */
/**
 * @brief create_layout_node.
 * @param dom_node Parameter dom_node.
 * @param style Parameter style.
 * @param is_anonymous Parameter is_anonymous.
 * @param out_node Parameter out_node.
 * @return Return value.
 */
static ui_error_t create_layout_node(const struct ui_dom_node *dom_node,
                                     struct ui_css_computed_style *style,
                                     int is_anonymous,
                                     struct ui_layout_node **out_node) {
  ui_error_t _prop_rc = UI_ERROR_NONE;
  struct ui_layout_node *node;
  const char *display_val;

  node = (struct ui_layout_node *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_layout_node));
  if (!node) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  node->dom_node = dom_node;
  node->computed_style = style;
  node->is_anonymous = is_anonymous;
  node->display_outside =
      UI_LAYOUT_DISPLAY_OUTSIDE_INLINE; /* Default CSS value */
  node->display_inside = UI_LAYOUT_DISPLAY_INSIDE_FLOW;
  node->x = 0.0f;
  node->y = 0.0f;
  node->width = 0.0f;
  node->height = 0.0f;
  node->parent = NULL;
  node->first_child = NULL;
  node->last_child = NULL;
  node->next_sibling = NULL;
  node->previous_sibling = NULL;

  if (style) {
    {
      _prop_rc =
          ui_css_computed_style_get_property(style, "display", &display_val);
      if (_prop_rc == UI_ERROR_NONE) {
        if (strstr(display_val, "none") != NULL) {
          node->display_outside = UI_LAYOUT_DISPLAY_OUTSIDE_NONE;
        } else {
          /* Parse outside */
          if (strstr(display_val, "inline") != NULL) {
            node->display_outside = UI_LAYOUT_DISPLAY_OUTSIDE_INLINE;
          } else if (strstr(display_val, "block") != NULL) {
            node->display_outside = UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK;
          } else if (strstr(display_val, "run-in") != NULL) {
            node->display_outside = UI_LAYOUT_DISPLAY_OUTSIDE_RUN_IN;
          } else {
            /* Default outside based on inside */
            if (strstr(display_val, "flex") != NULL ||
                strstr(display_val, "grid") != NULL ||
                strstr(display_val, "table") != NULL ||
                strstr(display_val, "flow-root") != NULL) {
              node->display_outside = UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK;
            } else {
              node->display_outside = UI_LAYOUT_DISPLAY_OUTSIDE_INLINE;
            }
          }

          /* Parse inside */
          if (strstr(display_val, "flex") != NULL) {
            node->display_inside = UI_LAYOUT_DISPLAY_INSIDE_FLEX;
          } else if (strstr(display_val, "grid") != NULL) {
            node->display_inside = UI_LAYOUT_DISPLAY_INSIDE_GRID;
          } else if (strstr(display_val, "table") != NULL) {
            node->display_inside = UI_LAYOUT_DISPLAY_INSIDE_TABLE;
          } else if (strstr(display_val, "ruby") != NULL) {
            node->display_inside = UI_LAYOUT_DISPLAY_INSIDE_RUBY;
          } else if (strstr(display_val, "flow-root") != NULL ||
                     strstr(display_val, "inline-block") != NULL) {
            node->display_inside = UI_LAYOUT_DISPLAY_INSIDE_FLOW_ROOT;
          } else {
            node->display_inside = UI_LAYOUT_DISPLAY_INSIDE_FLOW;
          }
        }
      }
    }
  }

  *out_node = node;
  return UI_ERROR_NONE;
}

/**
 * @brief append_layout_child.
 * @param parent Parameter parent.
 * @param child Parameter child.
 * @return Return value.
 */
static ui_error_t append_layout_child(struct ui_layout_node *parent,
                                      struct ui_layout_node *child) {
  child->parent = parent;
  if (!parent->last_child) {
    parent->first_child = child;
    parent->last_child = child;
  } else {
    parent->last_child->next_sibling = child;
    child->previous_sibling = parent->last_child;
    parent->last_child = child;
  }
  return UI_ERROR_NONE;
}

static ui_error_t
/**
 * @brief build_tree_recursive.
 * @param dom_node Parameter dom_node.
 * @param stylesheet Parameter stylesheet.
 * @param out_layout_node Parameter out_layout_node.
 * @return Return value.
 */
build_tree_recursive(const struct ui_dom_node *dom_node,
                     const struct ui_css_stylesheet *stylesheet,
                     struct ui_layout_node **out_layout_node) {
  struct ui_css_computed_style *style = NULL;
  struct ui_layout_node *lnode = NULL;
  struct ui_layout_node *child_lnode = NULL;
  struct ui_dom_node *dom_child;
  const char *display_val;
  ui_error_t err;

  *out_layout_node = NULL;

  /* 1. Resolve Style */
  err = ui_css_resolve_style(stylesheet, dom_node, &style);
  if (err != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* 2. Check for display: none */
  {
    ui_error_t disp_rc =
        ui_css_computed_style_get_property(style, "display", &display_val);

    if (disp_rc == UI_ERROR_NONE) {
      if (strcmp(display_val, "none") == 0) {
        (void)ui_css_computed_style_destroy(style);
        return UI_ERROR_NONE; /* Skip this node and its children */
      }
    }
  }

  /* 3. Create layout node */
  err = create_layout_node(dom_node, style, 0, &lnode);
  if (err != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* 3.5 Compute box model metrics */
  err = compute_box_model(lnode);
  if (err != UI_ERROR_NONE)
    goto cleanup;

  /* 4. Recursively build children */
  dom_child = dom_node->first_child;
  while (dom_child) {
    err = build_tree_recursive(dom_child, stylesheet, &child_lnode);
    if (err != UI_ERROR_NONE) {
      goto cleanup;
    }

    if (child_lnode) {
      (void)append_layout_child(lnode, child_lnode);
    }

    dom_child = dom_child->next_sibling;
  }

  /* 5. Anonymous box expansion */
  if (lnode->display_outside == UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK) {
    int has_block = 0;
    int has_inline = 0;
    struct ui_layout_node *curr = lnode->first_child;
    while (curr) {
      if (curr->display_outside == UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK)
        has_block = 1;
      if (curr->display_outside == UI_LAYOUT_DISPLAY_OUTSIDE_INLINE)
        has_inline = 1;
      curr = curr->next_sibling;
    }

    if (has_block && has_inline) {
      struct ui_layout_node *new_first = NULL;
      struct ui_layout_node *new_last = NULL;
      struct ui_layout_node *anon = NULL;

      curr = lnode->first_child;
      while (curr) {
        struct ui_layout_node *next = curr->next_sibling;
        curr->next_sibling = NULL;
        curr->previous_sibling = NULL;

        if (curr->display_outside == UI_LAYOUT_DISPLAY_OUTSIDE_INLINE) {
          if (1) {
            err = create_layout_node(NULL, NULL, 1, &anon);
            if (err != UI_ERROR_NONE)
              goto cleanup;
            anon->display_outside = UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK;

            /* Append anon to new children list */
            anon->parent = lnode;
            if (!new_last) {
              new_first = anon;
              new_last = anon;
            } else {
              new_last->next_sibling = anon;
              anon->previous_sibling = new_last;
              new_last = anon;
            }
          }
          {
            (void)append_layout_child(anon, curr);
          }
        } else {
          anon = NULL; /* Break the sequence of inlines */
          /* Append block child directly */
          curr->parent = lnode;
          if (!new_last) {
            new_first = curr;
            new_last = curr;
          } else {
            new_last->next_sibling = curr;
            curr->previous_sibling = new_last;
            new_last = curr;
          }
        }
        curr = next;
      }
      lnode->first_child = new_first;
      lnode->last_child = new_last;
    }
  }

  *out_layout_node = lnode;
  return UI_ERROR_NONE;

cleanup:
  if (style && !lnode) {
    (void)ui_css_computed_style_destroy(style);
  }
  if (lnode) {
    (void)ui_layout_tree_destroy(lnode);
  }
  return err;
}

/**
 * @brief Generates a layout tree from a DOM root and stylesheet.
 * @param[in] dom_root The root DOM node.
 * @param[in] stylesheet The computed stylesheet.
 * @param[out] out_layout_root Pointer to store the created layout root.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_layout_tree_generate(const struct ui_dom_node *dom_root,
                                   const struct ui_css_stylesheet *stylesheet,
                                   struct ui_layout_node **out_layout_root) {
  if (!dom_root || !stylesheet || !out_layout_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return build_tree_recursive(dom_root, stylesheet, out_layout_root);
}

ui_error_t ui_layout_tree_destroy(struct ui_layout_node *node) {
  struct ui_layout_node *child;
  struct ui_layout_node *next_child;

  if (!node) {
    return UI_ERROR_NONE;
  }

  child = node->first_child;
  while (child) {
    next_child = child->next_sibling;
    (void)ui_layout_tree_destroy(child);
    child = next_child;
  }

  if (node->computed_style) {
    (void)ui_css_computed_style_destroy(node->computed_style);
  }

  C_MULTIPLATFORM_FREE(node);
  return UI_ERROR_NONE;
}

/**
 * @brief Computes basic block layout for a node and its children.
 * @param[in,out] node The layout node to compute.
 * @param[in] available_width The width available for this node.
 * @return UI_ERROR_NONE on success.
 */
