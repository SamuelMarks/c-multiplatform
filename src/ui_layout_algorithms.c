static ui_error_t layout_block(struct ui_layout_node *node,
                               float available_width) {
  struct ui_layout_node *child;
  float current_y = 0.0f;
  float current_x = 0.0f;
  float total_height = 0.0f;
  float actual_column_width;
  int actual_column_count;

  /* If width is 0 and we are a block without explicit width, expand */
  if (node->width_type == UI_LAYOUT_SIZE_PERCENTAGE) {
    node->width = (available_width * node->raw_width / 100.0f) -
                  node->margin[3] - node->margin[1];
  } else if ((node->width == 0.0f && node->width_type == UI_LAYOUT_SIZE_AUTO) &&
             node->display_outside == UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK) {
    node->width = available_width - node->margin[3] - node->margin[1];
  }

  /* Intrinsic Sizing (before checking min/max clamps) */
  if (node->width_type == UI_LAYOUT_SIZE_MIN_CONTENT ||
      node->width_type == UI_LAYOUT_SIZE_MAX_CONTENT ||
      node->width_type == UI_LAYOUT_SIZE_FIT_CONTENT) {

    float intrinsic_min_width = 0.0f;
    float intrinsic_max_width = 0.0f;

    /* Extremely simplistic estimation of intrinsic content sizes for mock
     * testing */
    /* Real engine would measure text nodes and child blocks */
    struct ui_layout_node *calc_child = node->first_child;
    while (calc_child) {
      float child_min = 20.0f;  /* mock text word length */
      float child_max = 100.0f; /* mock text sentence length */
      intrinsic_min_width =
          child_min > intrinsic_min_width ? child_min : intrinsic_min_width;
      if (child_max > intrinsic_max_width)
        intrinsic_max_width = child_max;
      calc_child = calc_child->next_sibling;
    }

    if (node->width_type == UI_LAYOUT_SIZE_MIN_CONTENT) {
      node->width = intrinsic_min_width + node->padding[1] + node->padding[3] +
                    node->border[1] + node->border[3];
    } else if (node->width_type == UI_LAYOUT_SIZE_MAX_CONTENT) {
      node->width = intrinsic_max_width + node->padding[1] + node->padding[3] +
                    node->border[1] + node->border[3];
    } else if (node->width_type == UI_LAYOUT_SIZE_FIT_CONTENT) {
      float available_inner = available_width - node->margin[1] -
                              node->margin[3] - node->padding[1] -
                              node->padding[3] - node->border[1] -
                              node->border[3];
      float fit = intrinsic_max_width;
      if (fit > available_inner)
        fit = available_inner;
      if (fit < intrinsic_min_width)
        fit = intrinsic_min_width;
      node->width = fit + node->padding[1] + node->padding[3] +
                    node->border[1] + node->border[3];
    }
  }

  if (node->aspect_ratio > 0.0f && node->height == 0.0f) {
    node->height = node->width / node->aspect_ratio;
    node->content_height = node->height - node->padding[0] - node->padding[2] -
                           node->border[0] - node->border[2];
  }

  node->content_width = node->width - node->padding[1] - node->padding[3] -
                        node->border[1] - node->border[3];

  actual_column_width = node->content_width;
  actual_column_count = 1;

  if (node->column_count > 1 || node->column_width > 0.0f) {
    if (node->column_width > 0.0f) {
      actual_column_count = (int)((node->content_width + node->column_gap) /
                                  (node->column_width + node->column_gap));
    } else {
      actual_column_count = node->column_count;
    }
    if (actual_column_count < 1)
      actual_column_count = 1;
    actual_column_width =
        (node->content_width -
         (float)(actual_column_count - 1) * node->column_gap) /
        (float)actual_column_count;
  }

  current_x = node->padding[3] + node->border[3];
  current_y = node->padding[0] + node->border[0];

  child = node->first_child;
  while (child) {
    float child_available_width = actual_column_width;

    /* Handle break-before */
    if (child->break_before == UI_LAYOUT_BREAK_COLUMN ||
        child->break_before == UI_LAYOUT_BREAK_PAGE ||
        child->break_before == UI_LAYOUT_BREAK_ALWAYS) {
      if (actual_column_count > 1) {
        current_x += actual_column_width + node->column_gap;
        current_y = node->padding[0] + node->border[0];
      }
    }

    /* Margin Trim Logic */
    if (node->margin_trim == UI_LAYOUT_MARGIN_TRIM_ALL ||
        node->margin_trim == UI_LAYOUT_MARGIN_TRIM_BLOCK ||
        node->margin_trim == UI_LAYOUT_MARGIN_TRIM_BLOCK_START) {
      if (child == node->first_child) {
        child->margin[0] = 0.0f;
      }
    }
    if (node->margin_trim == UI_LAYOUT_MARGIN_TRIM_ALL ||
        node->margin_trim == UI_LAYOUT_MARGIN_TRIM_BLOCK ||
        node->margin_trim == UI_LAYOUT_MARGIN_TRIM_BLOCK_END) {
      if (child == node->last_child) {
        child->margin[2] = 0.0f;
      }
    }
    if (node->margin_trim == UI_LAYOUT_MARGIN_TRIM_ALL ||
        node->margin_trim == UI_LAYOUT_MARGIN_TRIM_INLINE ||
        node->margin_trim == UI_LAYOUT_MARGIN_TRIM_INLINE_START) {
      if (child == node->first_child) {
        child->margin[3] = 0.0f;
      }
    }
    if (node->margin_trim == UI_LAYOUT_MARGIN_TRIM_ALL ||
        node->margin_trim == UI_LAYOUT_MARGIN_TRIM_INLINE ||
        node->margin_trim == UI_LAYOUT_MARGIN_TRIM_INLINE_END) {
      if (child == node->last_child) {
        child->margin[1] = 0.0f;
      }
    }

    if (child->display_outside == UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK) {
      /* Block layout: stack vertically */
      child->x = current_x + child->margin[3];
      child->y = current_y + child->margin[0];

      { (void)ui_layout_compute(child, child_available_width, 0.0f); }

      current_y = child->y + child->height + child->margin[2];
    } else if (child->display_outside == UI_LAYOUT_DISPLAY_OUTSIDE_INLINE) {
      /* Inline layout (very basic horizontal packing) */
      child->x = current_x + child->margin[3];
      child->y = current_y + child->margin[0];

      { (void)ui_layout_compute(child, child_available_width, 0.0f); }

      current_x = child->x + child->width + child->margin[1];
      /* If child exceeds width, should wrap, but simplistic for now */
      if (child->height + child->margin[0] + child->margin[2] > total_height) {
        total_height = child->height + child->margin[0] + child->margin[2];
      }
      /* We do not increment current_y here unless we wrap, we just update
       * total_height */
      if (child->next_sibling && child->next_sibling->display_outside ==
                                     UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK) {
        current_y += total_height;
        current_x = node->padding[3] + node->border[3];
        total_height = 0.0f;
      }
    }

    /* Handle break-after */
    if (child->break_after == UI_LAYOUT_BREAK_COLUMN ||
        child->break_after == UI_LAYOUT_BREAK_PAGE ||
        child->break_after == UI_LAYOUT_BREAK_ALWAYS) {
      if (actual_column_count > 1) {
        current_x += actual_column_width + node->column_gap;
        current_y = node->padding[0] + node->border[0];
      }
    }

    child = child->next_sibling;
  }

  /* If it was an inline sequence at the end */
  if (total_height > 0.0f) {
    current_y += total_height;
  }

  /* Compute our own height if not explicitly set */
  if (node->height == 0.0f) {
    node->content_height = current_y - (node->padding[0] + node->border[0]);
    node->height = node->content_height + node->padding[0] + node->padding[2] +
                   node->border[0] + node->border[2];

    /* Clamp implicit height */
    if (node->height < node->min_height) {
      node->height = node->min_height;
    }

    node->content_height = node->height - node->padding[0] - node->padding[2] -
                           node->border[0] - node->border[2];

  } else {
    /* Height was explicitly set; check for Y overflow */
    float actual_children_height =
        current_y - (node->padding[0] + node->border[0]);
    if (node->overflow_y == UI_LAYOUT_OVERFLOW_SCROLL ||
        (node->overflow_y == UI_LAYOUT_OVERFLOW_AUTO &&
         actual_children_height > node->content_height)) {
      /* Allocate scrollbar width space (typically 16px) */
      node->scrollbar_width = 16.0f;
    }
  }

  /* Check for X overflow (simplistic) */
  if (node->width > 0.0f) {
  }

  return UI_ERROR_NONE;
}

/**
 * @struct ui_flex_line
 * \brief ui_flex_line
 */
struct ui_flex_line {
  struct ui_layout_node *first_child; /**< first_child */
  struct ui_layout_node *last_child;  /**< last_child */
  float main_size_sum;                /**< main_size_sum */
  float cross_max;                    /**< cross_max */
  float total_flex_grow;              /**< total_flex_grow */
  float total_flex_shrink;            /**< total_flex_shrink */
  int child_count;                    /**< child_count */
};

/*
 * \brief Computes flexbox layout for a node and its children.
 * \param[in,out] node The flex container node.
 * \param[in] available_width The width available for this flex container.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t layout_flex(struct ui_layout_node *node,
                              float available_width) {
  struct ui_layout_node *child;
  int is_row;
  int is_wrap;
  struct ui_flex_line lines[64];
  int line_count = 0;
  float start_x, start_y;
  float current_x, current_y;
  int i;
  float container_main_size;

  memset(lines, 0, sizeof(lines));

  /* If width is 0 and we are a block outside, expand */
  if (node->width_type == UI_LAYOUT_SIZE_PERCENTAGE) {
    node->width = (available_width * node->raw_width / 100.0f) -
                  node->margin[3] - node->margin[1];
  } else if ((node->width == 0.0f && node->width_type == UI_LAYOUT_SIZE_AUTO) &&
             node->display_outside == UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK) {
    node->width = available_width - node->margin[3] - node->margin[1];
  }

  node->content_width = node->width - node->padding[1] - node->padding[3] -
                        node->border[1] - node->border[3];

  is_row = (node->flex_direction == UI_LAYOUT_FLEX_DIRECTION_ROW ||
            node->flex_direction == UI_LAYOUT_FLEX_DIRECTION_ROW_REVERSE);
  is_wrap = (node->flex_wrap == UI_LAYOUT_FLEX_WRAP_WRAP ||
             node->flex_wrap == UI_LAYOUT_FLEX_WRAP_WRAP_REVERSE);

  container_main_size = is_row ? node->content_width : node->content_height;

  /* Pass 1: Measure intrinsic sizes & collect lines */
  child = node->first_child;
  if (child) {
    line_count = 1;
    lines[0].first_child = child;
  }

  while (child && line_count <= 64) {
    float basis = 0.0f;
    float child_main_outer = 0.0f;
    float child_cross_outer = 0.0f;
    struct ui_flex_line *current_line = &lines[line_count - 1];

    {
      (void)ui_layout_compute(child, node->content_width, 0.0f);
    }

    if (child->flex_basis_type == UI_LAYOUT_SIZE_PIXELS) {
      basis = child->flex_basis;
    } else if (child->flex_basis_type == UI_LAYOUT_SIZE_AUTO) {
      basis = is_row ? child->width : child->height;
    }

    if (is_row) {
      child->width = basis;
      child->content_width = basis - child->padding[1] - child->padding[3] -
                             child->border[1] - child->border[3];
      child_main_outer = child->width + child->margin[1] + child->margin[3];
      child_cross_outer = child->height + child->margin[0] + child->margin[2];
    } else {
      child->height = basis;
      child->content_height = basis - child->padding[0] - child->padding[2] -
                              child->border[0] - child->border[0];
      child_main_outer = child->height + child->margin[0] + child->margin[2];
      child_cross_outer = child->width + child->margin[1] + child->margin[3];
    }

    /* Wrap logic */
    if (is_wrap && current_line->child_count > 0 &&
        container_main_size > 0.0f &&
        current_line->main_size_sum + child_main_outer > container_main_size) {
      if (line_count < 64) {
        line_count++;
        current_line = &lines[line_count - 1];
        current_line->first_child = child;
      }
    }

    current_line->last_child = child;
    current_line->main_size_sum += child_main_outer;
    if (child_cross_outer > current_line->cross_max) {
      current_line->cross_max = child_cross_outer;
    }
    current_line->total_flex_grow += child->flex_grow;
    current_line->total_flex_shrink += child->flex_shrink;
    current_line->child_count++;

    child = child->next_sibling;
  }

  /* Pass 2: Distribute free space per line */
  for (i = 0; i < line_count; i++) {
    struct ui_flex_line *line = &lines[i];
    float free_space = 0.0f;

    if (is_row) {
      free_space = node->content_width - line->main_size_sum;
    } else {
      if (node->height > 0.0f) {
        float inner_height = node->height - node->padding[0] -
                             node->padding[2] - node->border[0] -
                             node->border[2];
        free_space = inner_height - line->main_size_sum;
      }
    }

    if (free_space > 0.0f && line->total_flex_grow > 0.0f) {
      child = line->first_child;
      while (1) {
        float extra = (child->flex_grow / line->total_flex_grow) * free_space;
        if (is_row) {
          child->width += extra;
          child->content_width += extra;
          (void)ui_layout_compute(child, child->content_width, 0.0f);
        } else {
          child->height += extra;
          child->content_height += extra;
        }
        if (child == line->last_child)
          break;
        child = child->next_sibling;
      }
      free_space = 0.0f;
    } else if (free_space < 0.0f && line->total_flex_shrink > 0.0f &&
               !is_wrap) {
      /* Only shrink if not wrapping (or if single line forced) */
      child = line->first_child;
      while (1) {
        float shrink =
            (child->flex_shrink / line->total_flex_shrink) * (-free_space);
        if (is_row) {
          child->width -= shrink;
          child->content_width -= shrink;
          (void)ui_layout_compute(child, child->content_width, 0.0f);
        } else {
          child->height -= shrink;
          child->content_height -= shrink;
        }
        if (child == line->last_child)
          break;
        child = child->next_sibling;
      }
      free_space = 0.0f;
    }
  }

  /* Pass 3: Final Positioning */
  start_x = node->padding[3] + node->border[3];
  start_y = node->padding[0] + node->border[0];
  current_y = start_y;
  current_x = start_x;

  for (i = 0; i < line_count; i++) {
    struct ui_flex_line *line = &lines[i];
    float free_space = 0.0f;
    float justify_gap = 0.0f;
    float line_start_x = start_x;
    float line_start_y = start_y;

    if (is_row) {
      free_space = node->content_width - line->main_size_sum;
      line_start_y = current_y;
    } else {
      float inner_height =
          node->height > 0.0f
              ? (node->height - node->padding[0] - node->padding[2] -
                 node->border[0] - node->border[2])
              : 0.0f;
      free_space = inner_height - line->main_size_sum;
      line_start_x = current_x;
    }

    if (free_space > 0.0f) {
      if (node->justify_content == UI_LAYOUT_ALIGN_CENTER) {
        if (is_row)
          line_start_x += free_space / 2.0f;
      } else if (node->justify_content == UI_LAYOUT_ALIGN_END) {
        if (is_row)
          line_start_x += free_space;
        else
          line_start_y += free_space;
      } else if (node->justify_content == UI_LAYOUT_ALIGN_SPACE_BETWEEN &&
                 line->child_count > 1) {
        justify_gap = free_space / (float)(line->child_count - 1);
      } else if (node->justify_content == UI_LAYOUT_ALIGN_SPACE_AROUND) {
        justify_gap = free_space / (float)line->child_count;
        if (is_row)
          line_start_x += justify_gap / 2.0f;
        else
          line_start_y += justify_gap / 2.0f;
      } else if (node->justify_content == UI_LAYOUT_ALIGN_SPACE_EVENLY) {
        justify_gap = free_space / (float)(line->child_count + 1);
        if (is_row)
          line_start_x += justify_gap;
        else
          line_start_y += justify_gap;
      }
    }

    child = line->first_child;
    while (1) {
      enum ui_layout_alignment align = child->align_self;
      if (align == UI_LAYOUT_ALIGN_AUTO)
        align = node->align_items;

      if (is_row) {
        child->x = line_start_x + child->margin[3];
        if (align == UI_LAYOUT_ALIGN_CENTER) {
          child->y = line_start_y + (line->cross_max - child->height) / 2.0f;
        } else if (align == UI_LAYOUT_ALIGN_END) {
          child->y =
              line_start_y + line->cross_max - child->height - child->margin[2];
        } else {
          child->y = line_start_y + child->margin[0];
        }
        line_start_x +=
            child->width + child->margin[1] + child->margin[3] + justify_gap;
      } else {
        if (align == UI_LAYOUT_ALIGN_CENTER) {
          child->x = line_start_x + (line->cross_max - child->width) / 2.0f;
        } else if (align == UI_LAYOUT_ALIGN_END) {
          child->x =
              line_start_x + line->cross_max - child->width - child->margin[1];
        } else {
          child->x = line_start_x + child->margin[3];
        }
        child->y = line_start_y + child->margin[0];
        line_start_y +=
            child->height + child->margin[0] + child->margin[2] + justify_gap;
      }

      if (child == line->last_child)
        break;
      child = child->next_sibling;
    }

    if (is_row) {
      current_y += line->cross_max;
    } else {
      current_x += line->cross_max;
    }
  }

  if (node->height == 0.0f) {
    if (is_row) {
      node->content_height = current_y - start_y;
    } else {
      node->content_height = lines[0].cross_max;
    }
    node->height = node->content_height + node->padding[0] + node->padding[2] +
                   node->border[0] + node->border[2];
  }

  return UI_ERROR_NONE;
}

/*
 * \brief Solves the layout for the entire viewport starting from the root.
 * \param[in,out] root The root layout node.
 * \param[in] window_width The width of the viewport window.
 * \param[in] window_height The height of the viewport window.
 * \return UI_ERROR_NONE on success.
 */
